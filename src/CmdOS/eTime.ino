
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
    period=-1 (one time, but off after)
 e.g.
    unsigned long *wifiTime = new unsigned long(0);
    if (isTimer(wifiTime, 1000)) {....}
*/
boolean isTimer(unsigned long *lastTime, unsigned long period) {
  if(*lastTime==2 ) { return false; } // lastTime=2 => OFF
  else if(*lastTime==0) {  // do now
    *lastTime=_timeMs; // 0= start now
    if(*lastTime>=0 && *lastTime<2) { *lastTime=3; }
    return true;
  }else if(*lastTime==1) {  
    *lastTime=_timeMs; // 1= start next period
    if(*lastTime>=0 && *lastTime<2) { *lastTime=3; }
    return false;  
  }else if (period!=-1 && _timeMs >= *lastTime+period) {  // next period found
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

MapList eventList;

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
