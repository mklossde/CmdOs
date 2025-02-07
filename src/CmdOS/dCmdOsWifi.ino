/*
 * Wifi
 */
 
#include <WiFi.h>
#include <DNSServer.h>

#include <esp_sntp.h> // time
#include <time.h>     // time

#ifdef ESP32
  #include <ESPmDNS.h>
#elif defined(ESP8266)
  #include <ESP8266mDNS.h>
#endif

// Bootloader version
char bootType[5] = "Os02"; // max 10 chars

// bootloader data struc
typedef struct {
  unsigned long timestamp;                 // timestamp of store  
  char wifi_ssid[32]="";                   // WIFI SSID of AccessPoint
  char wifi_pas[32]="";                     // WIFI password of AccessPoint
  char wifi_ntp[32]="";                   // ntp server
  // board
  char espName[32];                        // Name of device
  char espPas[32]="";                      // Password of device
  char espBoard[32]="";                    // BoardType
  // mqtt
  char mqtt[128]="";                        // mqtt Server
  boolean mqttLogEnable=false;              // enable send all logs to mqtt
  // access
  byte accessLevel=ACCESS_ADMIN;             // accessLevel 
} eeBoot_t;

eeBoot_t eeBoot;    // bootloader data 


//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
// EEPROM


/* save to ee */
void eeSave() {
  if(serialEnable) { Serial.println("### SAVE");}
  eeBoot.timestamp=timeSec();  
  int pos=0;
  EEPROM.begin(EEBOOTSIZE);
  EEPROM.put(pos, bootType ); pos+=5; // write type for validation
  EEPROM.put(pos, eeMode); pos+=1; 
  EEPROM.put(pos, eeBoot); pos+=sizeof(eeBoot); // save bootloader 
  EEPROM.commit();  // Only needed for ESP8266 to get data written
  eeAppPos=pos;
}

void eeRead() {   
  EEPROM.begin(EEBOOTSIZE);
  int pos=0;
  EEPROM.get( pos, eeType );  pos+=5; // eeType = "EspBoot100";
  if(strcmp(eeType,bootType)!=0) { eeMode=EE_MODE_FIRST; return ; }   // validate

  EEPROM.get(pos, eeMode ); pos+=1;
  EEPROM.get(pos, eeBoot); pos+=sizeof(eeBoot);// eeBoot read
  EEPROM.end(); 
  eeAppPos=pos;
}

void eeSetMode(byte mode) {
  EEPROM.begin(EEBOOTSIZE);
  EEPROM.put( 5, mode );
  EEPROM.end(); 
  eeMode=mode;
}

/* on first start prg */
void eeInit() {
  if(serialEnable) { Serial.println("### INIT");}
  eeMode=EE_MODE_SETUP; 

  uint32_t chipid=espChipId(); // or use WiFi.macAddress() ?
  if(!is(eeBoot.espName)) { snprintf(eeBoot.espName,20, "OpenOs%08X",chipid);  }
  if(!is(eeBoot.espPas)) { sprintf(eeBoot.espPas,user_pas); }     // my private esp password   
  if(!is(eeBoot.wifi_ssid)) {sprintf(eeBoot.wifi_ssid,wifi_ssid_default); } // my privat WIFI SSID of AccessPoint
  if(!is(eeBoot.wifi_pas)) {sprintf(eeBoot.wifi_ssid,wifi_pas_default); } // my privat WIFI SSID of AccessPoint
  if(!is(eeBoot.mqtt)) {sprintf(eeBoot.mqtt,mqtt_default); }           // my privat MQTT server

  eeSave();
}

