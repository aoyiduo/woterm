cmake_minimum_required(VERSION 3.1)

project(woterm LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:"LIBCMT.lib;LIBCMT.lib")
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

string(TIMESTAMP WOTERM_BUILD_TIME UTC)
add_definitions(-DWOTERM_BUILD_TIME="${WOTERM_BUILD_TIME}")

find_package(Qt5 COMPONENTS Core Gui Widgets Qml Quick QuickControls2 Xml Charts QuickWidgets SerialPort LinguistTools REQUIRED)
find_package(Qt5Core CONFIG REQUIRED Private)
find_package(Qt5Gui CONFIG REQUIRED Private)
if(ANDROID)
    find_package(Qt5 COMPONENTS AndroidExtras REQUIRED)
endif()

message(STATUS "root This is BINARY dir " ${PROJECT_BINARY_DIR})
message(STATUS "root This is SOURCE dir " ${PROJECT_SOURCE_DIR})

set(SOURCE_FILES
    qkxscriptlocalcommand.cpp
    qkxscriptfilecontentcommand.cpp
    qkxscriptfilelistcommand.cpp
    qkxscriptfiletransfercommand.cpp
    qkxscriptbigscriptcommand.cpp
    qkxscriptremotecommand.cpp
    qkxscriptcommand.cpp    
    qkxdockwidget.cpp
    qkxlabelassist.cpp
    qkxbuttonassist.cpp
    qkxdirassist.cpp
    qkxfileassist.cpp
    qkxfilterlineedit.cpp
    qkxfilterlistview.cpp    
    qkxmessagebox.cpp
    qkxtouchpoint.cpp
    qkxloadingbarwidget.cpp
    qkxpositionitem.cpp
    qkxbubblesyncwidget.cpp
    qkxplaintextedit.cpp
    qkxfilesystemwatcher.cpp
    #window object
    main.cpp
    qwotheme.cpp
    qwoapplication.cpp
    qwomenubutton.cpp
    qwoaboutdialog.cpp
    qwoadmindialog.cpp
    qwosystemoptiondialog.cpp
    qwocommandlineinput.cpp
    qwomessagequeuewidget.cpp
    qwomessagequeuemodel.cpp
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
    qwoidentifykeycontentdialog.cpp
    qwolinenoise.cpp
    qwolistview.cpp
    qwomainwindow.cpp
    qwopasswordinput.cpp
    qworenamedialog.cpp
    qwosftpnamedialog.cpp
    qwosessionlist.cpp
    qwosessionmanage.cpp
    qwosessionproperty.cpp
    qwosessionrdpproperty.cpp
    qwosessionvncproperty.cpp
    qwosessionttyproperty.cpp
    qwosessionftpproperty.cpp
    qwosessionfileassociationmodel.cpp
    qwosessionfileassociationadddialog.cpp
    qwosetting.cpp
    qwosettingdialog.cpp
    qwoshower.cpp
    qwoshowerwidget.cpp
    qwounioninputitem.cpp
    qwounioninputconfiguredialog.cpp
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
    qwosftpeditordialog.cpp
    qwosftpwidget.cpp
    qwosftpwidgetimpl.cpp
    qwosftplocalmodel.cpp
    qwosftpremotemodel.cpp
    qwosftpqueuemodel.cpp
    qwosftptransferwidget.cpp
    qwosftppermissiondialog.cpp
    qwosftprenamedialog.cpp
    qwosftpmovefiledialog.cpp
    qwosftptreeview.cpp
    qwoserialinput.cpp
    qwoserialtermwidget.cpp
    qwoserialwidgetimpl.cpp
    qwoserialinputhistorydialog.cpp
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
    qwodbsftpdetaildialog.cpp
    qwodbrestoredialog.cpp
    qwodbbackupdialog.cpp
    qwodbmigratedialog.cpp
    qwodbpowerbackupdialog.cpp
    qwodbpowerrestoredialog.cpp
    qwobindportpermissiondialog.cpp
    qwogroupinputdialog.cpp
    qwofontlistmodel.cpp
    qworecenthistory.cpp
    qwolicensedialog.cpp
    qwolicenseactivatedialog.cpp
    qwoopacitysettingdialog.cpp
    qwoversionupgradetipdialog.cpp
    qwolicensetrialapplydialog.cpp
    qwoplaybooktermwidget.cpp
    qwoplaybookmanagedialog.cpp
    qwoplaybookconfiguredialog.cpp
    qwoplaybookwidget.cpp
    qwoplaybookwidgetimpl.cpp
    qwoplaybookconsoleproxy.cpp
    qwoplaybookconsole.cpp
    qwolocalsshagent.cpp
    qwotunneldialog.cpp
    qwotunneladddialog.cpp
    qwotunnelmodifydialog.cpp
    qwotunnelmodel.cpp
    qwotunnelserver.cpp
    qwotelnet.cpp
    qwoutils.cpp
    qwowidget.cpp
    qwolineedit.cpp
    qwossh.cpp
    qwopty.cpp
    qwomodem.cpp

    #mobile object.
    qmoquickassist.cpp
    qmoapplication.cpp
    qmomainwindow.cpp
    qmorecenthistorymodel.cpp
    qmotermwidget.cpp
    qmoptytermwidget.cpp
    qmosshtermwidget.cpp
    qmoshowwidget.cpp
    qmotermkeyboard.cpp
    qmotelnettermwidget.cpp
    qmotermwidgetimpl.cpp
    qmotelnettermwidgetimpl.cpp
    qmosshtermwidgetimpl.cpp
    qmoptytermwidgetimpl.cpp
    qmomenu.cpp
    qmomenulistmodel.cpp
    qmokeyboard.cpp
    qmoredpoint.cpp
    qmomessageboxassist.cpp
    qmodbsftpdetailassist.cpp    
    qmosftpremotemodel.cpp
    qmosftpassist.cpp
    qmosftpwidgetimpl.cpp
    qmosftptransferassist.cpp
    qmosftpqueuemodel.cpp
    qmoaboutassist.cpp
    qmoandroidassist.cpp
    qmovncwidgetimpl.cpp
    qmovncwidget.cpp
    qmorlogintermwidget.cpp
    qmorlogintermwidgetimpl.cpp    
)

