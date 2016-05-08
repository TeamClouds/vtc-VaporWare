#include <stdio.h>
#include <stdint.h>

#include <Atomizer.h>
#include <Display.h>
#include <USB_VirtualCOM.h>

#include "display.h"
#include "font/font_vaporware.h"
#include "globals.h"
#include "helper.h"
#include "settings.h"
#include "images/watts.h"


// TODO: Move IntPid out to its own c/h for reuse
#define HISTLEN 16
struct IntPID {
    int32_t targetTemp;
    int32_t Max;
    int32_t Min;
    int32_t P;
    int32_t I;
    int32_t D;
    int32_t initWatts;
    int32_t Rave;
    int32_t Hist[HISTLEN];
    int32_t HIndex;
    int32_t Perror;
} I = {
};

int32_t getInitWattsDefault() {
    return s.initWatts;
}

void updateInitWatts(int32_t newWatts) {
    initWattsSet(newWatts);
}

void formatWatts(int32_t value, char *formatted) {
    siprintf(formatted, "%lu.%02lu", value / 1000, value % 1000 / 10);
}

int32_t getPidSwitchDefault() {
    return s.pidSwitch;
}

void updatePidSwitch(int32_t newpidswitch) {
    pidSwitchSet(newpidswitch);
}

void formatNumber(int32_t value, char *formatted) {
    siprintf(formatted, "%ld", value);
}

int32_t getPDefault() {
    return s.pidP;
}

int32_t getIDefault() {
    return s.pidI;
}

int32_t getDDefault() {
    return s.pidD;
}

void setP(int32_t p) {
    pidPSet(p);
}

void setI(int32_t i) {
    pidISet(i);
}

void setD(int32_t d) {
    pidDSet(d);
}

struct menuItem dragonItems[] = {
    {
        .type = EDIT,
        .label = "P",
        .editMin = MINPID,
        .editMax = MAXPID,
        .getEditStart = &getPDefault,
        .editCallback = &setP,
        .editStep = 100,
        .editFormat = &formatNumber
    },
    {
        .type = EDIT,
        .label = "I",
        .editMin = MINPID,
        .editMax = MAXPID,
        .getEditStart = &getIDefault,
        .editCallback = &setI,
        .editStep = 100,
        .editFormat = &formatNumber
    },
    {
        .type = EDIT,
        .label = "D",
        .editMin = MINPID,
        .editMax = MAXPID,
        .getEditStart = &getDDefault,
        .editCallback = &setD,
        .editStep = 100,
        .editFormat = &formatNumber
    },
    {
        .type = STARTBOTTOM,
    },
    {
        .type = LINE,
    },
    {
        .type = SPACE,
        .rows = 2,
    },
    {
        .type = EXITMENU,
        .label = "Back",
    },
    {
        .type = END,
    }
};

struct menuDefinition dragonMenu = {
        .name = "Display Settings",
        .font = FONT_SMALL,
        .cursor = "*",
        .prev_sel = "<",
        .next_sel = ">",
        .less_sel = "-",
        .more_sel = "+",
        .menuItems = &dragonItems,
};

struct menuItem tempSettingsOptions[] = {
    {
        .type = EDIT,
        .label = "Watts",
        .editMin = MINWATTS,
        .editMax = MAXWATTS,
        .getEditStart = &getInitWattsDefault,
        .editCallback = &updateInitWatts,
        .editStep = 100,
        .editFormat = &formatWatts,
    },
    {
        .type = EDIT,
        .label = "PID Switch",
        .editMin = STEMPMIN,
        .editMax = STEMPMAX,
        .getEditStart = &getPidSwitchDefault,
        .editCallback = &updatePidSwitch,
        .editStep = 10,
        .editFormat = &formatNumber,
    },
    {
        .type = STARTBOTTOM,
    },
    {
        .type = LINE,
    },
    {
        .type = SPACE,
        .rows = 2,
    },
    {
        .type = SUBMENU,
        .label = "Dragons",
        .subMenu = &dragonMenu,
    },
    {
        .type = EXITMENU,
        .label = "Back",
    },
    {
        .type = END,
    }
};