/** e setup */
void eeSetup() {
//TODO show restart reason
  eeRead();

  if(strcmp(eeType,bootType)!=0) {  // type wrong
    if(serialEnable) { Serial.println("### MODE WRONG"); }
    eeInit(); // first Time 
    return; 
  }else if(eeMode==EE_MODE_SETUP) {
    if(serialEnable) { Serial.println("### MODE SETUP "); }
    return ;
  } else if(eeMode==EE_MODE_AP) {
    if(serialEnable) { Serial.println("### MODE AP "); }
    return ;
  }

  if(!bootSafe) { 
    Serial.print("### MODE(NOBS) ");Serial.println(eeMode); // ignore all other on disable boot safe 
  }else if(eeMode==EE_MODE_ERROR) {
    if(serialEnable) { Serial.println("### MODE ERROR "); }
    setAccess(ACCESS_ADMIN);
    eeSetMode(EE_MODE_SYSERROR); // mark 
  } else if(eeMode==EE_MODE_SYSERROR) {
    setAccess(ACCESS_ADMIN);
    if(serialEnable) { Serial.println("### MODE SYSERROR "); }

  }else if(eeMode==EE_MODE_OK) { 
    if(serialEnable) { Serial.println("### MODE OK -> START"); }
    eeSetMode(EE_MODE_START); // mark  

  }else if(eeMode>EE_MODE_WRONG) {
    if(serialEnable) { Serial.println("### MODE RE-INIT"); }
    eeInit(); // re-init

  } else if(eeMode>=EE_MODE_START) {
    if(serialEnable) { Serial.println("### MODE RE-START"); }
    eeSetMode(eeMode+1); // mark wrong+1

  }else {
    if(serialEnable) { Serial.print("### MODE ");Serial.println(eeMode); }
  }
  
}

unsigned long *eeTime = new unsigned long(1);
int okWait=10000; // wait 10s before start => ok 

void eeLoop() {
  if(eeMode<EE_MODE_ERROR && eeMode>=EE_MODE_START && isTimer(eeTime, okWait)) { 
    eeSetMode(EE_MODE_OK); // mark  ok after start
  }
}

//-----------------------------------------------------------------------------

// info about boot 
char* bootInfo() {
   sprintf(buffer,"eeBoot eeMode:%d espName:%s espPas:%d espBoard:%s wifi_ssid:%s timestamp:%d mqtt:%s ntp:%s", 
   eeMode, eeBoot.espName,is(eeBoot.espPas),eeBoot.espBoard,eeBoot.wifi_ssid,eeBoot.timestamp,eeBoot.mqtt,eeBoot.wifi_ntp); 
   return buffer;
}

/* set espName,espInfo */ 
char* bootSet(char* espName,char* espPas) {
  if(is(espName) && isAccess(ACCESS_ADMIN)) { strcpy(eeBoot.espName,espName); }
  if(is(espPas) && isAccess(ACCESS_ADMIN)) { strcpy(eeBoot.espPas,espPas); }
  return bootInfo();
}

/* save bootloader from RAM into EEPROM [ADMIN] */
void bootSave() {
  if(!isAccess(ACCESS_ADMIN)) { return ; }

  // auto set WIFI_CL_TRY when in AP
  if((eeMode<EE_MODE_WIFI_OFF) && is(eeBoot.wifi_ssid) && is(eeBoot.wifi_pas)) {
    eeSetMode(EE_MODE_WIFI_TRY);
  }
 
  eeSave();
  //EEPROM.end(); DO NOT END HERE => Otherwise EEPROM is not written
  ledBlink(3,100); // save => direct blink 30x100ms
  logPrintln(LOG_SYSTEM,bootInfo());
  espRestart("EEPROM boot save"); // restart after save
}

// Loads configuration from EEPROM into RAM
void bootRead() {
  eeRead();
  if(strcmp(eeType,bootType)!=0) { 
    sprintf(buffer,"EEPROM wrong"); logPrintln(LOG_SYSTEM,buffer); // => eeprom error => direct blink 2x100ms
    ledBlink(2,100); return ; 
  }    

  sprintf(buffer,"EEPROM boot read mode:%d timestamp:%d espName:%s wifi_ssid:%s",eeMode,eeBoot.timestamp,eeBoot.espName,eeBoot.wifi_ssid); logPrintln(LOG_SYSTEM,buffer); 
  logPrintln(LOG_SYSTEM,bootInfo());
  mqttSet(eeBoot.mqtt);  // set mqtt
  if(!is(eeBoot.espPas)) { setAccess(ACCESS_ADMIN); } // without espApd admin=true
  ledBlink(1,100); // OK => direct blink 1x100ms
}

// Reset EEPROM bytes to '0' for the length of the data structure
void bootClear() {
  logPrintln(LOG_SYSTEM,"EEPROM boot clear"); ledBlink(10,100); // clear now => direct blink 10x100ms
  EEPROM.begin(EEBOOTSIZE);
  for (int i = 0 ; i < EEBOOTSIZE ; i++) {EEPROM.write(i, 0);}
  delay(200);
  EEPROM.commit();
  EEPROM.end();  
}

byte _bootRestVal=0;

