
// Serial Command Line

unsigned long *cmdTime = new unsigned long(0);
int _skipCmd=0; // >0 => number of cmd to skip
char LINEEND=';';

#define maxInData 150 // max line length
char inData [maxInData]; // read buffer
char inIndex = 0; // read index
char prgLine [maxInData]; // prgLine buffer


char* appInfo() {
   sprintf(buffer,"AppInfo %s %s CmdOs:%s bootType:%s login:%d access_level:%d",
      prgTitle,prgVersion,cmdOS,bootType,_isLogin,eeBoot.accessLevel); return buffer;
}

//------------------------------------------------------------------------------------------------

char* cmdInfo() {
  return "?|esp|stat save|load|rset|restart|set[ NAME PAS]|wifi[ SSID PAS]|scan|mqtt[ MQTTURL]|ping IP|DNS IP|sleep[ MODE TIME]|setup|time[ MS]|mode[ MODE]";
}

/* login (isAdmin=true) */
boolean cmdLogin(char *p) {
  if(login(p)) { logPrintln(LOG_SYSTEM,"login admin"); return true; }
  else { logPrintln(LOG_SYSTEM,"logout admin"); return false; }
}

// execute cmd 
char* cmdExec(char *cmd, char *p0, char *p1,char *p2,char *p3,char *p4,char *p5,char *p6,char *p7,char *p8,char *p9) {
  if(cmd==NULL || sizeof(cmd)==0) { return EMPTY; } 
  sprintf(buffer,"CMD :%s:%s:%s:%s:%s:%s:%s:%s:%s:",cmd,p0,p1,p2,p3,p4,p5,p6,p7,p8,p9); logPrintln(LOG_DEBUG,buffer); 

  if(equals(cmd, "?")) { return cmdInfo(); }
  else if(equals(cmd, "esp")) { return espInfo();  }// show esp status
  else if(equals(cmd, "stat")) { return appInfo(); }// show esp status

  else if(equals(cmd, "freeHeap")) { sprintf(buffer,"%d",ESP.getFreeHeap());return buffer; }// show free heap
  
  else if(equals(cmd, "login")) { cmdLogin(p0); return EMPTY;}
  else if(equals(cmd, "restart")) { espRestart("cmd restart"); return EMPTY; }// restart
  else if(equals(cmd, "sleep")) {  sleep(p0,p1); return EMPTY; }      // sleep TIMEMS MODE (e.g. sleep 5000 0) (TIMEMS=0=>EVER) (MODE=0=>WIFI_OFF)

  else if(equals(cmd, "attr")) { attrSet(p0,p1); return attrInfo(); }
  else if(equals(cmd, "attrDel")) { attrDel(p0); return EMPTY;  }
  else if(equals(cmd, "attrClear")) { attrClear(p0); return EMPTY;  }

  else if(equals(cmd, "wait")) { cmdWait(toULong(p0)); return EMPTY; }// wait for next exec
  else if(equals(cmd, "goto")) { boolean ok=cmdGoto(p0); sprintf(buffer,"%d",ok); return buffer; }// goto prg label or skip n steps
  else if(equals(cmd, "=")) { return cmdSet(p0,p1,p2); }// return p0p1p2
  else if(equals(cmd, "if")) { boolean ok=cmdIf(p0,p1,p2,p3); sprintf(buffer,"%d",ok); return buffer; }// if p0 <=> p2 => skip p4
  else if(equals(cmd, "random")) { int r=random(toInt(p0),toInt(p1));  sprintf(buffer,"%d",r); return buffer;  } // random min-max
  else if(equals(cmd,"extract")) { return extract(p0,p1,p2); } // extract start end str (e.g  "free:"," " from "value free:1000 colr:1" => 1000)
  else if(equals(cmd, "reset")) { return bootReset(p0); }// reset eeprom and restart    

  else if(equals(cmd, "setup") && isAccess(ACCESS_ADMIN)) { return setupEsp(p0,p1,p2,p3,p4); }// setup wifi-ssid wifi-pas espPas => save&restart
  else if(equals(cmd, "setupDev") && isAccess(ACCESS_ADMIN)) { return setupDev(p0); } // enable/disable setupDevices
  
  else if(equals(cmd, "log")) { sprintf(buffer,"%s %s %s %s %s %s %s %s",p0,p1,p2,p3,p4,p5,p6,p7); logPrintln(LOG_INFO,buffer); return EMPTY;}// log
  else if(equals(cmd, "logLevel")) { return setLogLevel(toInt(p0)); }  // set mode (e.g. "mode NR")

  else if(equals(cmd, "save")) { bootSave(); return EMPTY; }// write data to eeprom
  else if(equals(cmd, "load")) { bootRead(); return EMPTY;  }// load data from eprom
  else if(equals(cmd, "conf")) {  return bootSet(p0,p1,p3); }      // set esp name and password (e.g. "set" or "set NAME PAS")  
  else if(equals(cmd,"access")) { setAccessLevel(toInt(p0)); return EMPTY; } // set  AccessLevel (e.g. "access 5")
  else if(equals(cmd, "wifi")) { return wifiSet(p0,p1); }      // set wifi, restart wifi and info (e.g. "wifi" or "wifi SSID PAS")  
  else if(equals(cmd, "scan")) {  return wifiScan(); }         // scan wifi (e.g. "scan")
  else if(equals(cmd, "time")) { return timeSet(p0,p1); }       // set time (e.g. "time" or "time TIMEINMS")
  else if(equals(cmd, "mode")) { return bootMode(toInt(p0)); }  // set mode (e.g. "mode NR")
  
  else if(equals(cmd, "ping")) {  return cmdPing(p0); }         // wifi ping  (e.g. "ping web.de")
  else if(equals(cmd, "dns")) {  return netDns(p0); }         // wifi dns resolve (e.g. "dns web.de")

  else if(equals(cmd, "mqtt")) { return mqttSet(p0);  }      // set mqtt (e.g. "mqtt" or "mqtt mqtt://admin:pas@192.168.1.1:1833")  
  else if(equals(cmd, "mqttLog") && isAccess(ACCESS_READ)) { eeBoot.mqttLogEnable=toBoolean(p0);  return EMPTY; } // enable/disbale mqttLog
  else if(equals(cmd,"mqttSend") && isAccess(ACCESS_CHANGE)) { publishTopic(p0,p1); return EMPTY; } // mqtt send topic MESSAGE
  else if(equals(cmd, "mqttConnect") && isAccess(ACCESS_READ)) { mqttInit(); return EMPTY; }
  else if(equals(cmd, "mqttDisconnect") && isAccess(ACCESS_READ)) { mqttDisconnect(); return EMPTY; }
  else if(equals(cmd, "mqttAttr") && isAccess(ACCESS_READ)) { mqttAttr(p0,toBoolean(p1)); return EMPTY; }
  
  else if(equals(cmd, "run")) { return cmdFile(p0); } // run prg from file 
  else if(equals(cmd, "stop")) { return prgStop(); } // stop/halt prg
  else if(equals(cmd, "continue")) { return prgContinue(); } // continue prg
  else if(equals(cmd, "next")) { return prgNext(p0); } // next prg step

  else if(equals(cmd, "fsDir") && isAccess(ACCESS_READ)) { return fsDir(); }
  else if(equals(cmd, "fsCat") && isAccess(ACCESS_READ)) { fsCat(toString(p0)); return EMPTY; }
  else if(equals(cmd, "fsWrite") && isAccess(ACCESS_CHANGE) ) { boolean ok=fsWrite(toString(p0),p1); return EMPTY;}
  else if(equals(cmd, "fsDel") && isAccess(ACCESS_CHANGE)) { fsDelete(toString(p0)); return EMPTY; }
  else if(equals(cmd, "fsRen") && isAccess(ACCESS_CHANGE)) { fsRename(toString(p0),toString(p1)); return EMPTY; }  
  else if(equals(cmd, "fsFormat") && isAccess(ACCESS_ADMIN)) { fsFormat(); return EMPTY; }

  else if(equals(cmd, "fsDownload") && isAccess(ACCESS_CHANGE)) { return fsDownload(toString(p0),toString(p1)); }
  else if(equals(cmd, "rest")) { return rest(p0); } // 
  else if(equals(cmd, "cmdRest")) { return cmdRest(p0); } // call http/rest and exute retur nbody as cmd

  // timer 1 0 -1 -1 -1 -1 -1 "drawLine 0 0 20 20 888"
  else if(equals(cmd, "timer") && isAccess(ACCESS_CHANGE)) { timerAdd(toBoolean(p0),toInt(p1),toInt(p2),toInt(p3),toInt(p4),toInt(p5),toInt(p6),p7); return EMPTY; }
  else if(equals(cmd, "timerDel") && isAccess(ACCESS_CHANGE)) { timerDel(toInt(p0)); return EMPTY; }
  else if(equals(cmd, "timerGet") && isAccess(ACCESS_READ)) { 
      MyEventTimer* timer=(MyEventTimer*)eventList.get(toInt(p0)); 
      if(timer!=NULL) { return timer->info(); } else{ return EMPTY; }
    }
  else if(equals(cmd, "timers") && isAccess(ACCESS_READ)) { timerLog(); return EMPTY; }

  else { return appCmd(cmd,p0,p1,p2,p3,p4,p5,p6,p7,p8,p9); }

}

