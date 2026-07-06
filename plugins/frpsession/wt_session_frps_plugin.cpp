/**
 * wt_session_frps_plugin.cpp  –  FRP session plugin implementation
 *
 * This translation unit is the ONLY place that uses C++ STL, nlohmann/json,
 * and cpp-httplib.  None of those types ever appear in the public header, so
 * callers compiled with a different toolchain are completely unaffected by
 * changes here.
 *
 * Internal design
 * ---------------
 *  WtSessionContext  – the heap object behind the opaque WtSessionHandle.
 *                   All C++ objects (std::string, std::vector, …) live here.
 *
 * String ownership for callers
 * ----------------------------
 *  Session field strings are stored as std::string inside WtSessionContext.
 *  The plugin exposes them through WtSessionSessionView::char* pointers that
 *  point directly into those std::string objects.  They remain valid until
 *  the next mutation or wtSession_Destroy().
 *
 * Build (example):
 *   g++ -std=c++17 -DWT_SESSION_PLUGIN_BUILDING_DLL \
 *       -I../include \
 *       -shared -fPIC \
 *       wt_session_frps_plugin.cpp -o libwt_session_frps_plugin.so \
 *       -lssl -lcrypto -lpthread
 *
 *   # Windows / MSVC:
 *   cl /std:c++17 /DWT_SESSION_PLUGIN_BUILDING_DLL /I..\include \
 *      /LD wt_session_frps_plugin.cpp /link /OUT:wt_session_frps_plugin.dll
 */

#include "wt_session_plugin.h"

// --------------------------------------------------------------------------
// Pull in header-only libraries (implementation side only)
// --------------------------------------------------------------------------
#include <nlohmann/json.hpp>

// Uncomment the next line if the frps dashboard uses HTTPS
// #define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <chrono>
#include <cstring>
#include <cassert>

#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;
using json = nlohmann::json;

/* -------------------------------------------------------------------------
 * Initialization parameters (internal struct, C++ types allowed)
 * Used internally to parse the JSON configuration passed to wtSession_Init().
 * All strings are copied into the WtSessionContext during initialization.
 * ---------------------------------------------------------------------- */
typedef struct WtSessionInitParams {
    std::string server_addr;        /**< frps dashboard host (also used as default session host) */
    int32_t     dashboard_port;     /**< frps dashboard port, default is 7500 */
    std::string dashboard_user;     /**< dashboard Basic-Auth username */
    std::string dashboard_pwd;      /**< dashboard Basic-Auth password */
    std::string token;              /**< frps auth token for Authorization: Bearer header
                                         pass empty string to omit the header */
    std::string local_json_path;    /**< path to the local sessions JSON file */
    int32_t     http_timeout_sec;   /**< HTTP connect+read timeout in seconds, 0 uses default (10s) */
    std::string default_session_username; /**< default username for newly created sessions */
    std::string default_session_password; /**< default password for newly created sessions */
} WtSessionInitParams;
// ==========================================================================
// Internal session representation
// This struct holds the full session data in C++ types. It is converted to
// WtSessionSessionView (C-compatible) when exposing to the host application.
// ==========================================================================

struct SessionEntry {
    int64_t     id              = 0;           /**< unique session identifier */
    int32_t     type            = WT_HOST_INVALID; /**< session type (SSH, RDP, VNC, etc.) */
    std::string name;           /**< globally unique session name */
    std::string host;           /**< remote host address */
    int32_t     port            = 0;           /**< remote port number */
    std::string login_username; /**< login username */
    std::string login_password; /**< login password */
    std::string login_identity_file; /**< SSH identity file path */
    std::string proxy_jump;     /**< comma-separated jump-host IDs */
    std::string memo;           /**< user-defined description */
    std::string path;           /**< parent path node (for tree organization) */
    int32_t     order_num       = 0;           /**< display order number */
    bool        online          = false;       /**< online status flag */
};

// ==========================================================================
// Opaque context (definition visible only in this translation unit)
// This struct contains all internal state for the plugin instance.
// It is the implementation behind the opaque WtSessionHandle type.
// ==========================================================================

struct WtSessionContext {
    // --- configuration copied from WtSessionInitParams during initialization ---
    std::string default_session_username; /**< default username for new sessions */
    std::string default_session_password; /**< default password for new sessions */

