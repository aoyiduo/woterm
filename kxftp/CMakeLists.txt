cmake_minimum_required(VERSION 3.1)

project(kxftp LANGUAGES CXX)

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

add_definitions(-DKXFTP_LIBRARY)

find_package(Qt5 COMPONENTS Core Network Widgets)

set(SOURCE_FILES
    qkxftpserver.cpp
    qkxftpserverprivate.cpp
    qkxftpclient.cpp
    qkxftpresponse.cpp
    qkxftprequest.cpp
    qkxftpupload.cpp
    qkxftpdownload.cpp
    qkxftptransferwidget.cpp
    qkxftptransfermodel.cpp
    qkxftpremotemodel.cpp
    qkxftplocalmodel.cpp
)

set(HEADER_FILES
    qkxftp_share.h
    qkxftp_private.h
    qkxftpserver.h
    qkxftpserverprivate.h
    qkxftpclient.h
    qkxftpresponse.h
    qkxftprequest.h
    qkxftpupload.h
    qkxftpdownload.h
    qkxftptransferwidget.h
    qkxftptransfermodel.h
    qkxftpremotemodel.h
    qkxftplocalmodel.h
)

set(OTHER_FILES
    qkxftp.qrc
)

include_directories(${SERVICE_ROOT_DIR}
    ${KXUTIL_ROOT_DIR}
)
link_libraries(kxutil)

if(WIN32)
    link_libraries(ws2_32)
elseif(APPLE)
    message("APPLE Here")
else()
    message("other")
endif()

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES} ${OTHER_FILES})
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Network Qt5::Widgets)


