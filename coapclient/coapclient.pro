include(plugins.pri)

TARGET = guh_deviceplugincoapclient

message(============================================)
message("Qt version: $$[QT_VERSION]")
message("Building $$deviceplugin$${TARGET}.so")

SOURCES += \
    deviceplugincoapclient.cpp \

HEADERS += \
    deviceplugincoapclient.h \
