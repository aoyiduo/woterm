/**
 * wt_session_plugin.h  –  Public C ABI
 *
 * ABI stability guarantee
 * -----------------------
 * This header is the ONLY file that third-party callers should include.
 * All types visible here are plain-C: fixed-width integers, const char*,
 * and opaque handle pointers.  No C++ STL types, no virtual tables, no
 * exceptions, no name-mangled symbols cross the boundary.
 *
 * This means the DLL / .so can be rebuilt with any compiler (MSVC, GCC,
 * Clang) or C++ runtime version without breaking callers compiled against
 * an older copy of this header.
 *
 * Versioning
 * ----------
 * WT_SESSION_API_VERSION is bumped when a new function is appended at the
 * end of the vtable (backward-compatible) or when a break is required.
 * Callers check the version returned by wtSession_GetApiVersion() at
 * runtime before using features from a newer minor version.
 *
 * Ownership rules
 * ---------------
 * - Strings returned as const char* are owned by the plugin instance and
 *   remain valid until the next call that mutates the same session, or
 *   until wtSession_Destroy() is called.
 * - Callers must NOT free any pointer returned by the plugin.
 * - WtSessionSessionView is a lightweight read-only snapshot filled by the
 *   plugin; the caller owns the struct but NOT the char* fields inside it.
 *
 * Thread safety
 * -------------
 * No function is safe to call concurrently on the same handle.
 * Use external locking when sharing a handle across threads.
 *
 * Dependencies (implementation only, invisible to callers)
 * ---------------------------------------------------------
 *   nlohmann/json.hpp   https://github.com/nlohmann/json
 *   httplib.h           https://github.com/yhirose/cpp-httplib
 */

#ifndef WT_SESSION_PLUGIN_H
#define WT_SESSION_PLUGIN_H

#include <stdint.h>   /* int32_t, int64_t  – C89-compatible fixed-width types */
#include <stddef.h>   /* size_t */

/* -------------------------------------------------------------------------
 * Compiler / platform portability macros
 * ---------------------------------------------------------------------- */

#ifdef __cplusplus
#  define WT_SESSION_EXTERN_C_BEGIN  extern "C" {
#  define WT_SESSION_EXTERN_C_END    }
#else
#  define WT_SESSION_EXTERN_C_BEGIN
#  define WT_SESSION_EXTERN_C_END
#endif

#if defined(_WIN32) || defined(_WIN64)
#  ifdef WT_SESSION_PLUGIN_BUILDING_DLL
#    define WT_SESSION_API  __declspec(dllexport)
#  else
#    define WT_SESSION_API  __declspec(dllimport)
#  endif
#  define WT_SESSION_CALL  __cdecl
#else
#  if defined(__GNUC__) && __GNUC__ >= 4
#    define WT_SESSION_API  __attribute__((visibility("default")))
#  else
#    define WT_SESSION_API
#  endif
#  define WT_SESSION_CALL
#endif

/* -------------------------------------------------------------------------
 * API version
 * Encoded as  (major << 16) | minor
 * ---------------------------------------------------------------------- */
#define WT_SESSION_API_VERSION  (0x00010000u)   /* 1.0 */

/* -------------------------------------------------------------------------
 * Opaque plugin handle
 * Third-party code only ever holds a WtSessionHandle* and passes it back.
 * The struct layout is defined only inside the .cpp translation unit.
 * ---------------------------------------------------------------------- */
typedef struct WtSessionContext  WtSessionContext;
typedef        WtSessionContext* WtSessionHandle;

/* -------------------------------------------------------------------------
 * Host-type constants  (matches the WtHostType enum in the host app)
 * ---------------------------------------------------------------------- */
#define WT_HOST_INVALID         (0)
#define WT_HOST_SSH_WITH_SFTP   (1)
#define WT_HOST_SFTP_ONLY       (2)
/* 3 is reserved – do not use */
#define WT_HOST_TELNET          (4)
#define WT_HOST_RLOGIN          (5)
/* 6 is reserved – do not use */
#define WT_HOST_RDP             (7)
#define WT_HOST_VNC             (8)
#define WT_HOST_FTP             (9)

