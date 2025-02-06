
# CmdOs Web 

Use http://ESPIP/ or mdns http://ESPNAME.local

Use Access-Control on ALL calls if espPas is given !

# Web App
	http://ESPNAME.local/app
For Application Web implementation	
	
# Web Cmd / Rest-like
	http://ESPNAME.local/cmd/CMD
Execute a cmd with params. 
	e.g. http://ESPNAME.local/cmd/drawPixel+0+0+100+100 = "drawPixel 0 0 100 100"
All values of query-params are added from left to right (key is inimportent !)
	e.g. http://ESPNAME.local/cmd/set?name=MyEsp&something=MyEspPas => "cmd MyEsp MyEspPas"
The result of cmd will resposne as body	 	
		
# Config
	http://ESPNAME.local/config	
- save
- restart (without save)	
- reset

## config Esp
	Config the Esp (wifi,name,mqtt,ntp,..)
## Config App
	For App configuration 

# WebSerial
	http://ESPNAME.local/webserial
	WebSerial console which display logs (output) nd send cmds (input) - like Serial 
	
# Web Firmeware Update
	http://ESPNAME.local/firmware
	Select firmeware and upadte. 
	TAKE CARE ! NO CHECK IS DONE HERRE !

# Web File Browser 
	http://ESPNAME.local/file
	
## Feature
- Show/Download a File
- Edit a File
- Create a File
- Delete a file
- Rename a File
- Upload a File (+Drag and Drop)
- Upload URL to File 