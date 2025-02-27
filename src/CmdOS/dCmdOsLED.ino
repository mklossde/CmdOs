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

char* ledInit(int pin,boolean on) {
  if(pin!=-1) { ledGpio=pin; ledOnTrue=on; }
  sprintf(buffer,"led ping:%d on:%d",ledGpio,ledOnTrue); return buffer;
}

char* ledSwitch(char *c,char *c2) {
  if(isBoolean(c)) { boolean b=toBoolean(c); ledSet(b); sprintf(buffer,"%d",b); return buffer; }
  // ledBlink time speed 
  else if(isInt(c)) { int b=toInt(c); int b2=toInt(c2); ledBlink(b,b2); sprintf(buffer,"%d %d",b,b2); return buffer; } 
  else { return EMPTY; }
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
#else

void ledBlink(byte times, int blinkSpeed) {}
void ledBlinkPattern(byte max,int (*blinkPattern)[]) {}
void ledOff() {}

void ledSetup() {}
void ledLoop() {}

#endif

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
// sw

#if swEnable
byte sw_time_base=100; // time base of sw in ms

using ButtonEvent = void (*)(byte shortCount,unsigned long longTime); //type aliasing //C++ version of: typedef void (*InputEvent)(const char*)
unsigned long *switchTime = new unsigned long(0); // sw timer


#define  swTickShort 5 // 5*100 => 500ms;
#define swTickLong 10 // 10*100 => 1s;
#define swTickMax 255 // too long press 

class Switch { 
  
private:
  byte _swGpio;
  boolean _swOn=true;
  
  byte swLast=false; // last switch on/off 
  byte swShortCount=0;  // number of short-press count
  unsigned long swLastTime=0; // last change
  byte swTickCount=0;
  
  MyEvent _onDown=NULL;
  ButtonEvent _onPress=NULL;


//  SW SETUP    =>  3,5s = SETUP CLIENT (scan and setup client)
//  SW AP       =>  5,5s = mode AP
//  SW RESET    =>  10,5s = RESET ALL

  // sw press short times and  long time in ms (e.g. s_s_l => 2,600ms,2)  
  void swPress(byte shortCount,unsigned long longTime) {
    sprintf(buffer,"SW press short:%d long:%dms",shortCount,longTime); logPrintln(LOG_INFO,buffer); 
//    if(shortCount==5 && longTime>0) { logPrintln("SW RESET"); bootClear(); bootRestart(); }        //  10,5s = RESET ALL
//    else  if(shortCount==4 && longTime>0) { logPrintln("SW AP"); mode=MODE_WIFI_AP; wifiSetup(); } //  5,5s = switch to mode AP
//    else if(shortCount==3 && longTime>0) { logPrintln("SW ScanSetup"); wifiScanSetup(); }              //  3,5s = SETUP CLIENT  
    if(_onPress!=NULL) { _onPress(shortCount,longTime); }
  }
  
  // sw first (immediately) 
  void swFirstDown() {
//    sprintf(buffer,"SW DOWN"); logPrintln(buffer);     
    if(_onDown!=NULL) { _onDown(); }
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
  
  Switch(int gpio,boolean swOn,ButtonEvent onPress,MyEvent onDown) { 
    _swGpio=swGpio; _swOn=swOn; _onPress=onPress; _onDown=onDown; swLast=!swOn;
    pinMode(_swGpio, INPUT_PULLUP);  // input with interal pullup ( _swGpio=GND (false) => pressed) 
    sprintf(buffer,"SW setup gpio:%d on:%d",_swGpio,_swOn); logPrintln(LOG_INFO,buffer);
  }  
};

Switch* sw=NULL;

void swSetup() {
  if(!swEnable) { return ; }
  sw=new Switch(swGpio,swOnTrue,NULL,NULL);
}

void swLoop() {
  if(!swEnable) { return ; }
  if(sw!=NULL && isTimer(switchTime, sw_time_base)) { sw->loop(); } // every 100ms
}

#else
void swSetup() {}
void swLoop() {}
#endif


