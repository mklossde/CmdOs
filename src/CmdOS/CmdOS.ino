
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
#define mqttEnable true      // enable/disbale mqtt

#define otaEnable true        // enabled/disbale ota update 
#define updateEnable false     // enabled/disbale update firmware via web 

#define ledEnable false       // enable/disbale serial
#define ledGpio 4             // io of led
#define LED_ON true           // gpio false=led-on

#define swEnable false        // enable/disbale switch
#define swGpio 13             // io pin of sw 
#define SW_ON false           // gpio false=sw-pressed


int _webPort = 80;
AsyncWebServer server(_webPort);

//--------------------------------------------------------------


char* appCmd(char *cmd, char *p0, char *p1,char *p2,char *p3,char *p4,char *p5,char *p6,char *p7,char *p8,char *p9) {
    return "unkown cmd";
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
  if(isModeOk()) { 
    // all work fine - app setup here
  }  
}

void loop() {
  cmdOSLoop();
  if(isModeOk()) { 
    // all works fine - app loop here 
  }  
}
