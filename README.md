# eVic-VTC-VaporWare by TeamClouds

This is a fully[1] open source[2][3] firmware for the eVic-VTC (and maybe others).  It is built
off of the evic-sdk[4].

- [1] Our firmware is fully open source, and the evic-sdk is fully open source.  They both
    depend on the nuvotron-sdk, which is source available, but the license of that code
    is not completely open source friendly.
- [2] https://github.com/TeamClouds/vtc-VaporWare 
- [3] https://github.com/TeamClouds/evic-sdk
- [4] https://github.com/ReservedField/evic-sdk
##### Thanks to:

Big thanks go out to: ReservedField, Bane^, gxcreator, and kfazz
for all of the effort they put forth in making of the sdk, as well as the features they
added at our request to help make this firmware what it is.

##### TeamClouds:
- sebirdman - Founder of the project, lots of the UI work
- s0be - codes all the low level stuff
- romanbb - fonts, makes it build on mac, other UI work
- PsychoI3oy - QA.  Press button, get nicotine.

---
#### Release notes:
 V1-beta:
 - Everything

---
#### Buttons
- Main Interface
 - Fire button fires
 - Left button lowers mode setting
 - Right button increases mode setting
 - 3 Fire Button Enter Settings
- Settings
 - Left navigates up, right navigates down
 - Fire button selects item to edit or enters a submenu
- Info Screen
 - Press and hold for great justice
 - Gittish of build is that weird hex in the version (dev only)


---
### Settings

- Coil
 - Type
    - KA, Kanthal/No Temp Control
    - SS, Stainless (TCR: .0000105)
    - TI, Titanium (TCR: .0000350)
    - NI, Nickel (TCR: .0000620)
 - Res.
    - Auto - Trust our approximation of the coil resistance.
    - Set - Set what the resistance should be, resets to 'Lock' mode if you remove the atomizer
    - Lock - Trust our approximation of the coil resistance, but prompt for a change of more than 10%
 - Dragons
    - TCR - Edit the TCR value
    - B. Temp - Set what the ambient air temperature is in degrees C for the base res.
    - B. Res - Set what the base resistance is, ignoring what the device reads.
- Mode
 - Mode
   - Watt
   - Volt
   - Temperature (NI,TI,SS only)
     - Options
        - Watts - Initial Firing Watts - Only useful with next option
        - Pid Switch
           - How close to the target temperature to switch to pid control (if less than 150)
     - Dragons
       - P,I,D tuning values for the PID Algorithm used to control temperature
- Display
 - Scale - Temperature scale to display values in F, C, K (Cuz kelvin) 
 - Brightness (0-100)
 - FlipVape - Whether to flip the screen while firing
 - Invert - Invert all the pixels
- Advanced:
     - Reboot - (Reboots, hold right while selecting this to enter LDROM mode)
     - Factory Reset
     - (debug build) Inv.Fla - Invalidates the current flash settings, forcing a re-creation
                             or re-upgrade.  Requires compile time flag to be set.
     - (debug build) Era.Fla - Fully wipes dataflash

---

### General Info 
- Data Flash (Persistant Settings) - Writes settings when exiting menu, or vape mode
                                   controls (watt,temp,volt,baseRes,baseTemp) after
                                   5 minutes of inactivity.
- USB Settings
- USB Data Dumping
- System Sleep - Sleeps after 3 minutes of inactivity, or after
               writing any staged settings changes.
---

### Known Issues
* '.' character in Voltage/Wattage mode renders incorrectly
* Slight overshoot when firing from cold in TC mode.
* Occasional 'OPEN' errors will happen (may not be noticible except in debug builds)
   * If your target wattage (VW/VT mode only) causes your output to be approximatly your
     current battery voltage, it may not fire.  Bump target up/down 1 increment to work
     around this.

### Outstanding Features
* Need Smoother Font
* Code needs a few rounds of general 'clean up'.

### Todo
- More Materials - s0be needs more wire
- Pid tuning per material - s0be needs more wire (see above)
- Pid tunign per coil count - s0be needs to know if this is worth it

### USB Control
The device shows up as a cdc_acm serial device.  Under linux and mac, it should just work.
Under windows.... no clue.  To talk with it, send the 1 byte command, followed by a newline.
It eats line feeds.  Feel free to feed it them.  The response will be any appropriate messages
for the command, along with a command response packet.

  Commands are structured of the form:

     X,arg1,arg2,....,argN

##### The command response packet will be of the form: [-~$]X\r\n
where:
- - means command failed
- ~ means command not implemented
- $ means command succeeded
- X is whatever command you sent it

##### Implemented Commands:
- @ - No Arguments - Just returns a simple string
- A - 2 Arguments - Updates a value in the AtomizerInfo Struct
   - Arg1 - Field [baseResistance, baseTemperature]
   - Arg2 - Value
- a - No Arguments - Dumps the AtomizerInfo Struct
- S - 2 Arguments - Updates a value in the settings struct
   - Arg1 - Field (see list from s command output)
   - Arg2 - Value
- s - No Arguments - Dumps the settings struct
- U - Reboots to LDROM mode.  if you're not holding right, it'll immediately
         exit LDROM mode unless an update program is prepared to attach to it.


