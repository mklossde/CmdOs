
//--------------------------------------------------------------------------------------
// Serial Command Line

unsigned long *cmdTime = new unsigned long(0);
int _skipCmd=0; // >0 => number of cmd to skip
char LINEEND=';';


char prgLine [maxInData]; // prgLine buffer

char* _prg=NULL;
char *_prgPtr=NULL;
boolean prgLog=true; // show each prg step in log
boolean _lastIf=false;

char* appInfo() {
   sprintf(buffer,"AppInfo %s %s CmdOs:%s bootType:%s login:%d access_level:%d",
      prgTitle,prgVersion,cmdOS,bootType,_isLogin,eeBoot.accessLevel); return buffer;
}

// $a =
// attr $a =
// $a CMD

//------------------------------------------------------------------------------------------------

/* convert param to line in buffer */
char* paramToLine(char *param) {
  sprintf(buffer,"%s %s %s %s %s %s %s %s",
    cmdParam(&param),cmdParam(&param),cmdParam(&param),cmdParam(&param),cmdParam(&param),cmdParam(&param),cmdParam(&param),cmdParam(&param)); 

  return buffer;
}

char* cmdInfo() {
  return "?|esp|stat save|load|rset|restart|set[ NAME PAS]|wifi[ SSID PAS]|scan|mqtt[ MQTTURL]|ping IP|DNS IP|sleep[ MODE TIME]|setup|time[ MS]|mode[ MODE]";
}

/* login (isAdmin=true) */
boolean cmdLogin(char *p) {
  if(login(p)) { logPrintln(LOG_SYSTEM,"login admin"); return true; }
  else { logPrintln(LOG_SYSTEM,"logout admin"); return false; }
}

