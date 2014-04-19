TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11

SOURCES += main.cpp \
    device.cc
INCLUDEPATH += /usr/local/Cellar/libimobiledevice/1.1.5/include  /usr/local/Cellar/libplist/1.10/include/


QMAKE_CXXFLAGS += -isystem /usr/local/Cellar/libimobiledevice/1.1.5/include \
        -isystem /usr/local/Cellar/libplist/1.10/include/ -Wextra -Wall -O2
QMAKE_LIBS +=  -L/usr/local/Cellar/libplist/1.10/lib -L/usr/local/Cellar/libimobiledevice/1.1.5/lib/ -limobiledevice -lplist

HEADERS += \
    device.hpp \
    common.hpp \
    plist_af.hpp