/* -------------------------------------------------------------------------
 * Return-code constants
 * ---------------------------------------------------------------------- */
#define WT_SESSION_OK             (0)   /* success */
#define WT_SESSION_ERR_PARAM      (-1)   /* null or invalid argument */
#define WT_SESSION_ERR_NOT_FOUND  (-2)   /* session name not in local list */
#define WT_SESSION_ERR_IO         (-3)   /* file read/write failure */
#define WT_SESSION_ERR_NET        (-4)   /* HTTP/network failure */
#define WT_SESSION_ERR_PARSE      (-5)   /* JSON parse error */
#define WT_SESSION_ERR_STATE      (-6)   /* wtSession_Init() not called yet */

/* -------------------------------------------------------------------------
 * Read-only session snapshot
 * Filled by wtSession_GetSession() / wtSession_GetSessionAt().
 * All char* point into plugin-owned storage; do NOT free them.
 * The snapshot is invalidated by any subsequent mutating call.
 * ---------------------------------------------------------------------- */
typedef struct WtSessionSessionView {
    uint32_t    struct_size;        /**< must be sizeof(WtSessionSessionView) – version guard */
    int64_t     id;
    int32_t     type;               /**< one of the WT_HOST_* constants */
    const char *name;               /**< globally unique name */
    const char *host;
    int32_t     port;
    const char *login_username;
    const char *login_password;
    const char *login_identity_file;
    const char *proxy_jump;         /**< comma-separated jump-host IDs */
    const char *memo;
    const char *path;               /**< parent path node */
    int32_t     order_num;
    int32_t     online;             /**< 1 = online, 0 = offline */
} WtSessionSessionView;

/* -------------------------------------------------------------------------
 * Sync-result snapshot
 * Returned via a callback registered with wtSession_Sync() so the caller
 * never has to manage a heap-allocated result struct.
 * ---------------------------------------------------------------------- */
typedef struct WtSessionSyncResult {
    int32_t     status;             /**< WT_SESSION_OK or an error code */
    const char *error_msg;          /**< human-readable error; NULL when status==OK */
    int32_t     added_count;
    int32_t     updated_count;
    int32_t     removed_count;
    /**< Individual name arrays – valid only for the duration of the callback */
    const char *const *added_names;
    const char *const *updated_names;
    const char *const *removed_names;
} WtSessionSyncResult;

/** Callback invoked by wtSession_Sync() when the operation completes (or fails). */
typedef void (WT_SESSION_CALL *WtSessionSyncCallback)(
    const WtSessionSyncResult *result,
    void                   *user_data
);

/* =========================================================================
 * Public API functions
 * ======================================================================== */

WT_SESSION_EXTERN_C_BEGIN

/**
 * wtSession_GetApiVersion
 * Returns WT_SESSION_API_VERSION compiled into the DLL.
 * Callers can compare against their own WT_SESSION_API_VERSION to detect
 * header/library version mismatches.
 */
WT_SESSION_API uint32_t WT_SESSION_CALL
wtSession_GetApiVersion(void);

/* -------------------------------------------------------------------------
 * feature-code constants
 * ---------------------------------------------------------------------- */
#define WT_SESSION_FEATURE_NONE             (0)
#define WT_SESSION_FEATURE_SHOW_PASSWORD    (1)
#define WT_SESSION_FEATURE_UPDATE_INFO      (2)
#define WT_SESSION_FEATURE_RUN_AS_TUNNEL    (4)
/**
 * wtSession_GetFeature
 *
 * @return  WT_SESSION_FEATURE_SHOW_PASSWORD, WT_SESSION_FEATURE_UPDATE_INFO, WT_SESSION_FEATURE_RUN_AS_TUNNEL.
 */
WT_SESSION_API uint32_t WT_SESSION_CALL
wtSession_GetFeature(void);
/* -------------------------------------------------------------------------
 * Lifecycle
 * ---------------------------------------------------------------------- */

/**
 * wtSession_Create
 * Allocate a new plugin context. All memory is owned by the plugin.
 * The context is not usable until wtSession_Init() succeeds.
 *
 * @return  A non-NULL handle on success, NULL on allocation failure.
 */