// execute cmd 
char* cmdExec(char *cmd, char **param) {  
  if(!is(cmd)) { return EMPTY; } 
  sprintf(buffer,"->%s %s",cmd,to(*param)); logPrintln(LOG_DEBUG,buffer); 

  if(equals(cmd, "}")) { cmd=nextParam(param); } 

  char *ret= EMPTY;
  if(_skipCmd>0 || cmd[0]=='#') { _skipCmd--;  sprintf(buffer,"skip %s",cmd); logPrintln(LOG_DEBUG,buffer);  ret=cmd; } // skip line or comment   
  else if(cmd[0]=='$') { 
    if(!equals(nextParam(param), "=")) { cmdError("ERROR missing = after attr"); }
    else { ret=cmdSet(cmd,param);  }
  }

  else if(equals(cmd, "?")) { ret=cmdInfo(); }
  else if(equals(cmd, "esp")) { ret=espInfo();  }// show esp status
  else if(equals(cmd, "stat")) { ret=appInfo(); }// show esp status
  
  else if(equals(cmd, "login")) { cmdLogin(cmdParam(param)); }
  else if(equals(cmd, "restart")) { espRestart("cmd restart");  }// restart
  else if(equals(cmd, "sleep")) {  sleep(cmdParam(param),cmdParam(param));  }      // sleep TIMEMS MODE (e.g. sleep 5000 0) (TIMEMS=0=>EVER) (MODE=0=>WIFI_OFF)

  else if(equals(cmd, "attr")) { ret=cmdSet(nextParam(param),param); }
  else if(equals(cmd, "attrDel")) { attrDel(cmdParam(param));  }
  else if(equals(cmd, "attrClear")) { attrClear(cmdParam(param));  }
  else if(equals(cmd, "attrs")) { ret=attrInfo(); }

  else if(equals(cmd, "wait")) { cmdWait(toULong(cmdParam(param))); }// wait for next exec
  else if(equals(cmd, "exec")) { cmdExec(cmdParam(param),param); }// exec 
  else if(equals(cmd, "goto")) { ret=to(cmdGoto(_prg,cmdParam(param))); }// goto prg label or skip n steps
//  else if(equals(cmd, "=")) { ret=cmdSet(cmdParam(param),cmdParam(param),cmdParam(param)); }// return p0p1p2
  else if(equals(cmd, "if")) { ret=to(cmdIf(param));  }// if p0 <=> p2 => { }
  else if(equals(cmd, "elseif")) { ret=to(cmdElseIf(param)); }// else if p0 <=> p2 => { }
  else if(equals(cmd, "else")) { ret=to(cmdElse(param)); }// else => {}
  else if(equals(cmd, "until")) { ret=to(cmdUntil(param)); }// {} until p0 <=> p2 

  else if(equals(cmd, "random")) { int r=random(toInt(cmdParam(param)),toInt(cmdParam(param)));  sprintf(buffer,"%d",r); ret=buffer;  } // random min-max
  else if(equals(cmd,"extract")) { ret=extract(cmdParam(param),cmdParam(param),cmdParam(param)); } // extract start end str (e.g  "free:"," " from "value free:1000 colr:1" => 1000)
  else if(equals(cmd, "reset")) { ret=bootReset(cmdParam(param)); }// reset eeprom and restart    

  else if(equals(cmd, "setupDev") && isAccess(ACCESS_ADMIN)) { ret=setupDev(cmdParam(param)); } // enable/disable setupDevices
  else if(equals(cmd, "setup") && isAccess(ACCESS_ADMIN)) { ret=setupEsp(cmdParam(param),cmdParam(param),cmdParam(param),cmdParam(param),cmdParam(param)); }// setup wifi-ssid wifi-pas espPas => save&restart  

  else if(equals(cmd, "logLevel")) { ret=setLogLevel(toInt(cmdParam(param))); }  // set mode (e.g. "mode NR")
  else if(equals(cmd, "log")) { ret=paramToLine(*param); logPrintln(LOG_INFO,ret); } 

  else if(equals(cmd, "save")) { bootSave();  }// write data to eeprom
  else if(equals(cmd, "load")) { bootRead(); }// load data from eprom
  else if(equals(cmd, "conf")) {  ret=bootSet(cmdParam(param),cmdParam(param),cmdParam(param)); }      // set esp name and password (e.g. "set" or "set NAME PAS")  
  else if(equals(cmd,"access")) { setAccessLevel(toInt(cmdParam(param)));  } // set  AccessLevel (e.g. "access 5")
  else if(equals(cmd, "wifi")) { ret=wifiSet(cmdParam(param),cmdParam(param)); }      // set wifi, restart wifi and info (e.g. "wifi" or "wifi SSID PAS")  
  else if(equals(cmd, "scan")) {  ret=wifiScan(); }         // scan wifi (e.g. "scan")
  else if(equals(cmd, "time")) { ret=timeSet(cmdParam(param),cmdParam(param)); }       // set time (e.g. "time" or "time TIMEINMS")
  else if(equals(cmd, "mode")) { ret=bootMode(toInt(cmdParam(param))); }  // set mode (e.g. "mode NR")
  
  else if(equals(cmd, "ping")) {  ret=cmdPing(cmdParam(param)); }         // wifi ping  (e.g. "ping web.de")
  else if(equals(cmd, "dns")) {  ret=netDns(cmdParam(param)); }         // wifi dns resolve (e.g. "dns web.de")
   
  else if(equals(cmd, "mqttLog") && isAccess(ACCESS_READ)) { eeBoot.mqttLogEnable=toBoolean(cmdParam(param));   } // enable/disbale mqttLog
  else if(equals(cmd, "mqttPublish") && isAccess(ACCESS_USE)) { mqttPublish(cmdParam(param),cmdParam(param));  } // mqtt send topic MESSAGE
  else if(equals(cmd, "mqttConnect") && isAccess(ACCESS_READ)) { mqttOpen(toBoolean(cmdParam(param)));  }
  else if(equals(cmd, "mqttAttr") && isAccess(ACCESS_USE)) { mqttAttr(cmdParam(param),cmdParam(param));  }
  else if(equals(cmd, "mqttDel") && isAccess(ACCESS_USE)) { mqttDel(cmdParam(param));  }
  else if(equals(cmd, "mqtt")) { ret=mqttSet(cmdParam(param));  }      // set mqtt (e.g. "mqtt" or "mqtt mqtt://admin:pas@192.168.1.1:1833") 

  else if(equals(cmd, "params") && isAccess(ACCESS_USE)) { ret=paramsInfo();  }
  else if(equals(cmd, "paramsClear") && isAccess(ACCESS_USE)) { paramsClear(toInt(cmdParam(param)));  }

  else if(equals(cmd, "run")) { ret=cmdFile(cmdParam(param)); } // run prg from file 
  else if(equals(cmd, "end")) { ret=cmdFile(NULL); }// stop prg
  else if(equals(cmd, "stop")) { ret=prgStop(); } // stop/halt prg
  else if(equals(cmd, "continue")) { ret=prgContinue(); } // continue prg
  else if(equals(cmd, "next")) { ret=prgNext(cmdParam(param)); } // next prg step
  else if(equals(cmd, "error")) { cmdError(paramToLine(*param));  }// end prg with error

  else if(equals(cmd, "fsDir") && isAccess(ACCESS_USE)) { ret=fsDir(toString(cmdParam(param))); }
  else if(equals(cmd, "fsDirSize") && isAccess(ACCESS_USE)) { int count=fsDirSize(toString(cmdParam(param))); sprintf(buffer,"%d",count); ret=buffer; }
  else if(equals(cmd, "fsFile") && isAccess(ACCESS_USE)) { ret=fsFile(toString(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param))); }
  else if(equals(cmd, "fsCat") && isAccess(ACCESS_READ)) { fsCat(toString(cmdParam(param)));  }
  else if(equals(cmd, "fsWrite") && isAccess(ACCESS_CHANGE) ) { boolean ok=fsWrite(toString(cmdParam(param)),cmdParam(param)); }
  else if(equals(cmd, "fsDel") && isAccess(ACCESS_CHANGE)) { fsDelete(toString(cmdParam(param))); }
  else if(equals(cmd, "fsRen") && isAccess(ACCESS_CHANGE)) { fsRename(toString(cmdParam(param)),toString(cmdParam(param)));  }  
  else if(equals(cmd, "fsFormat") && isAccess(ACCESS_ADMIN)) { fsFormat();  }

  else if(equals(cmd, "fsDownload") && isAccess(ACCESS_CHANGE)) { ret=fsDownload(toString(cmdParam(param)),toString(cmdParam(param)),toInt(cmdParam(param))); }
  else if(equals(cmd, "rest") && isAccess(ACCESS_USE)) { ret=rest(cmdParam(param)); } // 
  else if(equals(cmd, "cmdRest") && isAccess(ACCESS_USE)) { ret=cmdRest(cmdParam(param)); } // call http/rest and exute retur nbody as cmd

  else if(equals(cmd, "ledInit") && isAccess(ACCESS_ADMIN)) { ret=ledInit(toInt(cmdParam(param)),toBoolean(cmdParam(param))); } 
  else if(equals(cmd, "led") && isAccess(ACCESS_USE)) { ret=ledSwitch(cmdParam(param),cmdParam(param)); }
  else if(equals(cmd, "swInit") && isAccess(ACCESS_ADMIN)) { ret=swInit(toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param))); } //
  else if(equals(cmd, "swCmd") && isAccess(ACCESS_USE)) { ret=swCmd(toInt(cmdParam(param)),cmdParam(param)); }

  // timer 1 0 -1 -1 -1 -1 -1 "drawLine 0 0 20 20 888"
  else if(equals(cmd, "timer") && isAccess(ACCESS_USE)) { timerAdd(toBoolean(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),cmdParam(param));  }
  else if(equals(cmd, "timerDel") && isAccess(ACCESS_USE)) { timerDel(toInt(cmdParam(param)));  }
  else if(equals(cmd, "timerGet") && isAccess(ACCESS_USE)) { 
      MyEventTimer* timer=(MyEventTimer*)eventList.get(toInt(cmdParam(param))); 
      if(timer!=NULL) { ret=timer->info(); } 
    }
  else if(equals(cmd, "timers") && isAccess(ACCESS_USE)) { timerLog();  }

  else { ret=appCmd(cmd,param); }

  logPrintln(LOG_DEBUG,ret);  // show return
  return ret;
}

