cmake_minimum_required(VERSION 3.1)

project(kxterm LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DQTERM_LIBRARY)

find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)
set(SOURCE_FILES
    qkxhistory.cpp
    qkxkeytranslator.cpp
    qkxscreen.cpp
    qkxtermitem.cpp
    qkxutils.cpp
    qkxview.cpp    
    qkxtermwidget.cpp
    qkxcolorschema.cpp
    qkxsearch.cpp
    qvte.cpp
    qvteimpl.cpp
    wcwidth.cpp
    qkxechoinput.cpp
)

set(HAEDER_FILES
    qkxhistory.h
    qkxkeytranslator.h
    qkxscreen.h
    qkxtermitem.h
    qkxutils.h
    qkxview.h
    qkxtermwidget.h
    qkxcolorschema.h
    qkxsearch.h
    qvte.h
    qvtedef.h
    qvteimpl.h
    wcwidth.h
    qkxechoinput.h
    qkxterm_share.h
)

set(OTHER_FILES
    qkxterm.qrc
    qkxsearch.ui
)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${HAEDER_FILES} ${OTHER_FILES})

target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Gui Qt5::Widgets)

install(TARGETS ${PROJECT_NAME}
       RUNTIME DESTINATION ${RUNTIME_OUTPUT_DIRECTORY}
       LIBRARY DESTINATION ${LIBRARY_OUTPUT_DIRECTORY}
       ARCHIVE DESTINATION ${ARCHIVE_OUTPUT_DIRECTORY}
)
