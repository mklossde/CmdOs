
CmdOS V.0.2.0 - 17.02.2025
    - "end" - cmdEnd prg execute
	- "error" - cmdError(message) end prg with message
	- end of line detect by ';' or '\n' or '\r' or '\0'
	- change to param-pointer (char* appCmd(char *cmd, char *param) { .. })
	- if RULE cmd: on true => execute cmd or {}
	- elseif RULE cmd: on true => execute cmd or {}
	- else => execute cmd or {}
	- until RUILE:  repeat sub (e.g. {} until RUILE }
	
		