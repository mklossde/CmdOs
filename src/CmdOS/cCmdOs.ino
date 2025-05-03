
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
const char *cmdOS="V.0.4.0";
char *APP_NAME_PREFIX="CmdOs";
 
String appIP="";

/* on init auto find wifi set_up (password set_up) and connect */
#define wifi_setup "set_up"

long freeHeapMax;

//-----------------------------------------------------------------------------


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

#define MAX_DONWLOAD_SIZE 50000

#define bufferMax 256
#define paramBufferMax 128
#define maxInData 128 // max line length

#define minValueLen 8

static char* buffer=new char[bufferMax]; // buffer for char/logging
static char* paramBuffer=new char[paramBufferMax]; // buffer for params
static char inData [maxInData]; // read buffer
static char inIndex = 0; // read index

static char* EMPTY="";
static String EMPTYSTRING="";


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
char* to(double d) { sprintf(buffer,"%.2f",d); return buffer; }
char* to(char *p) {if(p!=NULL && strlen(p)>0 && strlen(p)<bufferMax) { return p; } else { return EMPTY; } }
const char* to(const char *p) {if(p!=NULL && strlen(p)>0 && strlen(p)<bufferMax) { return p; } else { return EMPTY; } }

char* to(char *a,char *b) { sprintf(buffer,"%s%s",to(a),to(b)); return buffer; }
char* to(const char *a, const char *b,const char *c) {  sprintf(buffer,"%s%s%s",to(a),to(b),to(c)); return buffer; }
char* to(const char *a, const char *b,const char *c,const char *d) {  sprintf(buffer,"%s%s%s%s",to(a),to(b),to(c),to(d)); return buffer; }
char* to(const char *a, const char *b,const char *c,const char *d,const char *e) {  sprintf(buffer,"%s%s%s%s%s",to(a),to(b),to(c),to(d),to(e)); return buffer; }

void *toLower(char* str) {
    if(str==NULL) { return NULL; }
    int i=0;
    while(str[i]!='\0') { str[i] = (char)tolower((unsigned char)str[i]); }
}

/* convert cahr* to string */
String toString(const char *text) {  if(!is(text)) { return EMPTYSTRING; } return String(text); }
String toString(char *text) {  if(!is(text)) { return EMPTYSTRING; } return String(text); }

boolean toBoolean(int i) { return i>0; }
/* convert char* to boolean */
boolean toBoolean(char *p) { return p!=NULL && strlen(p)>0 && (strcmp(p, "on")==0 || strcmp(p, "true")==0 || atoi(p)>0); }
/* convert char* to int */
int toInt(char *p) { 
  if(!is(p)) { return -1; }
  else if(isInt(p)) { return atoi(p); } 
  if(toBoolean(p)) { return 1; }else { return 0; } 
} 
/* convert char* to double */
double toDouble(char *p) { 
  if(!is(p)) { return -1; }
  else if(isDouble(p)) { return atof(p); }
  if(toBoolean(p)) { return 1; } else { return 0; } 
}
/* convert char* to long */
long int toLong(char *p) { 
  if(!is(p)) { return -1; }
  else if(isInt(p)) { return atol(p); } 
  if(toBoolean(p)) { return 1; }else { return 0; } 
}
/* convert char* to unsigned long */
unsigned long toULong(char *p) { 
  if(!is(p)) { return -1; }
  else if(isBoolean(p)) {  if(toBoolean(p)) { return 1; }else { return 0; } }
  else { return strtoul(p, NULL, 0); } 
}

boolean isDouble(char *p) {
  char *x=p;
  if (x==NULL || x==EMPTY || *x == '\0') { return false; } // Empty string is not a number
  else if (*x == '+' || *x == '-')  { x++; } // Handle optional sign
  while (*x) { if (!isdigit(*x) && *x!='.') { return false; } else { x++;} }// Non-digit character found
  return true;  // All characters are digits
}

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
    sprintf(buffer,"ESP chip:%d free:%d/%d core:%s freq:%d flashChipId:%d flashSize:%d flashSpeed:%d SketchSize:%d FreeSketchSpace:%d",    
      espChipId(),ESP.getFreeHeap(),freeHeapMax, ESP.getSdkVersion(),ESP.getCpuFreqMHz()
      ,ESP.getFlashChipMode(),ESP.getFlashChipSize(),ESP.getFlashChipSpeed(),ESP.getSketchSize(),ESP.getFreeSketchSpace());           
    return buffer;
}

//-----------------------------------------------------------------------------
// Log

void webLogLn(String msg); // define in web
void mqttLog(char *message); // define in mqtt
void telnetLog(char *message); // define in telnet

/* log with level 
    e.g. logPrintln(LOG_INFO,"info"); 
        sprintf(buffer,"name:'%s'",name);logPrintln(LOG_INFO,buffer);  
*/
void logPrintln(int level,const char *text) { 
  if(level>logLevel || !is(text)) { return ; }
  if(serialEnable) { Serial.println(text); } 
  if(telnetEnable) { telnetLog((char*)text); }
  if(webSerialEnable) { webLogLn(toString(text)); }  
  if(mqttLogEnable) { mqttLog((char*)text); }
}

/* log with lvel and string 
    e.g. logPrintln(LOG_DEBUG,"info text");
*/
void logPrintln(int level,String text) {  
  if(level>logLevel || !is(text)) { return ; } 
  const char* log=text.c_str();
  if(serialEnable) { Serial.println(log); } 
  if(telnetEnable) { telnetLog((char*)log); }
  if(webSerialEnable) { webLogLn(text); }
  if(mqttLogEnable) { mqttLog((char*)log); }  
}

/* set actual logLevel - log this level and above
    e.g. setLogLevel(LOG_DEBUG) 
*/
char* setLogLevel(int level) {
  if(level>=0) { logLevel=level; }
  sprintf(buffer,"%d",logLevel); return buffer;
}
