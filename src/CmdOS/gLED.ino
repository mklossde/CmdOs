
//-------------------------------------------------------------------------------------------------------------------
// LED



// BOOT SWITCH: BOOT=>BLINK 2x=>PRESS 2s=>WIFI_AP / 5s=>clear

#if ledEnable
int ledPatternFlashSlow[] = { 5, 500, 0 };      // 1 Flash slow => Wifi SP mode
int ledPattern2Flash[] = { 5, 10, 5, 500, 0 };  // 2 Flash slow => Wifi CL connectiong
int ledPatternBlink1[] = { 10, 1, 0 };          //

unsigned long *ledTime = new unsigned long(0);  // led timer
boolean ledBlinkOn = false;                     // is blink on
boolean ledOn = false;                          // is led on
int (*ledPattern)[];                            // led blink pattern
int ledTicks = 0;
byte ledCount = 5;
byte ledIndex = 0;

// direct blink blinkSpeed in ms - just for test/debug/error
void ledBlink(byte times, int blinkSpeed) {
  if (times * blinkSpeed > 10000) { return; }  // ignore delay >10s
  if (ledEnable) {
    for (int i = 0; i < times; i++) {
      digitalWrite(ledGpio, ledOnTrue);
      delay(blinkSpeed);
      digitalWrite(ledGpio, !ledOnTrue);
      delay(blinkSpeed);
      ledOn = !ledOnTrue;
    }
  }
}
void ledSet(boolean on) {
  digitalWrite(ledGpio, on);
  ledOn = on;
}

void ledOff() {
  if (ledEnable) {
    digitalWrite(ledGpio, !ledOnTrue);
    ledOn = !ledOnTrue;
  }
  ledBlinkOn = false;
}

//-----

// show actual led blink
void ledShow() {
  if (ledEnable) {
    if (ledOn) {
      digitalWrite(ledGpio, ledOnTrue);
    } else {
      digitalWrite(ledGpio, !ledOnTrue);
    }
  }
}

// blink with pattern, max times (0=unlimited)
void ledBlinkPattern(byte max, int (*blinkPattern)[]) {
  ledPattern = blinkPattern;
  ledCount = max;
  ledTicks = 0;
  ledIndex = 0;
  ledBlinkOn = true;
  ledOn = true;
  ledShow();
  //  sprintf(buffer,"LED blink %d index:%d time:%d count:%d",ledGpio,ledIndex,(*ledPattern)[ledIndex],ledCount); logPrintln(buffer);
}

//--------------------------

void ledSetup() {
  if (!ledEnable) { return; }
  pinMode(ledGpio, OUTPUT);
  sprintf(buffer, "LED setup gpio:%d on:%d", ledGpio, ledOnTrue);
  logPrintln(LOG_INFO, buffer);
}

void ledLoop() {
  if (!ledEnable) { return; }
  if (!ledBlinkOn || !isTimer(ledTime, 10)) { return; }  // every 10ms

  if (!ledBlinkOn || (*ledPattern)[0] == 0) { return; }
  ledTicks++;
  if (ledTicks > (*ledPattern)[ledIndex]) {
    ledOn = !ledOn;
    ledTicks = 0;
    ledIndex++;
    if ((*ledPattern)[ledIndex] == 0) {
      ledIndex = 0;
      if (ledCount > 0) {
        ledCount--;
        if (ledCount == 0) {
          ledBlinkOn = false;
          ledOn = false;
        }
      }
    }
    ledShow();
  }
}


char *ledInit(int pin, boolean on) {
  if (pin != -1) {
    ledGpio = pin;
    ledOnTrue = on;
    ledSetup();
  }
  sprintf(buffer, "led pin:%d on:%d", ledGpio, ledOnTrue);
  return buffer;
}

char *ledSwitch(char *c, char *c2) {
  if (isBoolean(c)) {
    boolean b = toBoolean(c);
    ledSet(b);
    sprintf(buffer, "%d", b);
    return buffer;
  }
  // ledBlink time speed
  else if (isInt(c)) {
    int b = toInt(c);
    int b2 = toInt(c2);
    ledBlink(b, b2);
    sprintf(buffer, "%d %d", b, b2);
    return buffer;
  } else {
    return EMPTY;
  }
}