/* boot reset */
char* bootReset(char *p) {
  int i=toInt(p);
  if(i>1 && i==_bootRestVal) { bootClear(); return "reset done";} // do reset 
  else { _bootRestVal=random(2,99); sprintf(buffer,"%d",_bootRestVal); return buffer; } // without set new reset value
}

void bootPrivat() {
  sprintf(eeBoot.wifi_ssid,wifi_ssid_default); // my privat WIFI SSID of AccessPoint
  sprintf(eeBoot.wifi_pas,wifi_pas_default);   // my privat WIFI password of AccessPoint
  sprintf(eeBoot.espPas,user_pas);   // my privat WIFI password of AccessPoint
  sprintf(eeBoot.mqtt,mqtt_default);            // my privat MQTT server
  eeMode=EE_MODE_PRIVAT; // set privat mode 
}

char* bootMode(int mode) {
  if(mode>=0) { eeSetMode(mode); }
  return bootInfo();  
}

//-------------------------------------------------------------------------------------------------------------------
// Access

/* is actual login */
boolean _isLogin=false;

/* have access level */
bool isAccess(int requireLevel) {   
  if(_isLogin || !is(eeBoot.espPas)) { return true; } // isLogin or no password given
  else if(requireLevel>eeBoot.accessLevel) { return true; } // access free
  else { logPrintln(LOG_ERROR,"ACCESS DENIED"); return false; }
}

void setAccess(boolean login) { _isLogin=login; }
void setAccessLevel(byte accessLevel) { eeBoot.accessLevel=accessLevel; }

/* login (isAdmin=true) */
boolean login(char *p) {
  if(!is(eeBoot.espPas) || equals(p, eeBoot.espPas))  {  _isLogin=true; return true; }
  else { _isLogin=false; return false; }
}


//-------------------------------------------------------------------------------------------------------------------
// mDNS

void mdnsSetup() {
  if(!is(eeBoot.espName)) { return ; }
  else if(MDNS.begin(eeBoot.espName)) { 
    if(webEnable) { MDNS.addService("http", "tcp", 80); }
    sprintf(buffer,"MDNS setup %s http_tcp",eeBoot.espName); logPrintln(LOG_INFO,buffer); 
  }else { sprintf(buffer,"MDNS error"); logPrintln(LOG_ERROR,buffer); }
}

#ifdef ESP32
  void mdnsLoop() {  
  }

#elif defined(ESP8266)

  void mdnsLoop() {  
    MDNS.update();
  }

#endif

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
// Wifi

#ifdef ESP32
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(TARGET_RP2040)
  #include <WiFi.h>
#endif


void webSetup();

#define MAX_NO_WIFI 30 // Max time 60s no wifi
#define MAX_NO_SETUP 10 // Max time 60s no wifi
unsigned long *wifiTime = new unsigned long(0);

#define WIFI_CON_OFF 0
#define WIFI_CON_CONNECTING 2
#define WIFI_CON_CONNECTED 3

byte wifiStat=WIFI_CON_OFF; //  wifi status
int bootWifiCount=0; // counter for wifi not reached

    
// start scan network
char* wifiScan() { 
  byte networksFound=WiFi.scanNetworks(); 
  sprintf(buffer,"WIFI scan %d\n",networksFound);
  for (int i = 0; i < networksFound; i++) {
    if(strlen(buffer)<bufferMax-40) {    
      sprintf(buffer+strlen(buffer)," %d: %s (Ch:%d %ddBm) %d\n", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) );
    }
  }
  return buffer; 
}


//-------------------------------------------------------------------------------------------------------------------
// time

//const char* const PROGMEM NTP_SERVER[] = {"fritz.box", "de.pool.ntp.org", "at.pool.ntp.org", "ch.pool.ntp.org", "ptbtime1.ptb.de", "europe.pool.ntp.org"};
//const char *NTP_TZ    = "CET-1CEST,M3.5.0,M10.5.0/3";
#define timezone "CET-1CEST,M3.5.0/02,M10.5.0/03" // https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv

// callback when ntp time is given
void ntpSet(struct timeval *tv) {
  time(&timeNow);                    // read the current time
  localtime_r(&timeNow, &tm);           // update the structure tm with the current time
  sprintf(buffer,"NTP set %d",timeNow); logPrintln(LOG_INFO,buffer);
  ntpRunning=true;
}

