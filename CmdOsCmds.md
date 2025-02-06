
# CmdOS Cmds
	
	
## Format
Format: "cmd p1 p2 p3 p4 p5 p6 p7" (split by space)

Every cmd element is split by space (agin SPLIT BY SPACE, e.g. do not use $a=1!)
if required mark cmd end with ";" (e.g. "stat;")
 
# esp and app

## esp 
 	esp => show esp status
		e.g. "ESP chip:11924060 free:116340 core:v5.3.2-282-gcfea4f7c98-dirty freq:240 flashChipId:2 flashSize:4194304 flashSpeed:80000000 SketchSize:1289648 FreeSketchSpace:1310720"

## stats
	stat => show app status
		e.g. "AppInfo MatrixHub V1.0.0 eeType:Os02 login:1 access_level:0"
		
## login [password]
	login [PASSWORD] => login with password or logout without password

# attribntues

## text params 
use "text" for params with spavces (e.g. attr NAME "Max Mustermann"(

## $attr
use attr with $ (e.g. log $NAME )	

## attributes
set new attribue with $NAME infront of cmd
	e.g. $a stat => $a = "AppInfo MatrixHub V1.0.0 eeType:Os02 login:1 access_level:0"
or clculate with =
	e.g. $a = $a +1

attrDel $a => remove
attr => list all attributes 
attrClear PREFIX => clear all attributes with prefix (without prefix => all attributes)
random MIN MAX => generate random int betwen min and max

## mqttAttr TOPIC 0/1
receive all values from topic into $TOPIC (1=enable/0=disable)
e.g. mqttAttr dev/esp/energy => all posts will put into attr $dev/esp/energy 


## extract START END STRING
extract text betwenn start and end in string 
	e.g. extract esp: " " "Values esp:1000 freq:80" => 1000
	
# control cmds
	
## comments
use '#' as first char to add a comment (comments can and will be used for goto ;:-)
	
## goto
goto ANYTEXT - will jump to the start of "ANYTEXT"
e.g. 
"#loop"
"goto loop"

## if
if a == b GOTOorSKIP
the if will check if the int condition is true (possible are == != <= >= < >)
a and b will converted to int before validation. (e.g. a == b => toInt(a) == toInt(b))

If "GOTOorSKIP" it not an integer, on true, the prgramm will goto 
e.g. 
  "#loop"
  "if 1 == 1 loop"

if "GOTOorSKIP" is an integer n the next n lines will skiped on true
(without "GOTOorSKIP", one line will be skiped on true (same as 1) )
e.g. 
  if 1 == 1 2
    drawLine 0 0 10 10
	log "draw a line"
  drawRect 0 0 10 10

## wait TIME
wait timeInMs before next cmd

## log
log the given infos 
log Hello $name "this is CmdOS"

## logLevel LEVEL
set the actual log-level

# prg 
starts a set of cmds as a programm. line by line.
! only 1 programm runs at a time !

## run PRG
run prg from filesystem 


## stop
stop actual running prg

## next 
execute next step of actual program (afer stop)

## continue 
continue actual prg



# configuration

## reset
rest all configurations

## save
save all configurations

## load 
load all configurations

## set [espName] [espPas]
set name of esp (e.g. for mdns)
and password for (login/web/mqtt) access control
return esp-infs 
	e.g. "eeBoot eeMode:30 espName: espPas:1 espBoard: wifi_ssid:IoT timestamp:1738511365 mqtt:mqtt ntp:"

## access accessLevel
set accessLevel 

## wifi [wifi_ssid] [wifi_pas]
set ssid and wlan-password
return wifi infos:
	e.g. "WIFI mode:30 ip:10.0.0.116 wifi_ssid:IoT - mac:3FFB2030 status:3 signal:-63 rmac:3FFCA74C wifiStat:3 bootWifiCount:2 - AP_SSID: AP_IP:0.0.0.0"
	
## scan 
scan wifis and log results

## ping IP
ping ip and log result

## dns NAME
resolve dns-name and log result

## time TIME TIMESERVER
set the actual time and TIMESERVER
return time infos: e.g. TIME ntpServer:10.0.0.1 time:5.2.2025 3 21:34:6 timeNow:1738791246 timeMs:0

## mode MODE	
set the actual boot eeMode

# rest URL
GET url and return result
	e.g. $a url http://myEsp/api/v1/energy => set $a to body of resposne

## cmdRest 
call http/rest url and execute resposne body as cmd/prg
	e.g. "cmdRest http://server/example/logExample.cmd" => execute logExample on esp

# mqtt 
 
## mqtt SERVER
set mqtt server e.g. mqtt://admin:pas@192.168.1.1:1833

## mqttLog
enable/disable mqtt log 
=> send all logs to device/esp/ESPNAME/log

## mqttSend TOPIC MESSAGE
send message to topic

## mqttConnect / mqttDisconnect


# timer 
add a tiemr with "timer on sec min hour wday day month *cmd"
Each sec/min/hour/wday/day/month can be a value or undefined (255)
on 0=didabeld 1=enabled timer
The timer will execute the given "cmd" when its time.
e.g. 
	'timer 1 0 255 255 255 255 255 log "Hello timer"' => will execute ever minute 
	'timer 1 0 0 22 255 255 255 "lightOff.cmd"   => will call light Off ever day at 22:00 
timers => will list all timer (from 0..n)
timerDel n => will delete timer n 

# filesystem

## fsDir
list the filesystem to log

## fsCat FILE
cat the FILE to log

## fsWrite FILE MESSAGE
write MESSAGE into FILE

## fsDel FILE
delete FILE

## fsRen FILE NEWFILE
rename FILE to NEWFILE

## fsDownload URL FILE
deonload contetn of URL into [FILE] 
e.g. fsDownload https://www.w3.org/Icons/64x64/home.gif  => fill load url into file "home.gif"

## fsFormat 
format filesystem







