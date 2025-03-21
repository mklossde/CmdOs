//-------------------------------------------------------------------------------------------------------------------
// LED

// BOOT SWITCH: BOOT=>BLINK 2x=>PRESS 2s=>WIFI_AP / 5s=>clear 

#if ledEnable
int ledPatternFlashSlow[]={5,500,0}; // 1 Flash slow => Wifi SP mode
int ledPattern2Flash[]={5,10,5,500,0}; // 2 Flash slow => Wifi CL connectiong
int ledPatternBlink1[]={10,1,0}; // 

unsigned long *ledTime = new unsigned long(0); // led timer
boolean ledBlinkOn=false; // is blink on
boolean ledOn=false; // is led on
int (*ledPattern)[];  // led blink pattern
int ledTicks=0;
byte ledCount=5;
byte ledIndex=0;

// direct blink blinkSpeed in ms - just for test/debug/error
void ledBlink(byte times, int blinkSpeed) {
  if(times*blinkSpeed>10000) { return ; } // ignore delay >10s
  if(ledEnable) {
    for(int i=0;i<times;i++) {     
      digitalWrite(ledGpio, ledOnTrue); delay(blinkSpeed); digitalWrite(ledGpio, !ledOnTrue); delay(blinkSpeed); ledOn=!ledOnTrue;             
    }
  }
}
void ledSet(boolean on) { digitalWrite(ledGpio, on); ledOn=on; }

void ledOff() {
  if(ledEnable) { digitalWrite(ledGpio, !ledOnTrue); ledOn=!ledOnTrue; }
  ledBlinkOn=false; 
}

//-----

// show actual led blink 
void ledShow() {
    if(ledEnable) {
      if(ledOn) { digitalWrite(ledGpio,ledOnTrue); }else {  digitalWrite(ledGpio,!ledOnTrue); }     
    }
}

// blink with pattern, max times (0=unlimited) 
void ledBlinkPattern(byte max,int (*blinkPattern)[]) { 
  ledPattern=blinkPattern; ledCount=max; ledTicks=0; ledIndex=0; ledBlinkOn=true; ledOn=true; ledShow();
//  sprintf(buffer,"LED blink %d index:%d time:%d count:%d",ledGpio,ledIndex,(*ledPattern)[ledIndex],ledCount); logPrintln(buffer);
}

//--------------------------

void ledSetup() {
  if(!ledEnable) { return ; }
  pinMode(ledGpio, OUTPUT);  
  sprintf(buffer,"LED setup gpio:%d on:%d",ledGpio,ledOnTrue); logPrintln(LOG_INFO,buffer);
}

void ledLoop() {  
  if(!ledEnable) { return ; }
  if(!ledBlinkOn || !isTimer(ledTime, 10)) { return; } // every 10ms

  if(!ledBlinkOn || (*ledPattern)[0]==0) { return ; }
  ledTicks++;
  if(ledTicks>(*ledPattern)[ledIndex]) {
    ledOn=!ledOn; 
    ledTicks=0; 
    ledIndex++; 
    if((*ledPattern)[ledIndex]==0) { 
      ledIndex=0; 
      if(ledCount>0) { 
        ledCount--; if(ledCount==0) {  ledBlinkOn=false; ledOn=false; }
      }        
    }
    ledShow();      
  }
}  


char* ledInit(int pin,boolean on) {
  if(pin!=-1) { ledGpio=pin; ledOnTrue=on; ledSetup(); }
  sprintf(buffer,"led pin:%d on:%d",ledGpio,ledOnTrue); return buffer;
}

char* ledSwitch(char *c,char *c2) {
  if(isBoolean(c)) { boolean b=toBoolean(c); ledSet(b); sprintf(buffer,"%d",b); return buffer; }
  // ledBlink time speed 
  else if(isInt(c)) { int b=toInt(c); int b2=toInt(c2); ledBlink(b,b2); sprintf(buffer,"%d %d",b,b2); return buffer; } 
  else { return EMPTY; }
}

#else

void ledBlink(byte times, int blinkSpeed) {}
void ledBlinkPattern(byte max,int (*blinkPattern)[]) {}
void ledOff() {}

void ledSetup() {}
void ledLoop() {}

char* ledInit(int pin,boolean on) { return EMPTY; }
char* ledSwitch(char *c,char *c2) { return EMPTY; }

#endif

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
// sw

#if swEnable
//int _sw_time_base=100; // time base of sw in ms

//using ButtonEvent = void (*)(byte shortCount,unsigned long longTime); //type aliasing //C++ version of: typedef void (*InputEvent)(const char*)
unsigned long *switchTime = new unsigned long(0); // sw timer


#define  swTickShort 4 // 5*100 => 500ms;
#define swTickLong 5 // 10*100 => 1s;
#define swTickMax 255 // too long press 255*100 => 25.5s 

class Switch { 
  
private:
  byte _swGpio;
  boolean _swOn=true;
  
  byte swLast=false; // last switch on/off 
  byte swShortCount=0;  // number of short-press count
  unsigned long swLastTime=0; // last change
  byte swTickCount=0;
  //ButtonEvent _onPress=NULL;
  List cmdList; // 0=onDown,1..9=on n click,10=on Long
  
public:

