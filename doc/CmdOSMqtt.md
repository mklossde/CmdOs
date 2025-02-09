
# CmdOs MQTT

 
## mqtt SERVER
set mqtt server e.g. mqtt://admin:pas@192.168.1.1:1833

## mqttLog
enable/disable mqtt log 
=> send all logs to device/esp/ESPNAME/log

## mqttAttr Receive a topic 

use mqttAttr to recevie all messages from a topic into attr
	e.g. mqttAttr dev/myesp/energy 1
will subscribe topic "dev/myesp/energy" and set attr "$dev/myesp/energy" on every value change

	mqttAttr dev/myesp/energy 1
	wait 1000
	log "myesp energy" $dev/myesp/energy
	drawText 10 10 888 1 $dev/myesp/energy => draw value on display

e.g. mqttAttr dev/esp/energy => all posts will put into attr $dev/esp/energy 

## mqttSend TOPIC MESSAGE
send message to topic

## mqttConnect / mqttDisconnect


![LOGO](images/CmdOS_logo.gif) a OpenOn.org project - develop by mk@almi.de 

 