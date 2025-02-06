
# CmdOs Setup

## setup 

setup esp with "wifi_ssid wifi_pas espName espPas"
	e.g. "setup wifi_ssid wifi_pas espName espPas"
set esp values + save + reboot

## Auto wifi set_up
A auto-setup feature for wifi clients, designed by openon.org.
Connect to a pre-defiend wifi-network "set_up" which automaticly setup new devices.

A central router or simple esp-instance will provide a WLAN **set_up** which a client is looking, without wifi infos.
(before it opens a own AccessPoint, which will be the second init method.)
The new device call http://GATEWAYIP/setupDevice?name=ESPNAME" on the AccessPoint, which provide basic wifi config. 
   "setup wifi_ssid wifi_pas deviceName devicePas"

### init wifi "setp_up"

Auto connect to to SSID "set_up". 
If no wifi "set_up" if found (after 10 seconds) swith to EE_MODE_AP
The esp will stay connected to "setp_up" until reboot !
	
### remote pull setup 
Setup via pull with "/setupDevice".	After connect to wifi "set_up", a restCmd call to gateway occures.
	connct "set_up" + read cmd from gateway **http://GATEWAYIP/setupDevice** + execute cmd
		
#### Exmaple 

enable remote "setupDevice COUNT" with 1..n (allow number of deviceses). 0=disabled (default) 
	e.g. "setupDev 1" 
This switch to wifi in AccessPotint mode with name wifi "set_up" 
and allow remote-esp is able to call "cmdRest http://THISESP/setupDevice"	

Now switch on esp with new installed CmdOs or use "reset" on remote.

#### set_up flow
- OlDevice with "setupDev 1" have own "set_up" wifi as an AccessPoint
- NewDevice will boot and connect "set_up" wifi
- NewDevice will execute "http://OLDDEVICE/setupDevice on OlDevice => OldDevcice resonse "setup wifiSSID wifiPas "" espPas
- NewDevcice execute given setup and will save and reboot

