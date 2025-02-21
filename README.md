
# CmdOS V0.1.0 a OpenOn.org project

develop by mk@almi.de

![CmdOS LOGO](images/CmdOS.gif)

**CmdOS is a command based operation system for the ESP32.**


## Cmd 
	<a href="doc/CmdOscms.md">See cmd description for details</a>

	All functions are controled via cmd
	e.g. random 1 10 => get a random number
	
See <a href='example/example.md'>examples</a>

## Build own Application

CmdOS do not have to run on its own, like Tasmota or EspHome. 
The idea is to build up own development and include CmdOS as a singel file/tab <a href='release/CmdOs_V010.ino'>CmdOS</a>.

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
	
	char* appCmd(char *cmd, char **param) {
	  return cmd; // unkown cmd => use cmd as string
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

If you use/like this project please [https://buymeacoffee.com/openon](https://buymeacoffee.com/openon) (a good choise is 1Euro per ESP a year ;-)  

![LOGO](images/CmdOS_logo.gif) a OpenOn.org project - develop by mk@almi.de 



	

	

