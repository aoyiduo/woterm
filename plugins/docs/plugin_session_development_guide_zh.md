# WoTerm 会话插件开发指南

## 概述

WoTerm 通过 C ABI 接口支持会话管理插件。插件是一个独立的动态链接库（Windows 上为 `.dll`，Linux 上为 `.so`，macOS 上为 `.dylib`），实现预定义的 C API 函数集，供主程序调用。

本文档基于 `wt_session_plugin.h` 头文件编写，旨在帮助开发者快速理解并实现自己的会话管理插件。

## 核心设计原则

### ABI 稳定性保证

插件的公共接口必须是纯 C 类型：
- 使用固定宽度整数类型（`int32_t`, `int64_t`）
- 使用 `const char*` 传递字符串
- 使用不透明句柄指针（`WtSessionHandle`）
- 不使用 C++ STL 类型、虚函数表或异常

这意味着插件可以使用任意编译器（MSVC、GCC、Clang）或 C++ 运行时版本编译，而不会破坏使用旧版头文件编译的调用方。

### 字符串所有权规则

- 返回的 `const char*` 字符串由插件实例拥有，在下次修改相同会话的调用或 `wtSession_Destroy()` 调用之前保持有效
- 调用方**不得**释放插件返回的任何指针
- `WtSessionSessionView` 是轻量级只读快照，调用方拥有该结构体但**不拥有**其中的 `char*` 字段

### 线程安全

同一句柄上的任何函数调用都**不是**线程安全的。跨线程共享句柄时需要外部加锁。

## API 版本控制

`WT_SESSION_API_VERSION` 编码为 `(major << 16) | minor`。当在 vtable 末尾追加新函数（向后兼容）或需要破坏性变更时，版本号会增加。

调用方在运行时通过 `wtSession_GetApiVersion()` 检查版本，然后再使用新版本的功能。

## 必须实现的 C API 函数

所有函数都必须使用 `extern "C"` 声明，并使用 `WT_SESSION_API` 和 `WT_SESSION_CALL` 宏。

### 1. API 版本和特性查询

```c
WT_SESSION_API uint32_t WT_SESSION_CALL wtSession_GetApiVersion(void);
WT_SESSION_API uint32_t WT_SESSION_CALL wtSession_GetFeature(void);
```

**实现要点：**
- `wtSession_GetApiVersion()` 返回 `WT_SESSION_API_VERSION`
- `wtSession_GetFeature()` 返回特性位掩码，可选值：
  - `WT_SESSION_FEATURE_NONE` (0) - 无特殊特性
  - `WT_SESSION_FEATURE_SHOW_PASSWORD` (1) - 允许显示密码
  - `WT_SESSION_FEATURE_UPDATE_INFO` (2) - 支持更新会话信息
  - `WT_SESSION_FEATURE_RUN_AS_TUNNEL` (4) - 支持作为隧道运行

### 2. 生命周期管理

```c
WT_SESSION_API WtSessionHandle WT_SESSION_CALL wtSession_Create(void);
WT_SESSION_API int32_t WT_SESSION_CALL wtSession_Init(WtSessionHandle handle, const char *cfgJson);
WT_SESSION_API void WT_SESSION_CALL wtSession_Destroy(WtSessionHandle handle);
```

**实现要点：**
- `wtSession_Create()` 分配插件上下文，返回不透明句柄
- `wtSession_Init()` 使用 JSON 配置初始化上下文，加载本地会话列表
- `wtSession_Destroy()` 释放所有资源

**配置 JSON 参数：**

插件应通过 `wtPlugin_GetInitilizeParameterTemplate()` 返回支持的配置参数模板。FRP 插件示例：

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

### 3. 会话同步

```c
WT_SESSION_API int32_t WT_SESSION_CALL wtSession_Sync(WtSessionHandle handle,
                                                      WtSessionSyncCallback callback,
                                                      void *user_data);
```

**实现要点：**
- 从远程源拉取会话数据
- 与本地列表进行差异比较
- 添加远程存在但本地不存在的会话
- 更新两者都存在的会话的端口和在线状态
- 删除本地存在但远程不存在的会话
- 使用合并安全策略保存到磁盘