  char* setCmd(int nr,char *cmd) {
    if(nr>11) { return EMPTY; }
    else if(nr<0) {
      sprintf(buffer,"");
      for(int i=0;i<cmdList.size();i++) {  
        char *value=(char*)cmdList.get(i);
        if(is(value)) {
          sprintf(buffer+strlen(buffer),"swCmd %d \"%s\"\n",i,value);
        }
      }
      return buffer;
    }else if(!is(cmd)) { char *cmd=(char*)cmdList.get(nr); sprintf(buffer,"swCmd nr:%d cmd:%s",nr,to(cmd)); return buffer;}    
    else if(size(cmd)<2) { cmdList.del(nr); sprintf(buffer,"swCmd del nr:%d",nr); return buffer; }
    else { cmdList.addIndex(nr,copy(cmd)); sprintf(buffer,"swCmd set nr:%d cmd:%s",nr,to(cmd)); return buffer;}
  }

  // sw press short times and  long time in ms (e.g. s_s_l => 2,600ms,2)  
  void swPress(byte shortCount,unsigned long longTime) {
    sprintf(buffer,"SW press short:%d long:%dms",shortCount,longTime); logPrintln(LOG_DEBUG,buffer); 
    if(longTime==0) {
      char *cmd=(char*)cmdList.get(shortCount); if(is(cmd)) { char *c=copy(cmd); cmdLine(c); delete[] c; }
    }else {
      char *cmd=(char*)cmdList.get(10); if(is(cmd)) { char *c=copy(cmd); cmdLine(c); delete[] c; }
    }
  }
  
  // sw first (immediately) 
  void swFirstDown() {
//    sprintf(buffer,"SW DOWN"); logPrintln(LOG_DEBUG,buffer);     
    char *cmd=(char*)cmdList.get(0); if(is(cmd)) {char *c=copy(cmd); cmdLine(c); delete[] c; } // cmdLine / cmdPrg
  }

public:
  // read button
  byte swRead() { return digitalRead(_swGpio); }
  // is button press
  boolean isOn() { return digitalRead(_swGpio)==_swOn; }
    
  void loop() {
    byte swNow=digitalRead(_swGpio);
    if(swNow!=swLast) { // change
      if(swNow==_swOn) {  // change=>on
        if(swShortCount==0 && swTickCount==0) { swFirstDown(); }
        swShortCount++;
        swTickCount=1;
      }else if(swNow!=_swOn) { // change => off 
        if(swTickCount>=swTickLong) { swPress(swShortCount,swTickCount); swShortCount=0;swTickCount=0; } // relase => long press     
        else { swTickCount=1; }
      }
    }else if(swNow==_swOn && swShortCount>0) { // press
      swTickCount++;      
    }else if(swNow!=_swOn) { // not-pressed / released
      if(swShortCount>0) { 
        swTickCount++; 
        if(swTickCount>swTickShort) { swPress(swShortCount,0); swShortCount=0;swTickCount=0; } // not new press => short press 
      }      
    }
    
    swLast=swNow;
    if(swTickCount>=swTickMax) { swShortCount=0; swTickCount=0; } // max time => reset     
  }
  
  Switch(int gpio,boolean swOn) { 
    _swGpio=swGpio; _swOn=swOn; swLast=!swOn;
    if(swPullUp) { pinMode(_swGpio, INPUT_PULLUP); } // input with interal pullup ( _swGpio=GND (false) => pressed) 
    else { pinMode(_swGpio, INPUT); }
    sprintf(buffer,"SW setup gpio:%d on:%d swTimeBase:%d pullUp:%d",_swGpio,_swOn,swTimeBase,swPullUp); logPrintln(LOG_INFO,buffer);
  }  
};

Switch* sw=NULL;
unsigned long *switchStartup = new unsigned long(1); // sw timer

void swSetup() {
  if(!swEnable) { return ; }
  sw=new Switch(swGpio,swOnTrue); 
}

char* swInit(int pin,boolean on,boolean pullUp,int sw_time_base) {
  if(pin!=-1) { 
    swGpio=pin; swOnTrue=on; swTimeBase=sw_time_base; swPullUp=pullUp; ;
    swSetup(); 
  }
  sprintf(buffer,"sw pin:%d on:%d sw_time_base:%d swPullUp:%d",swGpio,swOnTrue,sw_time_base,swPullUp); return buffer;
}

char* swCmd(int i,char *cmd) {
  if(sw==NULL) { return EMPTY; } 
  return sw->setCmd(i,cmd);
}

void swLoop() {
  if(!swEnable) { return ; }
  else if(sw!=NULL && isTimer(switchTime, swTimeBase)) { sw->loop(); } // every 100ms
}

#else
void swSetup() {}
void swLoop() {}
char* swInit(int pin,boolean on) { return EMPTY; }
char* swCmd(int i,char *cmd) { return EMPTY; }
char* swInit(int pin,boolean on,boolean pullUp,int sw_time_base,int sw_setup_time) { return EMPTY; }

#endif


