include(plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginbuttons)

message(============================================)
message("Qt version: $$[QT_VERSION]")
message("Building $$deviceplugin$${TARGET}.so")

SOURCES += \
    devicepluginbuttons.cpp \

HEADERS += \
    devicepluginbuttons.h \