**同步回调：**

```c
typedef struct WtSessionSyncResult {
    int32_t     status;             // WT_SESSION_OK 或错误码
    const char *error_msg;          // 错误信息，status==OK 时为 NULL
    int32_t     added_count;        // 新增会话数量
    int32_t     updated_count;      // 更新会话数量
    int32_t     removed_count;      // 删除会话数量
    const char *const *added_names; // 新增会话名称数组
    const char *const *updated_names; // 更新会话名称数组
    const char *const *removed_names; // 删除会话名称数组
} WtSessionSyncResult;

typedef void (WT_SESSION_CALL *WtSessionSyncCallback)(
    const WtSessionSyncResult *result,
    void *user_data
);
```

### 4. 会话查询

```c
WT_SESSION_API int32_t WT_SESSION_CALL wtSession_GetSessionCount(WtSessionHandle handle);
WT_SESSION_API int32_t WT_SESSION_CALL wtSession_GetSessionAt(WtSessionHandle handle,
                                                              int32_t index,
                                                              WtSessionSessionView *view);
WT_SESSION_API int32_t WT_SESSION_CALL wtSession_GetSession(WtSessionHandle handle,
                                                           const char *name,
                                                           WtSessionSessionView *view);
```

**实现要点：**
- `wtSession_GetSessionCount()` 返回本地列表中的会话数量
- `wtSession_GetSessionAt()` 按索引获取会话快照
- `wtSession_GetSession()` 按名称获取会话快照

**会话视图结构体：**

```c
typedef struct WtSessionSessionView {
    uint32_t    struct_size;        // 必须设置为 sizeof(WtSessionSessionView)
    int64_t     id;                 // 会话 ID
    int32_t     type;               // 会话类型（WT_HOST_* 常量）
    const char *name;               // 全局唯一名称
    const char *host;               // 主机地址
    int32_t     port;               // 端口号
    const char *login_username;     // 登录用户名
    const char *login_password;     // 登录密码
    const char *login_identity_file; // SSH 身份文件路径
    const char *proxy_jump;         // 跳转主机 ID（逗号分隔）
    const char *memo;               // 备注信息
    const char *path;               // 父路径节点
    int32_t     order_num;          // 排序序号
    int32_t     online;             // 在线状态（1=在线，0=离线）
} WtSessionSessionView;
```

### 5. 会话更新

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

**实现要点：**
- `field` 参数是位掩码，指定要更新的字段
- 只修改 `field` 中指定的字段，其他字段保持不变
- 更新后使用合并安全策略保存到磁盘

**字段位掩码常量：**

| 常量 | 值 | 说明 |
|------|-----|------|
| `WT_UPDATE_FIELD_NAME` | 1 | 更新会话名称 |
| `WT_UPDATE_FIELD_USERNAME` | 2 | 更新用户名 |
| `WT_UPDATE_FIELD_PASSWORD` | 4 | 更新密码 |
| `WT_UPDATE_FIELD_IDENTFILE` | 8 | 更新身份文件 |
| `WT_UPDATE_FIELD_PATH` | 16 | 更新父路径 |
| `WT_UPDATE_FIELD_PROXYJUMP` | 32 | 更新跳转主机 |
| `WT_UPDATE_FIELD_MEMO` | 64 | 更新备注 |
| `WT_UPDATE_FIELD_TYPE` | 128 | 更新会话类型 |

### 6. 会话删除

```c
WT_SESSION_API int32_t WT_SESSION_CALL wtSession_RemoveByIds(WtSessionHandle handle,
                                                             const int64_t *ids,
                                                             int cnt);
```

**实现要点：**
- 按 ID 批量删除会话
- 删除后使用合并安全策略保存到磁盘

### 7. 持久化操作

```c
WT_SESSION_API int32_t WT_SESSION_CALL wtSession_Load(WtSessionHandle handle);
WT_SESSION_API int32_t WT_SESSION_CALL wtSession_Save(WtSessionHandle handle);
```

**实现要点：**
- `wtSession_Load()` 从磁盘重新加载会话列表，丢弃内存状态
- `wtSession_Save()` 使用合并安全策略保存内存列表