set(HEADER_FILES
    qkxscriptlocalcommand.h
    qkxscriptfilecontentcommand.h
    qkxscriptfilelistcommand.h
    qkxscriptfiletransfercommand.h
    qkxscriptbigscriptcommand.h
    qkxscriptremotecommand.h
    qkxscriptcommand.h
    qkxdockwidget.h
    qkxlabelassist.h
    qkxbuttonassist.h
    qkxdirassist.h
    qkxfileassist.h
    qkxfilterlineedit.h
    qkxfilterlistview.h    
    qkxtouchpoint.h
    qkxmessagebox.h
    qkxloadingbarwidget.h
    qkxpositionitem.h
    qkxbubblesyncwidget.h
    qkxplaintextedit.h
    qkxfilesystemwatcher.h
    # window object
    qwotheme.h
    qwoapplication.h
    qwomenubutton.h
    qwoaboutdialog.h
    qwoadmindialog.h
    qwosystemoptiondialog.h
    qwocommandlineinput.h
    qwomessagequeuewidget.h
    qwomessagequeuemodel.h
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
    qwoidentifykeycontentdialog.h
    qwolinenoise.h
    qwolistview.h
    qwomainwindow.h
    qwopasswordinput.h
    qworenamedialog.h
    qwosftpnamedialog.h
    qwosessionlist.h
    qwosessionmanage.h
    qwosessionproperty.h
    qwosessionrdpproperty.h
    qwosessionvncproperty.h
    qwosessionttyproperty.h
    qwosessionftpproperty.h
    qwosessionfileassociationmodel.h
    qwosessionfileassociationadddialog.h
    qwosetting.h
    qwosettingdialog.h
    qwoshower.h
    qwoshowerwidget.h
    qwounioninputitem.h
    qwounioninputconfiguredialog.h
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
    qwosftpeditordialog.h
    qwosftpwidgetimpl.h
    qwosftplocalmodel.h
    qwosftpremotemodel.h
    qwosftpqueuemodel.h
    qwosftptransferwidget.h
    qwosftppermissiondialog.h
    qwosftprenamedialog.h
    qwosftpmovefiledialog.h
    qwosftptreeview.h
    qwoserialinput.h
    qwoserialtermwidget.h
    qwoserialwidgetimpl.h
    qwoserialinputhistorydialog.h
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
    qwodbsftpdetaildialog.h
    qwodbrestoredialog.h
    qwodbbackupdialog.h
    qwodbmigratedialog.h
    qwodbpowerbackupdialog.h
    qwodbpowerrestoredialog.h
    qwobindportpermissiondialog.h
    qwogroupinputdialog.h
    qwofontlistmodel.h
    qworecenthistory.h
    qwolicensedialog.h
    qwolicenseactivatedialog.h
    qwoopacitysettingdialog.h
    qwoversionupgradetipdialog.h
    qwolicensetrialapplydialog.h
    qwoplaybooktermwidget.h
    qwoplaybookmanagedialog.h
    qwoplaybookconfiguredialog.h
    qwoplaybookwidget.h
    qwoplaybookwidgetimpl.h
    qwoplaybookconsoleproxy.h
    qwoplaybookconsole.h
    qwolocalsshagent.h
    qwotunneldialog.h
    qwotunneladddialog.h
    qwotunnelmodifydialog.h
    qwotunnelmodel.h
    qwotunnelserver.h
    qwotelnet.h
    qwoutils.h
    qwowidget.h
    qwolineedit.h
    qwossh.h
    qwopty.h
    qwomodem.h
    version.h

    #mobile object.
    qmoquickassist.h
    qmoapplication.h
    qmomainwindow.h
    qmorecenthistorymodel.h
    qmotermwidget.h
    qmoptytermwidget.h
    qmosshtermwidget.h
    qmoshowwidget.h
    qmotermkeyboard.h
    qmotelnettermwidget.h
    qmotermwidgetimpl.h
    qmotelnettermwidgetimpl.h
    qmosshtermwidgetimpl.h
    qmoptytermwidgetimpl.h
    qmomenu.h
    qmomenulistmodel.h
    qmokeyboard.h
    qmoredpoint.h
    qmomessageboxassist.h
    qmodbsftpdetailassist.h    
    qmosftpremotemodel.h
    qmosftpassist.h
    qmosftpwidgetimpl.h
    qmosftptransferassist.h
    qmosftpqueuemodel.h
    qmoaboutassist.h
    qmoandroidassist.h
    qmovncwidgetimpl.h
    qmovncwidget.h
    qmorlogintermwidget.h
    qmorlogintermwidgetimpl.h

)

