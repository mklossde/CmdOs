
// Application Start SRC

#include <ESPAsyncWebServer.h>
#include <privatdata.h>

const char *prgTitle = "MyApp";
const char *prgVersion = "V1.0.0";

const char* user_admin = "admin"; // default user
char user_pas[]="admin";   // default espPas

const char *wifi_ssid_default = ""; PRIVAT_WIFI_SSID; // PRIVAT_WIFI_SSID via #include <privatdata.h>
const char *wifi_pas_default = ""; PRIVAT_WIFI_PAS;   // PRIVAT_WIFI_PAS via #include <privatdata.h>
const char *mqtt_default = ""; //PRIVAT_MQTTSERVER;     // PRIVAT_MQTTSERVER via #include <privatdata.h>

byte MODE_DEFAULT=21; // normal=21=MODE_WIFI_CL_TRY /
//byte MODE_DEFAULT=20; //  MODE_PRIVAT=20=load privat values, use wifi_ssid_default and wifi_pas_default and mqtt_default
//byte MODE_DEFAULT=0; // EE_MODE_FIRST=0=RESET on start

boolean serialEnable=true; // enable/disbale serial in/out

boolean wifiEnable=true;  // enable/disbale wifi
boolean ntpEnable=true; // enable time server
boolean webEnable=true;    // enable/disbale http server
boolean mdnsEnable=true;   // enable/disable mDNS detection 
boolean bootSafe=true;    // enable/disbale boot safe

#define enableFs true         // enable fs / SPIFFS

#define netEnable true       // enable/disbale network ping/dns/HttpCLient 
#define webSerialEnable true // enable/disbale web serial
#define mqttEnable false      // enable/disbale mqtt

#define otaEnable true        // enabled/disbale ota update 
#define updateEnable true     // enabled/disbale update firmware via web 

#define ledEnable false       // enable/disbale serial
int ledGpio=15;            // io of led
boolean ledOnTrue=true;           // gpio false=led-on

#define SW_MODE_NORMAL 0 // Button
#define SW_MODE_PULLUP 1 // Button with pullUp enabled
#define SW_MODE_PULLDOWN 2 // Button with pullDown enabled
#define SW_MODE_TOUCH 3   // TOUCH BUTTON

#define swEnable true        // enable/disbale switch
int swGpio=32;                // io pin of sw 
byte swOn=3;                // 0=pressend on false, 1= pressed on true, 2=use actual as off, 3-9=use actual minus dif, >9 pressed below swON
byte swMode=SW_MODE_TOUCH;  // mode pullUp/pullDown/touch

int swTimeBase=100;     // unprell (max state change time) and tick time base (e.g. 100ms)
byte swTickShort=2;     // swTickShort*swTimeBase => 5*100 => 500ms;
byte swTickLong=5;      // swTickLong*swTimeBase => 10*100 => 1s;

int _webPort = 80;
AsyncWebServer server(_webPort);
 
//--------------------------------------------------------------


char* appCmd(char *cmd, char **param) {
  return cmd; // unkown cmd => use cmd as string
}

//--------------------------------------------------------------


/* callback to add app web pages */
void webApp() {
//  server.on("/app", HTTP_GET, [](AsyncWebServerRequest *request) { matrixWeb(request); });
//  server.on("/appSetup", HTTP_GET, [](AsyncWebServerRequest *request) { matrixWebSetup(request); });
}

//--------------------------------------------------------------

void setup() {
  cmdOSSetup();
  if(isModeNoError()) { 
    // all work fine - app setup here
    appSetup();
  }  
}

void loop() {
  cmdOSLoop();
  if(isModeNoError()) { 
    // all works fine - app loop here 
    appLoop();
  }  
}
