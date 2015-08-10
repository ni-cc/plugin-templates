include(plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginnetworkinfo)

message("Building $$deviceplugin$${TARGET}.so")

SOURCES += \
    devicepluginnetworkinfo.cpp \

HEADERS += \
    devicepluginnetworkinfo.h \