    std::string server_addr;              /**< frps dashboard host address */
    int32_t     dashboard_port     = 7500; /**< frps dashboard port */
    std::string dashboard_user;           /**< dashboard Basic-Auth username */
    std::string dashboard_pwd;            /**< dashboard Basic-Auth password */
    std::string local_json_path;          /**< path to local sessions JSON file */
    int32_t     http_timeout_sec   = 10;   /**< HTTP timeout in seconds */

    bool        initialized        = false; /**< flag indicating if init completed */

    // --- in-memory session list ---
    std::vector<SessionEntry> sessions;    /**< all sessions loaded from disk */

    // --- last error message (returned by wtSession_GetLastError) ---
    std::string last_error;                /**< cached error message */

    // --- string arena for WtSessionSessionView exposure ---
    // We keep a single SessionEntry "view_cache" so the char* pointers
    // in WtSessionSessionView remain stable for the duration of one API call.
    // The cache is re-filled on every wtSession_GetSession* call.
    SessionEntry view_cache;

    // --- sync result name caches (valid inside Sync callback) ---
    std::vector<std::string>  sync_added_names;  /**< names of newly added sessions */
    std::vector<std::string>  sync_updated_names;/**< names of updated sessions */
    std::vector<std::string>  sync_removed_names;/**< names of removed sessions */
    std::vector<const char*>  sync_added_ptrs;   /**< const char* pointers for C API */
    std::vector<const char*>  sync_updated_ptrs; /**< const char* pointers for C API */
    std::vector<const char*>  sync_removed_ptrs; /**< const char* pointers for C API */

    void setError(const std::string& msg) { last_error = msg; }
    void clearError()                     { last_error.clear(); }
};

// ==========================================================================
// JSON keys
// ==========================================================================

static constexpr const char* K_SESSIONS    = "sessions";
static constexpr const char* K_ID          = "id";
static constexpr const char* K_TYPE        = "type";
static constexpr const char* K_NAME        = "name";
static constexpr const char* K_HOST        = "host";
static constexpr const char* K_PORT        = "port";
static constexpr const char* K_USERNAME    = "login_username";
static constexpr const char* K_PASSWORD    = "login_password";
static constexpr const char* K_IDENT       = "login_identity_file";
static constexpr const char* K_PROXY_JUMP  = "proxy_jump";
static constexpr const char* K_MEMO        = "memo";
static constexpr const char* K_PATH        = "path";
static constexpr const char* K_ORDER       = "order_num";
static constexpr const char* K_ONLINE      = "online";

// ==========================================================================
// Host-type <-> string helpers  (internal linkage)
// ==========================================================================

static const char* hostTypeStr(int32_t t) {
    switch (t) {
        case WT_HOST_SSH_WITH_SFTP: return "ssh_with_sftp";
        case WT_HOST_SFTP_ONLY:     return "sftp_only";
        case WT_HOST_TELNET:        return "telnet";
        case WT_HOST_RLOGIN:        return "rlogin";
        case WT_HOST_RDP:           return "rdp";
        case WT_HOST_VNC:           return "vnc";
        case WT_HOST_FTP:           return "ftp";
        default:                    return "invalid";
    }
}

static int32_t parseHostType(const std::string& s) {
    if (s == "ssh_with_sftp") return WT_HOST_SSH_WITH_SFTP;
    if (s == "sftp_only")     return WT_HOST_SFTP_ONLY;
    if (s == "telnet")        return WT_HOST_TELNET;
    if (s == "rlogin")        return WT_HOST_RLOGIN;
    if (s == "rdp")           return WT_HOST_RDP;
    if (s == "vnc")           return WT_HOST_VNC;
    if (s == "ftp")           return WT_HOST_FTP;
    return WT_HOST_INVALID;
}

// ==========================================================================
// JSON serialisation helpers  (internal linkage)
// ==========================================================================

static json entryToJson(const SessionEntry& e) {
    json j;
    j[K_ID]       = e.id;
    j[K_TYPE]     = hostTypeStr(e.type);
    j[K_NAME]     = e.name;
    j[K_HOST]     = e.host;
    j[K_PORT]     = e.port;
    j[K_USERNAME] = e.login_username;
    j[K_PASSWORD] = e.login_password;
    j[K_IDENT]    = e.login_identity_file;
    j[K_PROXY_JUMP] = e.proxy_jump;
    j[K_MEMO]     = e.memo;
    j[K_PATH]     = e.path;
    j[K_ORDER]    = e.order_num;
    j[K_ONLINE]   = e.online;
    return j;
}