//------------------------------------------------------------------------------------------------

unsigned long *_prgTime = new unsigned long(0);
unsigned long _cmdWait= -1; // wait before next cmd time in ms (-1=do not wait, 1000=1s)

/* wait in prg for time in ms, until next step */
void cmdWait(unsigned long cmdWait) { _cmdWait=cmdWait; *_prgTime=1; }


void cmdError(char *error) {  
  logPrintln(LOG_ERROR,error);
//  _prgptr=NULL; 
  *_prgTime=2; // stop program
}

/* end of line (NULL = no line)*/
char* lineEnd(char* ptr) {
  if(ptr==NULL || *ptr=='\0') { return NULL; }   
  while(*ptr!=';' && *ptr!='\n' && *ptr!='\r' && *ptr!='\0') { 
    ptr++;   
  }
  return ptr;
} 


/* goto key in prg */
boolean cmdGoto(char *findPtr,char *p0) { 
  if(findPtr==NULL) { cmdError("ERROR findPtr missing"); return false; } 
  else if(!is(p0)) { sprintf(buffer,"ERROR goto dest missing"); cmdError(buffer); return false; } 
  if(isInt(p0)) { _skipCmd=toInt(p0); return true;}

  char *find=findPtr;
  while(find!=NULL) {    
    if(startWith(find,p0)) { _prgPtr=find; return true;  }    
    find=lineEnd(find); 
    if(find!=NULL) { find++; }
  }  
  sprintf(buffer,"ERROR goto '%s' missing",p0); cmdError(buffer); return false;
}