/* ntp/timeserver config */
void ntpSetup() {
//  esp_sntp_servermode_dhcp(1);  // (optional)

  if(is(eeBoot.wifi_ntp,1,32)) { 
    ntpServer=eeBoot.wifi_ntp;     
  }else { 
    String gw=WiFi.gatewayIP().toString();
    ntpServer = copy((char*)gw.c_str());
  }
  
  if(ntpServer==NULL) {  logPrintln(LOG_ERROR,"ntp server missing");  return ; }

  int gtm_timezone_offset=0; // gmt+ or gmt-
  int dst=0; // 0=winter-time / 1=summer-time
  sprintf(buffer,"NTP start '%s' gtm_timezone_offset:%d dst:%d",ntpServer,gtm_timezone_offset,dst); logPrintln(LOG_INFO,buffer); 
  configTime(gtm_timezone_offset * 3600, dst*3600, ntpServer); //ntpServer
  sntp_set_time_sync_notification_cb(ntpSet); // callback on ntp time set
}

/* set time and timeServer [ADMIN] */
char* timeSet(char* time,char* timeServer) {
  if(time!=NULL && strlen(time)>0) {
    if(!isAccess(ACCESS_ADMIN)) { return "access denied"; }
    time_t newTime=(time_t)atol(time);
    timeval tv;tv.tv_sec = newTime;    
    settimeofday(&tv,NULL); // set your time (e.g set time 1632839830)
  }
  if(timeServer!=NULL) { 
    if(!isAccess(ACCESS_ADMIN)) { return "access denied"; }
    strcpy(eeBoot.wifi_ntp,timeServer);    
  }
  return timeInfo();
}

//-------------------------------------------------------------------------------------------------------------------

#if netEnable

  #include <ESPping.h>
  
  IPAddress pingIP;

  /* dns resolve ip/host to ip (e.g. char* name=netDns("192.168.1.1"); */
  char* netDns(char *ipStr) {
      WiFi.hostByName(ipStr, pingIP);  
      sprintf(buffer,"%s",pingIP.toString().c_str()); return buffer;
  }

  /* ping given ip/host and return info (e.g. char* info=cmdPing("192.168.1.1"); ) */
  char* cmdPing(char *ipStr) { 
    WiFi.hostByName(ipStr, pingIP);  
    int time=-1;
    if(Ping.ping(pingIP,1)) { time=Ping.minTime(); } 
    sprintf(buffer,"PING %s=%s time:%d",ipStr,pingIP.toString().c_str(),time); return buffer;
  }

#else 
  char* netDns(char *ipStr) { return NULL; }
  char* cmdPing(char *ipStr) { return NULL; }
#endif

//-------------------------------------------------------------------------------------------------------------------

//TODO #include "user_interface.h"

void sleepOver(void) {
  sprintf(buffer,"SLEEP over mode:%d",eeMode); logPrintln(LOG_INFO,buffer);
    
  wifiInit();
}

void sleep(byte mode,long sleepTimeMS) { //10e3=10s
    if(mode==0) { 
        sleepOver();  // switch all on
        
    } else if(mode==1) { 
        wifiOff(); }  // Modem OFF
        
    else if(mode==2) {  // LightSleep
      WiFi.mode(WIFI_OFF);  // WIFI off
      delay(sleepTimeMS + 1);
      
    }else if(mode==3) {  // DeepSleep
      WiFi.mode(WIFI_OFF);  // WIFI off
      uint64_t sleepTimeMicroSeconds = 10e6;
      ESP.deepSleep(sleepTimeMS*1000); 
      // reset is called after deepsleep 
    }  
}


void sleep(char* sleepMode,char *sleepTimeMS) {
  byte m=atoi(sleepMode); int s=atoi(sleepTimeMS);
  sprintf(buffer,"SLEEP %d %d",m,s);logPrintln(LOG_INFO,buffer);
  sleep(m,(long)s);
}

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------

// boot clear with BOOT=>BLINK 2x=>PRESS 2s=>WIFI_AP / 5s=>clear 
#if swEnable
  void bootSW() {    
    if(sw==NULL) { return ; } // now swtich for SW-Boot  
    delay(1000);
    if(!sw->isOn()) { return ; }
    logPrintln(LOG_INFO,"bootSetupSW -----------------------------------");
    byte count=0;
    while(sw->isOn() && count<4) { // while on, max 4*5s=20s
      delay(5000);
      ledBlink(count,100); // blink count
      count++;    
    }
    if(count==1) {  logPrintln(LOG_SYSTEM,"BOOTSW ap mode");mode=MODE_WIFI_AP; } // hold sw >=5s => Mode Wifi AccessPoint
    else if(count==2) {  } 
    else if(count==3) {  logPrintln(LOG_SYSTEM,"BOOTSW clear");bootClear(); espRestart("SW clear"); } // hold sw >=5s => reset
  }