WT_SESSION_API WtSessionHandle WT_SESSION_CALL
wtSession_Create(void);

/**
 * wtSession_Init
 * Initialize the context with connection parameters and load the local
 * session list from disk (non-fatal if the file does not exist yet).
 *
 * The cfgJson parameter is a JSON string containing plugin-specific
 * configuration parameters. Use wtPlugin_GetInitilizeParameterTemplate()
 * to get the expected parameter structure.
 *
 * @param handle   Plugin context handle.
 * @param cfgJson  JSON string containing initialization parameters.
 *
 * @return  WT_SESSION_OK on success,
 *          WT_SESSION_ERR_PARAM for null/invalid arguments,
 *          WT_SESSION_ERR_IO if file read fails,
 *          or WT_SESSION_ERR_PARSE if JSON parsing fails.
 */
WT_SESSION_API int32_t WT_SESSION_CALL
wtSession_Init(WtSessionHandle handle, const char *cfgJson);

/**
 * wtSession_Destroy
 * Release all resources owned by the context.
 * The handle must not be used after this call.
 *
 * @param handle  Plugin context handle to destroy.
 */
WT_SESSION_API void WT_SESSION_CALL
wtSession_Destroy(WtSessionHandle handle);

/* -------------------------------------------------------------------------
 * Synchronisation
 * ---------------------------------------------------------------------- */

/**
 * wtSession_Sync
 * Synchronize session list from remote source.
 * 
 * This function is plugin-specific and typically:
 *   - Pulls session data from a remote service (e.g., frps dashboard API).
 *   - Diffs against the local session list.
 *   - Adds sessions present remotely but absent locally.
 *   - Updates fields (port, online status, etc.) for sessions present in both.
 *   - Removes sessions present locally but absent remotely.
 * 
 * Saves the updated list to disk (merge-safe) before returning.
 *
 * The result is delivered synchronously via callback before this function
 * returns. callback may be NULL (result is discarded).
 *
 * @param handle    Plugin context handle.
 * @param callback  Callback function to receive sync result (may be NULL).
 * @param user_data User-defined data passed to the callback.
 *
 * @return  WT_SESSION_OK on success, or an error code (also reflected in result.status).
 */
WT_SESSION_API int32_t WT_SESSION_CALL
wtSession_Sync(WtSessionHandle       handle,
            WtSessionSyncCallback callback,
            void              *user_data);

/* -------------------------------------------------------------------------
 * Query
 * ---------------------------------------------------------------------- */

/**
 * wtSession_GetSessionCount
 * Get the number of sessions in the local list.
 *
 * @param handle  Plugin context handle.
 *
 * @return  Number of sessions in the local list, or -1 on error.
 */
WT_SESSION_API int32_t WT_SESSION_CALL
wtSession_GetSessionCount(WtSessionHandle handle);

/**
 * wtSession_GetSessionAt
 * Fill *view with a read-only snapshot of the session at position index
 * (0-based).
 *
 * view->struct_size must be set by the caller before the call (must equal
 * sizeof(WtSessionSessionView)).
 *
 * @param handle  Plugin context handle.
 * @param index   0-based index of the session to retrieve.
 * @param view    Pointer to WtSessionSessionView to be filled.
 *
 * @return  WT_SESSION_OK on success,
 *          WT_SESSION_ERR_PARAM for null/invalid arguments,
 *          or WT_SESSION_ERR_NOT_FOUND if index is out of bounds.
 */
WT_SESSION_API int32_t WT_SESSION_CALL
wtSession_GetSessionAt(WtSessionHandle       handle,
                    int32_t            index,
                    WtSessionSessionView *view);

/**
 * wtSession_GetSession
 * Fill *view with a read-only snapshot of the session identified by name.
 *
 * view->struct_size must be set by the caller before the call (must equal
 * sizeof(WtSessionSessionView)).
 *
 * @param handle  Plugin context handle.
 * @param name    Name of the session to retrieve.
 * @param view    Pointer to WtSessionSessionView to be filled.
 *
 * @return  WT_SESSION_OK on success,
 *          WT_SESSION_ERR_PARAM for null/invalid arguments,
 *          or WT_SESSION_ERR_NOT_FOUND if session with given name does not exist.
 */
