
# CmdOS V0.2.0 a OpenOn.org project

develop by mk@almi.de

![CmdOS LOGO](images/CmdOS.gif)

**CmdOS is a command based operation system for the ESP32.**


## Cmd 
<a href="doc/CmdOsCmds.md">See cmd description for details</a>

All functions are controled via cmd. Mqtt exmaple 

	log "show shelly power every 10 seconds"
	mqttAttr shellies/shellyem3/emeter/0/power 1
	#loop
	  if $shellies/shellyem3/emeter/0/power < 0 {
		log "PV Power" $shellies/shellyem3/emeter/0/power
	  }else {
		log "Actual consume" $shellies/shellyem3/emeter/0/power
	  }
	  wait 10000
	  goto #loop
	
See <a href='example/example.md'>examples</a>

## Upload

see <a href="release/">release</a>

## Build new project with cmdOs

see <a href="simpleProject/">simpleProject</a>


## Feature:
- <a href='doc/CmdOsBoot.md'>EEPROM config and Failsafe boot</a>
- <a href='doc/CmdOsSetup.md'>Setup with AutoWifi set_up</a>
- <a href="doc/CmdOsWeb.md">Web</a>
- <a href='doc/CmdOSMqtt.md'>MQTT</a>
- <a href='doc/CmdOSRest.md'>Rest and Remote</a>
- <a href="doc/LedAndSwitch.md">Hardware LED and Button</a>
- <a href="doc/CmdOsCmds.md">Command</a>
- <a href="example/example.md">Programm examples</a>
- <a href='doc/CmdOSTimer.md'>Autostart and Timer</a>
- <a href='doc/CmdOsLog.md'>Logging</a>
- <a href='doc/CmdOsAccess.md'>AcessControl</a>
- <a href="doc/CmdOSFilesystem.md">SPIFF / Filesystem</a>
- Wifi 
- mDNS
- NTP
- OTA and WebFirmewareUpdate

## Cmd Interfaces:
- Serial cmd
- WebSerial cmd
- Rest cmd
- MQTT cmd


# Feature NTP
The Network time is received from ntp-server, or gateway (if no npt-server is defined)
	
[<a href="libs.md">Require Libs</a>]	

<hr>
If you use/like this project please [https://buymeacoffee.com/openon](https://buymeacoffee.com/openon) (a good choise is 1Euro per Device ;-)  
<hr>
:warning: Attention: The use of information and programmes can be dangerous !  This is a private hobby project and makes no claim to completeness, runnability or compliance with regulations. Any use of this or referenced content is at your own risk and excludes any liability by authors. 
<hr>
![LOGO](images/OpenOnOrg.gif) a OpenOn.org project - develop by mk@almi.de 


	

	

