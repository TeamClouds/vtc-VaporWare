TARGET = atomizer

OBJS = communication.o mode-helper.o helper.o display.o settings.o mode_watt.o mode_volt.o mode_temp.o main.o

include $(EVICSDK)/make/Base.mk
