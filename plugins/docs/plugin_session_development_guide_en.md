# WoTerm Session Plugin Development Guide

## Overview

WoTerm supports session management plugins through a C ABI interface. A plugin is an independent dynamic link library (`.dll` on Windows, `.so` on Linux, `.dylib` on macOS) that implements a predefined set of C API functions for the main application to call.

This document is based on the `wt_session_plugin.h` header file and aims to help developers quickly understand and implement their own session management plugins.

## Core Design Principles

### ABI Stability Guarantee

The plugin's public interface must use pure C types:
- Fixed-width integer types (`int32_t`, `int64_t`)
- `const char*` for string passing
- Opaque handle pointers (`WtSessionHandle`)
- No C++ STL types, virtual tables, or exceptions

This means the plugin can be compiled with any compiler (MSVC, GCC, Clang) or C++ runtime version without breaking callers compiled against an older copy of this header.

### String Ownership Rules

- Strings returned as `const char*` are owned by the plugin instance and remain valid until the next call that mutates the same session, or until `wtSession_Destroy()` is called
- Callers must **NOT** free any pointer returned by the plugin
- `WtSessionSessionView` is a lightweight read-only snapshot; the caller owns the struct but **NOT** the `char*` fields inside it

### Thread Safety

No function is safe to call concurrently on the same handle. Use external locking when sharing a handle across threads.

## API Versioning

`WT_SESSION_API_VERSION` is encoded as `(major << 16) | minor`. The version is bumped when a new function is appended at the end of the vtable (backward-compatible) or when a breaking change is required.

Callers check the version returned by `wtSession_GetApiVersion()` at runtime before using features from a newer minor version.

## Required C API Functions

All functions must be declared with `extern "C"` and use `WT_SESSION_API` and `WT_SESSION_CALL` macros.

### 1. API Version and Feature Query

```c
WT_SESSION_API uint32_t WT_SESSION_CALL wtSession_GetApiVersion(void);
WT_SESSION_API uint32_t WT_SESSION_CALL wtSession_GetFeature(void);
```

**Implementation Notes:**
- `wtSession_GetApiVersion()` returns `WT_SESSION_API_VERSION`
- `wtSession_GetFeature()` returns a feature bitmask with optional values:
  - `WT_SESSION_FEATURE_NONE` (0) - No special features
  - `WT_SESSION_FEATURE_SHOW_PASSWORD` (1) - Allow showing passwords
  - `WT_SESSION_FEATURE_UPDATE_INFO` (2) - Support updating session info
  - `WT_SESSION_FEATURE_RUN_AS_TUNNEL` (4) - Support running as tunnel

### 2. Lifecycle Management

```c
WT_SESSION_API WtSessionHandle WT_SESSION_CALL wtSession_Create(void);
WT_SESSION_API int32_t WT_SESSION_CALL wtSession_Init(WtSessionHandle handle, const char *cfgJson);
WT_SESSION_API void WT_SESSION_CALL wtSession_Destroy(WtSessionHandle handle);
```

**Implementation Notes:**
- `wtSession_Create()` allocates the plugin context and returns an opaque handle
- `wtSession_Init()` initializes the context with JSON configuration and loads the local session list
- `wtSession_Destroy()` releases all resources

**Configuration JSON Parameters:**

Plugins should return supported configuration parameter templates via `wtPlugin_GetInitilizeParameterTemplate()`. FRP plugin example:

```json
{
    "host": "example.com",
    "port": 7500,
    "user": "admin",
    "password": "password for login",
    "session_path": "frpsession.json",
    "http_timeout_sec": 10,
    "default_session_username": "xxxx",
    "default_session_password": "xxxx"
}
```

### 3. Session Synchronization

```c
WT_SESSION_API int32_t WT_SESSION_CALL wtSession_Sync(WtSessionHandle handle,
                                                      WtSessionSyncCallback callback,
                                                      void *user_data);
```

**Implementation Notes:**
- Pull session data from a remote source
- Diff against the local list
- Add sessions present remotely but absent locally
- Update port and online status for sessions present in both
- Remove sessions present locally but absent remotely
- Save to disk with merge-safe strategy

**Sync Callback:**

```c
typedef struct WtSessionSyncResult {
    int32_t     status;             // WT_SESSION_OK or error code
    const char *error_msg;          // Error message, NULL when status==OK
    int32_t     added_count;        // Number of added sessions
    int32_t     updated_count;      // Number of updated sessions
    int32_t     removed_count;      // Number of removed sessions
    const char *const *added_names; // Array of added session names
    const char *const *updated_names; // Array of updated session names
    const char *const *removed_names; // Array of removed session names
} WtSessionSyncResult;

typedef void (WT_SESSION_CALL *WtSessionSyncCallback)(
    const WtSessionSyncResult *result,
    void *user_data
);
```

