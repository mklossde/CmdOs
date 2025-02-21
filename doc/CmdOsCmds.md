
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

## freeHeap
	show the freeHeap of memory
		e.g. "196152"
		
## login [password]
	login [PASSWORD] => login with password or logout without password

## restart 
	restart the esp

## sleep [mode] [sleepTimeMS]
	goto sleep
	- mode 0 => swith all off
	- mode 1 => wifi off 
	- mode 2 => LightSleep for sleepTimeMS
	- mode 3 => DeepSleep for sleepTimeMS

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
	
attr a VALUE => set a to value
attrDel $a => remove
attrs => list all attributes 
attrClear PREFIX => clear all attributes with prefix (without prefix => all attributes)
random MIN MAX => generate random int betwen min and max ( e.g. $a = random 1 10)


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
if a == b CMD or {}
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

## elseif a == b CMD or {} 
	e.g. ..} elseif a == 1 {..
## else cmd or {}
	e.g. ..} else goto start 
	
## {} until RULE
repeat unitl rule 
e.g. .. } until $a > 10


## wait TIME
wait timeInMs before next cmd

## exec $a
execute a command (e.g. attr a "run t" =>  exec $a)

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

## end 
end a programm (and clear prg from memory)

## error MESSAGE
stop prg with an error
	e.g. stop prg an a program error 

## stop
stop actual running prg

## next 
execute next step of actual program (afer stop)

## continue 
continue actual prg



# configuration

## reset value
rest all configurations after validation. 
call "rest" will return a random numner. call "reset NUMBER" to rest all



## save
save all configurations

## load 
load all configurations

## conf [espName] [espPas] [espBoard]
set name of esp (e.g. for mdns)
and password for (login/web/mqtt) access control
and board name
	e.g. "conf myesp mypas myboard"
return config-infs 
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








![LOGO](images/CmdOS_logo.gif) a OpenOn.org project - develop by mk@almi.de 