**合并安全保存策略：**
1. 读取当前磁盘内容
2. 将内存会话覆盖到磁盘内容上（内存版本优先）
3. 保留仅存在于磁盘中的会话（其他客户端添加的）
4. 将合并结果写回磁盘

### 8. 诊断信息

```c
WT_SESSION_API const char * WT_SESSION_CALL wtSession_GetLastError(WtSessionHandle handle);
```

**实现要点：**
- 返回最近一次错误的描述信息
- 返回的指针在下次调用前保持有效

### 9. 插件信息查询

```c
WT_SESSION_API int32_t WT_SESSION_CALL wtPlugin_GetInfo(WtPluginInfo *info);
WT_SESSION_API const char* WT_SESSION_CALL wtPlugin_GetInitilizeParameterTemplate(void);
WT_SESSION_API const char* WT_SESSION_CALL wtPlugin_CheckInitializeParameter(const char* json);
```

**实现要点：**
- `wtPlugin_GetInfo()` 返回插件元数据
- `wtPlugin_GetInitilizeParameterTemplate()` 返回配置参数模板的 JSON 字符串
- `wtPlugin_CheckInitializeParameter()` 验证配置参数的有效性

**插件信息结构体：**

```c
typedef struct WtPluginInfo {
    uint32_t    struct_size;        // 必须设置为 sizeof(WtPluginInfo)
    int32_t     plugin_type;        // 插件类型（WT_PLUGIN_TYPE_SESSION）
    const char *plugin_name;        // 插件名称
    const char *plugin_version;     // 插件版本字符串
    const char *plugin_description; // 插件描述
    const char *plugin_vendor;      // 供应商/作者名称
    const char *plugin_id;          // 唯一标识符（反向域名格式）
    uint32_t    api_version;        // API 版本
} WtPluginInfo;
```

## 会话类型常量

| 常量 | 值 | 说明 |
|------|-----|------|
| `WT_HOST_INVALID` | 0 | 无效类型 |
| `WT_HOST_SSH_WITH_SFTP` | 1 | SSH 带 SFTP |
| `WT_HOST_SFTP_ONLY` | 2 | 仅 SFTP |
| `WT_HOST_TELNET` | 4 | Telnet |
| `WT_HOST_RLOGIN` | 5 | RLogin |
| `WT_HOST_RDP` | 7 | RDP |
| `WT_HOST_VNC` | 8 | VNC |
| `WT_HOST_FTP` | 9 | FTP |

## 返回码常量

| 常量 | 值 | 说明 |
|------|-----|------|
| `WT_SESSION_OK` | 0 | 成功 |
| `WT_SESSION_ERR_PARAM` | -1 | 参数为空或无效 |
| `WT_SESSION_ERR_NOT_FOUND` | -2 | 会话名称不在本地列表中 |
| `WT_SESSION_ERR_IO` | -3 | 文件读写失败 |
| `WT_SESSION_ERR_NET` | -4 | HTTP/网络失败 |
| `WT_SESSION_ERR_PARSE` | -5 | JSON 解析错误 |
| `WT_SESSION_ERR_STATE` | -6 | 未调用 wtSession_Init() |

## 构建命令

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

## 插件开发步骤

### 步骤 1: 创建插件文件

创建 `wt_session_myplugin.cpp`，包含必要的头文件：

```cpp
#include "wt_session_plugin.h"
#include <nlohmann/json.hpp>
#include <httplib.h>
#include <string>
#include <vector>
#include <unordered_map>

using json = nlohmann::json;
```