#else 
  void bootSW() {}
#endif

void bootSetup() {


  // reset sw
  bootRead();
  bootSW();
  if(MODE_DEFAULT==EE_MODE_PRIVAT) { bootPrivat(); } // is privat mode as default => use privat config

  sprintf(buffer,"BOOT init mode:%d espName:%s espBoard:%s wifi_ssid:%s timestamp:%d", eeMode,eeBoot.espName,eeBoot.espBoard,eeBoot.wifi_ssid,eeBoot.timestamp); logPrintln(LOG_INFO,buffer);
}

//--------------------------------------------------------------------------------------
// Wifi as AccessPoint


char apSSID[20]="";

IPAddress ap_IP(192,168,0,1);
IPAddress ap_gateway(192,168,0,1);
IPAddress ap_subnet(255,255,255,0);

DNSServer dnsServer;
boolean dnsRedirectEnable=true;

//-------------------------------------------

/* return info of wifi (log-buffer) */
char* wifiInfo() {
  sprintf(buffer,"WIFI mode:%d ip:%s wifi_ssid:%s - mac:%08X status:%d signal:%d rmac:%08X wifiStat:%d bootWifiCount:%d",
    eeMode,WiFi.localIP().toString().c_str(),WiFi.SSID().c_str()
    ,WiFi.macAddress(),WiFi.status(),WiFi.RSSI(),WiFi.BSSID()
    ,wifiStat,bootWifiCount); 
  
  if(is(apSSID)) {
    IPAddress myIP = WiFi.softAPIP(); 
    sprintf(buffer+strlen(buffer)," - AP_SSID:%s AP_IP:%s", apSSID,myIP.toString().c_str()); 
  }

  return buffer;
}

/* set wifi */ 
char* wifiSet(char *wifi_ssid,char *wifi_pas) {  
  if(is(wifi_ssid) && is(wifi_pas) && isAccess(ACCESS_ADMIN)) {     
    sprintf(buffer,"WIFI set %s %s",wifi_ssid,wifi_pas); logPrintln(LOG_SYSTEM,buffer);
    strcpy(eeBoot.wifi_ssid,wifi_ssid); 
    strcpy(eeBoot.wifi_pas,wifi_pas);
  }
  return wifiInfo();
}

/* setup wifi and espPas + save + boot */
char* setup(char *wifi_ssid, char *wifi_pas,char *espName, char *espPas) {
  if(isAccess(ACCESS_ADMIN)) { return "no access"; }
  if(!is(wifi_ssid) && !(wifi_pas)) { return "wrong"; }

  eeBoot= eeBoot_t(); // reinit 
  strcpy(eeBoot.wifi_ssid,wifi_ssid); 
  strcpy(eeBoot.wifi_pas,wifi_pas);  
  if(is(espName)) { strcpy(eeBoot.espName,espName); }
  if(is(espPas)) { strcpy(eeBoot.espPas,espPas); }
  bootSave();
  return bootInfo();
}

//-------------------------------------------

// start WIFI as AccessPoint
void wifiAccessPoint(boolean setpUpAP) {   
//TODO  ledBlinkPattern(0,&ledPatternFlashSlow); // blink AP mode
  if(setpUpAP) {
    sprintf(apSSID,"%s",wifi_setup);
  }else {
    uint32_t chipid=espChipId(); // or use WiFi.macAddress() ?
    snprintf(apSSID,20, "%s%08X",APP_NAME_PREFIX,chipid);
  }

  WiFi.softAPConfig(ap_IP, ap_gateway, ap_subnet);  
  WiFi.softAP(apSSID);
  setAccess(ACCESS_ADMIN); // enable admin in AP
  IPAddress myIP = WiFi.softAPIP();
  sprintf(buffer,"WIFI AccessPoint SSID:%s IP:%s", apSSID,myIP.toString().c_str()); logPrintln(LOG_SYSTEM,buffer); 
//TODO  ledBlinkPattern(0,&ledPatternFlashSlow); // blink AP mode

  dnsServer.start(53, "*", myIP); // redirect all dns request to esp
  dnsRedirectEnable=true;

  if(webEnable) { webSetup(); } // start web

  appIP=ap_IP.toString(); // set ip to ap_IP
  bootWifiCount=1;
}

