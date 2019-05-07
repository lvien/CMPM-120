TEMPLATE = app
CONFIG += console c++14
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    bigint.cpp \
    ubigint.cpp \
    scanner.cpp \
    debug.cpp \
    libfns.cpp \
    util.cpp \
    junk.cpp

HEADERS += \
    bigint.h \
    relops.h \
    ubigint.h \
    debug.h \
    iterstack.h \
    libfns.h \
    scanner.h \
    util.h
