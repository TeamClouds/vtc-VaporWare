TARGET = atomizer

OBJS = globals.o button.o menu.o communication.o mode-helper.o helper.o display_helper.o display.o settings.o images/temperature.o mode_watt.o mode_volt.o mode_temp.o main.o

include $(EVICSDK)/make/Base.mk