struct menuDefinition tempSettings = {
    .name = "Display Settings",
    .font = FONT_SMALL,
    .cursor = "*",
    .prev_sel = "<",
    .next_sel = ">",
    .less_sel = "-",
    .more_sel = "+",
    .menuItems = &tempSettingsOptions,
};

volatile int prline = 0;

void initPid() {
    int i = 0;
    for (i = 0; i<HISTLEN; i++) {
        I.Hist[i] = 0;
    }
    I.HIndex = 0;
    I.Rave = 0;
    I.P = s.pidP;
    I.I = s.pidI;
    I.D = s.pidD;
    
    I.Max = MAXWATTS;
    I.Min = MINWATTS;

    g.watts = s.initWatts;
    g.volts = wattsToVolts(g.watts, g.atomInfo.resistance);
    Atomizer_SetOutputVoltage(g.volts);
}

void setTarget(int32_t ttemp) {
    I.targetTemp = ttemp;
}

// TODO: Move to pidTuning struct and make compilation optional
uint32_t start = 0;
uint32_t atTemp = 0;
uint32_t freqNow = 0;
uint32_t count = 0;
uint32_t samples = 0;
int8_t curSign = 1;

// Curently runs at about 132hz
int32_t getNext(int32_t c_temp) {
    int32_t error = I.targetTemp - c_temp;
    int32_t aveError;
    int32_t diffError;
    samples++;
    freqNow = gv.uptime;

    if (s.tunePids) {
        if (!atTemp && error <= 0) {
            atTemp = gv.uptime;
            char buff[64];
            siprintf(buff, "INFO," UPTIME ",At Temp\r\n", (atTemp - start)/100, (atTemp - start)%100);
            USB_VirtualCOM_SendString(buff);
        }
    }

    int j = I.HIndex - 1;
    j = (j < 0) ? HISTLEN : j;
    I.Hist[I.HIndex] = error;
    I.Rave = I.Rave + error;
    I.Rave = I.Rave - I.Hist[j];
    I.HIndex = (I.HIndex + 1) % HISTLEN;

    aveError = I.Rave;
    diffError = I.Perror - error;
    I.Perror = error;

    int32_t next = error * I.P / 100 +
               aveError * I.I / 100 +
                   diffError * I.D / 100;

    if (s.tunePids) {
        if (prline) {
                 char buff[63];
                 siprintf(buff, UPTIME ",%ld,%ld,%lu,%ld,%ld,%ld,%ld,%ld,%ld,%ld\r\n",
                          UPTIMEVAL,
                          I.targetTemp,
                          c_temp,
                          g.watts, 
                          I.P,
                          error,
                          I.I,
                          aveError,
                          I.D,
                      diffError,
                          g.watts + next
                 );
                 USB_VirtualCOM_SendString(buff);
        }
    }

    // TODO: there needs to be 'scaling' to scale dT to dW    
    return next;
}

void tempInit() {
    //  set this initial value because we may be switching
    // from another mode that changes our watts.
}

