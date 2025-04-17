
# CmdOS Boot

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
	
![LOGO](images/CmdOS_logo.gif) a OpenOn.org project - develop by michael@openon.org 