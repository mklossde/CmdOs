
#include <WiFi.h>
#include <EEPROM.h>       // EEprom read/write

#include <time.h>         // time 
#include <sys/time.h>     // time

/* cmdOS from openON.org develop by mk@almi.de */
const char *cmdOS="V.0.2.1-SNAPSHOT";
char *APP_NAME_PREFIX="CmdOs";

String appIP="";
#define MAX_DONWLOAD_SIZE 10000

/* on init auto find wifi set_up (password set_up) and connect */
#define wifi_setup "set_up"

//-----------------------------------------------------------------------------
// new [] => delete[]
// NEW => delete
// malloc() or calloc() => free

#define EE_MODE_FIRST 0 // First init => EEPROM Wrong
#define EE_MODE_SETUP 1 // EEInit / wifi Setup mode
#define EE_MODE_AP 2 // EEInit / wifi AP mode

#define EE_MODE_WIFI_OFF 10             // OFF 
#define EE_MODE_PRIVAT 20              // Private 
#define EE_MODE_WIFI_TRY 21          // Wifi first client access try 
#define EE_MODE_WIFI_CL_RECONNECT 23    // Wifi is reconnetion (to a Router)

#define EE_MODE_OK 30  // MODE OK

#define EE_MODE_START 40  // MODE START
#define EE_MODE_WRONG 45  // MODE WRONG

#define EE_MODE_ERROR 50  // MODE ERROR , Enable only WIFI
#define EE_MODE_SYSERROR 51  // MODE ERROR , Enable only CMD


//-----------------------------------------------------------------------------
// access

#define ACCESS_ADMIN 1 // admin function 
#define ACCESS_CHANGE 2 // user function 
#define ACCESS_READ 3 // info function 
#define ACCESS_ALL 4 // general function

/* log Level */
#define LOG_SYSTEM 0
#define LOG_ERROR 2
#define LOG_INFO 5
#define LOG_DEBUG 10

/** actual log level **/
byte logLevel=LOG_INFO;

/** do have access */
bool isAccess(int requireLevel);

//-----------------------------------------------------------------------------

#define EEBOOTSIZE 500

char eeType[5];
byte eeMode=0;
int eeAppPos=0;

//-----------------------------------------------------------------------------

char* cmdLine(char* prg); // execute one line 
char* cmdPrg(char* prg); // execute a cmd-prg 
char* cmdFile(char* p0); // execute a cmd-file 

//-----------------------------------------------------------------------------
// char utils

#define valueMax 32
#define bufferMax 500
static char* buffer=new char[bufferMax]; // buffer for char/logging
static char* EMPTY="";
#define paramBufferMax 128
static char* paramBuffer=new char[paramBufferMax]; // buffer for params

static String EMPTYSTRING="";
//static String NOT_IMPLEMENTED="NOT IMPLEMENTED";

//-------------------------------------------------------------------------------------------------------------------
//  List

int minValueLen=11;

/* list of object and map of key=value */
class List {
private:
  int _index=0; int _max=0;
  void** _array=NULL; // contains values 
  char** _key=NULL; // contains keys
  int* _vsize=NULL; // contains alloc-size of each value
  boolean _isMap=false;

  void grow(int grow) {
    _max+=grow; 
    if(_array==NULL) {          
      _array = (void**) malloc(_max * sizeof(void*));
      if(_isMap) { _key = (char**) malloc(_max * sizeof(char*)); }
      _vsize = (int*) malloc(_max * sizeof(int));
  }else {
      _array = (void**)realloc(_array, _max * sizeof(void*)); 
      if(_isMap) { _key = (char**)realloc(_key, _max * sizeof(char*)); }    
      _vsize = (int*)realloc(_vsize, _max * sizeof(int));
    }
  }
  void growTo(int max,void *obj) {
    for(int i=_max;i<max;i++) { grow(1);_array[_max-1]=obj; }
  }

public:
  // map
  /* set by copy key and value and replace value on change */
  void replace(char *key,char *obj,int len) {
    if(!is(key)) { return ; } 
    int index=find(key);
    if(index==-1) {
      if(_index>=_max) { grow(1); }       
      int size=len; if(size<minValueLen) { size=minValueLen; }
      char* to=new char[size+1]; if(to==NULL) { espRestart("replace() memory error"); }
      if(len>0) { memcpy( to, obj, len); } 
      to[len]='\0'; 
      _key[_index]=copy(key); _array[_index]=to; _vsize[_index]=size;      
      sprintf(buffer,"set %d '%s'='%s' len:%d size:%d",_index,key,to,len,size); logPrintln(LOG_DEBUG,buffer);
      _index++;
    }else {
      void* old=(void*)_array[index];
      int oldSize=_vsize[index];
      if(oldSize<=len) {
        _array[index] = (void*)realloc(_array[index], len+1); if( _array[index]==NULL) { espRestart("map-replace memory error"); }
        _vsize[index]=len;        
      }         
      char* o=(char*)_array[index]; 
      if(len>0) { memcpy(o, obj, len); } o[len]='\0';
      sprintf(buffer,"replace '%s'='%s' len:%d oldSize:%d",key,o,len,oldSize); logPrintln(LOG_DEBUG,buffer);
    }
  }
  
