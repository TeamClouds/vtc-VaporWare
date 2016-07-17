TARGET = atomizer

#ISDEV := "Yes, damange my device"
GIT_VERSION := $(shell git describe --abbrev=4 --dirty --always --tags)
#ATY_DEBUG := 1
#SHOWOVERFLOW := 1

CFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"
CFLAGS += -Iinclude/
CFLAGS += -Ispace/include/
CFLAGS += -Ivaptris/include/
CFLAGS += -I.
CFLAGS += -Werror
CFLAGS += -Wpedantic

#CFLAGS += -fstack-usage
#LDFLAGS=-Map=whatever.map

OBJS = \
    src/globals.o \
    src/materials.o \
    src/settings/settings_helpers.o \
    src/settings/dataflash.o \
    src/utils/button.o \
    src/utils/menu.o \
    src/utils/communication.o \
    src/utils/variabletimer.o \
    font/Font_Large.o \
    font/Font_Medium.o \
    font/Font_Small.o \
    images/images.o \
    src/utils/helper.o \
    src/display/drawables/battery_drawables.o \
    src/display/drawables/atomizer_drawables.o \
    src/display/drawables/modes_drawables.o \
    src/display/drawables.o \
    src/display/screens/charging_screen.o \
    src/display/screens/main_screen.o \
    src/display/atomizer_query.o \
    src/display/display_helper.o \
    src/display/display.o \
    src/settings/settings.o \
    images/temperature.o \
    src/modes/mode.o \
    src/modes/mode_watt.o \
    src/modes/mode_volt.o \
    src/modes/mode_temp.o \
    space/space_res.o \
    space/game_globals.o \
    space/alien.o \
    space/levels/level.o \
    space/levels/level1.o \
    space/levels/level2.o \
    space/game.o \
    vaptris/vaptris.o \
    src/main.o

ifeq ($(ISDEV),"Yes, damange my device")
	CFLAGS += -DWITHFLASHDAMAGESUPPORT=1
endif

ifeq ($(ATY_DEBUG),1)
	CFLAGS += -DATYDEBUG=1
endif

ifeq ($(SHOWOVERFLOW),1)
	CFLAGS += -DSHOWOVERFLOW=1
endif

include $(EVICSDK)/make/Base.mk
