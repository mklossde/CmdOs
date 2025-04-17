
# CmdOs Access

login/logout via cmd/web/mqtt. 
A login is valid everywhere until logout or reboot (on all cmd/web/mqtt) ! 

Set accessLevel with cmd "setAccess ACCESS_LEVEL". The Access-Level define which functions (or below) needs login.
	e.g. "setAccess 3" => admin-functions + change-functions + read-functions are secured and need login 

## Feature AccessControl 
- ACCESS_ADMIN 1 // admin function 
- ACCESS_CHANGE 2 // user function 
- ACCESS_READ 3 // info function 
- ACCESS_ALL 4 // general function

![LOGO](images/CmdOS_logo.gif) a OpenOn.org project - develop by michael@openon.org 