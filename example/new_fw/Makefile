TARGET = atomizer

#ISDEV := "Yes, damange my device"

OBJS = \
    globals.o \
    materials.o \
    settings_helpers.o \
    dataflash.o \
    button.o \
    menu.o \
    communication.o \
    mode-helper.o \
    font/Font_Large.o \
    font/Font_Medium.o \
    font/Font_Small.o \
    helper.o \
    display_helper.o \
    display.o \
    settings.o \
    images/temperature.o \
    mode_watt.o \
    mode_volt.o \
    mode_temp.o \
    main.o

ifeq ($(ISDEV),"Yes, damange my device")
	CFLAGS += -DWITHFLASHDAMAGESUPPORT=1
endif

include $(EVICSDK)/make/Base.mk
