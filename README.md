
# CmdOS V0.1.0 a OpenOn.org project

develop by mk@almi.de

![CmdOS LOGO](images/CmdOS.gif)

If you use/like this project please <a href='http://paypal.me/openonorg<'>donate to http://paypal.me/openonorg</a> (a good choise is 1Euro per ESP a year ;-)

[<a href="libs.md">Require Libs</a>]

**CmdOS is a command based operation system for the ESP32.**

## Feature:
- <a href='doc/CmdOsBoot.md'>EEPROM config and Failsafe boot</a>
- SPIFF / Filesystem
- <a href='doc/CmdOsSetup.md'>Setup with AutoWifi set_up</a>
- Wifi 
- mDNS
- NTP
- OTA and WebFirmewarteUpdate
- <a href="doc/CmdOsWeb.md">Web</a>
- MQTT
- <a href="doc/LedAndSwitch.md">Hardware LED and Button</a>
- <a href="doc/CmdOsCmds.md">Command</a>
- <a href="doc/CmdOsPrg.md">Program</a>
- Auto Startup programm
- Timer programm
- <a href='doc/CmdOsLog.md'>Logging</a>
- <a href='doc/CmdOsAccess.md'>AcessControl</a>

## Cmd Interfaces:
- Serial cmd
- WebSerial cmd
- Rest cmd
- MQTT cmd

## Cmd 
	<a href="doc/CmdOscms.md">See cmd description for details</a>

	All functions are controled via cmd
	e.g. drawLine 0 0 10 0 => draw a line on a display
	

# Feature Autostart
A filesytem file "/startup.cmd" is automaticly started after startup (after 10 seconds)

# Feature Timer
Add timers via ntp
	e.g. timer 1 0 0 22 255 255 255 "lightOff.cmd" => will call light Off ever day at 22:00 

# Feature NTP
The Network time is received from ntp-server, or gateway (if no npt-server is defined)

# Feature FileSystem
As default a SPIFF filesystem is used
	

![LOGO](images/CmdOS_logo.gif) a OpenOn.org project - develop by mk@almi.de 



	

	

