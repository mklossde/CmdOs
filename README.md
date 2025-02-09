
# CmdOS V0.1.0 a OpenOn.org project

develop by mk@almi.de

![CmdOS LOGO](images/CmdOS.gif)

If you use/like this project please <a href='http://paypal.me/openonorg<'>donate to http://paypal.me/openonorg</a> (a good choise is 1Euro per ESP a year ;-)

[<a href="libs.md">Require Libs</a>]

**CmdOS is a command based operation system for the ESP32.**

CmdOS do not have to run on its one, like Tasmota or EspHome. 
The idea is to build up own development and include basic functions with <a href='release/CmdOs_V010.ino'>CmdOS</a>.


	// On Application

	#include <ESPAsyncWebServer.h>
	 
	const char *prgTitle = "MyApp";
	const char *prgVersion = "V1.0.0";

	const char* user_admin = "admin"; // default user
	char user_pas[]="";   // default espPas
	const char *wifi_ssid_default = ""; // PRIVAT_WIFI_SSID; // define in privatdata.h 
	const char *wifi_pas_default = ""; //PRIVAT_WIFI_PAS;   // define in privatdata.h 
	const char *mqtt_default = ""; //PRIVAT_MQTTSERVER;     // define in privatdata.h 

	byte MODE_DEFAULT=21; // MODE_WIFI_CL_TRY=21 / MODE_PRIVAT

	boolean serialEnable=true; // enable/disbale serial in/out
	boolean wifiEnable=true;  // enable/disbale wifi
	boolean ntpEnable=true; // enable time server
	boolean webEnable=true;    // enable/disbale http server
	boolean mdnsEnable=true;   // enable/disable mDNS detection 
	boolean bootSafe=true;    // enable/disbale boot safe
	#define enableFs true         // enable fs / SPIFFS
	#define netEnable true       // enable/disbale network ping/dns/HttpCLient 
	#define webSerialEnable false // enable/disbale web serial
	#define mqttEnable true      // enable/disbale mqtt
	#define otaEnable true        // enabled/disbale ota update 
	#define updateEnable false     // enabled/disbale update firmware via web 
	#define ledEnable false       // enable/disbale serial
	#define ledGpio 4             // io of led
	#define LED_ON true           // gpio false=led-on
	#define swEnable false        // enable/disbale switch
	#define swGpio 13             // io pin of sw 
	#define SW_ON false           // gpio false=sw-pressed

	int _webPort = 80;
	AsyncWebServer server(_webPort);
	
	char* appCmd(char *cmd, char *p0, char *p1,char *p2,char *p3,char *p4,char *p5,char *p6,char *p7,char *p8,char *p9) {
		return "unkown cmd"; // add application cmd handling here 
	}

	void webApp() {	} // add application web handling here ----------------------------------------------------------

	void setup() {
	  cmdOSSetup();
	  if(isModeOk()) { 
		// add application setup here 
	  }  
	}

	void loop() {
	  cmdOSLoop();
	  if(isModeOk()) { 
		// add application loop here 
	  }  
	}


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
	
See <a href='example/example.md'>examples</a>

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



	

	

