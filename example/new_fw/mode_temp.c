#include <stdio.h>
#include <stdint.h>

#include <USB_VirtualCOM.h>

#include "main.h"

struct IntPID {
    int32_t targetTemp;
    int32_t Max;
    int32_t Min;
    int32_t P;
    int32_t I;
    int32_t D;
    int32_t initWatts;
    int32_t Rave;
    int32_t Perror;
} I = {
};

volatile int prline = 0;

void initPid() {
    I.Rave = 0;
    I.P = s.pidP;
    I.I = s.pidI;
    I.D = s.pidD;
    g.watts = s.initWatts;
    I.Max = 60000;		// Never fire over 60 watts
    I.Min = 0;
    USB_VirtualCOM_SendString("INFO,Start the fire\r\n");
    g.volts = wattsToVolts(g.watts, g.atomInfo.resistance);
    Atomizer_SetOutputVoltage(g.volts);
}

void setTarget(int32_t ttemp) {
    I.targetTemp = ttemp;
}

int32_t getNext(int32_t c_temp) {
    int32_t error = I.targetTemp - c_temp;
    int32_t aveError;
    int32_t diffError;

    I.Rave = I.Rave + error;
    aveError = I.Rave;
    diffError = I.Perror - error;
    I.Perror = error;

    int32_t next = error * I.P / 100 +
	           aveError * I.I / 100 +
                   diffError * I.D / 100;

#if 0
    if(prline) {
                 char buff[63];
                 siprintf(buff, "%lu,%ld,%ld,%ld,%ld,%ld,%ld,%ld\r\n",
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
#endif

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
    while (gv.fireButtonPressed) {
	// Handle fire button
	if (!Atomizer_IsOn() && g.atomInfo.resistance != 0
	    && Atomizer_GetError() == OK) {
	    // Power on
	    Atomizer_Control(1);
	}
	// Update info
	// If resistance is zero voltage will be zero
	Atomizer_ReadInfo(&g.atomInfo);

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
	    g.watts += getNext(g.atomInfo.temperature);
            pidactive = 1;
        } 
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