### 4. Session Query

```c
WT_SESSION_API int32_t WT_SESSION_CALL wtSession_GetSessionCount(WtSessionHandle handle);
WT_SESSION_API int32_t WT_SESSION_CALL wtSession_GetSessionAt(WtSessionHandle handle,
                                                              int32_t index,
                                                              WtSessionSessionView *view);
WT_SESSION_API int32_t WT_SESSION_CALL wtSession_GetSession(WtSessionHandle handle,
                                                           const char *name,
                                                           WtSessionSessionView *view);
```

**Implementation Notes:**
- `wtSession_GetSessionCount()` returns the number of sessions in the local list
- `wtSession_GetSessionAt()` gets a session snapshot by index
- `wtSession_GetSession()` gets a session snapshot by name

**Session View Structure:**

```c
typedef struct WtSessionSessionView {
    uint32_t    struct_size;        // Must be set to sizeof(WtSessionSessionView)
    int64_t     id;                 // Session ID
    int32_t     type;               // Session type (WT_HOST_* constant)
    const char *name;               // Globally unique name
    const char *host;               // Host address
    int32_t     port;               // Port number
    const char *login_username;     // Login username
    const char *login_password;     // Login password
    const char *login_identity_file; // SSH identity file path
    const char *proxy_jump;         // Jump host IDs (comma-separated)
    const char *memo;               // Memo/description
    const char *path;               // Parent path node
    int32_t     order_num;          // Display order number
    int32_t     online;             // Online status (1=online, 0=offline)
} WtSessionSessionView;
```

### 5. Session Update

```c
WT_SESSION_API int32_t WT_SESSION_CALL wtSession_UpdateInfo(WtSessionHandle handle,
                                                           int64_t id,
                                                           const char *name,
                                                           const char *username,
                                                           const char *password,
                                                           const char *ident_file,
                                                           const char *path,
                                                           const char *proxy_jump,
                                                           const char *memo,
                                                           int32_t type,
                                                           int32_t field);
```

**Implementation Notes:**
- The `field` parameter is a bitmask specifying which fields to update
- Only fields specified in the bitmask are modified; other fields remain unchanged
- Save to disk with merge-safe strategy after updating

**Field Bitmask Constants:**

| Constant | Value | Description |
|----------|-------|-------------|
| `WT_UPDATE_FIELD_NAME` | 1 | Update session name |
| `WT_UPDATE_FIELD_USERNAME` | 2 | Update username |
| `WT_UPDATE_FIELD_PASSWORD` | 4 | Update password |
| `WT_UPDATE_FIELD_IDENTFILE` | 8 | Update identity file |
| `WT_UPDATE_FIELD_PATH` | 16 | Update parent path |
| `WT_UPDATE_FIELD_PROXYJUMP` | 32 | Update proxy jump |
| `WT_UPDATE_FIELD_MEMO` | 64 | Update memo |
| `WT_UPDATE_FIELD_TYPE` | 128 | Update session type |

### 6. Session Removal

```c
WT_SESSION_API int32_t WT_SESSION_CALL wtSession_RemoveByIds(WtSessionHandle handle,
                                                             const int64_t *ids,
                                                             int cnt);
```

**Implementation Notes:**
- Batch remove sessions by ID
- Save to disk with merge-safe strategy after removal

### 7. Persistence Operations

```c
WT_SESSION_API int32_t WT_SESSION_CALL wtSession_Load(WtSessionHandle handle);
WT_SESSION_API int32_t WT_SESSION_CALL wtSession_Save(WtSessionHandle handle);
```

**Implementation Notes:**
- `wtSession_Load()` reloads the session list from disk, discarding in-memory state
- `wtSession_Save()` saves the in-memory list with merge-safe strategy

**Merge-Safe Save Strategy:**
1. Re-read the current file on disk
2. Overlay in-memory sessions (in-memory wins on name collision)
3. Retain disk-only sessions not present in memory (from other clients)
4. Write the merged result back to disk

### 8. Diagnostics

```c
WT_SESSION_API const char * WT_SESSION_CALL wtSession_GetLastError(WtSessionHandle handle);
```

**Implementation Notes:**
- Returns a human-readable description of the last error
- The returned pointer is valid until the next call

### 9. Plugin Information Query

```c
WT_SESSION_API int32_t WT_SESSION_CALL wtPlugin_GetInfo(WtPluginInfo *info);
WT_SESSION_API const char* WT_SESSION_CALL wtPlugin_GetInitilizeParameterTemplate(void);
WT_SESSION_API const char* WT_SESSION_CALL wtPlugin_CheckInitializeParameter(const char* json);
```