### 步骤 2: 定义内部数据结构

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
    // ... 其他配置字段 ...
    bool initialized = false;
    std::vector<SessionEntry> sessions;
    std::string last_error;
    SessionEntry view_cache;
    // ... sync result caches ...
};
```

### 步骤 3: 实现生命周期函数

```cpp
extern "C" {

WT_SESSION_API WtSessionHandle WT_SESSION_CALL
wtSession_Create(void) {
    try { return new WtSessionContext(); }
    catch (...) { return nullptr; }
}

WT_SESSION_API int32_t WT_SESSION_CALL
wtSession_Init(WtSessionHandle handle, const char* cfgJson) {
    // 解析 JSON 配置
    // 加载本地会话文件
    // 初始化上下文
    return WT_SESSION_OK;
}

WT_SESSION_API void WT_SESSION_CALL
wtSession_Destroy(WtSessionHandle handle) {
    delete handle;
}

// ... 实现其他 API 函数 ...

} // extern "C"
```

### 步骤 4: 实现会话同步逻辑

```cpp
WT_SESSION_API int32_t WT_SESSION_CALL
wtSession_Sync(WtSessionHandle handle,
               WtSessionSyncCallback callback,
               void* user_data) {
    // 1. 从远程源获取会话数据
    // 2. 与本地列表进行差异比较
    // 3. 添加/更新/删除会话
    // 4. 保存到磁盘
    // 5. 调用回调返回结果
    return WT_SESSION_OK;
}
```

### 步骤 5: 实现查询和更新函数

```cpp
WT_SESSION_API int32_t WT_SESSION_CALL
wtSession_GetSessionAt(WtSessionHandle handle,
                       int32_t index,
                       WtSessionSessionView* view) {
    // 验证参数
    // 填充 view 结构体
    return WT_SESSION_OK;
}

WT_SESSION_API int32_t WT_SESSION_CALL
wtSession_UpdateInfo(WtSessionHandle handle, int64_t id, ...) {
    // 根据 field 位掩码更新字段
    // 保存到磁盘
    return WT_SESSION_OK;
}
```

### 步骤 6: 实现插件信息函数

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
        // ... 添加其他参数 ...
        my = out.dump();
    }
    return my.data();
}
```

## 最佳实践

### 1. 字符串生命周期管理

使用内部缓存（如 `view_cache`）确保返回的 `const char*` 在单次 API 调用期间保持有效：

```cpp
static void fillView(WtSessionSessionView& v, const SessionEntry& e) {
    v.id = e.id;
    v.name = e.name.c_str();
    // ... 其他字段 ...
}

WT_SESSION_API int32_t WT_SESSION_CALL
wtSession_GetSessionAt(WtSessionHandle handle, int32_t index, WtSessionSessionView* view) {
    handle->view_cache = handle->sessions[index];
    fillView(*view, handle->view_cache);
    return WT_SESSION_OK;
}
```

### 2. 合并安全保存

实现合并安全策略，避免覆盖其他客户端的更改：

```cpp
static bool saveToDisk(const std::string& path,
                       const std::vector<SessionEntry>& mem, bool merge,
                       std::string& errMsg) {
    // 读取磁盘内容
    // 合并内存和磁盘会话
    // 排序并写入
}
```

### 3. 错误处理

使用 `setError()` 记录详细错误信息：

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

### 4. 输入验证

在 `wtPlugin_CheckInitializeParameter()` 中验证配置参数：

```cpp
WT_SESSION_API const char* WT_SESSION_CALL
wtPlugin_CheckInitializeParameter(const char* cfgJson) {
    static std::string errMessage;
    // 验证必填字段
    // 验证路径可写性
    // 返回错误信息或 nullptr
}
```

## 参考实现

参考 FRP 会话插件的完整实现：`wt_session_frps_plugin.cpp`

## 常见问题

### Q: 为什么必须使用 C ABI？

A: C ABI 保证了二进制兼容性。使用 C++ 虚函数或 STL 类型会导致不同编译器或运行时版本之间的不兼容。

### Q: 字符串什么时候失效？

A: `WtSessionSessionView` 中的字符串在下次调用 `wtSession_GetSession*` 或任何修改会话的操作后失效。调用方应立即复制需要保留的数据。

### Q: 如何处理大文件？

A: 使用流式读写，避免一次性加载整个文件到内存。

### Q: 插件支持哪些平台？

A: 插件理论上支持所有 WoTerm 支持的平台（Windows、Linux、macOS），只要使用相应平台的编译器构建即可。

## 版本历史

| 版本 | 日期 | 变更 |
|------|------|------|
| 1.0 | 初始版本 | 基础会话管理 API |