set(OTHER_FILES
    woterm.qrc
    qwoplaybookmanagedialog.ui
    qwoplaybookconfiguredialog.ui
    qkxbubblesyncwidget.ui
    # window object
    qwoplaybookwidget.ui
    qkxfilterlistview.ui    
    qwoaboutdialog.ui
    qwomenubutton.ui
    qwoadmindialog.ui
    qwosystemoptiondialog.ui
    qwocommandlineinput.ui
    qwomessagequeuewidget.ui
    qwohostinfo.ui
    qwohostinfolist.ui
    qwohostlist.ui
    qwoidentifydialog.ui
    qwoidentifycreatedialog.ui
    qwoidentifykeycontentdialog.ui
    qwomainwindow.ui
    qwobasetoolform.ui
    qwopasswordinput.ui
    qworenamedialog.ui
    qwosftpnamedialog.ui
    qwosessionmanage.ui
    qwosessionproperty.ui
    qwosessionrdpproperty.ui
    qwosessionvncproperty.ui
    qwosessionttyproperty.ui
    qwosessionftpproperty.ui
    qwosessionfileassociationadddialog.ui
    qwosettingdialog.ui
    qwotermmask.ui
    qwoserialinput.ui
    qwoserialinputhistorydialog.ui
    qwounioninputitem.ui
    qwounioninputconfiguredialog.ui
    qwosftpwidget.ui
    qwosftpeditordialog.ui
    qwosftptransferwidget.ui
    qwosftppermissiondialog.ui
    qwosftprenamedialog.ui
    qwosftpmovefiledialog.ui
    qwovnctoolform.ui
    qwovncftpwidget.ui
    qwodbsftpdetaildialog.ui
    qwodbrestoredialog.ui
    qwodbbackupdialog.ui
    qwodbmigratedialog.ui
    qwodbpowerbackupdialog.ui
    qwodbpowerrestoredialog.ui
    qwoplaybookconsole.ui
    qwobindportpermissiondialog.ui
    qwodbsftpdownlistdialog.ui
    qwogroupinputdialog.ui
    qwolicensedialog.ui
    qwolicenseactivatedialog.ui
    qwoopacitysettingdialog.ui
    qwoversionupgradetipdialog.ui
    qwolicensetrialapplydialog.ui
    qwotunneldialog.ui
    qwotunneladddialog.ui

    #mobile object
    qmomainwindow.ui
    qmomenu.ui
    qmokeyboard.ui
)