  /* set key=value into list e.g. list.set("key",value); */
  void* set(char *key,void *obj) {  
    int index=find(key);   
    if(index>=0) { void* old=_array[index]; _array[index]=obj; return old; } // overwrite 
    else {
      if(_index>=_max) { grow(1); } 
      _key[_index]=copy(key); _array[_index]=obj; _index++;return NULL;
    }
  }  
  /* get key at index e.g. char *key=list.key(0); */
  char* key(int index) { if(index>=0 && index<_index) { return _key[index]; } else { return NULL; } }  
  /* get value with key e.g. char *value=(char*)list.get(key); */
  void* get(char *key) {  
    if(!is(key)) { return NULL; }
    for(int i=0;i<_index;i++) {  if(equals(_key[i],key)) { return _array[i]; } } 
    return NULL;
  } 
  /* del key=value e.g. char* old=list.del(key); */
  boolean del(char *key) { 
    if(!is(key)) { return false; }
    int index=find(key); if(index==-1) { return false; }
    del(index);        
    return true;
  }  
  /* find index of key e.g. int index=list.find(key); */
  int find(char *key) { 
    if(!is(key)) { return -1; }
    for(int i=0;i<_index;i++) {  if(equals(_key[i],key)) { return i; } } return -1; }

  // list ------------------------------------------------
  /* add object to list e.g. list.add(obj); */
  void add(void *obj) { if(_index>=_max) { grow(1); } _array[_index++]=obj; } 
  void addIndex(int index,void *obj) { 
//    if(index>=_max) { grow(index-_max+1); } 
    if(index>=_max) { growTo(index+1,NULL); }     
    _array[index]=obj; if(index>=_index) { _index=index+1; } } 
  /* get obejct at index e.g. char* value=(char*)list.get(0); */
  void* get(int index) { if(index>=0 && index<_index) { return _array[index]; } else { return NULL; } }  
  /* del object at index e.g. char* old=(char*)list.del(0); */
  void del(int index) {   
    if(index<0 || index>=_index) {return ; }      
    void *obj=_array[index]; if(obj!=NULL) { delete obj; } 
    if(_isMap) { void *oldKey=_key[index]; if(oldKey!=NULL) { delete oldKey; } }        
    for(int i=_index-2;i>=index;i--) { 
      _array[i]=_array[i+1]; 
      if(_isMap) { _key[i]=_key[i+1]; }
      _vsize[i]=_vsize[i+1];
    } 
    _index--;      
  }
  /* clear all (without prefix) / clear with prefix (e.g. clear my ) */
  void clear(char *prefix) { for(int i=_index;i>=0;i--) { if(!is(prefix) || startWith(key(i),prefix)) { del(i); }} }

  /* size of list e.g. int size=list.size(); */
  int size() { return _index; }
  List(int max) {  grow(max); }
  List() {   }
  List(boolean isMap) {  _isMap=isMap;  } // enable as map
  ~List() { delete _array; if(_isMap) { delete _key; } }

};

List attrMap(true); 

//-----------------------------------------------------------------------------

/* copy org* to new (NEW CHAR[]
    e.g. char* n=copy(old); 
*/
char* copy(char* org) { 
  if(org==NULL) { return NULL; }
  int len=strlen(org);
  char* newStr=new char[len+1]; 
  memcpy( newStr, org, len); newStr[len]='\0'; 
  return newStr;
}

/* create a copy of org with new char[max] (NEW CHAR[])*/
char* copy(char *to,char* org,int max) { 
  if(to==NULL) { to=new char[max+1]; }
  if(to==NULL) { espRestart("copy() memory error"); }
  if(org!=NULL) { 
    int len=strlen(org); if(len>max) { len=max; }
    memcpy( to, org, len); to[len]='\0'; 
  }else { to[0]='\0'; }
  return to;
}

/* copy (MALLOC) */
char* copy(char *to,String str,int max) { 
  if(to==NULL) { to = (char*)malloc((max + 1)*sizeof(char));  }     
  if(to==NULL) { espRestart("copy() memory error"); }
  if(str!=NULL) {         
//TODO take care on string len    
    strcpy(to, str.c_str()); 
  }
  return to;
}

