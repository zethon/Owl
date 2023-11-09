set (SOURCE_FILES
    DateTimeParser.cpp
    Exception.cpp
    Moment.cpp
    QSgml.cpp
    QSgmlTag.cpp
    Settings.cpp
    StringMap.cpp
    OwlLogger.cpp
    OwlUtils.cpp
    SimpleArgs.cpp
    Version.cpp
    WebClient.cpp
)

set (MOC_HEADERS
    Settings.h
    WebClient.h
)

set (HEADER_FILES
    DateTimeParser.h
    Exception.h
    Moment.h
    QSgml.cpp
    QSgmlTag.cpp
    QThreadEx.h
    OwlLiterals.h
    OwlLogger.h
    OwlUtils.h
    SimpleArgs.h
    StringMap.h
    Version.h
    ${MOC_HEADERS}
)

qt5_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADERS})

add_library(Utils STATIC
    ${SOURCE_FILES}
    ${HEADER_FILES}
    ${MOC_SOURCE_FILES}
    OwlLiterals.h
)

target_link_libraries(Utils
    ${CONAN_LIBS}
    Qt5::Concurrent
    Qt5::Core
    Qt5::Network
    Qt5::Widgets
    Qt5::Xml
)
