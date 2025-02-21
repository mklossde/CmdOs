
CmdOS V.0.2.0 - 17.02.2025
<<<<<<< HEAD
    - "end" - cmdEnd prg execute
	- "error" - cmdError message => end prg with message
	- end of line detect by ';' or '\n' or '\r' or '\0'
	- change to param-pointer (char* appCmd(char *cmd, char *param) { .. })
	- "if RULE cmd" on true => execute cmd or {}
		- if inside if NOT supported yet
	- "elseif RULE cmd" on true => execute cmd or {}
	- "else" => execute cmd or {}
	- "{} until RUILE"  repeat sub (e.g. {} until RUILE }
	- cmdAttr execute attr, $a = and $a 
	- "fsDir filter" - list dir with name contain filter (e.g .gif)
	- "fsDirSize filter" - number of files in dir contains filter (e.g. .gif)
	- "fsDirName filter index type" - get file match filter, in dir at index (e.g. .gif,0 => first gif-file) (type<=0 =>name,type=1=>size)
	- add paramCalc to all parameter (e.g log 1 + 1 )
	
=======
- "end" - cmdEnd prg execute
- "error" - cmdError message => end prg with message
- end of line detect by ';' or '\n' or '\r' or '\0'
- change to param-pointer (char* appCmd(char *cmd, char *param) { .. })
- "if RULE cmd" on true => execute cmd or {}
	- if inside if NOT supported yet
- "elseif RULE cmd" on true => execute cmd or {}
- "else" => execute cmd or {}
- "{} until RUILE"  repeat sub (e.g. {} until RUILE }
- cmdAttr execute attr, $a = and $a 
- "fsDir filter" - list dir with name contain filter (e.g .gif)
- "fsDirSize filter" - number of files in dir contains filter (e.g. .gif)
- "fsDirName filter index type" - get file match filter, in dir at index (e.g. .gif,0 => first gif-file) (type<=0 =>name,type=1=>size)
- add paramCalc to all parameter (e.g log 1 + 1 )

  
>>>>>>> 9feedd0439336a51f79b4228f39bc4a170bf23d4
TODO:
	- if inside if