char* copy(String str) {  
  if(str==NULL || str==EMPTYSTRING) { return NULL; } 
  char* s = (char*)malloc(str.length() + 1); 
  if(s==NULL) { espRestart("to() memory error"); }
  strcpy(s, str.c_str());
  return s;
}
char* copy(String str,char* def) {  
  if(str==NULL || str==EMPTYSTRING) { return def; } 
  int len  =str.length()+1; if(len==0) { return def; } char ca[len]; str.toCharArray(ca,len); return(ca);
}

//------------------------------------


/* replace all old_car with new_cahr in str 
    e.g. replace(str,' ','+');
*/
void replace(char *str, char old_char, char new_char) {
    if (str == NULL) { return; }
    while (*str != '\0') { // Iterate through the string until the null terminator
        if (*str == old_char) { *str = new_char; } // Replace the character
        str++; // Move to the next character
    }
}

/* return if str ends with find 
    e.g. if(endsWith(str,".gif"))
*/
boolean endsWith(char *str,char *find) {
  if(str==NULL || find==NULL) { return false; }
  int len=strlen(str);
  int findLend=strlen(find);
 return len >= findLend && strcmp(str + len - findLend, find) == 0;
}

/* return if str start with find 
    e.g. if(startWith(str,"/"))
*/
boolean startWith(char *str,char *find) {
  if(!is(str) || !is(find)) { return false; }
  //return strcmp(str, find) == 0;
  int l1=strlen(str); int l2=strlen(find);
  if(l1<l2) { return false; }
  for(int i=0;i<l2;i++) {  if(*str++!=*find++) { return false; } }
  return true;
}

/** extract from src (NEW char[]) */
char* extract(char *start, char *end, char *src) {
    const char *start_ptr = strstr(src, start); if (!start_ptr) { return NULL; }
    start_ptr += strlen(start);  // Move past 'start'
    const char *end_ptr = strstr(start_ptr, end); if (!end_ptr) { return NULL; }
    size_t len = end_ptr - start_ptr; 
    char *result=new char(len+1);
    strncpy(result, start_ptr, len);  result[len] = '\0';  
    return result;
}

/* validate is cstr equals to find  
    e.g. if(equals(cmd,"stat")) */
boolean equals(char *str,char *find) {
//sprintf(buffer,"equals '%s' '%s' ",to(str),to(find)); logPrintln(LOG_SYSTEM,buffer);
  if(!is(str) || !is(find)) { return false; }
  int l1=strlen(str); int l2=strlen(find);
//sprintf(buffer,"equals len '%d' '%d' ",l1,l2); logPrintln(LOG_SYSTEM,buffer);  
  if(l1!=l2) { return false; }
//  return strcmp(str, find)==0;
  for(int i=0;i<l2;i++) {  
//sprintf(buffer,"equals is %d '%s' '%s' => %d",i,str,find,(*str==*find)); logPrintln(LOG_SYSTEM,buffer);      
    if(*str!=*find) { return false; } 
    str++; find++;
  }
//sprintf(buffer,"equals found '%s' '%s' ",str,find); logPrintln(LOG_SYSTEM,buffer);   
  return true;
}

/* size/len of text  
    int len=size(text);
*/
int size(char *text) { if(text==NULL) { return -1; } else { return strlen(text); }}

/** insert at pos into buffer */
void insert(char* buffer,int pos,char* insertText) {
    size_t insertLen = strlen(insertText);
    size_t len = strlen(buffer);
    size_t newLen = insertLen + len;      
    // Shift existing text to the right
    memmove(buffer + pos + insertLen, buffer + pos , len - pos + 1);  // +1 for null terminator
    // Copy the prefix at the beginning
    memcpy(buffer+pos, insertText, insertLen);
} 

 
/*  validate if chars not NULL  
    e.g. if(is(text))
*/
boolean is(char *p) { return p!=NULL && p!=EMPTY; }
/*  validate if chars have size betwee >=min <max
    e.g. if(is(text,1,32))
*/
boolean is(char *p,int min,int max) { return p!=NULL && strlen(p)>=min && strlen(p)<max; }

boolean is(String str) { return (str!=NULL || str!=EMPTYSTRING); }
boolean is(String str,int min,int max) { if(str==NULL || str==EMPTYSTRING ) { return false; } int len=str.length(); return len>=min && len<max; }

/* convert to correct char */
char* to(byte d) { sprintf(buffer,"%d",d); return buffer; }
char* to(int d) { sprintf(buffer,"%d",d); return buffer; }
char* to(long d) { sprintf(buffer,"%d",d); return buffer; }
char* to(boolean d) { sprintf(buffer,"%d",d); return buffer; }
char* to(char *p) {if(p!=NULL && strlen(p)>0 && strlen(p)<bufferMax) { return p; } else { return EMPTY; } }
const char* to(const char *p) {if(p!=NULL && strlen(p)>0 && strlen(p)<bufferMax) { return p; } else { return EMPTY; } }