static SessionEntry entryFromJson(const json& j) {
    SessionEntry e;
    auto gs = [&](const char* k, std::string& f) {
        if (j.contains(k) && j[k].is_string()) f = j[k].get<std::string>();
    };
    if (j.contains(K_ID)    && j[K_ID].is_number())    e.id        = j[K_ID].get<int64_t>();
    if (j.contains(K_PORT)  && j[K_PORT].is_number())  e.port      = j[K_PORT].get<int32_t>();
    if (j.contains(K_ORDER) && j[K_ORDER].is_number()) e.order_num = j[K_ORDER].get<int32_t>();
    if (j.contains(K_ONLINE)&& j[K_ONLINE].is_boolean()) e.online  = j[K_ONLINE].get<bool>();

    std::string typeStr;
    gs(K_TYPE, typeStr);  e.type = parseHostType(typeStr);
    gs(K_NAME,       e.name);
    gs(K_HOST,       e.host);
    gs(K_USERNAME,   e.login_username);
    gs(K_PASSWORD,   e.login_password);
    gs(K_IDENT,      e.login_identity_file);
    gs(K_PROXY_JUMP, e.proxy_jump);
    gs(K_MEMO,       e.memo);
    gs(K_PATH,       e.path);
    return e;
}

// ==========================================================================
// ID generation
// ==========================================================================

static int64_t generateId(const WtSessionContext& ctx) {
    int64_t maxId = 0;
    for (const auto& s : ctx.sessions) {
        if (s.id > maxId) maxId = s.id;
    }
    if (maxId > 0) return maxId + 1;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  std::chrono::system_clock::now().time_since_epoch()).count();
    return static_cast<int64_t>(ms);
}

// ==========================================================================
// Remote fetch
// ==========================================================================

/**
 * RemoteProxy struct holds parsed proxy data from frps API response.
 */
struct RemoteProxy {
    std::string name;   /**< proxy name (used as session name) */
    int32_t     port   = 0; /**< remote port number */
    bool        online = false; /**< online status from frps */
};

/**
 * Fetch /api/proxy/tcp from frps dashboard and parse the response.
 *
 * Supported frps response formats:
 *   v0.51+ : { "proxies": [ { "name":"x", "conf":{"remotePort":N}, "status":"online" }, ... ] }
 *   older  : [ { "proxyName":"x", "remotePort":N, "status":"online" }, ... ]
 *
 * @param ctx     Plugin context containing connection parameters.
 * @param out     Output vector to store parsed proxy entries.
 * @param errMsg  Output error message if fetch fails.
 *
 * @return true on success, false on failure with errMsg set.
 */
static bool fetchRemoteProxies(const WtSessionContext& ctx,
                                std::vector<RemoteProxy>& out,
                                std::string& errMsg)
{
    out.clear();

    httplib::Client cli(ctx.server_addr, ctx.dashboard_port);
    cli.set_follow_location(true);
    cli.set_connection_timeout(ctx.http_timeout_sec, 0);
    cli.set_read_timeout(ctx.http_timeout_sec, 0);
    cli.set_default_headers({
        {"Content-Type", "application/json"}
    });
    if (!ctx.dashboard_user.empty()) {
        cli.set_basic_auth(ctx.dashboard_user.c_str(), ctx.dashboard_pwd.c_str());
    }

    auto res = cli.Get("/api/proxy/tcp");
    if (!res) {
        errMsg = "HTTP request failed: " + httplib::to_string(res.error());
        return false;
    }
    if (res->status != 200) {
        errMsg = "HTTP " + std::to_string(res->status) + ": " + res->body;
        return false;
    }

    json root;
    try { root = json::parse(res->body); }
    catch (const std::exception& e) {
        errMsg = std::string("JSON parse error: ") + e.what();
        return false;
    }

    const json* arr = nullptr;
    if (root.contains("proxies") && root["proxies"].is_array())
        arr = &root["proxies"];
    else if (root.is_array())
        arr = &root;
    else { errMsg = "Unexpected JSON structure from /api/proxy/tcp"; return false; }

    for (const auto& entry : *arr) {
        RemoteProxy rp;
        // name
        if (entry.contains("name") && entry["name"].is_string())
            rp.name = entry["name"].get<std::string>();
        else if (entry.contains("proxyName") && entry["proxyName"].is_string())
            rp.name = entry["proxyName"].get<std::string>();
        if (rp.name.empty()) continue;

        // port
        if (entry.contains("conf") && entry["conf"].is_object()) {
            const auto& c = entry["conf"];
            if (c.contains("remotePort") && c["remotePort"].is_number())
                rp.port = c["remotePort"].get<int32_t>();
        } else if (entry.contains("remotePort") && entry["remotePort"].is_number()) {
            rp.port = entry["remotePort"].get<int32_t>();
        } else if (entry.contains("port") && entry["port"].is_number()) {
            rp.port = entry["port"].get<int32_t>();
        }

        // online
        if (entry.contains("status") && entry["status"].is_string())
            rp.online = (entry["status"].get<std::string>() == "online");
        else if (entry.contains("online") && entry["online"].is_boolean())
            rp.online = entry["online"].get<bool>();

        out.push_back(std::move(rp));
    }
    return true;
}