set(ANDROID_FILES
    android/AndroidManifest.xml
    android/build.gradle
    android/res/xml/file_provider.xml
    android/res/values/libs.xml
    android/res/values/splashscreentheme.xml
    android/res/drawable/splashscreen.xml
    android/src/com/aoyiduo/woterm/MainActivity.java
    android/src/com/aoyiduo/woterm/AppUtils.java
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
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /INCREMENTAL:NO" )
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

    add_custom_command(TARGET ${PROJECT_NAME}
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "begin to find rc.exe path"
        COMMAND where rc.exe
        COMMAND ${CMAKE_COMMAND} -E echo "finish to find rc.exe path"
    )
elseif(APPLE)
    set(SOURCE_FILES ${SOURCE_FILES} qwomac.mm)
    set(HEADER_FILES ${HEADER_FILES} qwomac.h)
    message("apple system.")
    link_libraries(ptyqt SQLiteCpp sqlite3 crypto ssh qssh rdp zmodem libtelnet)

    add_library(openssl_crypto STATIC IMPORTED)
    set_target_properties(openssl_crypto PROPERTIES IMPORTED_LOCATION ${OPENSSL_ROOT_DIR}/lib/libcrypto.dylib)
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
    target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS})
    add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "${CMAKE_CURRENT_SOURCE_DIR}/../private"
        COMMAND ${CMAKE_COMMAND} -E echo "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        COMMAND cp -Rf "${CMAKE_CURRENT_SOURCE_DIR}/../private" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../private"
        COMMAND ls -l "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../*"
    )
elseif(ANDROID)
    message("android system.")
    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android" CACHE INTERNAL "")

    link_libraries(dl ptyqt SQLiteCpp sqlite3 crypto ssh qssh rdp zmodem libtelnet SQLiteCpp)
    add_library(openssl_crypto SHARED IMPORTED)
    set_target_properties(openssl_crypto PROPERTIES IMPORTED_LOCATION ${OPENSSL_ROOT_DIR}/lib/libcrypto.so)
    link_libraries(openssl_crypto)

    add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${HEADER_FILES} ${OTHER_FILES} ${LangSrcs} ${ANDROID_FILES})
    target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS})
    target_include_directories(${PROJECT_NAME} PRIVATE ${Qt5AndroidExtras_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} Qt5::AndroidExtras)
else()
    message("linux system.")
    link_libraries(ptyqt SQLiteCpp sqlite3 crypto ssh qssh rdp zmodem libtelnet SQLiteCpp)
    add_library(openssl_crypto SHARED IMPORTED)
    set_target_properties(openssl_crypto PROPERTIES IMPORTED_LOCATION ${OPENSSL_ROOT_DIR}/lib/libcrypto.so)
    link_libraries(openssl_crypto)

    add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES} ${OTHER_FILES} ${LangSrcs})
    target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS})
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::CorePrivate Qt5::Gui Qt5::GuiPrivate Qt5::Widgets  Qt5::SerialPort Qt5::Qml Qt5::Xml Qt5::Charts Qt5::Quick Qt5::QuickControls2 Qt5::QuickWidgets)


