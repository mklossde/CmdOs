
# CmdOS V0.1.0 by OpenOn.org

develop by mk@almi.de

If you use/like this project please donate to paypal.me/openonorg (a good choise is 1Euro per ESP a year ;-)

!(images/CmdOS.gif)

[<a href="libs.md">Require Libs</a>]

A cmd based operation system for the ESP32.

## Feature:
- EEPROM config and Failsafe boot
- SPIFF / Filesystem
- <a href='CmdOsSetup.md'>Setup with AutoWifi Set_up'</a>
- Wifi 
- mDNS
- NTP
- OTA and WebFirmewarteUpdate
- <a href="CmdOsWeb.md">Web</a>
- MQTT
- <a href="LedAndSwitch.md">Hardware LED and Button</a>
- <a href="CmdOsCmds.md">Command</a>
- <a href="CmdOsPrg.md">Program</a>
- Auto Startup programm
- Timer programm
- Logging
- AcessControl

## Web:
- Filesystem Browser

## Cmd Interfaces:
- Serial cmd
- WebSerial cmd
- Rest cmd
- MQTT cmd

## Cmd 
	<a href="CmdOscms.md">See cmd description for details</a>

	All functions are controled via cmd
	e.g. drawLine 0 0 10 0 => draw a line on a display
	
## Feature Logging	
	
logPrintln(LOG_LEVEL,char* or string)
	e.g. sprintf(buffer,"draw pixel %dx%d",x,y); logPrintln(LOG_DEBUG,buffer);
or as cmd
	e.g. log "draw pixel" $x $y
	
The LOG_LEVEL define wich logs (and below) are printed. 
Change the LOG_LEVEL with cmd "logLevel LOG_LEVEL".
	e.g. logLevel 5 => will now print levels 0..5	
	
### Log-Level:
- LOG_SYSTEM 0
- LOG_ERROR 2
- LOG_INFO 5
- LOG_DEBUG 10	

### Log-Output
- Serial 
- Web Serial ( http://ESPNAME.local/webserial
- MQTT to device/esp/ESPNAME/log (mqttLog enabled)
	
# Feature access
login/logout via cmd/web/mqtt. 
A login is valid everywhere until logout or reboot (on all cmd/web/mqtt) ! 

Set accessLevel with cmd "setAccess ACCESS_LEVEL". The Access-Level define which functions (or below) needs login.
	e.g. "setAccess 3" => admin-functions + change-functions + read-functions are secured and need login 

## Feature AccessControl 
- ACCESS_ADMIN 1 // admin function 
- ACCESS_CHANGE 2 // user function 
- ACCESS_READ 3 // info function 
- ACCESS_ALL 4 // general function

# Feature Autostart
A filesytem file "/startup.cmd" is automaticly started after startup (after 10 seconds)

# Feature Timer
Add timers via ntp
	e.g. timer 1 0 0 22 255 255 255 "lightOff.cmd" => will call light Off ever day at 22:00 

# Feature NTP
The Network time is received from ntp-server, or gateway (if no npt-server is defined)

# Feature FileSystem
As default a SPIFF filesystem is used

	
## Feature boot + failsafe 
The actual boot eeMode is stored in config

- EE_MODE_FIRST 0 // First init => EEPROM Wrong
	Automaitcly setup config with pre-defined values
	
- EE_MODE_SETUP 1 // EEInit / wifi Setup mode 
	see AutSetup wifi
	
- EE_MODE_AP 2 // EEInit / wifi AP mode
	Setup own wifi as AccessPoint.
	wlan ssid starts with "CmdOs" ( CmdOsCHIPID)
	
- EE_MODE_WIFI_OFF 10             // OFF 
	Wifi switch off
	
- EE_MODE_WIFI_TRY 21          // Wifi first client access try 
   First try to connecto to new SSID   
- EE_MODE_WIFI_CL_RECONNECT 23    // Wifi is reconnetion (to a Router)
	reconnect to new SSID
- EE_MODE_OK 30  // MODE OK
	wifi work find
- EE_MODE_START 40  // MODE START
	wifi work fine - wait some time (10 seconds) to switch to EE_MODE_OK

- EE_MODE_WRONG 45  // MODE WRONG
	something happends 	
- EE_MODE_ERROR 50  // MODE ERROR , Enable only WIFI
    something bad happend / only wifi (e.g. when something happends *5 times)
- EE_MODE_SYSERROR 51  // MODE ERROR , Enable only CMD
    something very bad happends / only serial cmd




	

	