**Implementation Notes:**
- `wtPlugin_GetInfo()` returns plugin metadata
- `wtPlugin_GetInitilizeParameterTemplate()` returns a JSON string of configuration parameter template
- `wtPlugin_CheckInitializeParameter()` validates configuration parameter validity

**Plugin Info Structure:**

```c
typedef struct WtPluginInfo {
    uint32_t    struct_size;        // Must be set to sizeof(WtPluginInfo)
    int32_t     plugin_type;        // Plugin type (WT_PLUGIN_TYPE_SESSION)
    const char *plugin_name;        // Plugin name
    const char *plugin_version;     // Plugin version string
    const char *plugin_description; // Plugin description
    const char *plugin_vendor;      // Vendor/author name
    const char *plugin_id;          // Unique identifier (reverse domain notation)
    uint32_t    api_version;        // API version
} WtPluginInfo;
```

## Host Type Constants

| Constant | Value | Description |
|----------|-------|-------------|
| `WT_HOST_INVALID` | 0 | Invalid type |
| `WT_HOST_SSH_WITH_SFTP` | 1 | SSH with SFTP |
| `WT_HOST_SFTP_ONLY` | 2 | SFTP only |
| `WT_HOST_TELNET` | 4 | Telnet |
| `WT_HOST_RLOGIN` | 5 | RLogin |
| `WT_HOST_RDP` | 7 | RDP |
| `WT_HOST_VNC` | 8 | VNC |
| `WT_HOST_FTP` | 9 | FTP |

## Return Code Constants

| Constant | Value | Description |
|----------|-------|-------------|
| `WT_SESSION_OK` | 0 | Success |
| `WT_SESSION_ERR_PARAM` | -1 | Null or invalid argument |
| `WT_SESSION_ERR_NOT_FOUND` | -2 | Session name not in local list |
| `WT_SESSION_ERR_IO` | -3 | File read/write failure |
| `WT_SESSION_ERR_NET` | -4 | HTTP/network failure |
| `WT_SESSION_ERR_PARSE` | -5 | JSON parse error |
| `WT_SESSION_ERR_STATE` | -6 | wtSession_Init() not called yet |

## Build Commands

### Linux / GCC

```bash
g++ -std=c++17 -DWT_SESSION_PLUGIN_BUILDING_DLL \
    -I../include \
    -shared -fPIC \
    wt_session_myplugin.cpp -o libwt_session_myplugin.so \
    -lssl -lcrypto -lpthread
```

### Windows / MSVC

```cmd
cl /std:c++17 /DWT_SESSION_PLUGIN_BUILDING_DLL /I..\include \
   /LD wt_session_myplugin.cpp /link /OUT:wt_session_myplugin.dll
```

### macOS / Clang

```bash
clang++ -std=c++17 -DWT_SESSION_PLUGIN_BUILDING_DLL \
        -I../include \
        -shared -fPIC \
        wt_session_myplugin.cpp -o libwt_session_myplugin.dylib \
        -lssl -lcrypto -lpthread
```

## Plugin Development Steps

### Step 1: Create Plugin File

Create `wt_session_myplugin.cpp` with necessary includes:

```cpp
#include "wt_session_plugin.h"
#include <nlohmann/json.hpp>
#include <httplib.h>
#include <string>
#include <vector>
#include <unordered_map>

using json = nlohmann::json;
```

### Step 2: Define Internal Data Structures

```cpp
struct SessionEntry {
    int64_t     id = 0;
    int32_t     type = WT_HOST_INVALID;
    std::string name;
    std::string host;
    int32_t     port = 0;
    std::string login_username;
    std::string login_password;
    std::string login_identity_file;
    std::string proxy_jump;
    std::string memo;
    std::string path;
    int32_t     order_num = 0;
    bool        online = false;
};

struct WtSessionContext {
    std::string server_addr;
    // ... other configuration fields ...
    bool initialized = false;
    std::vector<SessionEntry> sessions;
    std::string last_error;
    SessionEntry view_cache;
    // ... sync result caches ...
};
```

### Step 3: Implement Lifecycle Functions

```cpp
extern "C" {

WT_SESSION_API WtSessionHandle WT_SESSION_CALL
wtSession_Create(void) {
    try { return new WtSessionContext(); }
    catch (...) { return nullptr; }
}

WT_SESSION_API int32_t WT_SESSION_CALL
wtSession_Init(WtSessionHandle handle, const char* cfgJson) {
    // Parse JSON configuration
    // Load local session file
    // Initialize context
    return WT_SESSION_OK;
}

WT_SESSION_API void WT_SESSION_CALL
wtSession_Destroy(WtSessionHandle handle) {
    delete handle;
}

// ... implement other API functions ...

} // extern "C"
```