//char* to(const char *a) {  sprintf(buffer,"%s",to(a)); return buffer; }
char* to(char *a,char *b) { sprintf(buffer,"%s%s",to(a),to(b)); return buffer; }
char* to(const char *a, const char *b,const char *c) {  sprintf(buffer,"%s%s%s",to(a),to(b),to(c)); return buffer; }
char* to(const char *a, const char *b,const char *c,const char *d) {  sprintf(buffer,"%s%s%s%s",to(a),to(b),to(c),to(d)); return buffer; }
char* to(const char *a, const char *b,const char *c,const char *d,const char *e) {  sprintf(buffer,"%s%s%s%s%s",to(a),to(b),to(c),to(d),to(e)); return buffer; }

/* convert cahr* to string */
String toString(const char *text) {  if(!is(text)) { return EMPTYSTRING; } return String(text); }

boolean toBoolean(int i) { return i>0; }
/* convert char* to boolean */
//boolean toBoolean(char *p) { return p!=NULL && (strcmp(p, "on")==0 || strcmp(p, "true")==0 || strcmp(p, "1")==0); }
boolean toBoolean(char *p) { return p!=NULL && strlen(p)>0 && (strcmp(p, "on")==0 || strcmp(p, "true")==0 || atoi(p)>0); }
/* convert char* to int */
int toInt(char *p) { if(p!=NULL && strlen(p)>0) { return atoi(p); } else { return -1; } }
/* convert char* to double */
double toDouble(char *p) { if(p!=NULL && strlen(p)>0) { return atof(p); } else { return -1; } }
/* convert char* to long */
long int toLong(char *p) { if(p!=NULL && strlen(p)>0) { return atol(p); } else { return -1; } }
/* convert char* to unsigned long */
unsigned long toULong(char *p) { if(p!=NULL && strlen(p)>0) { return strtoul(p, NULL, 0); } else { return -1; } }

boolean isInt(char *p) {
  char *x=p;
  if (x==NULL || x==EMPTY || *x == '\0') { return false; } // Empty string is not a number
  else if (*x == '+' || *x == '-')  { x++; } // Handle optional sign
  while (*x) { if (!isdigit(*x)) { return false; } else { x++;} }// Non-digit character found
  return true;  // All characters are digits
}

boolean isBoolean(char *p) { 
  char *x=p;
  if (x==NULL || x==EMPTY || *x == '\0') { return false; } // Empty string is not a number
  if(*x=='t' || *x=='T' || *x=='f' || *x=='F') { return true; }
  return false;
}

//-----------------------------------------------------------------------------

/* append text1 and text2 => text1text2 (by use paramBuffer) 
    e.g. char* param=paramAppend("/",file)
*/
char* paramAppend(char *text1,char *text2) {  
  if(size(text1)+size(text2) >= paramBufferMax) { return NULL; }
  paramBuffer[0]= '\0';  
  if(is(text1)) { strcpy(paramBuffer, text1);  }
  if(is(text2)) { strcat(paramBuffer, text2); }
  return paramBuffer;
}

//-----------------------------------------------------------------------------
// ESP Tools

/* reboot esp 
    e.g. espRestart("restart after time")
*/
void espRestart(char* message) {  
  if(serialEnable) { Serial.print("### Rebooting "); Serial.println(message); delay(1000); }
  ESP.restart();
}

/* espChip ID */
uint32_t espChipId() {
    uint32_t chipId=0;
    for (int i = 0; i < 17; i = i + 8) { chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;}
    return chipId;
}

/* esp info */
char* espInfo() {
    sprintf(buffer,"ESP chip:%d free:%d core:%s freq:%d flashChipId:%d flashSize:%d flashSpeed:%d SketchSize:%d FreeSketchSpace:%d",    
      espChipId(),ESP.getFreeHeap(), ESP.getSdkVersion(),ESP.getCpuFreqMHz()
      ,ESP.getFlashChipMode(),ESP.getFlashChipSize(),ESP.getFlashChipSpeed(),ESP.getSketchSize(),ESP.getFreeSketchSpace());           
    return buffer;
}

/* enlabel info 
char* enableInfo() {
   sprintf(buffer,"serialEnable:%d cmdEnable:%d ledEnable:%d swEnable:%d wifiEnable:%d webEnable:%d updateEnable:%d mdnsEnable:%d mqttEnable:%d",
    serialEnable,cmdEnable,ledEnable,swEnable,wifiEnable,webEnable,updateEnable,mdnsEnable,mqttEnable);
   return buffer; 
}
*/

//-----------------------------------------------------------------------------
// Time

unsigned long _timeMs;           // actual time in mills (set by loop)
time_t timeNow;                     // e.g. ntp time
tm tm;                              // the structure tm holds time information in a more convient way
char *ntpServer="unkown";           // actual ntp server
boolean ntpRunning=false;           // is ntpServer running

