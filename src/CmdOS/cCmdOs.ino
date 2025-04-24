
// CmdOS 
// dev by OpenOn.org source from http://github.com/mklossde/CmdOS

#ifdef ESP32
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(TARGET_RP2040)
  #include <WiFi.h>
#endif

#include <EEPROM.h>       // EEprom read/write

#include <time.h>         // time 
#include <sys/time.h>     // time

/* cmdOS by michael@OpenON.org */
const char *cmdOS="V.0.3.0";
char *APP_NAME_PREFIX="CmdOs";
 
String appIP="";
#define MAX_DONWLOAD_SIZE 10000

/* on init auto find wifi set_up (password set_up) and connect */
#define wifi_setup "set_up"

long freeHeapMax;

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
#define ACCESS_USE 5 // user function 
#define ACCESS_ALL 10 // general function

/* log Level */
#define LOG_SYSTEM 0
#define LOG_ERROR 2
#define LOG_INFO 5
#define LOG_DEBUG 10

/* actual log level */
byte logLevel=LOG_INFO;

/* do have access */
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
class MapList {
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
    if(index>=_max) { growTo(index+1,NULL); }     
    _array[index]=obj; if(index>=_index) { _index=index+1; } } 
  /* get obejct at index e.g. char* value=(char*)list.get(0); */
  void* get(int index) { if(index>=0 && index<_index) { return _array[index]; } else { return NULL; } }  
  /* del object at index e.g. char* old=(char*)list.del(0); */
  boolean del(int index) {   
    if(index<0 || index>=_index) {return false; }      
    void *obj=_array[index]; if(obj!=NULL) { delete obj; } 
    if(_isMap) { void *oldKey=_key[index]; if(oldKey!=NULL) { delete oldKey; } }        
    for(int i=index;i<_index-1;i++) { 
      _array[i]=_array[i+1]; 
      if(_isMap) { _key[i]=_key[i+1]; }
      _vsize[i]=_vsize[i+1];
    } 
    _index--; 
    return true;    
  }
  /* clear all (without prefix) / clear with prefix (e.g. clear my ) */
  void clear(char *prefix) { for(int i=_index;i>=0;i--) { if(!is(prefix) || startWith(key(i),prefix)) { del(i); }} }

  /* size of list e.g. int size=list.size(); */
  int size() { return _index; }
  MapList(int max) {  grow(max); }
  MapList() {   }
  MapList(boolean isMap) {  _isMap=isMap;  } // enable as map
  ~MapList() { delete _array; if(_isMap) { delete _key; } }

};


//-----------------------------------------------------------------------------

/* concat char to new char*, use NULL as END, (e.g char *res=concat("one","two",NULL); ), dont forget to free(res); */
char* concat(char* first, ...) {
    size_t total_len = 0;

    va_list args;
    va_start(args, first);
    size_t l=0;
    for (char* s = first; s != NULL && (l=strlen(s))>0; s = va_arg(args, char*)) {  total_len += l;  }
    va_end(args);

    char *result = (char*)malloc(sizeof(char) *(total_len + 1)); // +1 for null terminator
    if (!result) return NULL;
    result[0] = '\0'; // initialize empty string

    va_start(args, first);
    for (char* s = first; s != NULL; s = va_arg(args, char*)) { strcat(result, s); }
    va_end(args);
    return result;
}

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

/* extract from src (NEW char[]) */
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
  if(!is(str) || !is(find)) { return false; }
  int l1=strlen(str); int l2=strlen(find); 
  if(l1!=l2) { return false; }
  for(int i=0;i<l2;i++) {        
    if(*str!=*find) { return false; } 
    str++; find++;
  }  
  return true;
}

/* size/len of text  
    int len=size(text);
*/
int size(char *text) { if(text==NULL) { return -1; } else { return strlen(text); }}

/* insert at pos into buffer */
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

char* to(char *a,char *b) { sprintf(buffer,"%s%s",to(a),to(b)); return buffer; }
char* to(const char *a, const char *b,const char *c) {  sprintf(buffer,"%s%s%s",to(a),to(b),to(c)); return buffer; }
char* to(const char *a, const char *b,const char *c,const char *d) {  sprintf(buffer,"%s%s%s%s",to(a),to(b),to(c),to(d)); return buffer; }
char* to(const char *a, const char *b,const char *c,const char *d,const char *e) {  sprintf(buffer,"%s%s%s%s%s",to(a),to(b),to(c),to(d),to(e)); return buffer; }

/* convert cahr* to string */
String toString(const char *text) {  if(!is(text)) { return EMPTYSTRING; } return String(text); }
String toString(char *text) {  if(!is(text)) { return EMPTYSTRING; } return String(text); }

boolean toBoolean(int i) { return i>0; }
/* convert char* to boolean */
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
  #ifdef ESP32
    uint32_t chipId=0;
    for (int i = 0; i < 17; i = i + 8) { chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;}
    return chipId;
  #elif defined(ESP8266)
    return ESP.getChipId();
  #endif
}

/* esp info */
char* espInfo() {
    sprintf(buffer,"ESP chip:%d free:%d core:%s freq:%d flashChipId:%d flashSize:%d flashSpeed:%d SketchSize:%d FreeSketchSpace:%d",    
      espChipId(),ESP.getFreeHeap(), ESP.getSdkVersion(),ESP.getCpuFreqMHz()
      ,ESP.getFlashChipMode(),ESP.getFlashChipSize(),ESP.getFlashChipSpeed(),ESP.getSketchSize(),ESP.getFreeSketchSpace());           
    return buffer;
}

//-----------------------------------------------------------------------------
// Log

void webLogLn(String msg); // define in web
void mqttLog(char *message); // define in mqtt

/* log with level 
    e.g. logPrintln(LOG_INFO,"info"); 
        sprintf(buffer,"name:'%s'",name);logPrintln(LOG_INFO,buffer);  
*/
void logPrintln(int level,const char *text) { 
  if(level>logLevel || !is(text)) { return ; }
  if(serialEnable) { Serial.println(text); } 
  if(webEnable) { webLogLn(toString(text)); }
  if(mqttLogEnable) { mqttLog((char*)text); }
}

/* log with lvel and string 
    e.g. logPrintln(LOG_DEBUG,"info text");
*/
void logPrintln(int level,String text) {  
  if(level>logLevel || !is(text)) { return ; } 
  const char* log=text.c_str();
  if(serialEnable) { Serial.println(log); } 
  if(webEnable) { webLogLn(text); }
  if(mqttLogEnable) { mqttLog((char*)log); }
}

/* set actual logLevel - log this level and above
    e.g. setLogLevel(LOG_DEBUG) 
*/
char* setLogLevel(int level) {
  if(level>=0) { logLevel=level; }
  sprintf(buffer,"%d",logLevel); return buffer;
}






