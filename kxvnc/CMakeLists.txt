cmake_minimum_required(VERSION 3.1)

project(kxvnc LANGUAGES CXX)

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

add_definitions(-DQKXVNC_LIBRARY)

find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

message("ZLIB_ROOT_DIR:${ZLIB_ROOT_DIR}")
message("OPENSSL_ROOT_DIR:${OPENSSL_ROOT_DIR}")

if (NOT DEFINED ZLIB_ROOT_DIR)
    if(WIN32)
        set(ZLIB_ROOT_DIR ${PROJECT_SOURCE_DIR}/../third/zlib/winx)
    else()
        set(ZLIB_ROOT_DIR ${PROJECT_SOURCE_DIR}/../third/zlib/unix)
    endif()
endif()

set(SOURCE_FILES
    d3des.cpp
    qkxbuffer.cpp
    qkxsocket.cpp
    qkxvnc.cpp
    qkxvncwidget.cpp
    qkxutils.cpp
    qkxzip.cpp
    qkxh264decoder.cpp
    qkxjpegdecoder.cpp
    qkxaudioplayer.cpp
    qkxopusdecoder.cpp
    qkxresample.cpp
    resample/filterkit.cpp
    resample/resample.cpp
    resample/resamplesubs.cpp
)

set(HEADER_FILES
    qkxvnc_share.h
    keysymdef.h
    d3des.h
    qkxreader.h
    qkxbuffer.h
    qkxsocket.h
    qkxvnc.h
    qkxvncwidget.h
    qkxutils.h
    qkxzip.h
    qkxh264decoder.h
    qkxjpegdecoder.h
    qkxaudioplayer.h
    qkxopusdecoder.h
    qkxresample.h
    resample/filterkit.h
    resample/configtemplate.h
    resample/libresample.h
    resample/resample_defs.h
)

include_directories(
    ${LIBYUV_ROOT_DIR}/include
    ${LIBOPENH264_ROOT_DIR}/include
    ${OPENSSL_ROOT_DIR}/include
    ${LIBOPENH264_ROOT_DIR}/include
    ${ZLIB_ROOT_DIR}/include
    ${KXUTIL_ROOT_DIR}
    ${LIBPORTAUDIO_ROOT_DIR}/include
    ${LIBOPUS_ROOT_DIR}/include
    ${LIBJPEG_ROOT_DIR}/include
    ${LIBYUV_ROOT_DIR}/include
)

link_directories(${ZLIB_ROOT_DIR}/lib
    ${OPENSSL_ROOT_DIR}/lib
    ${LIBOPENH264_ROOT_DIR}/lib
    ${LIBPORTAUDIO_ROOT_DIR}/lib
    ${LIBOPUS_ROOT_DIR}/lib
    ${LIBJPEG_ROOT_DIR}/lib
    ${LIBYUV_ROOT_DIR}/lib
)

if(WIN32)
    message("WIN32 Here")
    set(ZLIB_LIBRARY zlibstatic)
    link_libraries(zlibstatic ws2_32 crypt32)

    add_library(portaudio_main STATIC IMPORTED)
    set_target_properties(portaudio_main PROPERTIES IMPORTED_LOCATION
        ${LIBPORTAUDIO_ROOT_DIR}/lib/portaudio.lib)
    link_libraries(portaudio_main)

    add_library(yuv_main STATIC IMPORTED)
    set_target_properties(yuv_main PROPERTIES IMPORTED_LOCATION
        ${LIBYUV_ROOT_DIR}/lib/yuv.lib)
    link_libraries(yuv_main)

    add_library(opus_main STATIC IMPORTED)
    set_target_properties(opus_main PROPERTIES IMPORTED_LOCATION
        ${LIBOPUS_ROOT_DIR}/lib/opus.lib)
    link_libraries(opus_main)

    add_library(openh264_main STATIC IMPORTED)
    set_target_properties(openh264_main PROPERTIES IMPORTED_LOCATION
        ${LIBOPENH264_ROOT_DIR}/lib/openh264.lib)
    link_libraries(openh264_main)

    add_library(jpeg_main STATIC IMPORTED)
    set_target_properties(jpeg_main PROPERTIES IMPORTED_LOCATION
        ${LIBJPEG_ROOT_DIR}/lib/turbojpeg-static.lib)
    link_libraries(jpeg_main)