/* periodical/interval timing 
 nextTime=nexctTime in ms: 
    nextTime=0 => startNow, 
    nextTime=1 => start period,
    nextTime=2 => OFF
    nextTime=0 => off, nextTime=executeTime
 period= next period in ms
 e.g.
    unsigned long *wifiTime = new unsigned long(0);
    if (isTimer(wifiTime, 1000)) {....}
*/
boolean isTimer(unsigned long *lastTime, unsigned long period) {
  if(*lastTime==2) { return false; } // lastTime=2 => OFF
  else if(*lastTime==0) {  // do now
    *lastTime=_timeMs; // 0= start now
    if(*lastTime>=0 && *lastTime<2) { *lastTime=3; }
    return true;
  }else if(*lastTime==1) {  
    *lastTime=_timeMs; // 1= start next period
    if(*lastTime>=0 && *lastTime<2) { *lastTime=3; }
    return false;  
  }else if (_timeMs >= *lastTime+period) {  // next period found
    *lastTime = _timeMs; 
      if(*lastTime>=0 && *lastTime<2) { *lastTime=3; }
    return true;
  } else {  return false; }// not yet
}


/* show state of intervall timer */
char* timerShow(char *info,unsigned long *lastTime, unsigned long period) {
  sprintf(buffer,"%s now:%d timer:%d period:%d next:%d",info,_timeMs,*lastTime,period,(*lastTime+period-_timeMs));
  return buffer;
}

//---------------------------

/* get date as char (e.g. char* date=getDate(); "01.01.2025" ) */
char* getDate() { sprintf(buffer,"%02d.%02d.%04d",tm.tm_mday,tm.tm_mon + 1,tm.tm_year + 1900);  return buffer;  }
/* getTime  as char* (e.g. char* time=getTime(); 13:50) */
char* getTime() { sprintf(buffer,"%02d:%02d:%02d",tm.tm_hour,tm.tm_min,tm.tm_sec);  return buffer; }
/* get actual time in ms (without NTP get ms since startup) */
unsigned long timeMs() { if(timeNow>0) { return timeNow*1000; } else { return _timeMs; } }
/* get actual time in seconds (without NTP get ms since startup) */
unsigned long timeSec() { if(timeNow>0) { return timeNow; } else { return _timeMs/1000;} }

/* get time info */
char* timeInfo() {
//  long t=(tm.tm_hour*3600+tm.tm_min*60+tm.tm_sec);
  if(is(ntpServer)) { 
    sprintf(buffer,"TIME ntpServer:%s time:%d.%d.%d %d %d:%d:%d timeNow:%d timeMs:%lu",
      ntpServer,tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_wday, tm.tm_hour, tm.tm_min, tm.tm_sec,timeNow,_timeMs); 
  }else {
    sprintf(buffer,"TIME up:%lu ",_timeMs); 
  }

  return buffer;  
}

//---------------------------------------------------------------
// EventTimer 
// instance new Time(firstTimeInMs,nextTimeInMs,executeEvent) 
//  e.g. EventTimer *myTimer=new EventTimer(10000,2000,&info); myTimer->start();  // after 10s excute info() and repeat every 2s
//

List eventList;

class MyEventTimer {
  private:
    boolean _on=false;
    byte _sec; byte _min; byte _hour; byte _day; byte _wday; byte _month;
    char *_cmd;
    byte _lastSec=255;
  public:
    void start() { _on=true; }
    void stop() { _on=false; }
    boolean _isTime() {      
      //tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_wday, tm.tm_hour, tm.tm_min, tm.tm_sec
      if(_lastSec==tm.tm_sec) { return false; } else { _lastSec=tm.tm_sec; } // take care on double execute in one second              
      if(_sec<60 && _sec!=tm.tm_sec){ return false; }
      if(_min<60 && _min!=tm.tm_min){ return false; }
      if(_hour<60 && _hour!=tm.tm_hour){ return false; }
      if(_day>0 && _day<60 && _day!=tm.tm_mday){ return false; }
      if(_wday>0 && _wday<60 && _wday!=tm.tm_wday){ return false; }
      if(_month>0 && _month<60 && _month!=(tm.tm_mon+1)){ return false; }
      return true;
    }
    void event() { logPrintln(LOG_DEBUG,"event start"); String ret=cmdLine(_cmd); logPrintln(LOG_INFO,ret); }
    void loop() { if(_on && _isTime()) { event(); } }  
    char* info() { sprintf(buffer,"timer %d %d %d %d %d %d %d \"%s\"",_on,_sec,_min,_hour,_day,_wday,_month,_cmd); return buffer;}

    MyEventTimer(boolean on,byte sec,byte min,byte hour,byte wday,byte day,byte month,char *cmd) {
      _on=on;
      _sec=sec; _min=min;_hour=hour; _wday=wday; _day=day; _month=month;
      _cmd=cmd;
    }
     ~MyEventTimer() { if(_cmd!=NULL)  { free(_cmd); } }
};