//------------------------------------------------------------------------------------------------

char* _prg=NULL;
char *_prgPtr=NULL;
boolean prgLog=true; // show each prg step in log

unsigned long *_prgTime = new unsigned long(0);
unsigned long _cmdWait= -1; // wait before next cmd time in ms (-1=do not wait, 1000=1s)

/* wait in prg for time in ms, until next step */
void cmdWait(unsigned long cmdWait) { _cmdWait=cmdWait; *_prgTime=1; }

/** goto key in prg */
boolean cmdGoto(char *p0) { 
  if(_prg==NULL || p0==NULL) { return "goto prg/label NULL"; } 
  if(isInt(p0)) { _skipCmd=toInt(p0); return false;}

  char *findPtr=_prg;
  while(true) {
    char *find = strstr(findPtr, p0);
    if(find==NULL) { return "goto unkown"; }
    else if((find-1)[0]==LINEEND) { _prgPtr=find; return true;  }
    findPtr=find+1;   
  }   
  return false;
}

boolean cmdIf(char *a,char *match,char *b,char *gotoOnTrue) {
  int ai=toInt(a),bi=toInt(b);
  boolean ok=false;
  if(equals(match,"==") && ai==bi) { ok=true; }
  else if(equals(match,"!=") && ai!=bi) { ok=true; }
  else if(equals(match,">") && ai>bi) { ok=true; }
  else if(equals(match,">=") && ai>=bi) { ok=true; }
  else if(equals(match,"<") && ai<bi) { ok=true; }
  else if(equals(match,"<=") && ai<=bi) { ok=true; }

  if(is(gotoOnTrue)) {
    if(ok) { if(isInt(gotoOnTrue)) { return true;} else { return cmdGoto(gotoOnTrue); } }
    else { if(isInt(gotoOnTrue)) { _skipCmd=toInt(gotoOnTrue); return false; } else { return false; } }
    
  }else {
    if(ok) { return true; }
    else {_skipCmd=1; return false; }
  }  
}

