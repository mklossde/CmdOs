
# CmdOs Timer


## Feature Autostart
A filesytem file "/startup.cmd" is automaticly started after startup (after 10 seconds)


## Feature Timer
Add timers via ntp
	e.g. timer 1 0 0 22 255 255 255 "lightOff.cmd" => will call light Off ever day at 22:00 

add a tiemr with "timer on sec min hour wday day month *cmd"
Each sec/min/hour/wday/day/month can be a value or undefined (255)
on 0=didabeld 1=enabled timer
The timer will execute the given "cmd" when its time.
e.g. 
	'timer 1 0 255 255 255 255 255 log "Hello timer"' => will execute ever minute 
	'timer 1 0 0 22 255 255 255 "lightOff.cmd"   => will call light Off ever day at 22:00 
timers => will list all timer (from 0..n)
timerDel n => will delete timer n 
	

![LOGO](images/CmdOS_logo.gif) a OpenOn.org project - develop by michael@openon.org

 