/* find up start "{" of {..} in prg, start at prgPtr */
char* subStart(char *prgPtr) { 
  int sub=0;
  while(prgPtr!=_prg) { 
    if(*prgPtr=='{') { sub--; if(sub==0) { return prgPtr; }  }
    else if(*prgPtr=='}') { sub++; }
    prgPtr--;
  }
  return NULL;
} 

/* find down end "} of {..} in prg, start at prgPtr */
char* subEnd(char *prgPtr) { 
  if(prgPtr==NULL) { return NULL; }
  int sub=0;
  while(*prgPtr!='\0') { 
    if(*prgPtr=='{') { sub++; }
    else if(*prgPtr=='}') { if(sub==0) { return ++prgPtr; }  else if(sub>0) { sub--; } }
    prgPtr++;
  }
  return NULL;
} 

/* goto/skip until */
boolean gotoSubEnd(char *prgPtr) {
  char *end=subEnd(prgPtr);
  if(end==NULL) { cmdError("ERROR subEnd not found"); return false; } else { _prgPtr=end; return true;}
}

/* start sub on "ok" otherweise skip until }  */
boolean startSub(char *param,boolean ok) {  
  char *cmdOnTrue=nextParam(&param);

  if(equals(cmdOnTrue,"{")) {     
    if(ok)  { return true; } // on true => execute next line 
    else { return gotoSubEnd(_prgPtr); }
  }else {
    if(ok) { char* ret=cmdExec(cmdOnTrue,&param); return true; } // on true => execute cmd 
    else { return false; }
  }
}

/* repeat last sub  }  */
boolean repeatSub(char *param) {
  char *start=subStart(_prgPtr);
  if(start==NULL) { cmdError("ERROR repeatSub not found"); return false; } else { _prgPtr=start; return true;}
}



/* if RULE {} or else cmd */
boolean cmdUntil(char **param) {
  boolean ok=toBoolean(calcParam(param));
  if(!ok) { return repeatSub(*param); }
  else { return false; }
}

/* if RULE {} or else cmd */
boolean cmdIf(char **param) {  
  _lastIf=toBoolean(calcParam(param));
  return startSub(*param,_lastIf);
}

/* elseif RULE {} or else cmd */
boolean cmdElseIf(char **param) {  
  if(!_lastIf) { 
    _lastIf=toBoolean(calcParam(param));
}
  return startSub(*param,_lastIf);
}

/* else {} or else cmd */
boolean cmdElse(char **param) {  
  boolean startElse=!_lastIf; if(!_lastIf) { _lastIf=true; }  
  return startSub(*param,startElse);
}

//------------------------------------



boolean pIsNumber(char *param) {
  if(param==NULL) { return false; }
  else if(*param>='0' && *param<='9') { return true; }
  else if(*param=='+' || *param=='-') { return true; }
  else { return false; }
}

boolean pIsCalc(char *param) {
  if(param==NULL) {   
    return false; }
  if(*param=='<' || *param=='>' || *param=='=' || *param=='!') { return true; }
  else if(*param=='&' || *param=='|' ) { return true; }
  else if((*param=='+' || *param=='-') && (*(param+1)<'0' || *(param+1)>'9')) { return true; } // +- without number (e.g. -1) 
  else if(*param=='*' || *param=='/') { return true; }
  else { return false; }
}

