
# CmdOs Rest and Remote

Rest can be used to remote control and to read external values from other devices.

## Remote Control
The rest enpoint can be used to remote control.

Execute a simple CMD (like execute a programe) via REST.
(example with espName set to myesp)

e.g http://myesp.local/cmd/run+counter.cmd
Will execute "run counter.cmd" on ESPNAME, which read the "counter.cmd" programm from filesystem and execute it. 	

	
## Read Remote Values / States

use command "rest" to read remote values 

e.g. 
	Read body via rest call
	$v rest http://myesp.local/cmd/stat
	=> $v="AppInfo MatrixCOS V1.0.0 eeType:Os02 login:0 access_level:0"
	read stat of esp into attr $v
    $v extract eeType: " " $v 
	=> $v = "Os02"

For Basi-Auth calls use	
	e.g. http://admin:pas@myesp.local/cmd/set



![LOGO](images/CmdOS_logo.gif) a OpenOn.org project - develop by michael@openon.org 

 