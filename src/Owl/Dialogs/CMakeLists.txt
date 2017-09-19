# Owl - www.owlclient.com
# Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

project(Dialogs)

# we need this to be able to include headers produced by uic in our code
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set (SOURCE_FILES
    AboutDlg.cpp
    ConfiguringBoardDlg.cpp
    EditBoardDlg.cpp
    ErrorReportDlg.cpp
    NewThreadDlg.cpp
    PreferencesDlg.cpp
    QuickAddDlg.cpp
)

set (MOC_HEADERS
    AboutDlg.h
    ConfiguringBoardDlg.h
    EditBoardDlg.h
    ErrorReportDlg.h
    NewThreadDlg.h
    PreferencesDlg.h
    QuickAddDlg.h
)

set (HEADER_FILES
    ${MOC_HEADERS}
)

SET (UI_SOURCES
    AboutDlg.ui
    ConfiguringBoardDlg.ui
    EditBoardDlg.ui
    ErrorReportDlg.ui
    NewThreadDlg.ui
    PreferencesDlg.ui
    QuickAddDlg.ui
)

qt5_wrap_ui(UI_HEADERS ${UI_SOURCES})
qt5_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADERS})

set(DEPENDENCY_LIST
    log4qt
    Parsers
    Utils
)

add_library(Dialogs STATIC
    ${SOURCE_FILES}
    ${HEADER_FILES}
    ${MOC_SOURCE_FILES}
)

target_link_libraries(Dialogs
    Qt5::Concurrent
    Qt5::Core
    Qt5::Gui
    Qt5::Sql
    Qt5::Widgets
    Qt5::Xml
)
