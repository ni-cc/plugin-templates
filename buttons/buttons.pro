include(plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginbuttons)

message("Building $$deviceplugin$${TARGET}.so")

SOURCES += \
    devicepluginbuttons.cpp \

HEADERS += \
    devicepluginbuttons.h \