#else

void ledBlink(byte times, int blinkSpeed) {}
void ledBlinkPattern(byte max, int (*blinkPattern)[]) {}
void ledOff() {}

void ledSetup() {}
void ledLoop() {}

char* ledInit(int pin, boolean on) {
  return EMPTY;
}
char* ledSwitch(char* c, char* c2) {
  return EMPTY;
}

#endif

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
// sw

#if swEnable
//int _sw_time_base=100; // time base of sw in ms

#define swTickMax 254  // too long press 255*100 => 25.5s

//using ButtonEvent = void (*)(byte shortCount,unsigned long longTime); //type aliasing //C++ version of: typedef void (*InputEvent)(const char*)
unsigned long *switchTime = new unsigned long(0);  // sw timer

class Switch {

private:
  byte _swGpio;
  byte _swOn = 1;
  byte _swMode = 0;

  byte swLast = false;           // last switch on/off
  byte swShortCount = 0;         // number of short-press count
  unsigned long swLastTime = 0;  // last change
  byte swTickCount = 0;
  //ButtonEvent _onPress=NULL;
  MapList cmdList;  // 0=onDown,1..9=on n click,10=on Long

public:

  char *setCmd(int nr, char *cmd) {
    if (nr > 11) {
      return EMPTY;
    } else if (nr < 0) {
      sprintf(buffer, "");
      for (int i = 0; i < cmdList.size(); i++) {
        char *value = (char *)cmdList.get(i);
        if (is(value)) {
          sprintf(buffer + strlen(buffer), "swCmd %d \"%s\"\n", i, value);
        }
      }
      return buffer;
    } else if (!is(cmd)) {
      char *cmd = (char *)cmdList.get(nr);
      sprintf(buffer, "swCmd nr:%d cmd:%s", nr, to(cmd));
      return buffer;
    } else if (size(cmd) < 2) {
      cmdList.del(nr);
      sprintf(buffer, "swCmd del nr:%d", nr);
      return buffer;
    } else {
      cmdList.addIndex(nr, copy(cmd));
      sprintf(buffer, "swCmd set nr:%d cmd:%s", nr, to(cmd));
      return buffer;
    }
  }

  // sw press short times and  long time in ms (e.g. s_s_l => 2,600ms,2)
  void swPress(byte shortCount, unsigned long longTime) {
    sprintf(buffer, "SW press short:%d long:%dms", shortCount, longTime);
    logPrintln(LOG_DEBUG, buffer);
    if (longTime == 0) {
      char *cmd = (char *)cmdList.get(shortCount);
      if (is(cmd)) {
        char *c = copy(cmd);
        cmdLine(c);
        delete[] c;
      }
    } else {
      char *cmd = (char *)cmdList.get(10);
      if (is(cmd)) {
        char *c = copy(cmd);
        cmdLine(c);
        delete[] c;
      }
    }
  }

  // sw first (immediately)
  void swFirstDown(byte swNow) {
    sprintf(buffer, "SW DOWN %d", swNow);
    logPrintln(LOG_DEBUG, buffer);
    char *cmd = (char *)cmdList.get(0);
    if (is(cmd)) {
      char *c = copy(cmd);
      cmdLine(c);
      delete[] c;
    }  // cmdLine / cmdPrg
  }

public:
  // read button
  byte swRead() {
    if (_swMode == SW_MODE_TOUCH) {
      #ifdef ESP32
        return touchRead(_swGpio);
      #else 
        return digitalRead(_swGpio);
      #endif 
    } else {
      return digitalRead(_swGpio);
    }
  }
  // is button press
  boolean isOn() {
    return isOn(swRead());
  }
  boolean isOn(byte swNow) {
    if (_swOn < 2) {
      return swNow == _swOn;
    } else {
      return swNow < _swOn;
    }
  }

