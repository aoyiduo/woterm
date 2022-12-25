cmake_minimum_required(VERSION 3.1)

project(woterm LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(CMAKE_EXE_LINKER_FLAGS /NODEFAULTLIB:"LIBCMT.lib;LIBCMT.lib")
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

find_package(Qt5 COMPONENTS Core Gui Widgets SerialPort LinguistTools REQUIRED)

message(STATUS "root This is BINARY dir " ${PROJECT_BINARY_DIR})
message(STATUS "root This is SOURCE dir " ${PROJECT_SOURCE_DIR})

set(SOURCE_FILES
    main.cpp
    qwoapplication.cpp
    qkxbuttonassist.cpp
    qwomenubutton.cpp
    qwoaboutdialog.cpp
    qwoadmindialog.cpp
    qwocommandlineinput.cpp
    qwoevent.cpp
    qwohostinfoedit.cpp
    qwohostinfolist.cpp
    qwohosttreemodel.cpp
    qwohostlistmodel.cpp
    qwohostsimplelist.cpp
    qwosortfilterproxymodel.cpp
    qwoidentifydialog.cpp
    qwoidentify.cpp
    qwoidentifycreatedialog.cpp
    qwoidentifypublickeydialog.cpp
    qwolinenoise.cpp
    qwolistview.cpp
    qwomainwindow.cpp
    qwopasswordinput.cpp
    qworenamedialog.cpp
    qwosftpnamedialog.cpp
    qwosessionlist.cpp
    qwosessionmanage.cpp
    qwosessionproperty.cpp
    qwosessionmoreproperty.cpp
    qwosetting.cpp
    qwosettingdialog.cpp
    qwoshower.cpp
    qwoshowerwidget.cpp
    qwosshconf.cpp
    qwotermmask.cpp
    qwotermstyle.cpp
    qwotermwidget.cpp
    qwotermwidgetimpl.cpp
    qwobasetoolform.cpp
    qwosshtermwidget.cpp
    qwosshtermwidgetimpl.cpp
    qwotelnettermwidget.cpp
    qwotelnettermwidgetimpl.cpp
    qwoptytermwidget.cpp
    qwoptytermwidgetimpl.cpp
    qworlogin.cpp
    qworlogintermwidget.cpp
    qworlogintermwidgetimpl.cpp
    qwotreeview.cpp
    qwosftptask.cpp
    qwosftpwidget.cpp
    qwosftpwidgetimpl.cpp
    qwosftplocalmodel.cpp
    qwosftpremotemodel.cpp
    qwosftpqueuemodel.cpp
    qwosftptransferwidget.cpp
    qwoserialinput.cpp
    qwoserialwidgetimpl.cpp
    qwordpwidget.cpp
    qwordpwidgetimpl.cpp
    qwovncwidget.cpp
    qwovncwidgetimpl.cpp
    qwovncplaywidget.cpp
    qwovncftpwidget.cpp
    qwovnctoolform.cpp
    qwovncftprequest.cpp
    qwofloatwindow.cpp
    qwoloadingwidget.cpp
    qwoshortcutmodel.cpp
    qwoshortcutdelegate.cpp
    qwodbsftpuploadsync.cpp
    qwodbsftpdownsync.cpp
    qwodbmergemodel.cpp
    qwodbidentitymergemodel.cpp
    qwodbgroupmergemodel.cpp
    qwodbservermergemodel.cpp
    qwodbsftpdownlistdialog.cpp
    qwodbbackupdialog.cpp
    qwodbsftpdetaildialog.cpp
    qwodbrestoredialog.cpp
    qwodbpowerrestoredialog.cpp
    qwogroupinputdialog.cpp
    qwotelnet.cpp
    qwoutils.cpp
    qwowidget.cpp
    qwolineedit.cpp
    qwossh.cpp
    qwopty.cpp
    qmodem.cpp
)

set(HEADER_FILES
    qwoapplication.h
    qkxbuttonassist.h
    qwomenubutton.h
    qwoaboutdialog.h
    qwoadmindialog.h
    qwocommandlineinput.h
    qwoevent.h
    qwoglobal.h
    qwohostinfoedit.h
    qwohostinfolist.h
    qwohostlistmodel.h
    qwohosttreemodel.h
    qwohostsimplelist.h
    qwosortfilterproxymodel.h
    qwoidentifydialog.h
    qwoidentify.h
    qwoidentifycreatedialog.h
    qwoidentifypublickeydialog.h
    qwolinenoise.h
    qwolistview.h
    qwomainwindow.h
    qwopasswordinput.h
    qworenamedialog.h
    qwosftpnamedialog.h
    qwosessionlist.h
    qwosessionmanage.h
    qwosessionproperty.h
    qwosessionmoreproperty.h
    qwosetting.h
    qwosettingdialog.h
    qwoshower.h
    qwoshowerwidget.h
    qwosshconf.h
    qwotermmask.h
    qwotermstyle.h
    qwotermwidget.h
    qwotermwidgetimpl.h
    qwobasetoolform.h
    qwosshtermwidget.h
    qwosshtermwidgetimpl.h
    qwotelnettermwidget.h
    qwotelnettermwidgetimpl.h
    qwoptytermwidget.h
    qwoptytermwidgetimpl.h
    qworlogin.h
    qworlogintermwidget.h
    qworlogintermwidgetimpl.h
    qwotreeview.h
    qwosftptask.h
    qwosftpwidget.h
    qwosftpwidgetimpl.h
    qwosftplocalmodel.h
    qwosftpremotemodel.h
    qwosftpqueuemodel.h
    qwosftptransferwidget.h
    qwoserialinput.h
    qwoserialwidgetimpl.h
    qwordpwidget.h
    qwordpwidgetimpl.h
    qwovncwidget.h
    qwovncwidgetimpl.h
    qwovncplaywidget.h
    qwovncftpwidget.h
    qwovnctoolform.h
    qwovncftprequest.h
    qwofloatwindow.h
    qwoloadingwidget.h
    qwoshortcutmodel.h
    qwoshortcutdelegate.h
    qwodbsftpuploadsync.h
    qwodbsftpdownsync.h
    qwodbmergemodel.h
    qwodbidentitymergemodel.h
    qwodbgroupmergemodel.h
    qwodbservermergemodel.h
    qwodbsftpdownlistdialog.h
    qwodbbackupdialog.h
    qwodbsftpdetaildialog.h
    qwodbrestoredialog.h
    qwodbpowerrestoredialog.h
    qwogroupinputdialog.h
    qwotelnet.h
    qwoutils.h
    qwowidget.h
    qwolineedit.h
    qwossh.h
    qwopty.h
    qmodem.h
    version.h
)

set(OTHER_FILES
    qwoterm.qrc
    qwoaboutdialog.ui
    qwomenubutton.ui
    qwoadmindialog.ui
    qwocommandlineinput.ui
    qwohostinfo.ui
    qwohostinfolist.ui
    qwohostlist.ui
    qwoidentifydialog.ui
    qwoidentifycreatedialog.ui
    qwoidentifypublickeydialog.ui
    qwomainwindow.ui
    qwobasetoolform.ui
    qwopasswordinput.ui
    qworenamedialog.ui
    qwosftpnamedialog.ui
    qwosessionmanage.ui
    qwosessionproperty.ui
    qwosessionmoreproperty.ui
    qwosettingdialog.ui
    qwotermmask.ui
    qwoserialinput.ui
    qwosftpwidget.ui
    qwosftptransferwidget.ui
    qwovnctoolform.ui
    qwovncftpwidget.ui
    qwodbbackupdialog.ui
    qwodbsftpdetaildialog.ui
    qwodbrestoredialog.ui
    qwodbpowerrestoredialog.ui
    qwodbsftpdownlistdialog.ui
    qwogroupinputdialog.ui
)

include_directories(${KXVER_ROOT_DIR}
    ${KXTERM_ROOT_DIR}
    ${KXVNC_ROOT_DIR}
    ${MODEM_ROOT_DIR}
    ${KXFTP_ROOT_DIR}
    ${KXUTIL_ROOT_DIR}
    ${LIBQSSH_ROOT_DIR}/include
    ${OPENSSL_ROOT_DIR}/include
    ${LIBTELNET_ROOT_DIR}/include
    ${ZLIB_ROOT_DIR}/include
    ${FREERDP_ROOT_DIR}/include
    ${LIBSSH_ROOT_DIR}/include
    ${LIBYUV_ROOT_DIR}/include
    ${LIBMODEM_ROOT_DIR}/include
    ${LIBSQLITE_ROOT_DIR}/include
    ${LIBWINPTY_ROOT_DIR}/include
    ${LIBPTYQT_ROOT_DIR}/include
)


link_directories(${OPENSSL_ROOT_DIR}/lib
    ${ZLIB_ROOT_DIR}/lib
    ${LIBSSH_ROOT_DIR}/lib
    ${LIBQSSH_ROOT_DIR}/lib
    ${FREERDP_ROOT_DIR}/lib
    ${LIBYUV_ROOT_DIR}/lib
    ${LIBTELNET_ROOT_DIR}/lib
    ${LIBMODEM_ROOT_DIR}/lib
    ${LIBSQLITE_ROOT_DIR}/lib
    ${LIBWINPTY_ROOT_DIR}/lib
    ${LIBPTYQT_ROOT_DIR}/lib
)

link_libraries(kxver kxterm kxutil kxftp kxvnc)
if(WIN32)
    message("window system.")
    link_libraries(ws2_32)
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        link_libraries(winpty ptyqtd SQLiteCppd sqlite3d sshd qsshd rdpd zmodemd libtelnetd)
    else()
        link_libraries(winpty ptyqt SQLiteCpp sqlite3 ssh qssh rdp zmodem libtelnet)
    endif()

    add_library(openssl_crypto STATIC IMPORTED)
    set_target_properties(openssl_crypto PROPERTIES IMPORTED_LOCATION
        ${OPENSSL_ROOT_DIR}/lib/libcrypto.lib)
    link_libraries(openssl_crypto)
    set(BUILD_VERSION_MAJOR 0)
    set(BUILD_VERSION_MINOR 0)
    set(BUILD_VERSION_PATCH 0)
    file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/version.h BUILD_VERSION)
    message("buildVersion:${BUILD_VERSION}")
    string(REGEX MATCHALL "WOTERM_VERSION.*\\(\"(.*)\"\\)" BUILD_VERSION ${BUILD_VERSION})
    set(BUILD_VERSION ${CMAKE_MATCH_1})
    message("versionContent:${BUILD_VERSION}")
    string(REPLACE "." ";" VERSION_LIST ${BUILD_VERSION})
    message("versionList:${VERSION_LIST}")
    list(LENGTH VERSION_LIST len)
    list(GET VERSION_LIST 0 BUILD_VERSION_MAJOR)
    list(GET VERSION_LIST 1 BUILD_VERSION_MINOR)
    if( ${len} EQUAL 3)
        list(GET VERSION_LIST 2 BUILD_VERSION_PATCH)
    endif()
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/versioninfo.rc.in versioninfo.rc @ONLY)
    add_executable(${PROJECT_NAME} WIN32 ${SOURCE_FILES} ${HEADER_FILES} ${OTHER_FILES} ${QM_FILES})
    target_sources(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/versioninfo.rc")
elseif(APPLE)
    message("apple system.")
    link_libraries(ptyqt SQLiteCpp sqlite3 crypto ssh qssh rdp zmodem libtelnet)

    add_library(openssl_crypto STATIC IMPORTED)
    set_target_properties(openssl_crypto PROPERTIES IMPORTED_LOCATION
        ${OPENSSL_ROOT_DIR}/lib/libcrypto.a)
    link_libraries(openssl_crypto)

    # NOTE: Don't include the path in MACOSX_BUNDLE_ICON_FILE -- this is
    # the property added to Info.plist    

    # And this part tells CMake where to find and install the file itself
    set(PROGRAM_VERSION "1.0" CACHE STRINGS "you can modify this value by command : cmake -DPROGRAM_VERSION=\"xx\"")
    set(ICNS_NAME  "woterm.icns")
    set(myApp_ICON ${CMAKE_CURRENT_SOURCE_DIR}/resource/${ICNS_NAME})
    set_source_files_properties(${myApp_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${myApp_ICON} ${SOURCE_FILES} ${HEADER_FILES} ${OTHER_FILES})
    set_target_properties(${PROJECT_NAME} PROPERTIES
        BUNDLE True
        MACOSX_BUNDLE_INFO_STRING "WoTerm Remote Assistant"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.aoyiduo.woterm"
        MACOSX_BUNDLE_ICON_FILE "${ICNS_NAME}"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROGRAM_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROGRAM_VERSION}
        MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}"
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/resource/template.plist.in
        LINK_FLAGS "-sectcreate __CGPreLoginApp __cgpreloginapp /dev/null"
    )
else()
    message("linux system.")
    link_libraries(dl ptyqt SQLiteCpp sqlite3 crypto ssh qssh rdp zmodem libtelnet SQLiteCpp)
    add_library(openssl_crypto STATIC IMPORTED)
    set_target_properties(openssl_crypto PROPERTIES IMPORTED_LOCATION
        ${OPENSSL_ROOT_DIR}/lib/libcrypto.so.1.1)
    link_libraries(openssl_crypto)

    add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES} ${OTHER_FILES} ${LangSrcs})
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::SerialPort)