void tempFire() {
    uint16_t newVolts;

    setTarget(s.targetTemperature);
    initPid();
    int pidactive = 0;
    start = gv.uptime;
    atTemp = 0;
    uint32_t last = gv.uptime;
    uint32_t now;
    while (gv.fireButtonPressed) {
        now = gv.uptime;
        
        if (!Atomizer_IsOn() && g.atomInfo.resistance != 0
            && Atomizer_GetError() == OK) {
            // Power on
            Atomizer_Control(1);
        }

        // Update info
        // If resistance is zero voltage will be zero
        Atomizer_ReadInfo(&g.atomInfo);
        EstimateCoilTemp();

        if (!pidactive) {
            if (s.targetTemperature - g.curTemp >= s.pidSwitch) {
                g.watts = s.initWatts;
            } else {
                if (s.dumpPids) {
                    char b[63];
                    siprintf(b, "INFO,Switching to PID %ld %d\r\n", s.targetTemperature, g.curTemp);
                    USB_VirtualCOM_SendString(b);
                }
                pidactive = 1;
            }
        }
    // Don't allow firing > 1 ohm in temp mode.
/*  TODO: Maybe make this baseRes dependant
    if (g.atomInfo.resistance > 1000) {
        g.watts = 0;
    } */

        if (g.watts < 0)
            g.watts = 1000;

        if (g.watts > 100000) // Pid probably went c-razy on us, so drop watts.
            g.watts = 1000;

        if (g.watts > MAXWATTS)
            g.watts = MAXWATTS;
           

        newVolts = wattsToVolts(g.watts, g.atomInfo.resistance);

        if (newVolts != g.volts) {
            if (Atomizer_IsOn()) {
                // Update output voltage to correct res variations:
                // If the new voltage is lower, we only correct it in
                // 10mV steps, otherwise a flake res reading might
                // make the voltage plummet to zero and stop.
                // If the new voltage is higher, we push it up by 100mV
                // to make it hit harder on TC coils, but still keep it
                // under control.
                if (newVolts < g.volts) {
                    newVolts = g.volts - (g.volts >= 10 ? 10 : 0);
                } else {
                    newVolts = g.volts + 100;
                }
            }

            if (newVolts > MAXVOLTS) {
                newVolts = MAXVOLTS;
            }

            g.volts = newVolts;

            Atomizer_SetOutputVoltage(g.volts);
        }

        Atomizer_ReadInfo(&g.atomInfo);
        EstimateCoilTemp();

        g.watts = getNext(g.curTemp);

        if (g.watts < 0)
            g.watts = 1000;

        if (g.watts > 100000)
            g.watts = 1000;

        if (g.watts > MAXWATTS)
            g.watts = MAXWATTS;

        prline = now - last >= 10;
        now = last;

        if (1 || prline) {
            if (s.dumpPids) {
                 char buff[63];
                 siprintf(buff, "PID,%ld,%d,%ld,%d\r\n",
                          s.targetTemperature, 
                          g.curTemp,
                          g.watts,
                      g.atomInfo.resistance);
                 USB_VirtualCOM_SendString(buff);
             }
             updateScreen(&g);
        }
    }

    if (Atomizer_IsOn())
        Atomizer_Control(0);
}

void tempUp() {
    int32_t dT = s.displayTemperature + 10;
    int32_t mT = tempScaleType[s.tempScaleTypeIndex].max;
    if (dT <= mT) {
        displayTemperatureSet(dT);
        targetTemperatureSet(displayToC(dT));
    } else {
        displayTemperatureSet(mT);
        targetTemperatureSet(displayToC(mT));
    }
}

void tempDown() {
    int32_t dT = s.displayTemperature - 10;
    int32_t mT = tempScaleType[s.tempScaleTypeIndex].min;
    int32_t MT = tempScaleType[s.tempScaleTypeIndex].max;
    if (dT >= mT && dT < MT) {
        displayTemperatureSet(dT);
        targetTemperatureSet(displayToC(dT));
    } else {
        displayTemperatureSet(mT);
        targetTemperatureSet(displayToC(mT));
    }
}

void tempDisplay(uint8_t atomizerOn) {
    char buff[9];
    if (atomizerOn) {
        printNumber(buff, CToDisplay(g.curTemp));
    } else {
        printNumber(buff, s.displayTemperature);
    }
    Display_PutText(0, 5, buff, FONT_LARGE);
    getString(buff, (char *) tempScaleType[s.tempScaleTypeIndex].display);
    Display_PutText(48, 2, buff, FONT_SMALL);

    // Material
    getString(buff, vapeMaterialList[s.materialIndex].name);
    Display_PutText(48, 15, buff, FONT_SMALL);
}

void tempBottomDisplay(uint8_t atomizerOn) {
    char buff[9];
    Display_PutPixels(0, 100, watts, watts_width, watts_height);

    if (atomizerOn) {
        getFloating(buff, g.watts);
    } else {
        getFloating(buff, s.initWatts);
    }
    Display_PutText(26, 105, buff, FONT_MEDIUM);
}