// ==========================================================================
// Persistence helpers
// ==========================================================================

static bool loadFromDisk(const std::string& path,
                          std::vector<SessionEntry>& out,
                          std::string& errMsg)
{
    out.clear();
    std::ifstream ifs(path);
    if (!ifs.is_open()) return true;   // no file yet – not an error

    json root;
    try { ifs >> root; }
    catch (const std::exception& e) {
        errMsg = std::string("JSON parse error in ") + path + ": " + e.what();
        return false;
    }

    if (!root.contains(K_SESSIONS) || !root[K_SESSIONS].is_array()) return true;

    for (const auto& j : root[K_SESSIONS]) {
        try { out.push_back(entryFromJson(j)); }
        catch (...) { /* skip malformed entries silently */ }
    }
    return true;
}

/**
 * Merge-safe save to disk:
 *  1. Read current disk content (if merge is true).
 *  2. Apply in-memory sessions on top (in-memory wins on name collision).
 *  3. Keep disk-only sessions absent from memory (preserves changes from other instances).
 *  4. Sort by order_num and write to file.
 *
 * @param path   Path to the sessions JSON file.
 * @param mem    In-memory session list to save.
 * @param merge  If true, merge with existing disk content; if false, overwrite.
 * @param errMsg Output error message if save fails.
 *
 * @return true on success, false on failure with errMsg set.
 */
static bool saveToDisk(const std::string& path,
                        const std::vector<SessionEntry>& mem, bool merge,
                        std::string& errMsg)
{
    // Step 1: read disk
    std::vector<SessionEntry> disk;
    std::string loadErr;
    if(merge) {
        loadFromDisk(path, disk, loadErr);   // errors here are non-fatal
    }

    // Step 2 & 3: merge
    std::unordered_map<std::string, const SessionEntry*> memIdx;
    for (const auto& s : mem) memIdx[s.name] = &s;

    std::vector<SessionEntry> merged;
    merged.reserve(mem.size() + disk.size());

    // Disk-only entries first
    for (const auto& d : disk) {
        if (memIdx.find(d.name) == memIdx.end())
            merged.push_back(d);
    }
    // All memory entries
    for (const auto& s : mem)
        merged.push_back(s);

    // Step 4: sort + write
    std::sort(merged.begin(), merged.end(),
              [](const SessionEntry& a, const SessionEntry& b) {
                  return a.order_num < b.order_num; });

    json root;
    root[K_SESSIONS] = json::array();
    for (const auto& e : merged)
        root[K_SESSIONS].push_back(entryToJson(e));

    std::ofstream ofs(path);
    if (!ofs.is_open()) {
        errMsg = "Cannot open for writing: " + path;
        return false;
    }
    ofs << root.dump(4);
    if (!ofs.good()) {
        errMsg = "Write error: " + path;
        return false;
    }
    return true;
}

// ==========================================================================
// Helper: fill WtSessionSessionView from SessionEntry
// ==========================================================================

static void fillView(WtSessionSessionView& v, const SessionEntry& e) {
    v.id                  = e.id;
    v.type                = e.type;
    v.name                = e.name.c_str();
    v.host                = e.host.c_str();
    v.port                = e.port;
    v.login_username      = e.login_username.c_str();
    v.login_password      = e.login_password.c_str();
    v.login_identity_file = e.login_identity_file.c_str();
    v.proxy_jump          = e.proxy_jump.c_str();
    v.memo                = e.memo.c_str();
    v.path                = e.path.c_str();
    v.order_num           = e.order_num;
    v.online              = e.online ? 1 : 0;
}

// ==========================================================================
// Null / state guards
// ==========================================================================