WT_SESSION_API int32_t WT_SESSION_CALL
wtSession_GetSession(WtSessionHandle       handle,
                  const char        *name,
                  WtSessionSessionView *view);

/* -------------------------------------------------------------------------
 * Mutation  –  UpdateInfo
 * ---------------------------------------------------------------------- */


/**
 * wtSession_UpdateInfo
 * Update session information fields for the session identified by id.
 *
 * Rules:
 *  - The field parameter is a bitmask of WT_UPDATE_FIELD_* constants specifying
 *    which fields to update.
 *  - Only fields specified in the bitmask are modified; other fields remain unchanged.
 *  - The corresponding string parameters for fields not in the bitmask are ignored.
 *
 * Saves the list (merge-safe) before returning.
 *
 * @param handle   Plugin context handle.
 * @param id       Session ID to update.
 * @param name     New session name (used only if field includes WT_UPDATE_FIELD_NAME).
 * @param username New login username (used only if field includes WT_UPDATE_FIELD_USERNAME).
 * @param password New login password (used only if field includes WT_UPDATE_FIELD_PASSWORD).
 * @param ident_file New identity file path (used only if field includes WT_UPDATE_FIELD_IDENTFILE).
 * @param path     New parent path node (used only if field includes WT_UPDATE_FIELD_PATH).
 * @param proxy_jump New proxy jump hosts (used only if field includes WT_UPDATE_FIELD_PROXYJUMP).
 * @param memo     New memo text (used only if field includes WT_UPDATE_FIELD_MEMO).
 * @param type     New host type (used only if field includes WT_UPDATE_FIELD_TYPE).
 * @param field    Bitmask of WT_UPDATE_FIELD_* constants indicating which fields to update.
 *
 * @return  WT_SESSION_OK on success,
 *          WT_SESSION_ERR_PARAM for null/invalid arguments,
 *          WT_SESSION_ERR_NOT_FOUND if session with given id does not exist,
 *          or WT_SESSION_ERR_IO if save to disk fails.
 */
#define WT_UPDATE_FIELD_NAME                (1)
#define WT_UPDATE_FIELD_USERNAME            (1<<1)
#define WT_UPDATE_FIELD_PASSWORD            (1<<2)
#define WT_UPDATE_FIELD_IDENTFILE           (1<<3)
#define WT_UPDATE_FIELD_PATH                (1<<4)
#define WT_UPDATE_FIELD_PROXYJUMP           (1<<5)
#define WT_UPDATE_FIELD_MEMO                (1<<6)
#define WT_UPDATE_FIELD_TYPE                (1<<7)
WT_SESSION_API int32_t WT_SESSION_CALL
wtSession_UpdateInfo(WtSessionHandle handle,
                         int64_t id,
                         const char  *name,
                         const char  *username,
                         const char  *password,
                         const char  *ident_file,
                         const char  *path,
                         const char  *proxy_jump,
                         const char  *memo,
                         int32_t      type,
                         int32_t field);


/**
 * wtSession_RemoveByIds
 * Remove sessions from the local list by their IDs.
 *
 * @param handle  Plugin context handle.
 * @param ids     Array of session IDs to remove.
 * @param cnt     Number of IDs in the array.
 *
 * Saves the updated list (merge-safe) before returning.
 *
 * @return  WT_SESSION_OK on success,
 *          WT_SESSION_ERR_PARAM for null/invalid arguments,
 *          WT_SESSION_ERR_STATE if context not initialized,
 *          or WT_SESSION_ERR_IO if save to disk fails.
 */
WT_SESSION_API int32_t WT_SESSION_CALL
wtSession_RemoveByIds(WtSessionHandle handle, const int64_t *ids, int cnt);
/* -------------------------------------------------------------------------
 * Persistence
 * ---------------------------------------------------------------------- */