/* 
  timer 1 -1 -1 -1 -1 -1 -1 "stat"
*/
void timerAdd(boolean on,byte sec,byte min,byte hour,byte wday,byte day,byte month,char *cmd) {
  MyEventTimer* et=new MyEventTimer(on,sec,min,hour,wday,day,month,copy(cmd));
  eventList.add(et);
}

void timerDel(int index) { 
  eventList.del(index);  
}

int timerSize() { return eventList.size(); } 

/* show all timers */
void timerLog() {
  for(int i=0;i<eventList.size();i++) {
    MyEventTimer* timer=(MyEventTimer*)eventList.get(i); 
    logPrintln(LOG_INFO,timer->info());
  }
}

/* loop all timers */
void timerLoop() { 
  for(int i=0;i<eventList.size();i++) {
//    MyEventTimer* timer=timerGet(i);
    MyEventTimer* timer=(MyEventTimer*)eventList.get(i); 
    timer->loop();
  }
}

//-----------------------------------------------------------------

/* ntp intervall timer */
unsigned long *ntpTimer = new unsigned long(0);

/* time setup */
void timeSteup() {
  _timeMs = millis(); // set actual millis
}

/* time loop */
void timeLoop() {
  _timeMs = millis(); // set actual millis
  if(isTimer(ntpTimer, 1000)) { // every second
    if(ntpRunning && ntpEnable)  { 
      time(&timeNow);                  // read the current time    
      localtime_r(&timeNow, &tm);      // update the structure tm with the current time
    }
    timerLoop();
  }
}

//-----------------------------------------------------------------------------
// Log

void webLogLn(String msg); // define in web
void mqttLog(char *message); // define in mqtt
//void mqttSet(char *mqtt); // set mqtt

/* log with level 
    e.g. logPrintln(LOG_INFO,"info"); 
        sprintf(buffer,"name:'%s'",name);logPrintln(LOG_INFO,buffer);  
*/
void logPrintln(int level,const char *text) { 
  if(level>logLevel || !is(text)) { return ; }
  if(serialEnable) { Serial.println(text); } 
  if(webEnable) { webLogLn(toString(text)); }
  if(mqttEnable) { mqttLog((char*)text); }
}

/* log with lvel and string 
    e.g. logPrintln(LOG_DEBUG,"info text");
*/
void logPrintln(int level,String text) {  
  if(level>logLevel || !is(text)) { return ; } 
  const char* log=text.c_str();
  if(serialEnable) { Serial.println(log); } 
  if(webEnable) { webLogLn(text); }
  if(mqttEnable) { mqttLog((char*)log); }
}

/* set actual logLevel - log this level and above
    e.g. setLogLevel(LOG_DEBUG) 
*/
char* setLogLevel(int level) {
  if(level>=0) { logLevel=level; }
  sprintf(buffer,"%d",logLevel); return buffer;
}

//-----------------------------------------------------------------------------
// SPIFFS

