TARGET = atomizer

#ISDEV := "Yes, damange my device"
GIT_VERSION := $(shell git describe --abbrev=4 --dirty --always --tags)

CFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"
CFLAGS += -Iinclude/
CFLAGS += -I.
CFLAGS += -Werror

OBJS = \
    src/globals.o \
    src/materials.o \
    src/settings/settings_helpers.o \
    src/settings/dataflash.o \
    src/utils/button.o \
    src/utils/menu.o \
    src/utils/communication.o \
    font/Font_Large.o \
    font/Font_Medium.o \
    font/Font_Small.o \
    src/utils/helper.o \
    src/display/display_helper.o \
    src/display/display.o \
    src/settings/settings.o \
    images/temperature.o \
    src/modes/mode.o \
    src/modes/mode_watt.o \
    src/modes/mode_volt.o \
    src/modes/mode_temp.o \
    src/main.o

ifeq ($(ISDEV),"Yes, damange my device")
	CFLAGS += -DWITHFLASHDAMAGESUPPORT=1
endif

include $(EVICSDK)/make/Base.mk
