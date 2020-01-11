# we need this to be able to include headers produced by uic in our code
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

# the folder where Owl's default parsers exists
set(OWL_PARSERS_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/../parsers")

SET (SOURCE_FILES
    AboutDlg.cpp
    AspectRatioPixmapLabel.cpp
    BoardIconView.cpp
    BoardTreeView.cpp
    BoardsModel.cpp
    BoardUpdateWorker.cpp
    ClickableLabel.cpp
    ConfiguringBoardDlg.cpp
    ContentView.cpp
    EditBoardDlg.cpp
    ErrorReportDlg.cpp
    FocusEventLineEdit.cpp
    ForumView.cpp
    main.cpp
    MainWindow.cpp
    NewThreadDlg.cpp
    PaginationWidget.cpp
    PostTextEditor.cpp
    PreferencesDlg.cpp
    OwlApplication.cpp
    PostListWidget.cpp
    QuickAddDlg.cpp
    ThreadListWidget.cpp
)

SET (MOC_HEADERS
    AboutDlg.h
    AspectRatioPixmapLabel.h
    BoardsModel.h
    BoardIconView.h
    BoardTreeView.h
    BoardUpdateWorker.h
    ClickableLabel.h
    ConfiguringBoardDlg.h
    ContentView.h
    EditBoardDlg.h
    ErrorReportDlg.h
    FocusEventLineEdit.h
    ForumView.h
    MainWindow.h
    NewThreadDlg.h
    OwlApplication.h
    PaginationWidget.h
    PostListWidget.h
    PostTextEditor.h
    PreferencesDlg.h
    QuickAddDlg.h
    ThreadListWidget.h
)

SET (UI_SOURCES
    AboutDlg.ui
    ConfiguringBoardDlg.ui
    EditBoardDlg.ui
    ErrorReportDlg.ui
    NewThreadDlg.ui
    PreferencesDlg.ui
    QuickAddDlg.ui
    MainWindow.ui
)

SET (HEADER_FILES
    Core.h
    ${MOC_HEADERS}
)

set(QRC_SOURCE_FILES 
    res/Owl.qrc
)

file(GLOB_RECURSE QML_FILES res/qml/*.qml res/qml/*.js)

IF (APPLE)
    SET(RESOURCE_FILES "Resources/Owl.icns")
    list(APPEND SOURCE_FILES
        TitleBarSetupMac.mm
    )
ENDIF()

if (WIN32)
    SET(CMAKE_EXE_LINKER_FLAGS "/NODEFAULTLIB:library")
    SET(RESOURCE_FILES "res/Owl.rc")
    list(APPEND SOURCE_FILES
        TitleBarSetupWindows.cpp
    )
endif()

QT5_ADD_RESOURCES(RRC_SOURCE_FILES ${QRC_SOURCE_FILES})
QT5_WRAP_UI(UI_HEADERS ${UI_SOURCES})
QT5_WRAP_CPP(MOC_SOURCE_FILES ${MOC_HEADERS})

SET_SOURCE_FILES_PROPERTIES(
    ${RESOURCE_FILES}
    PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources
)

if (APPLE)
    SET(GUI_TYPE MACOSX_BUNDLE)
endif(APPLE)

if (WIN32)
    SET(GUI_TYPE WIN32)
endif(WIN32)

ADD_EXECUTABLE(Owl ${GUI_TYPE} 
    ${SOURCE_FILES}
    ${MOC_SOURCE_FILES}
    ${RRC_SOURCE_FILES}
    ${HEADER_FILES}
    ${RESOURCE_FILES}
    ${UI_HEADERS}
    ${QML_FILES}
)

set(QT_MODULE_LIST
    Qt5::Concurrent
    Qt5::Core
    Qt5::Network
    Qt5::Quick
    Qt5::QuickWidgets
    Qt5::Sql
    Qt5::UiTools
    Qt5::WebEngine
    Qt5::WebEngineWidgets
    Qt5::Widgets
    Qt5::Xml
    Qt5::XmlPatterns
)

if (APPLE)
    list(APPEND QT_MODULE_LIST 
        Qt5::MacExtras
    )
endif (APPLE)
    
SET(DEPENDENCY_LIST 
    Data
    Parsers
    Utils
)

TARGET_LINK_LIBRARIES(Owl 
    ${CONAN_LIBS}
    ${QT_MODULE_LIST}
    ${DEPENDENCY_LIST}
)

if(WIN32 AND MSVC)
    set_target_properties(Owl PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
    set_target_properties(Owl PROPERTIES COMPILE_DEFINITIONS_DEBUG "_CONSOLE")
    set_target_properties(Owl PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
    set_target_properties(Owl PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "_CONSOLE")
endif()

set(LUA_PARSERS
    parser-vbulletin3x.lua
)

# the folder where Owl's default config files exist
set(OWL_CONFIG_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/config")

# folder in which the dictionaries live
set(OWL_DICT_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/../dictionaries")

if (APPLE)
    SET(MACOSX_BUNDLE_BUNDLE_NAME Owl)
    SET(MACOSX_BUNDLE_ICON_FILE Owl.icns)
    SET(MACOSX_BUNDLE_GUI_IDENTIFIER "com.owlclient.full")
    SET(MACOSX_BUNDLE_BUNDLE_VERSION "100")
    SET(MACOSX_BUNDLE_LONG_VERSION_STRING "Owl Version ${OWL_VERSION}")
    SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "${OWL_VERSION}")
    SET(MACOSX_BUNDLE_COPYRIGHT "Copyright (c) 2012-2019, Adalid Claure")

    #SET_TARGET_PROPERTIES(Owl PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Mac Developer")
    #SET_TARGET_PROPERTIES(Owl PROPERTIES XCODE_ATTRIBUTE_OTHER_CODE_SIGN_FLAGS "--deep")

    # the escaped \$ in \${CONFIGURATION} is an Xcode variable
    # see: http://stackoverflow.com/questions/13420117/how-to-get-a-path-of-the-app-bundle-in-a-cmake-script-to-supply-it-to-the-fixup
    if("${CMAKE_GENERATOR}" MATCHES "Xcode")
            set(DEST_BIN_PATH 
                ${CMAKE_BINARY_DIR}/src/\${CONFIGURATION}/Owl.app/Contents/Resources)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            set(DEST_BIN_PATH 
                ${CMAKE_BINARY_DIR}/src/Owl/Owl.app/Contents/Resources)
    endif()

    # NOTE: this may need to change for non-Xcode compilers
    set(BUNDLE_PATH ${CMAKE_BINARY_DIR}/Owl/Release/Owl.app)

    # copy the default user settings file (owl.ini)
    ADD_CUSTOM_COMMAND(TARGET Owl POST_BUILD
        COMMAND "cp" ARGS "${OWL_CONFIG_FOLDER}/macOS/owl.ini" "${DEST_BIN_PATH}"
        COMMENT "Copying owl.ini")

    # copy the included parsers
    if (LUAPARSERS_FOLDER)
        foreach(PARSER ${LUA_PARSERS})
            ADD_CUSTOM_COMMAND(TARGET Owl POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${LUAPARSERS_FOLDER}/${PARSER}"
                        "${DEST_BIN_PATH}/parsers/${PARSER}"
                COMMENT "Copying parser ${LUAPARSERS_FOLDER}/${PARSER}")
        endforeach(PARSER)
    endif(LUAPARSERS_FOLDER)

    # copy the included dictionaries
    ADD_CUSTOM_COMMAND(TARGET Owl POST_BUILD
        COMMAND "cp" ARGS -R "${OWL_DICT_FOLDER}" "${DEST_BIN_PATH}"
        COMMENT "Copying dictionaries folder")

elseif (WIN32)
    FILE(TO_NATIVE_PATH "${OWL_CONFIG_FOLDER}/Windows/owl.ini" OWL_INI_TO_COPY)
    ADD_CUSTOM_COMMAND(TARGET Owl POST_BUILD
        COMMAND "xcopy" ARGS /Y /D "${OWL_INI_TO_COPY}" "$(OutDir)"
        COMMENT "Copying owl.ini")
elseif (LINUX)
    # nothing yet for Linux		
endif()

install (
    TARGETS Owl 
    COMPONENT Owl
    RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	BUNDLE DESTINATION .
)

set(CMAKE_INSTALL_UCRT_LIBRARIES FALSE)
include(InstallRequiredSystemLibraries)
INSTALL(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
        DESTINATION bin
        COMPONENT Owl)

if (WIN32)
    INSTALL(CODE "include(BundleUtilities) 
    fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/bin/Owl.exe\" \"\" \"\")" 
    COMPONENT Owl)

    get_target_property(QT_QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
    
    execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_BINS OUTPUT_VARIABLE QT_INSTALL_BINS  OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_PREFIX OUTPUT_VARIABLE QT_INSTALL_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_PLUGINS OUTPUT_VARIABLE QT_INSTALL_PLUGINS OUTPUT_STRIP_TRAILING_WHITESPACE)

    set(QWINDOWSDLL_PATH "${QT_INSTALL_PLUGINS}/platforms/qwindows$<$<CONFIG:Debug>:d>.dll")
    install(FILES ${QWINDOWSDLL_PATH}
        DESTINATION bin/platforms
        COMPONENT Owl)

    set(QSQLITEDLL_PATH "${QT_INSTALL_PLUGINS}/sqldrivers/qsqlite$<$<CONFIG:Debug>:d>.dll")
    install(FILES ${QSQLITEDLL_PATH}
        DESTINATION bin/sqldrivers
        COMPONENT Owl)

    set(BIN_FILES
        "${QT_INSTALL_PREFIX}/bin/libEGL.dll"
        "${QT_INSTALL_PREFIX}/bin/libGLESv2.dll"
        "${QT_INSTALL_PREFIX}/bin/opengl32sw.dll"
        "${QT_INSTALL_PREFIX}/bin/QtWebEngineProcess.exe"
        "${QT_INSTALL_PREFIX}/resources/icudtl.dat"
        "${QT_INSTALL_PREFIX}/resources/qtwebengine_devtools_resources.pak"
        "${QT_INSTALL_PREFIX}/resources/qtwebengine_resources.pak"
        "${QT_INSTALL_PREFIX}/resources/qtwebengine_resources_100p.pak"
        "${QT_INSTALL_PREFIX}/resources/qtwebengine_resources_200p.pak"
    )

    install(FILES ${BIN_FILES}
        DESTINATION bin
        COMPONENT Owl)

    add_custom_command(TARGET Owl POST_BUILD
        COMMAND "${QT_INSTALL_BINS}/windeployqt.exe"
            --no-plugins 
            --no-libraries 
            --no-translations 
            --no-angle 
            --no-opengl-sw
            --no-compiler-runtime
            --qmldir=${CMAKE_SOURCE_DIR}/src/Owl/res/qml
            "$<TARGET_FILE:Owl>"
    )
    
    set(_directories_
        "$<TARGET_FILE_DIR:Owl>/Qt"
        "$<TARGET_FILE_DIR:Owl>/QtGraphicalEffects" 
        "$<TARGET_FILE_DIR:Owl>/QtQml"
        "$<TARGET_FILE_DIR:Owl>/QtQuick"
        "$<TARGET_FILE_DIR:Owl>/QtQuick.2"
        "$<TARGET_FILE_DIR:Owl>/QtWebEngine"
    )

    install(DIRECTORY
        ${_directories_}
        DESTINATION bin
        COMPONENT Owl
    )
endif(WIN32)