//------------------------------------

/* set-attr KEY VALUE (e.g. $a = ..  or attr $a ... )  
    return VALUE or attrInfo (if KEY missing) 
*/
char* cmdSet(char *key,char **param) {
  char* ret=cmdParam(param);
  if(key!=NULL) {  attrSet(key,ret);  }   
  return ret;
}



//--------------------------------

int xCalc(int ai,char *calc,char **param) {   
  if(!is(calc)) { return ai;}
  else if(equals(calc,"++")) { return ai+1; }
  else if(equals(calc,"--")) { return ai-1; }

  int ret=0;
  char* bb=cmdParam(param);
  int bi=toInt(bb);   
  if(equals(calc,"==")) { ret=ai==bi; }
  else if(equals(calc,"!=")) { ret=ai!=bi; }
  else if(equals(calc,">")) { ret=ai>bi; }
  else if(equals(calc,">=")) { ret=ai>=bi; }
  else if(equals(calc,"<")) { ret=ai<bi; }
  else if(equals(calc,"<=")) { ret=ai<=bi; }

  else if(equals(calc,"+")) { ret=ai+bi; }
  else if(equals(calc,"-")) { ret=ai-bi; }
  else if(equals(calc,"*")) { ret=ai*bi; }
  else if(equals(calc,"/")) { ret=ai/bi; }

  else if(equals(calc,"||")) { ret=ai || bi; }
  else if(equals(calc,"&&")) { ret=ai && bi; }

  else if(equals(calc,">>")) { ret=ai >> bi; }
  else if(equals(calc,"<<")) { ret=ai << bi; }

  else { sprintf(buffer,"ERROR unkown calc '%s'",calc); cmdError(buffer); ret=0; }
  return ret;
}

int calcParam(char **param) { return calcParam(cmdParam(param),param); }
int calcParam(char *val,char **param) {
  int a=toInt(val);
  cmdParamSkip(param); // skip spaces
  while(pIsCalc(*param))  {    
    char *calc=cmdParam(param);       
    a=xCalc(a,calc,param);
    cmdParamSkip(param); // skip spaces
  }
  return a;
}

void cmdParamSkip(char **pp) {
    if(pp==NULL || *pp==NULL || **pp=='\0') { return ; }    
    while(**pp==' ' || **pp=='\t') { (*pp)++; } // skip spaces and tabs    
}

/* read next param */
char* cmdParam(char **pp) {
    cmdParamSkip(pp);
    if(pp==NULL || *pp==NULL || **pp=='\0') { return EMPTY; }

    char* p1;    
    if(**pp=='"') { // read string "param"
      (*pp)++; // skip first "
      p1 = strtok_r(NULL, "\"",pp);  
      if(p1==NULL) { return EMPTY; }   
      return p1;   

    }else if(**pp=='$') { // attribute
      (*pp)++; // skip first $
      p1 = strtok_r(NULL, " ",pp); 
      p1=attrGet(p1);
    }else if(**pp=='~') { // sysAttribute
      (*pp)++; // skip first $
      p1 = strtok_r(NULL, " ",pp); 
      p1=sysAttr(p1);
    }else if(pIsNumber(*pp) || pIsCalc(*pp)) {
        p1 = strtok_r(NULL, " ",pp);  
    }else { 
      p1 = strtok_r(NULL, " ",pp);            
      if(is(p1)) { p1=cmdExec(p1, pp); }
    }

    if(p1==NULL) { return EMPTY; } 
    cmdParamSkip(pp); // skip spaces
    if(pIsCalc(*pp)) { // next param calc 
      sprintf(buffer,"  calc after %s",p1); logPrintln(LOG_DEBUG,buffer);
      int ret=calcParam(p1,pp);
      sprintf(paramBuffer,"%d",ret);
      p1=paramBuffer;
    }
    return p1;
}

char* nextParam(char **pp) {
    if(pp==NULL || *pp==NULL || **pp=='\0') { return EMPTY; }
    while(**pp==' ' || **pp=='\t' || **pp=='}') { (*pp)++; } // skip spaces and tabs
    if(pp==NULL || *pp==NULL || **pp=='\0') { return EMPTY; }
    char* cmd = strtok_r(NULL, " ",pp); 
    if(cmd==NULL) { return EMPTY; } else { return cmd; }
}