/* this ap is connected from client */
void wifiAPClientConnect() {
//  if (WiFi.status() == WL_AP_CONNECTED) {
//    sprintf(buffer,"client connect to ap"); logPrintln(LOG_DEBUG,buffer);    
//  }  
}

/** this client connected to remote set_up */
void wifiAPConnectoToSetup() {
    if (WiFi.status() == WL_CONNECTED) {      
      String gw=WiFi.gatewayIP().toString();
      String setupUrl="http://"+gw+"/setupDevice";
      sprintf(buffer,"wifiAPConnectoToSetup call %s",setupUrl.c_str()); logPrintln(LOG_SYSTEM,buffer);
      char* ret=cmdRest((char*)setupUrl.c_str());
      logPrintln(LOG_SYSTEM,buffer);
    }
}

//--------------------------------------------------------------------------------------

// wifi check connecting
void wifiConnecting() {
    if (WiFi.status() == WL_CONNECTED) {      
      String gw=WiFi.gatewayIP().toString();
      appIP=WiFi.localIP().toString();      
      sprintf(buffer,"WIFI mode:%d Connectd IP:%s Gateway:%s DNS:%s ", eeMode,appIP.c_str(), gw, WiFi.dnsIP().toString()); logPrintln(LOG_SYSTEM,buffer); 
      wifiStat=WIFI_CON_CONNECTED;

      if(eeMode==EE_MODE_WIFI_TRY) { eeSetMode(EE_MODE_OK); eeSave(); } // try => cl  
      ledOff();

      // enable services
      if(webEnable) { webSetup(); }
      if(mdnsEnable) { mdnsSetup(); } 
      if(ntpEnable) { ntpSetup(); }

    }else { // Connecting
      if(bootWifiCount==0) { }
      else if( eeMode == EE_MODE_SETUP && bootWifiCount<MAX_NO_SETUP) {  // try faild
        logPrintln(LOG_INFO,"no wifi setup"); 
        eeSetMode(EE_MODE_AP); eeSave();espRestart("no setup wifi, fallback ap"); // fallback to AccessPoint on faild try  

      }else if(bootWifiCount<MAX_NO_WIFI) {              
        if(serialEnable) {sprintf(buffer,"%d",WiFi.status()); Serial.print(buffer); }   
        bootWifiCount++;    
      
      } else if( eeMode == EE_MODE_WIFI_TRY) {  // try faild
        sprintf(buffer,"WIFI error CL-TRY ssid:%s reset to AP", eeBoot.wifi_ssid); logPrintln(LOG_SYSTEM,buffer); 
        eeSetMode(EE_MODE_SETUP); eeSave(); espRestart("no wifi, fallback setup"); // fallback to AccessPoint on faild try      

      } else {
         //sprintf(buffer,"WIFI error connect ssid:%s mode:%d", eeBoot.wifi_ssid,eeBoot.mode); logPrintln(LOG_SYSTEM,buffer); 
         bootWifiCount=1; // try again
      }
    }  
}

//--------------------------------------------------------------------------------------


/* wifi login to SSID Router */
void wifiStart() {
//TODO  ledBlinkPattern(0,&ledPattern2Flash); // Blink unlimeted 2Flash
//TODO  configTime(NTP_TZ, NTP_SERVER[1]); // define timezone
//TODO  settimeofday_cb(ntpSet);  /// callback on ntp time set

  sprintf(buffer,"WIFI connecting SSID:%s ...", eeBoot.wifi_ssid); logPrintln(LOG_INFO,buffer); 
  delay(100);
//TODO ?  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(eeBoot.wifi_ssid, eeBoot.wifi_pas);
  bootWifiCount=1;  
  wifiStat=WIFI_CON_CONNECTING;
}