  void loop() {
    byte swNow = swRead();
    boolean isNowOn = isOn(swNow);

    if (isNowOn != swLast) {  // change
      if (isNowOn) {          // change=>on
        if (swShortCount == 0 && swTickCount == 0) { swFirstDown(swNow); }
        swTickCount = 1;                  // start on tick
        swShortCount++;                   // inc press
      } else {                            // change => off
        if (swTickCount >= swTickLong) {  // off => last on-swTickCount >swTickLong
          swPress(swShortCount, swTickCount);
          swShortCount = 0;
          swTickCount = 0;  // relase => long press
        } else {
          swTickCount = 1;  // start off tick
        }
      }

      swLast = isNowOn;
    } else if (isNowOn && swShortCount > 0) {   // on is hold
      swTickCount++;                            // count on tick
    } else if (!isNowOn && swShortCount > 0) {  // is off hold
      swTickCount++;                            // count off tick
      if (swTickCount > swTickShort) {
        swPress(swShortCount, 0);
        swShortCount = 0;
        swTickCount = 0;
      }  // no new press => short press
    }

    if (swTickCount >= swTickMax) {        // max on/off  => reset
      swPress(swShortCount, swTickCount);  // max => long press
      swShortCount = 0;
      swTickCount = 0;
    }
  }

  char *swInfo() {
    sprintf(buffer, "SW %d gpio:%d on:%d swMode:%d - swTimeBase:%d swTickShort:%d swTickLong:%d", swRead(), _swGpio, _swOn, _swMode,
            swTimeBase, swTickShort, swTickLong);
    return buffer; 
  }

  Switch(int gpio, byte swOn, byte swMode) {
    _swGpio = swGpio;
    _swOn = swOn;
    _swMode = swMode;
    swLast = !swOn;
    if (_swMode == SW_MODE_TOUCH) {
    }                                                                        //
    else if (_swMode == SW_MODE_PULLUP) { pinMode(_swGpio, INPUT_PULLUP);   // input with interal pullup ( _swGpio=GND (false) => pressed)
    #ifdef ESP32
      }else if (_swMode == SW_MODE_PULLDOWN) { pinMode(_swGpio, INPUT_PULLDOWN); // input with interal pulldown
    #endif
    }  else { pinMode(_swGpio, INPUT); }
    if (_swOn == 2) {
      byte swNow = swRead();
      if (swNow == 0) {
        _swOn = 1;
      } else if (swNow == 2) {
        _swOn = 0;
      } else {
        _swOn = swNow;
      }
    }  // swOn==2 => use actual as !on
    else if (_swOn > 2 && _swOn < 10) { _swOn = swRead() - _swOn; }
    logPrintln(LOG_INFO, swInfo());
  }
};

Switch *sw = NULL;
unsigned long *switchStartup = new unsigned long(1);  // sw timer

void swSetup() {
  if (!swEnable) { return; }
  sw = new Switch(swGpio, swOn, swMode);
}

// swInit 32 3 3 100 4 5
char *swInit(int pin, int on, byte mode, int timeBase, byte tickShort, byte tickLong) {
  if (pin != -1) {
    swGpio = pin;
    swOn = on;
    swMode = swMode;
    swTimeBase = timeBase;
    swTickShort = tickShort;
    swTickLong = tickLong;
    swSetup();
    return EMPTY;
  } else {
    return sw->swInfo();
  }
}

char *swCmd(int i, char *cmd) {
  if (sw == NULL) { return EMPTY; }
  return sw->setCmd(i, cmd);
}

void swLoop() {
  if (!swEnable) {
    return;
  } else if (sw != NULL && isTimer(switchTime, swTimeBase)) {
    sw->loop();
  }  // every 100ms
}

#else
void swSetup() {}
void swLoop() {}
char* swCmd(int i, char* cmd) {
  return EMPTY;
}
char* swInit(int pin, boolean on, byte mode, int timeBase, byte tickShort, byte tickLong) {
  return EMPTY;
}

#endif

//----------------------------------------------------------------------

/* set digital pin */
void gpioSet(int pin,boolean value) { pinMode(pin, OUTPUT); digitalWrite(pin, value); }
/* read digital pin */
boolean gpio(int pin) { pinMode(pin, INPUT);  return digitalRead(pin); }


