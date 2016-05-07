#ifndef __DEFAULTS_H
#define __DEFAULTS_H
// 10/second 0.5-60s range, 3s default
#define SCREENMINTIMEOUT 5
#define SCREENMAXTIMEOUT 600
#define SCREENDEFAULTTIMEOUT 30

#define FADEOUTTIME 60
#define FADEINTIME 50
#define MAXFADE (SCREENDEFAULTTIMEOUT / 2)
#define MINFADE 0

#define MINBRIGHTNESS 0
#define MAXBRIGHTNESS 100
#define DEFBRIGHTNESS 50

// After screen off
#define SYSSLEEPDEFAULT 180 * 10
// After screen off, may block sys sleep
#define SETTINGSWRITEDEFAULT 300 * 10

// Kanthal Default
#define DEFAULTMATERIAL 0

// Default mode: Wattage
#define DEFAULTMODE 0

// Temp Scale, Default F (Sorry Europe)
#define DEFAULTTEMPSCALE 1

// Pid Vals
#define MINPID 0
#define MAXPID 30000
// TODO: Move these to material struct
#define DEFPIDP 17000
#define DEFPIDI 5500
#define DEFPIDD 0

// Temp mode watts (and wattage mode default)
#define MINWATTS 0
#define MAXWATTS 60000
#define DEFWATTS 15000

// Volt mode
#define MINVOLTS 0
// May not be sane... 60 watts at 3.5o = 14V, cap at 9 like OFW
#define MAXVOLTS 9000
#define DEFVOLTS 3000

// Temp mode pid handoff temp differential
#define STEMPMIN -600
#define STEMPMAX  600
#define STEMPDEF  600

// Invert display pixel default
#define INVERTDEF 0

// Flip display while vaping
#define FLIPDEF 1

// TCR
#define TCRMIN 0
#define TCRMAX 1000
#define TCRDEF 0

// Base Temp degrees C
#define BTEMPMIN -20
#define BTEMPMAX 50
#define BTEMPDEF -20

// Base Res, mOhm
#define BRESMIN 50
#define BRESMAX 3450
#define BRESDEF 0

// Screen brightness
#define SBRIGHTMIN 0
#define SBRIGHTMAX 255
#define SBRIGHTDEF 50

// Default stealth mode off
#define STEALTHDEF 0

// Default temp change on
#define VSETLOCKDEF 0

#endif