/* try connect wifi*/
void wifiTry() {
//TODO  ledBlinkPattern(0,&ledPattern2Flash); // Blink unlimeted 2Flash
  sprintf(buffer,"WIFI try connecting SSID:%s ...", eeBoot.wifi_ssid); logPrintln(LOG_INFO,buffer); 
  delay(10);  
  WiFi.persistent(false);
  WiFi.mode(WIFI_AP_STA); //WIFI_STA
  // WIFI
  WiFi.begin(eeBoot.wifi_ssid, eeBoot.wifi_pas);
  // AP
  WiFi.softAPConfig(ap_IP, ap_gateway, ap_subnet);  
  WiFi.softAP(apSSID);
  bootWifiCount=1;  
  wifiStat=WIFI_CON_CONNECTING;
}

/* wifi login to setup Router */
void wifiStartSetup() {
  sprintf(buffer,"WIFI setup SSID:%s ...", wifi_setup); logPrintln(LOG_INFO,buffer); 
  delay(10);
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_setup, wifi_setup);
  bootWifiCount=1;  
  wifiStat=WIFI_CON_CONNECTING;
}

/* set wifi offline */
void wifiOff() {
  logPrintln(LOG_INFO,"WIFI off");
  WiFi.mode(WIFI_OFF);
}

// validate wifi connection
void wifiValidate() {
  if(eeMode==EE_MODE_AP) { wifiAPClientConnect(); }
  else if(eeMode==EE_MODE_SETUP) { wifiAPConnectoToSetup(); }

  
  else if (wifiStat==WIFI_CON_CONNECTING) { // Connect or Reconnect
     wifiConnecting();    
     
  } else if (eeMode < EE_MODE_OK) {  

  } else {
    if (WiFi.status() != WL_CONNECTED) { // connection loose
      wifiStart();
    }

  }
}


// start wifi 
void wifiInit() {
    boolean ssidOk=is(eeBoot.wifi_ssid) && is(eeBoot.wifi_pas);

    if(ssidOk && eeMode==EE_MODE_WIFI_TRY) { // => Try 
      wifiTry(); // start Client
    
    }else if(ssidOk && (eeMode>=EE_MODE_PRIVAT && eeMode<=EE_MODE_WRONG )) { // => NormalMode
      wifiStart(); // start Client 
        
    }else if(eeMode==EE_MODE_SETUP) { // => SetupMode
      wifiStartSetup(); // start AccessPoint     

    }else if(eeMode==EE_MODE_AP || eeMode==EE_MODE_ERROR) { // => SetupMode
      wifiAccessPoint(false); // start AccessPoint 

    }else {
        wifiOff(); // => OFF  
    }
}


//----------------------------

void wifiSetup() {
  if(wifiEnable) { wifiInit(); }
}

void wifiLoop() {
  if(wifiEnable && isTimer(wifiTime, 1000)) { wifiValidate(); } // every second
  if(mdnsEnable) { mdnsLoop(); } 
  if(dnsRedirectEnable) { dnsServer.processNextRequest();  } // Process DNS requests 
}

void wifiStart(boolean on) { 
  wifiEnable=on; if(on) { eeSetMode(EE_MODE_WIFI_TRY); wifiSetup(); } 
  else { WiFi.mode(WIFI_OFF); eeSetMode(EE_MODE_WIFI_OFF); }
}

//-------------------------------------------------------------

#if otaEnable
  #include <NetworkUdp.h>
  #include <ArduinoOTA.h>

void otaSetup() {
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) { type = "sketch"; }
      else {  type = "filesystem"; } // U_SPIFFS
      FILESYSTEM.end();
      logPrintln(LOG_INFO,"Start updating " + type);
    })
    .onEnd([]() { logPrintln(LOG_INFO,"End");})
    .onProgress([](unsigned int progress, unsigned int total) { sprintf(buffer,"Progress: %u%%", (progress / (total / 100))); logPrintln(LOG_DEBUG,buffer); })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) { logPrintln(LOG_INFO,"Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) { logPrintln(LOG_INFO,"Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) { logPrintln(LOG_INFO,"Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) { logPrintln(LOG_INFO,"Receive Failed");
      } else if (error == OTA_END_ERROR) { logPrintln(LOG_INFO,"End Failed");
      }
    });

  if(is(eeBoot.espName)) { ArduinoOTA.setHostname(eeBoot.espName); }
  if(is(eeBoot.espPas)) { ArduinoOTA.setPassword(eeBoot.espPas);}

  logPrintln(LOG_DEBUG,"ota start");
  ArduinoOTA.begin();
}

void otaLoop() {
  ArduinoOTA.handle();
}
#else 
void otaSetup() {}
void otaLoop() {}
#endif