/* parse and execute a cmd line */
char* cmdLine(char* line) {  
  char* cmd=nextParam(&line);
  return cmdExec(cmd,&line);
}

//--------------------------------

/* get next line as a copy of prg  */
char *nextCmd() {
  if(_prgPtr==NULL) { return NULL; }

  char *line_end=lineEnd(_prgPtr);
  if(line_end==NULL)  { return NULL; }
  int len=line_end-_prgPtr;
  if(len<=0) { _prgPtr+=len+1; return EMPTY; }
  memcpy( prgLine, _prgPtr, len); prgLine[len]='\0'; 
  _prgPtr+=len; // set next pos
  return prgLine;
}

/* loop next prg line */
char* prgLoop() {
  if(_prg==NULL || _prgPtr==NULL) { return "prg missing"; }
  char *line = nextCmd();  
  while(line==EMPTY) { line = nextCmd(); } // skip empty lines 
  if(line!=NULL) {  return cmdLine(line); }
  else {  
    logPrintln(LOG_DEBUG,"PRG end"); 
    if(_prg!=NULL) { delete[] _prg; _prg=NULL; _prgPtr=NULL; }  
    return "prg end";  
  }
}

/* next n steps */
char* prgNext(char *p0) {
  int steps=toInt(p0);
  if(steps<=1) { return prgLoop(); }

  char* ret=EMPTY;  
  int count=0;
  while(_prgPtr!=NULL && count++<steps) {
    ret=prgLoop();
    if(prgLog){ logPrintln(LOG_DEBUG,ret); }
  }
  return ret;
}  

char* prgContinue() { *_prgTime=0; return "continue";}
char* prgStop() { *_prgTime=2; return "stop"; }

char* cmdPrg(char* prg) {   
  if(_prg!=NULL) { delete[] _prg; _prg=NULL; _prgPtr=NULL; } // clear old prg
  if(!is(prg)) { return "prg missing"; }
  _prg=prg;
  _prgPtr=_prg;  // set ptr to prg start
  *_prgTime=0; // set prgTime to run
  return prgLoop();   
}

char* cmdFile(char* p0) {
  if(_prg!=NULL) { delete[] _prg; _prg=NULL; _prgPtr=NULL; } // clear old prg
  if(!is(p0)) { return "end"; }

  String name=toString(p0);  
  char* prg = fsRead(name);
  if(prg==NULL) { return "cmdFile missing "; }
  else { char* ret=cmdPrg(prg); return ret; }
}

//-----------------

/* call http/rest and execute return body as cmd */
char* cmdRest(char *url) {
  char* ret=rest(toString(url));  
  if(!is(ret)) { return NULL; } 
  sprintf(buffer,"cmdRest %s",ret); logPrintln(LOG_DEBUG,buffer);
  return cmdPrg(ret);
}

//-----------------

// read serial in as cmd
void cmdRead() {
    if (Serial.available() > 0) {
    char c = Serial.read();
    if (c != '\n' && c != '\r' && inIndex < maxInData-1) { inData[inIndex++] = c; } // read char 
    else if(inIndex>0) { // RETURN or maxlength 
      inData[inIndex++] = '\0';
      char* ret=cmdLine(inData);
      if(logLevel!=LOG_DEBUG) { logPrintln(LOG_SYSTEM,ret); }      
      inIndex = 0;
    }
  }
}

void cmdRead(char c) {

}

//--------------------------

// exec "startup.cmd" afer 10s
unsigned long *cmdStartTime = new unsigned long(1);
int startupWait=10000; // wait before startup.cmd

void cmdLoop() {
  // serial in
  if(serialEnable && isTimer(cmdTime, 10)) { cmdRead(); } // exec cmd 
  // prg
  if(eeMode<EE_MODE_SYSERROR) {
    if(_prgPtr!=NULL && isTimer(_prgTime, _cmdWait)) {  _cmdWait=0; prgLoop();  } 
  }
  // statup 
  if(eeMode==EE_MODE_START || eeMode==EE_MODE_OK) {
    if(_prg==NULL && isTimer(cmdStartTime, 10000)) { 
      if(fsSize("/startup.cmd")>0) { cmdFile("/startup.cmd");  } // exec startup-file cmd 
      *cmdStartTime=2;
    }
  }
}