char* cmdSet(char *a,char *b,char *c) {
  if(equals(b,"+")) { int s=toInt(a)+toInt(c); sprintf(buffer,"%d",s); }
  else if(equals(b,"-")) { int s=toInt(a)-toInt(c); sprintf(buffer,"%d",s); }
  else if(equals(b,"*")) { int s=toInt(a)*toInt(c); sprintf(buffer,"%d",s); }
  else if(equals(b,"/")) { int s=toInt(a)/toInt(c); sprintf(buffer,"%d",s); }
  else {  sprintf(buffer,"%s",a);  }
  return buffer;
}

//------------------------------------

char* attrGet(char *p) {  return (char*)attrMap.get(p); }
void attrSet(char *key,String value) {    if(is(key)) { char *v=(char*)value.c_str(); attrMap.replace(key,v,strlen(v));} }
void attrSet(char *key,char *value) {  if(is(key)) {attrMap.replace(key,value,strlen(value)); } }
void attrDel(char *key) { attrMap.del(key); }
char* attrInfo() {
   sprintf(buffer,"");
  for(int i=0;i<attrMap.size();i++) {  
    char *key=attrMap.key(i);
    char *value=(char*)attrMap.get(i);
    if(is(key) && is(value)) {
      sprintf(buffer+strlen(buffer),"attr %s \"%s\"\n",key,value);
    }
  }
  return buffer;
}
void attrClear(char *prefix) { attrMap.clear(prefix); }