#if enableFs
  #include <SPIFFS.h>
  #ifdef ESP32
    #define FILESYSTEM SPIFFS
  #elif defined(ESP8266)
    #define FILESYSTEM U_FS
  #endif

  String rootDir="/";

  /* delete file in SPIFFS [ADMINI] */ 
  boolean fsDelete(String file) {     
    if(!is(file)) { return false; }
    else if(!file.startsWith(rootDir)) { file=rootDir+file; } 
    boolean ok=FILESYSTEM.remove(file);  
    sprintf(buffer,"fsDel '%s' ok:%d",file.c_str(),ok);logPrintln(LOG_INFO,buffer);  
    return true;
  }
  boolean fsRename(String oldFile,String newFile) { 
    if(!is(oldFile) || !is(newFile)) { return false; }
    else if(!oldFile.startsWith(rootDir)) { oldFile=rootDir+oldFile; } 
    else if(!newFile.startsWith(rootDir)) { newFile=rootDir+newFile; } 
    boolean ok=FILESYSTEM.rename(oldFile,newFile);  
    sprintf(buffer,"fsRename '%s' => '%s' OK:%d",oldFile.c_str(),newFile.c_str(),ok);logPrintln(LOG_INFO,buffer);  
    return ok;
  }
  
  /* create SPIFFS file and write p1 into file */
  boolean fsWrite(String file,char *p1) { 
    if(!is(file)) { return false; }
    else if(!file.startsWith(rootDir)) { file=rootDir+file; }
    File ff = FILESYSTEM.open(file, FILE_WRITE);
    if(!ff){ return false; }
    int len=strlen(p1);
    if(p1!=NULL && len>0) { ff.print(p1); }
    ff.close();
    sprintf(buffer,"fsWrite '%s %d",file.c_str(),len);logPrintln(LOG_INFO,buffer); 
    return true;
  }

  /* create SPIFFS file and write p1 into file */
  boolean fsWriteBin(String file,uint8_t *p1,int len) { 
    if(!is(file)) { return false; }
    else if(!file.startsWith(rootDir)) { file=rootDir+file; }    
    File ff = FILESYSTEM.open(file, FILE_WRITE);
    if(!ff){ return false; }
    ff.write(p1,len);
    ff.close();
    sprintf(buffer,"fsWriteBin '%s %d",file,len);logPrintln(LOG_INFO,buffer); 
    return true;
  }


  /* read file as char array 
        char *data;  
        data = fsRead(name); 
        delete[] data;
  */
  char* fsRead(String file) {  
    if(!is(file)) { return NULL; }
    else if(!file.startsWith(rootDir)) { file=rootDir+file; }
    File ff = FILESYSTEM.open(file, FILE_READ);  
    if(ff==NULL) { sprintf(buffer,"fsRead unkown '%s'",file.c_str());logPrintln(LOG_INFO,buffer);   return NULL; } 
    size_t fileSize= ff.size();

    char *charArray = new char[fileSize + 1];
    ff.readBytes(charArray, fileSize);
    charArray[fileSize] = '\0';
    ff.close();

    sprintf(buffer,"fsRead '%s' %d",file.c_str(),fileSize);logPrintln(LOG_INFO,buffer);  
    return charArray;
  }


  /* read file as bin 
        size_t dataSize = 0; // gif data size
        uint8_t *data = fsReadBin(name, dataSize); 
        delete[] data;
  */
  uint8_t* fsReadBin(String file, size_t& fileSize) {
    if(!is(file)) { return NULL; }
    else if(!file.startsWith(rootDir)) { file=rootDir+file; }
    File ff = FILESYSTEM.open(file, FILE_READ);  
    if(ff==NULL) { sprintf(buffer,"fsReadBin unkown '%s'",file.c_str());logPrintln(LOG_INFO,buffer);   return NULL; } 
    fileSize= ff.size();

    uint8_t *byteArray = new uint8_t[fileSize];
    ff.read(byteArray, fileSize);

    ff.close();
    sprintf(buffer,"fsReadBin '%s' %d",file.c_str(),fileSize);logPrintln(LOG_INFO,buffer);  
    return byteArray;
  }


  int fsSize(String file) { 
    if(!is(file)) { return -1; }
    else if(!file.startsWith(rootDir)) { file=rootDir+file; }
    File ff = FILESYSTEM.open(file);
    if(ff==NULL) { logPrintln(LOG_INFO,"missing"); return -1; } 
    int len=ff.size();
    ff.close();
    return len;
  }

  /* show a file */
  void fsCat(String file) { 
    if(!is(file)) { return ; }
    else if(!file.startsWith(rootDir)) { file=rootDir+file; }
    File ff = FILESYSTEM.open(file, FILE_READ);
    if(ff==NULL) { logPrintln(LOG_INFO,"missing");  } 
    char buffer[50];
    while (ff.available()) {
      int l = ff.readBytes(buffer, sizeof(buffer));
      buffer[l] = '\0';
  //TODO print with ln    
      logPrintln(LOG_INFO,buffer);
    }
    ff.close();
  }

  /* list files in SPIFFS of dir (null=/) */
  char* fsDir(String find) {
    if(!isAccess(ACCESS_READ))  { return "NO ACCESS fsDir"; }
    sprintf(buffer,"Files:\n");
    File root = FILESYSTEM.open(rootDir);
    File foundfile = root.openNextFile();
    while (foundfile) {
      String file=foundfile.name();
      if(!is(find) || file.indexOf(find)!=-1) { 
        sprintf(buffer+strlen(buffer),"%s (%d)\n",file,foundfile.size());        
      }
      foundfile = root.openNextFile();
    }
    root.close();
    foundfile.close();
    return buffer; 
  }

  /* list number of files in SPIFFS of dir (null=/) */
  int fsDirSize(String find) {
    int count=0;
    File root = FILESYSTEM.open(rootDir);
    File foundfile = root.openNextFile();
    while (foundfile) {
      String file=foundfile.name();
      if(!is(find) || file.indexOf(find)!=-1) { count++; }
      foundfile = root.openNextFile();
    }
    root.close();
    foundfile.close();
    return count; 
  }

  /* get file-name match filter, in dir at index (e.g. .gif,0 => first gif-file) 
      type<=0 => name of file
      type=1 => size of file
  */
  char* fsFile(String find,int count,int type) {
    File root = FILESYSTEM.open(rootDir);
    File foundfile = root.openNextFile();
    while (foundfile) {
      String file=foundfile.name();
      if(!is(find) || file.indexOf(find)!=-1) { 
        if(count--<=0) { 
          if(type<=0) { sprintf(buffer,"%s",(char*)file.c_str()); return buffer;  }
          else if(type==1) { sprintf(buffer,"%d",foundfile.size()); return buffer;  }
          else { return "unkown type"; }
        }
      }
      foundfile = root.openNextFile();
    }
    root.close();
    foundfile.close();
    return EMPTY; 
  }

  /* format SPIFFS */
  void fsFormat() {
    sprintf(buffer,"FS formating..."); logPrintln(LOG_DEBUG,buffer); 
    if (SPIFFS.format()) { sprintf(buffer,"FS format DONE"); logPrintln(LOG_SYSTEM,buffer);  }
    else { sprintf(buffer,"FS format FAILD"); logPrintln(LOG_ERROR,buffer); }    
  }

  #if netEnable

    #include <HTTPClient.h>
    #include <WiFiClient.h>

    // e.g. https://www.w3.org/Icons/64x64/home.gif
    char* fsDownload(String url,String name) {
      if(!is(url,0,250)) { return "missing url"; }

      HTTPClient http;
      if(name==NULL) { name=url.substring(url.lastIndexOf('/')); }
      if(!name.startsWith("/")) { name="/"+name; }

      http.begin(url); 
      int httpCode = http.GET();
      int size = http.getSize();
      if(size>MAX_DONWLOAD_SIZE) { http.end(); return "download maxSize error"; }

      FILESYSTEM.remove(name);  // remove old file
      uint8_t buff[128] PROGMEM = {0};
      if (httpCode == 200) {
        sprintf(buffer,"fs downloading '%s' size %d to '%s'", url.c_str(), size,name.c_str());logPrintln(LOG_INFO,buffer);
        File ff = FILESYSTEM.open(name, FILE_WRITE); 
        http.writeToStream(&ff);
        ff.close();

        sprintf(buffer,"fs download '%s' size %d to '%s'", url.c_str(), size,name.c_str());logPrintln(LOG_INFO,buffer);
        http.end();
        return "download ok";
      } else {
        sprintf(buffer,"fs download '%s' error %d",name.c_str(),httpCode);logPrintln(LOG_INFO,buffer);
        http.end();
        return "download error";
      }

    } 

    /* do rest call and return result */
    char* rest(String url) {
      if(!isAccess(ACCESS_READ))  { return "NO ACCESS"; }
      if(!is(url,0,250)) { return "missing url"; }

      HTTPClient http;
      http.begin(url); 
      int httpCode = http.GET();

      if (httpCode == 200) {
        int size = http.getSize();
        if(size>bufferMax-1) { http.end(); return "response size error"; }

        String payload = http.getString();
        http.end();
        return (char*)payload.c_str();

      } else {
        sprintf(buffer,"rest '%s' error %d",url.c_str(),httpCode);
        http.end();
        return buffer;
      }  
    }

  #else 
    String fsDownload(String url,String name) { return EMPTY; }
    String rest(String url) { return EMPTY; }  
  #endif


