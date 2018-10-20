# Owl - www.owlclient.com
# Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

project(Utils)

set (SOURCE_FILES
    Base64.cpp
    DateTimeParser.cpp
    Exception.cpp
    Moment.cpp
    QSgml.cpp
    QSgmlTag.cpp
    Rijndael.cpp
    Settings.cpp
    StringMap.cpp
    OwlUtils.cpp
    Version.cpp
    WebClient.cpp
)

set (MOC_HEADERS
    Settings.h
    WebClient.h
)

set (HEADER_FILES
    Base64.h
    DateTimeParser.h
    Exception.h
    Moment.h
    QSgml.cpp
    QSgmlTag.cpp
    QThreadEx.h
    OwlUtils.h
    Rijndael.h
    StringMap.h
    Version.h
    ${MOC_HEADERS}
)

qt5_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADERS})

add_library(Utils STATIC
    ${SOURCE_FILES}
    ${HEADER_FILES}
    ${MOC_SOURCE_FILES}
)

target_link_libraries(Utils
    Qt5::Concurrent
    Qt5::Core
    Qt5::Network
    Qt5::Widgets
    Qt5::Xml
)