#define CHECK_HANDLE(h)  if (!(h)) return WT_SESSION_ERR_PARAM
#define CHECK_INIT(h)    if (!(h)->initialized) { \
                             (h)->setError("Not initialized. Call wtSession_Init() first."); \
                             return WT_SESSION_ERR_STATE; }

// ==========================================================================
// Exported C functions
// ==========================================================================

extern "C" {

// --------------------------------------------------------------------------

WT_SESSION_API uint32_t WT_SESSION_CALL
wtSession_GetApiVersion(void) {
    return WT_SESSION_API_VERSION;
}

// --------------------------------------------------------------------------

WT_SESSION_API uint32_t WT_SESSION_CALL
wtSession_GetFeature(void) {
    return WT_SESSION_FEATURE_SHOW_PASSWORD|WT_SESSION_FEATURE_UPDATE_INFO|WT_SESSION_FEATURE_RUN_AS_TUNNEL;
}
// --------------------------------------------------------------------------

WT_SESSION_API WtSessionHandle WT_SESSION_CALL
wtSession_Create(void) {
    try { return new WtSessionContext(); }
    catch (...) { return nullptr; }
}

// --------------------------------------------------------------------------

WT_SESSION_API int32_t WT_SESSION_CALL
wtSession_Init(WtSessionHandle handle, const char* cfgJson)
{
    CHECK_HANDLE(handle);
    WtSessionInitParams params;
    const char* errMsg = wtPlugin_CheckInitializeParameter(cfgJson);
    if(errMsg != nullptr) {
        handle->setError(errMsg);
        return WT_SESSION_ERR_PARSE;
    }
    try{
        json cfg = json::parse(cfgJson);
        params.server_addr = cfg.value<std::string>("host", "");
        params.dashboard_port = cfg.value<int>("port", 7500);
        params.dashboard_user = cfg.value<std::string>("user", "admin");
        params.dashboard_pwd = cfg.value<std::string>("password", "admin");
        params.local_json_path = cfg.value<std::string>("session_path", "sessions.json");
        params.http_timeout_sec = cfg.value<int>("http_timeout_sec", 10);
        params.default_session_username = cfg.value<std::string>("default_session_username", "");
        params.default_session_password = cfg.value<std::string>("default_session_password", "");
    }catch(...) {
        return WT_SESSION_ERR_PARSE;
    }
    if (params.server_addr.empty()) {
        handle->setError("server_addr is required");
        return WT_SESSION_ERR_PARAM;
    }
    if (params.local_json_path.empty()) {
        handle->setError("local_json_path is required");
        return WT_SESSION_ERR_PARAM;
    }

    handle->default_session_password = params.default_session_password;
    handle->default_session_username = params.default_session_username;

    handle->server_addr      = params.server_addr;
    handle->dashboard_port   = params.dashboard_port > 0 ? params.dashboard_port : 7500;
    handle->dashboard_user   = params.dashboard_user.empty() ? "admin" : params.dashboard_user;
    handle->dashboard_pwd    = params.dashboard_pwd.empty()  ? "admin" : params.dashboard_pwd;
    handle->local_json_path  = params.local_json_path;
    handle->http_timeout_sec = params.http_timeout_sec > 0 ? params.http_timeout_sec : 10;

    handle->initialized = true;
    handle->sessions.clear();
    handle->clearError();

    // Load existing data (non-fatal if file absent)
    std::string loadErr;
    if (!loadFromDisk(handle->local_json_path, handle->sessions, loadErr)) {
        handle->setError(loadErr);
        return WT_SESSION_ERR_PARSE;
    }
    return WT_SESSION_OK;    
}

// --------------------------------------------------------------------------

WT_SESSION_API void WT_SESSION_CALL
wtSession_Destroy(WtSessionHandle handle) {
    delete handle;
}

// --------------------------------------------------------------------------

WT_SESSION_API int32_t WT_SESSION_CALL
wtSession_Sync(WtSessionHandle       handle,
            WtSessionSyncCallback callback,
            void*              user_data)
{
    CHECK_HANDLE(handle);
    CHECK_INIT(handle);
    handle->clearError();

    // --- fetch remote ---
    std::vector<RemoteProxy> remote;
    std::string fetchErr;
    if (!fetchRemoteProxies(*handle, remote, fetchErr)) {
        handle->setError(fetchErr);
        if (callback) {
            WtSessionSyncResult res{};
            res.status    = WT_SESSION_ERR_NET;
            res.error_msg = handle->last_error.c_str();
            callback(&res, user_data);
        }
        return WT_SESSION_ERR_NET;
    }

    // --- build remote lookup ---
    std::unordered_map<std::string, const RemoteProxy*> remoteMap;
    for (const auto& rp : remote) remoteMap[rp.name] = &rp;

    // --- build local name → index map ---
    std::unordered_map<std::string, size_t> localIdx;
    for (size_t i = 0; i < handle->sessions.size(); ++i)
        localIdx[handle->sessions[i].name] = i;

    handle->sync_added_names.clear();
    handle->sync_updated_names.clear();
    handle->sync_removed_names.clear();

    int32_t orderCounter = static_cast<int32_t>(handle->sessions.size()) + 1;

    // --- add or update ---
    for (const auto& rp : remote) {
        auto it = localIdx.find(rp.name);
        if (it == localIdx.end()) {
            SessionEntry e;
            e.id        = generateId(*handle);
            e.name      = rp.name;
            e.host      = handle->server_addr;
            e.port      = rp.port;
            e.online    = rp.online;
            e.type      = WT_HOST_SSH_WITH_SFTP;
            e.login_username = handle->default_session_username;
            e.login_password = handle->default_session_password;
            e.order_num = orderCounter++;
            handle->sessions.push_back(std::move(e));
            handle->sync_added_names.push_back(rp.name);
        } else {
            SessionEntry& e  = handle->sessions[it->second];
            bool changed     = false;
            if (e.port   != rp.port)   { e.port   = rp.port;   changed = true; }
            if (e.online != rp.online) { e.online = rp.online; changed = true; }
            if (changed) handle->sync_updated_names.push_back(rp.name);
        }
    }

    // --- remove sessions absent from remote ---
    {
        auto it = handle->sessions.begin();
        while (it != handle->sessions.end()) {
            if (!remoteMap.count(it->name)) {
                handle->sync_removed_names.push_back(it->name);
                it = handle->sessions.erase(it);
            } else ++it;
        }
    }

    // --- save ---
    std::string saveErr;
    int32_t rc = WT_SESSION_OK;
    if (!saveToDisk(handle->local_json_path, handle->sessions, true, saveErr)) {
        handle->setError(saveErr);
        rc = WT_SESSION_ERR_IO;
    }

    // --- build callback data ---
    if (callback) {
        // Build pointer arrays from name caches
        auto mkPtrs = [](const std::vector<std::string>& names,
                         std::vector<const char*>& ptrs) {
            ptrs.resize(names.size());
            for (size_t i = 0; i < names.size(); ++i)
                ptrs[i] = names[i].c_str();
        };
        mkPtrs(handle->sync_added_names,   handle->sync_added_ptrs);
        mkPtrs(handle->sync_updated_names, handle->sync_updated_ptrs);
        mkPtrs(handle->sync_removed_names, handle->sync_removed_ptrs);

        WtSessionSyncResult res{};
        res.status         = rc;
        res.error_msg      = rc != WT_SESSION_OK ? handle->last_error.c_str() : nullptr;
        res.added_count    = static_cast<int32_t>(handle->sync_added_names.size());
        res.updated_count  = static_cast<int32_t>(handle->sync_updated_names.size());
        res.removed_count  = static_cast<int32_t>(handle->sync_removed_names.size());
        res.added_names    = handle->sync_added_ptrs.empty()   ? nullptr : handle->sync_added_ptrs.data();
        res.updated_names  = handle->sync_updated_ptrs.empty() ? nullptr : handle->sync_updated_ptrs.data();
        res.removed_names  = handle->sync_removed_ptrs.empty() ? nullptr : handle->sync_removed_ptrs.data();
        callback(&res, user_data);
    }
    return rc;
}

// --------------------------------------------------------------------------

WT_SESSION_API int32_t WT_SESSION_CALL
wtSession_GetSessionCount(WtSessionHandle handle) {
    if (!handle) return -1;
    if (!handle->initialized) return -1;
    return static_cast<int32_t>(handle->sessions.size());
}

// --------------------------------------------------------------------------

WT_SESSION_API int32_t WT_SESSION_CALL
wtSession_GetSessionAt(WtSessionHandle       handle,
                    int32_t            index,
                    WtSessionSessionView* view)
{
    CHECK_HANDLE(handle);
    CHECK_INIT(handle);
    if (!view || view->struct_size < sizeof(WtSessionSessionView))
        return WT_SESSION_ERR_PARAM;
    if (index < 0 || static_cast<size_t>(index) >= handle->sessions.size())
        return WT_SESSION_ERR_NOT_FOUND;

    handle->view_cache = handle->sessions[static_cast<size_t>(index)];
    fillView(*view, handle->view_cache);
    return WT_SESSION_OK;
}

// --------------------------------------------------------------------------

WT_SESSION_API int32_t WT_SESSION_CALL
wtSession_GetSession(WtSessionHandle       handle,
                  const char*        name,
                  WtSessionSessionView* view)
{
    CHECK_HANDLE(handle);
    CHECK_INIT(handle);
    if (!name || !view || view->struct_size < sizeof(WtSessionSessionView))
        return WT_SESSION_ERR_PARAM;

    for (const auto& s : handle->sessions) {
        if (s.name == name) {
            handle->view_cache = s;
            fillView(*view, handle->view_cache);
            return WT_SESSION_OK;
        }
    }
    return WT_SESSION_ERR_NOT_FOUND;
}

// --------------------------------------------------------------------------

WT_SESSION_API int32_t WT_SESSION_CALL
wtSession_UpdateInfo(WtSessionHandle handle,
                         int64_t id,
                         const char*  name,
                         const char*  username,
                         const char*  password,
                         const char*  ident_file,
                         const char*  path,
                         const char*  proxy_jump,
                         const char*  memo,
                         int32_t      type,
                         int32_t      field)
{
    CHECK_HANDLE(handle);
    CHECK_INIT(handle);
    handle->clearError();

    for (auto& s : handle->sessions) {
        if (s.id == id) {
            if (field & WT_UPDATE_FIELD_NAME)       s.name       = name;
            if (field & WT_UPDATE_FIELD_USERNAME)   s.login_username       = username;
            if (field & WT_UPDATE_FIELD_PASSWORD)   s.login_password       = password;
            if (field & WT_UPDATE_FIELD_IDENTFILE)  s.login_identity_file  = ident_file;
            if (field & WT_UPDATE_FIELD_PATH)       s.path       = path;
            if (field & WT_UPDATE_FIELD_PROXYJUMP)  s.proxy_jump = proxy_jump;
            if (field & WT_UPDATE_FIELD_MEMO)       s.memo       = memo;
            if (field & WT_UPDATE_FIELD_TYPE)       s.type                 = type;

            std::string saveErr;
            if (!saveToDisk(handle->local_json_path, handle->sessions, true, saveErr)) {
                handle->setError(saveErr);
                return WT_SESSION_ERR_IO;
            }
            return WT_SESSION_OK;
        }
    }
    return WT_SESSION_ERR_NOT_FOUND;
}

WT_SESSION_API int32_t WT_SESSION_CALL
wtSession_RemoveByIds(WtSessionHandle handle, const int64_t *ids, int cnt) {
    CHECK_HANDLE(handle);
    CHECK_INIT(handle);
    handle->clearError();

    auto fnIndexOf=[=](int64_t id) {
        for(int i = 0; i < cnt; i++) {
            if(ids[i] == id) {
                return i;
            }
        }
        return -1;
    };

    auto it = handle->sessions.begin();
    while (it != handle->sessions.end()) {
        if (fnIndexOf(it->id) >= 0) {
            it = handle->sessions.erase(it);
        } else{
            ++it;
        }
    }
    std::string saveErr;
    if (!saveToDisk(handle->local_json_path, handle->sessions, false, saveErr)) {
        handle->setError(saveErr);
        return WT_SESSION_ERR_IO;
    }
    return WT_SESSION_OK;
}
// --------------------------------------------------------------------------

WT_SESSION_API int32_t WT_SESSION_CALL
wtSession_Load(WtSessionHandle handle)
{
    CHECK_HANDLE(handle);
    CHECK_INIT(handle);
    handle->clearError();

    std::string loadErr;
    if (!loadFromDisk(handle->local_json_path, handle->sessions, loadErr)) {
        handle->setError(loadErr);
        return WT_SESSION_ERR_PARSE;
    }
    return WT_SESSION_OK;
}

// --------------------------------------------------------------------------

WT_SESSION_API int32_t WT_SESSION_CALL
wtSession_Save(WtSessionHandle handle)
{
    CHECK_HANDLE(handle);
    CHECK_INIT(handle);
    handle->clearError();

    std::string saveErr;
    if (!saveToDisk(handle->local_json_path, handle->sessions, true, saveErr)) {
        handle->setError(saveErr);
        return WT_SESSION_ERR_IO;
    }
    return WT_SESSION_OK;
}

// --------------------------------------------------------------------------

WT_SESSION_API const char* WT_SESSION_CALL
wtSession_GetLastError(WtSessionHandle handle)
{
    if (!handle) return "";
    return handle->last_error.c_str();
}

// --------------------------------------------------------------------------

WT_SESSION_API int32_t WT_SESSION_CALL
wtPlugin_GetInfo(WtPluginInfo* info)
{
    if (!info || info->struct_size < sizeof(WtPluginInfo))
        return WT_SESSION_ERR_PARAM;

    info->plugin_type        = WT_PLUGIN_TYPE_SESSION;
    info->plugin_name        = "FRP Session Plugin";
    info->plugin_version     = "1.0.0";
    info->plugin_description = "FRP session management plugin for woterm - synchronizes remote sessions from frps dashboard";
    info->plugin_vendor      = "woterm";
    info->plugin_id          = "com.woterm.plugins.frpsession";
    info->api_version        = WT_SESSION_API_VERSION;

    return WT_SESSION_OK;
}

WT_SESSION_API const char* WT_SESSION_CALL
wtPlugin_GetInitilizeParameterTemplate(void) {
    static std::string my;
    if(my.empty()) {
        json out;
        out["host"] = "example.com";
        out["port"] = 7500;
        out["user"] = "admin";
        out["password"] = "password for login";
        out["session_path"] = "frpsession.json";
        out["http_timeout_sec"] = 10;
        out["default_session_username"] = "xxxx";
        out["default_session_password"] = "xxxx";
        my = out.dump();
    }
    return my.data();
}

bool isFilenameUsableEx(const std::string& filePath)
{
    fs::path p(filePath);
    fs::path parent = p.parent_path();

    // Parent directory does not exist, cannot create target file
    if (!parent.empty() && !fs::exists(parent))
    {
        return false;
    }

    if (fs::exists(p))
        return true;

    std::ofstream file(p, std::ios::out | std::ios::trunc);
    if (file.is_open())
    {
        file.close();
        fs::remove(p);
        return true;
    }
    return false;
}

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Forward declaration
bool isFilenameUsableEx(const std::string& filePath);

/**
 * @brief Check validity of initialization configuration JSON string
 * @param cfgJson Input configuration JSON string
 * @return Error string pointer if validation failed; nullptr if all parameters are valid
 * @note Internal static buffer stores error message to avoid dangling pointer
 */
WT_SESSION_API const char* WT_SESSION_CALL
wtPlugin_CheckInitializeParameter(const char* cfgJson)
{
    // Persistent buffer for error message, avoid dangling pointer
    static std::string errMessage;
    errMessage.clear();

    try
    {
        if (cfgJson == nullptr)
        {
            errMessage = "input cfgJson pointer is null";
            return errMessage.c_str();
        }

        json cfg = json::parse(cfgJson);

        // Check required field: host (no default value, must exist and non-empty)
        if (!cfg.contains("host") || cfg["host"].is_null() || cfg["host"].get<std::string>().empty())
        {
            errMessage = "host field should not be empty or missing";
            return errMessage.c_str();
        }

        // Check required field: user
        if (!cfg.contains("user") || cfg["user"].is_null() || cfg["user"].get<std::string>().empty())
        {
            errMessage = "user field should not be empty or missing";
            return errMessage.c_str();
        }

        // Check required field: password
        if (!cfg.contains("password") || cfg["password"].is_null() || cfg["password"].get<std::string>().empty())
        {
            errMessage = "password field should not be empty or missing";
            return errMessage.c_str();
        }

        // Check session path
        std::string path;
        if (!cfg.contains("session_path") || cfg["session_path"].is_null())
        {
            errMessage = "session_path field is missing";
            return errMessage.c_str();
        }
        path = cfg["session_path"].get<std::string>();
        if (path.empty())
        {
            errMessage = "session_path field should not be empty";
            return errMessage.c_str();
        }

        // Fixed logic: path invalid when isFilenameUsableEx returns false
        if (!isFilenameUsableEx(path))
        {
            errMessage = "session_path field is not a valid writable path";
            return errMessage.c_str();
        }

        // All parameters valid
        return nullptr;
    }
    catch (const std::exception& e)
    {
        errMessage = std::string("json parse or validation exception: ") + e.what();
        return errMessage.c_str();
    }
    catch (...)
    {
        errMessage = "unknown unexpected error during parameter checking";
        return errMessage.c_str();
    }
}

} // extern "C"