elseif(APPLE)
    message("APPLE Here")
    set(ZLIB_LIBRARY  libz.a)
    link_libraries(libz.a)

    add_library(portaudio_main STATIC IMPORTED)
    set_target_properties(portaudio_main PROPERTIES IMPORTED_LOCATION
        ${LIBPORTAUDIO_ROOT_DIR}/lib/libportaudio.a)
    link_libraries(portaudio_main)

    add_library(yuv_main STATIC IMPORTED)
    set_target_properties(yuv_main PROPERTIES IMPORTED_LOCATION
        ${LIBYUV_ROOT_DIR}/lib/libyuv.a)
    link_libraries(yuv_main)

    add_library(opus_main STATIC IMPORTED)
    set_target_properties(opus_main PROPERTIES IMPORTED_LOCATION
        ${LIBOPUS_ROOT_DIR}/lib/libopus.a)
    link_libraries(opus_main)

    add_library(openh264_main STATIC IMPORTED)
    set_target_properties(openh264_main PROPERTIES IMPORTED_LOCATION
        ${LIBOPENH264_ROOT_DIR}/lib/libopenh264.a)
    link_libraries(openh264_main pthread)

    add_library(jpeg_main STATIC IMPORTED)
    set_target_properties(jpeg_main PROPERTIES IMPORTED_LOCATION
        ${LIBJPEG_ROOT_DIR}/lib/libturbojpeg.a)
    link_libraries(jpeg_main pthread)

    find_library(COREAUDIO_LIBRARY CoreAudio REQUIRED)
    find_library(AUDIOTOOLBOX_LIBRARY AudioToolbox REQUIRED)
    find_library(AUDIOUNIT_LIBRARY AudioUnit REQUIRED)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation REQUIRED)
    find_library(CORESERVICES_LIBRARY CoreServices REQUIRED)
    link_libraries("${COREAUDIO_LIBRARY}"
       "${AUDIOTOOLBOX_LIBRARY}"
       "${AUDIOUNIT_LIBRARY}"
       "${COREFOUNDATION_LIBRARY}"
       "${CORESERVICES_LIBRARY}"
     )
else()
    message("other")
    set(ZLIB_LIBRARY  libz.a)
    link_libraries(libz.a)

    add_library(portaudio_main STATIC IMPORTED)
    set_target_properties(portaudio_main PROPERTIES IMPORTED_LOCATION
        ${LIBPORTAUDIO_ROOT_DIR}/lib/libportaudio.a)
    link_libraries(portaudio_main)

    add_library(yuv_main STATIC IMPORTED)
    set_target_properties(yuv_main PROPERTIES IMPORTED_LOCATION
        ${LIBYUV_ROOT_DIR}/lib/libyuv.a)
    link_libraries(yuv_main)

    add_library(opus_main STATIC IMPORTED)
    set_target_properties(opus_main PROPERTIES IMPORTED_LOCATION
        ${LIBOPUS_ROOT_DIR}/lib/libopus.a)
    link_libraries(opus_main)

    add_library(openh264_main STATIC IMPORTED)
    set_target_properties(openh264_main PROPERTIES IMPORTED_LOCATION
        ${LIBOPENH264_ROOT_DIR}/lib/libopenh264.a)
    link_libraries(openh264_main pthread)

    add_library(jpeg_main STATIC IMPORTED)
    set_target_properties(jpeg_main PROPERTIES IMPORTED_LOCATION
        ${LIBJPEG_ROOT_DIR}/lib/libturbojpeg.a)
    link_libraries(jpeg_main pthread)
endif()

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${HEADER_FILES} ${OTHER_FILES})
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(${PROJECT_NAME} Qt5::Core  Qt5::Gui Qt5::Widgets)

install(TARGETS ${PROJECT_NAME}
       RUNTIME DESTINATION ${RUNTIME_OUTPUT_DIRECTORY}
       LIBRARY DESTINATION ${LIBRARY_OUTPUT_DIRECTORY}
       ARCHIVE DESTINATION ${ARCHIVE_OUTPUT_DIRECTORY}
)
