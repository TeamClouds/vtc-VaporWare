#include <stdio.h>
#include <stdint.h>

#include <USB_VirtualCOM.h>

#include "main.h"

#define PIDLEN 10

struct IntPID {
    int32_t targetTemp;
    int32_t Max;
    int32_t Min;
    int32_t P;
    int32_t I;
    int32_t D;
    int32_t Rave;
    int32_t Rvals[PIDLEN];
    uint8_t i;
} I = {
};

void initPid() {
    int i;
    for (i = 0; i < PIDLEN; i++)
	I.Rvals[i] = 0;
    I.Rave = 0;
    I.i = 0;
}

void setTarget(int32_t ttemp) {
    I.targetTemp = ttemp;
}

int32_t getNext(int32_t c_temp, int32_t c_fire) {
    int32_t error = I.targetTemp - c_temp;
    int32_t aveError;
    int32_t diffError;
    int i = I.i;
    int l = I.i > 0 ? I.i - 1 : PIDLEN;

    I.Rave -= I.Rvals[i];
    I.Rvals[i] = error;
    I.Rave += I.Rvals[i];

    aveError = I.Rave / PIDLEN;

    diffError = I.Rvals[i] - I.Rvals[l];

    I.i++;
    I.i %= PIDLEN;

    /* */
    int32_t next = I.P * error / 1000 +
	I.I * aveError / 1000 + I.D * diffError / 1000;
    next += c_fire;

    if (next > 60000)
	next = 60000;
    if (next < 1000)
	next = 1000;

    char buff[63];
    siprintf(buff, "PID,%ld,%ld,%ld,%d\r\n", I.targetTemp, c_temp, next,
	     g.atomInfo.resistance);
    USB_VirtualCOM_SendString(buff);

    // TODO: there needs to be 'scaling' to scale dT to dW    
    return next;
}

void tempInit() {
    I.P = s.pidP;
    I.I = s.pidI;
    I.D = s.pidD;
    I.Max = 60000;		// Never fire over 60 watts
    I.Min = 0;

    if (!s.targetTemperature)
	s.targetTemperature = 400;

	// set this initial value because we may be switching
	// from another mode that changes our watts.
    g.watts = 15000;
    g.volts = wattsToVolts(g.watts, g.atomInfo.resistance);
    Atomizer_SetOutputVoltage(g.volts);
}

void tempFire() {
    g.vapeCnt++;
    setTarget(s.targetTemperature);
    initPid();
    g.watts = 15000;		// Start Firing at 15 watts.
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

	if (g.atomInfo.temperature > s.targetTemperature) {
	    if (g.atomInfo.temperature > g.maxTemp) {
		g.maxTemp = g.atomInfo.temperature;
	    }

	}
	if (g.maxTemp) {
	    if (g.atomInfo.temperature < g.minTemp || !g.minTemp) {
		g.minTemp = g.atomInfo.temperature;
	    }
	}
	// Don't allow firing > 1 ohm in temp mode.
	if (g.atomInfo.resistance > 1000) {
	    g.watts = 0;
	}

	g.newVolts = wattsToVolts(g.watts, g.atomInfo.resistance);

	if (g.newVolts != g.volts || !g.volts) {
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
	g.vapeCnt++;
	Atomizer_ReadInfo(&g.atomInfo);
	// TODO: We might need a short sleep here?
	g.watts = getNext(g.atomInfo.temperature, g.watts);
	if (!g.vapeCnt % 10)
	    updateScreen(&g);
    }
    if (Atomizer_IsOn())
	Atomizer_Control(0);
    g.vapeCnt = 0;
}

void tempUp() {
    if (s.targetTemperature + 10 <= g.vapeModes[s.mode]->maxSetting) {
	s.targetTemperature += 10;
    } else {
	s.targetTemperature = g.vapeModes[s.mode]->maxSetting;
    }
}

void tempDown() {
    if (s.targetTemperature - 10 >= 0) {
	s.targetTemperature -= 10;
    } else {
	s.targetTemperature = 0;
    }
}