### Step 4: Implement Session Synchronization Logic

```cpp
WT_SESSION_API int32_t WT_SESSION_CALL
wtSession_Sync(WtSessionHandle handle,
               WtSessionSyncCallback callback,
               void* user_data) {
    // 1. Fetch session data from remote source
    // 2. Diff against local list
    // 3. Add/update/remove sessions
    // 4. Save to disk
    // 5. Call callback with results
    return WT_SESSION_OK;
}
```

### Step 5: Implement Query and Update Functions

```cpp
WT_SESSION_API int32_t WT_SESSION_CALL
wtSession_GetSessionAt(WtSessionHandle handle,
                       int32_t index,
                       WtSessionSessionView* view) {
    // Validate parameters
    // Fill view structure
    return WT_SESSION_OK;
}

WT_SESSION_API int32_t WT_SESSION_CALL
wtSession_UpdateInfo(WtSessionHandle handle, int64_t id, ...) {
    // Update fields based on field bitmask
    // Save to disk
    return WT_SESSION_OK;
}
```

### Step 6: Implement Plugin Information Functions

```cpp
WT_SESSION_API int32_t WT_SESSION_CALL
wtPlugin_GetInfo(WtPluginInfo* info) {
    info->plugin_type = WT_PLUGIN_TYPE_SESSION;
    info->plugin_name = "My Session Plugin";
    info->plugin_version = "1.0.0";
    info->plugin_description = "My custom session management plugin";
    info->plugin_vendor = "My Company";
    info->plugin_id = "com.mycompany.plugins.mysession";
    info->api_version = WT_SESSION_API_VERSION;
    return WT_SESSION_OK;
}

WT_SESSION_API const char* WT_SESSION_CALL
wtPlugin_GetInitilizeParameterTemplate(void) {
    static std::string my;
    if(my.empty()) {
        json out;
        out["host"] = "example.com";
        // ... add other parameters ...
        my = out.dump();
    }
    return my.data();
}
```

## Best Practices

### 1. String Lifecycle Management

Use internal caching (e.g., `view_cache`) to ensure returned `const char*` remains valid during a single API call:

```cpp
static void fillView(WtSessionSessionView& v, const SessionEntry& e) {
    v.id = e.id;
    v.name = e.name.c_str();
    // ... other fields ...
}

WT_SESSION_API int32_t WT_SESSION_CALL
wtSession_GetSessionAt(WtSessionHandle handle, int32_t index, WtSessionSessionView* view) {
    handle->view_cache = handle->sessions[index];
    fillView(*view, handle->view_cache);
    return WT_SESSION_OK;
}
```

### 2. Merge-Safe Saving

Implement merge-safe strategy to avoid overwriting changes from other clients:

```cpp
static bool saveToDisk(const std::string& path,
                       const std::vector<SessionEntry>& mem, bool merge,
                       std::string& errMsg) {
    // Read disk content
    // Merge memory and disk sessions
    // Sort and write
}
```

### 3. Error Handling

Use `setError()` to record detailed error information:

```cpp
WT_SESSION_API int32_t WT_SESSION_CALL
wtSession_Init(WtSessionHandle handle, const char* cfgJson) {
    if (!cfgJson) {
        handle->setError("cfgJson is null");
        return WT_SESSION_ERR_PARAM;
    }
    // ...
}
```

### 4. Input Validation

Validate configuration parameters in `wtPlugin_CheckInitializeParameter()`:

```cpp
WT_SESSION_API const char* WT_SESSION_CALL
wtPlugin_CheckInitializeParameter(const char* cfgJson) {
    static std::string errMessage;
    // Validate required fields
    // Validate path writability
    // Return error message or nullptr
}
```

## Reference Implementation

Refer to the FRP session plugin complete implementation: `wt_session_frps_plugin.cpp`

## FAQ

### Q: Why must C ABI be used?

A: C ABI guarantees binary compatibility. Using C++ virtual functions or STL types would cause incompatibility between different compilers or runtime versions.

### Q: When do strings become invalid?

A: Strings in `WtSessionSessionView` become invalid after the next call to `wtSession_GetSession*` or any session-modifying operation. Callers should immediately copy data they need to retain.

### Q: How to handle large files?

A: Use streaming read/write to avoid loading the entire file into memory at once.

### Q: Which platforms are supported?

A: Plugins theoretically support all platforms where WoTerm runs (Windows, Linux, macOS), as long as they are built with the corresponding platform compiler.

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | Initial | Basic session management API |