/* read next param */
char* cmdParma(char **pp) {
    if(pp==NULL || *pp==NULL || **pp=='\0') { return EMPTY; }
    while(**pp==' ' || **pp=='\t') { (*pp)++; } // skip spaces and tabs
    
    char* p1;
    if(pp==NULL || *pp==NULL || **pp=='\0') { return EMPTY; }
    else if(**pp=='"') { // read "param"
      (*pp)++; // skip first "
      p1 = strtok_r(NULL, "\"",pp); 
    }else if(**pp=='$') { // attribute
      (*pp)++; // skip first $
      p1 = strtok_r(NULL, " ",pp); 
      p1=attrGet(p1);
    }else {
      p1 = strtok_r(NULL, " ",pp);  
    }
    if(p1==NULL) { return EMPTY; } 
    return p1;
}


/* parse and execute a cmd line */
char* cmdLine(char* line) {
  char *key=NULL;
  char *ptr;
  char *command = strtok_r(line, " ",&ptr);
  if(command==NULL) { return EMPTY; } // no command 
  else if(_skipCmd>0 || command[0]=='#') { _skipCmd--;  return "skip"; } // skip line or comment 
  else if(command[0]=='$') { key=command+1; command=cmdParma(&ptr);  }

  char *p0=EMPTY,*p1=EMPTY,*p2=EMPTY,*p3=EMPTY,*p4=EMPTY,*p5=EMPTY,*p6=EMPTY,*p7=EMPTY,*p8=EMPTY,*p9=EMPTY;
  if(ptr!=NULL) { p0=cmdParma(&ptr);}
  if(ptr!=NULL) { p1=cmdParma(&ptr);}
  if(ptr!=NULL) { p2=cmdParma(&ptr);}
  if(ptr!=NULL) { p3=cmdParma(&ptr);}
  if(ptr!=NULL) { p4=cmdParma(&ptr);}
  if(ptr!=NULL) { p5=cmdParma(&ptr);}
  if(ptr!=NULL) { p6=cmdParma(&ptr);}
  if(ptr!=NULL) { p7=cmdParma(&ptr);}
  if(ptr!=NULL) { p8=cmdParma(&ptr);}
  if(ptr!=NULL) { p9=cmdParma(&ptr);}

  char *ret=cmdExec(command, p0, p1,p2,p3,p4,p5,p6,p7,p8,p9);
  if(key!=NULL) { attrSet(key,ret);  } 

  logPrintln(LOG_DEBUG,ret);  // show return
  return ret;
}

/* get next line as a copy of prg  */
char *nextCmd() {
  if(_prgPtr==NULL) { return NULL; }
  else if(*_prgPtr=='\0') { return NULL; } // end found
  char *line_end = strchr(_prgPtr, ';');
  if(line_end!=NULL) { 
    int len=line_end-_prgPtr;
    if(len<=0) { _prgPtr+=len+1; return EMPTY; }
    else if(len>=maxInData-1) { len=maxInData-1; }
    memcpy( prgLine, _prgPtr, len); prgLine[len]='\0';
    _prgPtr+=len+1; // set next pos
    return prgLine;
  }else {
    int len=strlen(_prgPtr);
    memcpy( prgLine, _prgPtr, len+1); //prgLine[len]='\0';
    _prgPtr+=len; // set end pos
    return prgLine;
  }
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
  if(prg==NULL) { return "prg missing"; }
  _prg=prg;
  replace(_prg,'\r',';'); // 
  replace(_prg,'\n',';'); // newLine => cmd End  
  _prgPtr=_prg;  // set ptr to prg start
  return prgLoop();   
}

char* cmdFile(char* p0) {
  if(_prg!=NULL) { delete[] _prg; _prg=NULL; _prgPtr=NULL; } // clear old prg
  String name=toString(p0);  

  char* prg = fsRead(name);
  if(prg==NULL) { return "cmdFile missing "; }
  else { char* ret=cmdPrg(prg); return ret; }
}

//-----------------

/* call http/rest and execute return body as cmd */
char* cmdRest(char *url) {
  char* ret=rest(String(url));  
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
    else { // RETURN or maxlength 
      inData[inIndex++] = '\0';
      String ret=cmdLine(inData);
      logPrintln(LOG_SYSTEM,ret);
      inIndex = 0;
    }
  }
}

//--------------------------

// exec "startup.cmd" afer 10s
unsigned long *cmdStartTime = new unsigned long(1);
int startupWait=10000; // wait before startup.cmd

void cmdLoop() {
  // serial in
  if(cmdEnable && isTimer(cmdTime, 10)) { cmdRead(); } // exec cmd 
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
