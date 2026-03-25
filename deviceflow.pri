# Path setup - ensures the compiler finds headers in this folder
INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

# Modular dependencies
QT += quick

# C++ Source Files
SOURCES += \
    $$PWD/deviceflow.cpp \
    $$PWD/sequencebase.cpp \
    $$PWD/state.cpp \
    $$PWD/trigger.cpp

# C++ Header Files
HEADERS += \
    $$PWD/deviceflow.h \
    $$PWD/sequencebase.h \
    $$PWD/state.h \
    $$PWD/trigger.h

# Files to show in the Project Tree (Designer support)
DISTFILES += $$PWD/Sequence.qml

RESOURCES += \
    $$PWD/df.qrc
