TEMPLATE = app
CONFIG += console c++14
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    bigint.cpp \
    debug.cpp \
    libfns.cpp \
    main.cpp \
    scanner.cpp \
    ubigint.cpp \
    util.cpp

HEADERS += \
    bigint.h \
    debug.h \
    iterstack.h \
    libfns.h \
    relops.h \
    scanner.h \
    ubigint.h \
    util.h
