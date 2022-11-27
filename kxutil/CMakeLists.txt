cmake_minimum_required(VERSION 3.1)

project(kxutil LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
else()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

add_definitions(-DKXUTIL_LIBRARY)

find_package(Qt5 COMPONENTS Core Network Gui Widgets REQUIRED)

set(SOURCE_FILES
    rc4.cpp
    qkxzip.cpp
    qkxsetting.cpp
    qkxhttpclient.cpp
    qkxrc4crypt.cpp
    qkxsharememory.cpp
    qkxlocalpeer.cpp
    qkxmacaddress.cpp
    qkxlengthbodypacket.cpp
    qkxkeepwakeup.cpp
    qkxprocesslaunch.cpp
    qkxmessagebox.cpp
)

set(HEADER_FILES
    rc4.h
    qkxzip.h
    qkxsetting.h
    qkxhttpclient.h
    qkxrc4crypt.h
    qkxutil_private.h
    qkxutil_share.h
    qkxsharememory.h
    qkxlocalpeer.h
    qkxmacaddress.h
    qkxwallpaper.h
    qkxlengthbodypacket.h
    qkxkeepwakeup.h
    qkxprocesslaunch.h
    qkxmessagebox.h
)

if(WIN32)
    set(SOURCE_FILES ${SOURCE_FILES}
        qkxwallpaper_winx.cpp
    )
elseif(APPLE)
    message("APPLE Here")
    find_library(corefoundation_lib CoreFoundation REQUIRED)
    find_library(cocoa_lib Cocoa REQUIRED)
    find_library(coremedia_lib CoreMedia REQUIRED)
    find_library(avfoundation_lib AVFoundation REQUIRED)
    find_library(coregraphics_lib CoreGraphics REQUIRED)
    find_library(corevideo_lib CoreVideo REQUIRED)
    find_library(iokit_lib IOKit REQUIRED)
    find_library(appkit_lib APPKit REQUIRED)
    find_library(iosurface_lib IOSurface REQUIRED)

    set(SOURCE_FILES ${SOURCE_FILES}
        qkxwallpaper_unix.cpp
        qkxmactimer.mm
    )

    set(HEADER_FILES ${HEADER_FILES}
        qkxmactimer.h
    )

    link_libraries(
        ${corefoundation_lib}
        ${cocoa_lib}
        ${coremedia_lib}
        ${avfoundation_lib}
        ${coregraphics_lib}
        ${corevideo_lib}
        ${iokit_lib}
        ${appkit_lib}
        ${iosurface_lib}
    )
else()
    set(SOURCE_FILES ${SOURCE_FILES}
        qkxwallpaper_unix.cpp
    )
    #set(SOURCE_FILES ${SOURCE_FILES} qtservice_unix.cpp qtunixserversocket.cpp qtunixsocket.cpp)
    #set(HEADER_FILES qtunixserversocket.h qtunixsocket.h)
endif()

include_directories(${ZLIB_ROOT_DIR}/include)

link_directories(${ZLIB_ROOT_DIR}/lib)

if(WIN32)
    link_libraries(ws2_32)
    add_library(libz_main STATIC IMPORTED)
    set_target_properties(libz_main PROPERTIES IMPORTED_LOCATION
        ${ZLIB_ROOT_DIR}/lib/zlibstatic.lib)
    link_libraries(libz_main)
elseif(APPLE)
    message("APPLE Here")
    add_library(libz_main STATIC IMPORTED)
    set_target_properties(libz_main PROPERTIES IMPORTED_LOCATION
        ${ZLIB_ROOT_DIR}/lib/libz.a)
    link_libraries(libz_main)
else()
    message("other")
    add_library(libz_main STATIC IMPORTED)
    set_target_properties(libz_main PROPERTIES IMPORTED_LOCATION
        ${ZLIB_ROOT_DIR}/lib/libz.a)
    link_libraries(libz_main)
endif()

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Network Qt5::Gui Qt5::Widgets)


