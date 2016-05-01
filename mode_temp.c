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

struct menuItem tempSettingsOptions[] = {
    {
        .type = EXITMENU,
        .label = "temp",
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

void showTempMenu() {
    runMenu(&tempSettings);
}

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
    g.watts = s.initWatts;
    I.Max = 60000;		// Never fire over 60 watts
    I.Min = 0;
    g.volts = wattsToVolts(g.watts, g.atomInfo.resistance);
    Atomizer_SetOutputVoltage(g.volts);
}

void setTarget(int32_t ttemp) {
    I.targetTemp = ttemp;
}

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
    g.vapeCnt++;
    setTarget(s.targetTemperature);
    initPid();
    int pidactive = 0;
    start = gv.uptime;
    atTemp = 0;
    uint32_t last = gv.uptime;
    uint32_t now;
    while (gv.fireButtonPressed) {
        now = gv.uptime;
        // Handle fire button
        if (!Atomizer_IsOn() && g.atomInfo.resistance != 0
	    && Atomizer_GetError() == OK) {
	    // Power on
	    Atomizer_Control(1);
	}
	// Update info
	// If resistance is zero voltage will be zero
	Atomizer_ReadInfo(&g.atomInfo);
        if (!pidactive) {
            if ((int32_t)s.targetTemperature - (int32_t)g.atomInfo.temperature >= s.pidSwitch) {
                g.watts = s.initWatts;
            } else {
                if (s.dumpPids) {
                    char b[63];
                    siprintf(b, "INFO,Switching to PID %ld %ld\r\n", s.targetTemperature, g.atomInfo.temperature);
                    USB_VirtualCOM_SendString(b);
                }
                pidactive = 1;
            }
        }
	// Don't allow firing > 1 ohm in temp mode.
	if (g.atomInfo.resistance > 1000) {
	    g.watts = 0;
	}

        if (g.watts < 0)
            g.watts = 1000;

        if (g.watts > 100000)
            g.watts = 1000;

        if (g.watts > 60000)
            g.watts = 60000;
           

	g.newVolts = wattsToVolts(g.watts, g.atomInfo.resistance);

	if (g.newVolts != g.volts) {
	    if (Atomizer_IsOn()) {

		// Update output voltage to correct res variations:
		// If the new voltage is lower, we only correct it in
		// 10mV steps, otherwise a flake res reading might
		// make the voltage plummet to zero and stop.
		// If the new voltage is higher, we push it up by 100mV
		// to make it hit harder on TC coils, but still keep it
		// under control.
		if (g.newVolts < g.volts) {
		    g.newVolts = g.volts - (g.volts >= 10 ? 10 : 0);
		} else {
		    g.newVolts = g.volts + 100;
		}

	    }

	    if (g.newVolts > ATOMIZER_MAX_VOLTS) {
		g.newVolts = ATOMIZER_MAX_VOLTS;
	    }

	    g.volts = g.newVolts;

	    Atomizer_SetOutputVoltage(g.volts);
	}
	Atomizer_ReadInfo(&g.atomInfo);
	// TODO: We might need a short sleep here?
//        if (pidactive || s.targetTemperature < g.atomInfo.temperature + 50) 
{
	    g.watts = getNext(g.atomInfo.temperature);
            pidactive = 1;
        } 
        if (g.watts < 0)
            g.watts = 1000;

        if (g.watts > 100000)
            g.watts = 1000;

        if (g.watts > 60000)
            g.watts = 60000;
        prline = now - last >= 10;
        now = last;
	if (1 || prline) {
	     if (s.dumpPids) {
                 char buff[63];
                 siprintf(buff, "PID,%ld,%ld,%ld,%d\r\n",
                          s.targetTemperature, 
                          g.atomInfo.temperature,
                          g.watts,
	                  g.atomInfo.resistance);
                 USB_VirtualCOM_SendString(buff);
             }
             updateScreen(&g);
        }
	g.vapeCnt++;
        prline = (g.vapeCnt % 10) == 0;
    }
    if (Atomizer_IsOn())
	Atomizer_Control(0);
    g.vapeCnt = 0;
}

void tempUp() {
    uint32_t dT = s.displayTemperature + 10;
    uint32_t mT = tempScaleType[s.tempScaleTypeIndex].max;
    if (dT <= mT) {
	s.displayTemperature = dT;
        s.targetTemperature = displayToC(dT);
    } else {
        s.displayTemperature = mT;
	s.targetTemperature = displayToC(mT);
    }
}

void tempDown() {
    uint32_t dT = s.displayTemperature - 10;
    uint32_t mT = tempScaleType[s.tempScaleTypeIndex].min;
    if (dT >= mT) {
        s.displayTemperature = dT;
	s.targetTemperature = displayToC(dT);
    } else {
        s.displayTemperature = mT;
	s.targetTemperature = displayToC(mT);
    }
}

void tempDisplay(uint8_t atomizerOn) {
    char buff[9];
    if (atomizerOn) {
        printNumber(buff, CToDisplay(g.atomInfo.temperature));
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

	getFloating(buff, g.watts);
	Display_PutText(26, 105, buff, FONT_MEDIUM);
}
