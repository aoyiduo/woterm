cmake_minimum_required(VERSION 3.16)
project(wt_session_frps_plugin C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# ---------------------------------------------------------------------------
# Hide all symbols by default on GCC / Clang; only WT_SESSION_API exports.
# On MSVC the __declspec(dllexport/dllimport) macros handle this already.
# ---------------------------------------------------------------------------
if(NOT MSVC)
    set(CMAKE_CXX_VISIBILITY_PRESET hidden)
    set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
endif()

add_definitions(-DWT_SESSION_PLUGIN_BUILDING_DLL)

# ---------------------------------------------------------------------------
# Header-only dependencies
# Place the single-header files here:
#   include/nlohmann/json.hpp   https://github.com/nlohmann/json
#   include/httplib.h           https://github.com/yhirose/cpp-httplib
# ---------------------------------------------------------------------------
include_directories(
    ${PLUGINS_ROOT_DIR}/include
    ${HTTPLIB_ROOT_DIR}/include
    ${NLOHMANN_JSON_ROOT_DIR}/include
)

# ---------------------------------------------------------------------------
# Shared library  (DLL on Windows, .so on Linux, .dylib on macOS)
# ---------------------------------------------------------------------------
add_library(wt_session_frps_plugin SHARED
    wt_session_frps_plugin.cpp
)

if(OpenSSL_FOUND)
    target_compile_definitions(wt_session_frps_plugin PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)
    target_link_libraries(wt_session_frps_plugin PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif()

if(UNIX)
    target_link_libraries(wt_session_frps_plugin PRIVATE pthread)
endif()
