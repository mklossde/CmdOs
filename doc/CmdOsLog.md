
# CmdOs Log
	
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

![LOGO](images/CmdOS_logo.gif) a OpenOn.org project - develop by michael@openon.org 