//----------------------------------------------

String fsToSize(const size_t bytes) {
  if (bytes < 1024) return String(bytes) + " B";
  else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + " KB";
  else if (bytes < (1024 * 1024 * 1024)) return String(bytes / 1024.0 / 1024.0) + " MB";
  else return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
}

/* filesystem setup */
void fsSetup() {
  if(!enableFs) { return ; }
  if (!FILESYSTEM.begin(true)) {    // if you have not used SPIFFS before on a ESP32, it will show this error. after a reboot SPIFFS will be configured and will happily work.
    espRestart("SPIFFS ERROR: Cannot mount SPIFFS");
  }
  if(!FILESYSTEM.begin()){
    logPrintln(LOG_SYSTEM,"SPIFFS Mount Failed");
  } else {
    sprintf(buffer,"SPIFFS Free:%s Used:%s Total:%s",
      fsToSize((FILESYSTEM.totalBytes() - FILESYSTEM.usedBytes())),fsToSize(FILESYSTEM.usedBytes()),fsToSize(FILESYSTEM.totalBytes()));logPrintln(LOG_INFO,buffer);
  }
}

#else
  boolean fsDelete(String file) { return false; }
  boolean fsWrite(String file,char *p1) { return false; }
  boolean fsRename(String oldFile,String newFile) { return false; }
  char* fsRead(String file) { return NULL; }
  int8_t* fsReadBin(String file, size_t& fileSize) { return NULL; }
  int fsSize(String file) { return -1; }
  void fsCat(String file) {}
  char* fsDir() { return "fs not implemented";}  
  char* fsDownload(String url,String name) { return "fs not implemented"; }
  char* rest(String url) { return "fs not implemented"; }  
  char* fsToSize(const size_t bytes) { return "fs not implemented"; }  
  void fsSetup() {}
  void fsFormat() {}
#endif





