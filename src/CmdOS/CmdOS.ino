
// CmdOS 
// Application Start SRC

#include <ESPAsyncWebServer.h>
 
const char *prgTitle = "MyApp";
const char *prgVersion = "V1.0.0";

const char* user_admin = "admin"; // default user
char user_pas[]="";   // default espPas

const char *wifi_ssid_default = ""; // PRIVAT_WIFI_SSID; // define in privatdata.h 
const char *wifi_pas_default = ""; //PRIVAT_WIFI_PAS;   // define in privatdata.h 
const char *mqtt_default = ""; //PRIVAT_MQTTSERVER;     // define in privatdata.h 

byte MODE_DEFAULT=21; // MODE_WIFI_CL_TRY=21 / MODE_PRIVAT

boolean serialEnable=true; // enable/disbale serial in/out

boolean wifiEnable=true;  // enable/disbale wifi
boolean ntpEnable=true; // enable time server
boolean webEnable=true;    // enable/disbale http server
boolean mdnsEnable=true;   // enable/disable mDNS detection 
boolean bootSafe=true;    // enable/disbale boot safe

#define enableFs true         // enable fs / SPIFFS

#define netEnable true       // enable/disbale network ping/dns/HttpCLient 
#define webSerialEnable false // enable/disbale web serial
#define mqttEnable false      // enable/disbale mqtt

#define otaEnable false        // enabled/disbale ota update 
#define updateEnable false     // enabled/disbale update firmware via web 

#define ledEnable true       // enable/disbale serial
int ledGpio=15;            // io of led
boolean ledOnTrue=true;           // gpio false=led-on

#define swEnable true        // enable/disbale switch
int swGpio=0;                // io pin of sw 
int swTimeBase=100;       // prell and lonePress Timebase (e.g. 100ms)
boolean swOnTrue=false;      // gpio false=sw-pressed


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
