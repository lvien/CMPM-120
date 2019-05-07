TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    commands.cpp \
    debug.cpp \
    file_sys.cpp \
    main.cpp \
    util.cpp

HEADERS += \
    commands.h \
    debug.h \
    file_sys.h \
    util.h