/**
 * wtSession_Load
 * Reload the session list from disk, discarding in-memory state.
 *
 * @param handle  Plugin context handle.
 *
 * @return  WT_SESSION_OK on success,
 *          WT_SESSION_ERR_STATE if context not initialized,
 *          WT_SESSION_ERR_IO if file read fails,
 *          or WT_SESSION_ERR_PARSE if JSON parsing fails.
 */
WT_SESSION_API int32_t WT_SESSION_CALL
wtSession_Load(WtSessionHandle handle);

/**
 * wtSession_Save
 * Persist the in-memory list with merge-safe strategy:
 *   1. Re-read the file currently on disk.
 *   2. Overlay in-memory sessions (in-memory wins on name collision).
 *   3. Retain disk-only sessions not present in memory.
 *   4. Write the merged result back to disk.
 *
 * @param handle  Plugin context handle.
 *
 * @return  WT_SESSION_OK on success,
 *          WT_SESSION_ERR_STATE if context not initialized,
 *          or WT_SESSION_ERR_IO if file write fails.
 */
WT_SESSION_API int32_t WT_SESSION_CALL
wtSession_Save(WtSessionHandle handle);

/* -------------------------------------------------------------------------
 * Plugin types  (for distinguishing different plugin categories)
 * ---------------------------------------------------------------------- */
#define WT_PLUGIN_TYPE_UNKNOWN      (0)   /**< unknown or undefined type */
#define WT_PLUGIN_TYPE_SESSION      (1)   /**< session management plugin */
#define WT_PLUGIN_TYPE_THEME        (2)   /**< theme customization plugin */
#define WT_PLUGIN_TYPE_TERMINAL     (3)   /**< terminal enhancement plugin */
#define WT_PLUGIN_TYPE_UTILITY      (4)   /**< utility/tool plugin */

/* -------------------------------------------------------------------------
 * Generic plugin information (common across all plugin types)
 * ---------------------------------------------------------------------- */

/**
 * Read-only generic plugin information snapshot.
 * Filled by wtPlugin_GetInfo().
 * All char* point into plugin-owned storage; do NOT free them.
 */
typedef struct WtPluginInfo {
    uint32_t    struct_size;        /**< must be sizeof(WtPluginInfo) – version guard */
    int32_t     plugin_type;        /**< one of WT_PLUGIN_TYPE_* constants */
    const char *plugin_name;        /**< plugin name */
    const char *plugin_version;     /**< plugin version string (e.g., "1.0.0") */
    const char *plugin_description; /**< brief description */
    const char *plugin_vendor;      /**< vendor/author name */
    const char *plugin_id;          /**< unique identifier (reverse domain notation) */
    uint32_t    api_version;        /**< API version (encoded as (major << 16) | minor) */
} WtPluginInfo;

/* -------------------------------------------------------------------------
 * Diagnostics
 * ---------------------------------------------------------------------- */

/**
 * wtSession_GetLastError
 * Return a human-readable description of the last error that occurred on
 * this handle, or an empty string if no error is recorded.
 * The returned pointer is valid until the next call on the same handle.
 */
WT_SESSION_API const char * WT_SESSION_CALL
wtSession_GetLastError(WtSessionHandle handle);

/**
 * wtPlugin_GetInfo
 * Fill *info with read-only generic plugin information.
 *
 * This is a common interface across all woterm plugins for querying
 * plugin metadata and type classification.
 *
 * info->struct_size must be set by the caller before the call.
 *
 * @return  WT_SESSION_OK or WT_SESSION_ERR_PARAM.
 */
WT_SESSION_API int32_t WT_SESSION_CALL
wtPlugin_GetInfo(WtPluginInfo *info);

/**
 * wtPlugin_GetParameterTemplate
 * Returns a JSON string containing the parameter template for plugin configuration.
 * The template includes default values for all required and optional parameters.
 * The returned string is owned by the plugin and should not be freed by the caller.
 */
WT_SESSION_API const char* WT_SESSION_CALL
wtPlugin_GetInitilizeParameterTemplate(void);

WT_SESSION_API const char* WT_SESSION_CALL
wtPlugin_CheckInitializeParameter(const char* json);


WT_SESSION_EXTERN_C_END

#endif /* WT_SESSION_PLUGIN_H */
