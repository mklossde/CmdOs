
#if mqttEnable

// MQTT
#include <PubSubClient.h>

unsigned long *mqttTime = new unsigned long(0); // mqtt timer

unsigned int mqttConFail=0;  // number of connection faild
static int mqttStatus = 0; // 0=Off, 1=Connecting, 2=On

char* mqttPrefix="device/esp";

boolean mqttSSL=false;
char* mqttClientName;
char* mqttServer;
int mqttPort;
char* mqttUser;
char* mqttPas;


char *mqttTopicOnline;   // topic device online/offline
char *mqttTopicStat;    // topic to send status of entitys
char *mqttTopicReceive; // topic path of all commands (e.g. device/esp/EspBoot00DC9235/control)

char* mqttTopicCmd=NULL; // topic for cmd messages (e.g. device/esp/EspBoot00DC9235/control/cmd)
char* mqttTopicResponse=NULL; // topic for resposne of cmd messages (e.g. device/esp/EspBoot00DC9235/stat_cmd) 

WiFiClient *mqttWifiClient=NULL;
#ifdef ESP32
  #include <NetworkClientSecure.h>
  NetworkClientSecure *mqttClientSSL=NULL; // WiFiClientSecure / NetworkClientSecure
#elif defined(ESP8266)
  #include <WiFiClientSecure.h>
  WiFiClientSecure *mqttClientSSL=NULL; // WiFiClientSecure / NetworkClientSecure
#endif   

PubSubClient *mqttClient=NULL;

static char* mqttTopic=new char[64]; // buffer of topic
static char* mqttMessage=new char[1024]; // buffer of message

/*
class Param {
  private:
    char *_name;
    byte _type;
    char *_remote;
    boolean _change=false;
  public:
    Param(char *name,byte type,char *remote) { _name=name; _type=type; _remote=remote; _change=change; }
};

MapList params;
*/


//-------------------------------------------------------------------------------------
// mqtt-url:  mqtt://USER:PAS@SERVER:PORT or mqtts://USER:PAS@SERVER:PORT /

/* get mqtt infos */
char* mqttInfo() {
  if(!is(eeBoot.mqtt) || !is(mqttUser) || !is(mqttServer) ) { return "mqtt not defined"; }
  char *type="mqtt"; if(mqttSSL) { type="mqtts"; }  
  if(is(mqttUser)) {
    sprintf(buffer,"MQTT status:%d type:%s user:%s pas:%d server:%s port:%d (ee:%s)",
      mqttStatus,type,to(mqttUser),is(mqttPas),to(mqttServer),mqttPort,to(eeBoot.mqtt)); return buffer;
  }else { sprintf(buffer,"MQTT status:%d  %s://%s:%d (ee:%s)",mqttStatus,type,to(mqttServer),mqttPort,to(eeBoot.mqtt)); return buffer;}
}

/* split mqtt-url */
void mqttSetUrl(char* mqttUrl) {
  mqttUser=NULL; mqttPas=NULL; mqttServer=NULL; mqttPort=1833;  
  if(!is(mqttUrl,3,127)) { logPrintln(LOG_ERROR,"MQTT missing/wrong"); return ; }

  char *mqtt=copy(mqttUrl);  
   if(strncmp(mqtt, "mqtt://",7)==0) { mqttSSL=false;  } 
   else if(strncmp(mqtt, "mqtts://",7)==0) { mqttSSL=false; }
   else { return ; } 

   char *ptr; strtok_r(mqtt, "://",&ptr); ptr+=2; // start
   if(ptr==NULL) { logPrintln(LOG_ERROR,"MQTT mqtt/mqtts user"); return ;} 

   if(strchr(ptr, '@')!=NULL) {
    mqttUser = strtok_r(NULL, ":",&ptr); if(mqttUser==NULL) { logPrintln(LOG_ERROR,"MQTT missing user"); return ;} 
    mqttPas = strtok_r(NULL, "@",&ptr); if(mqttPas==NULL) { logPrintln(LOG_ERROR,"MQTT missing pas");  return ;} 
   }
   
   mqttServer = strtok_r(NULL, ":",&ptr); if(mqttServer==NULL) { logPrintln(LOG_ERROR,"MQTT missing server"); return ;} 
   char *port=strtok_r(NULL, "",&ptr); if(port==NULL) { logPrintln(LOG_ERROR,"MQTT missing port"); return ;} 
   mqttPort=atoi(port);

   sprintf(buffer,"MQTT set ssl:%d server:%s port:%d user:%s pas:%s", mqttSSL, to(mqttServer),mqttPort,to(mqttUser),to(mqttPas));  logPrintln(LOG_INFO,buffer);
//TODO memory leek here   delete[] mqtt; 
}

/* set mqtt url */
char* mqttSet(char* mqtt) {
  if(is(mqtt,1,128) && isAccess(ACCESS_ADMIN)) {     
    strcpy(eeBoot.mqtt,mqtt);  

    mqttSetUrl(eeBoot.mqtt);
  }
  return mqttInfo();
}


//-------------------------------------------------------------------------------------
// publish messages

/* publich log to mqtt */
void mqttLog(char *message) {
  if(eeBoot.mqttLogEnable) {
      if (mqttStatus != 2) { return ; }
      sprintf(mqttTopic,"%slog",mqttTopicStat);
      mqttClient->publish(mqttTopic, message);
  }
}

/*
void publishValueMessage(char *name,char *message) {
  if (mqttStatus != 2) { return ; }  
  sprintf(mqttTopic,"%s/%s%/value/%s",mqttPrefix,mqttClientName,name);
  boolean ok=mqttClient->publish(mqttTopic, message);
  sprintf(buffer,"MQTT publish %s => %s ok:%d", mqttTopic,message,ok);  logPrintln(LOG_DEBUG,buffer);
}


void publishValue(char *key,char *value) {
//  sprintf(message,"{\"%s\":\"%s\"}",key,value);  
//  publishStatus(message);
  publishValueMessage(key,to(value));
}
*/

/*
void publishResponse(char *id,char *result) {
  if (mqttStatus != 2) { return ; }
  if(result!=NULL && sizeof(result)>0) {
    if(id!=NULL) { sprintf(mqttMessage,"%s:%s", id,result); }
    else { sprintf(mqttMessage,result); }
    boolean ok=mqttClient->publish(mqttTopicResponse, mqttMessage);
    sprintf(buffer,"MQTT publish %s => %s ok:%d", mqttTopicResponse,mqttMessage,ok);  logPrintln(LOG_DEBUG,buffer);
  }
}
*/

/** subcribe topic to attr **/
void mqttAttr(char *topic,boolean on) {
  if(mqttStatus != 2) { return ; }  
  sprintf(buffer,"MQTT attr via topic %s",topic);
  if(on) { 
    if(attrHave(topic)) {  return ; } // alrady have
    char* t=copy(topic);
    attrMap.replace(t,(char*)"",0); boolean ok=mqttClient->subscribe(t); 
    sprintf(buffer,"MQTT subsrcibe '%s' attr:%s", topic,topic,ok); logPrintln(LOG_DEBUG,buffer);
  } else { 
    boolean ok=mqttClient->unsubscribe(topic); attrMap.del(topic); 
    sprintf(buffer,"MQTT unsubsrcibe '%s' attr:%s ok:%d", topic,topic,ok); logPrintln(LOG_DEBUG,buffer);
  } 
}

//-----------------------------------------------------

/* publish a maessage */
boolean mqttPublish(char* topic,char *message) {
  if (mqttStatus != 2) { return false; }
  boolean ok=mqttClient->publish(topic, message);  
  if(!ok) { sprintf(buffer,"MQTT publish ERROR %s len:%d",topic,strlen(message));logPrintln(LOG_ERROR,buffer); }
  else { 
//Serial.print(topic); Serial.print("=");Serial.println(message); Serial.print(" len:");Serial.println(strlen(message));
    sprintf(buffer,"MQTT publish %s",topic); logPrintln(LOG_DEBUG,buffer);
  }
  return ok;
}

/* subscibe a topic */
boolean mqttSubscribe(char *topic) {
    if (mqttStatus != 2) { return false; }
    boolean ok=mqttClient->subscribe(topic); // subscribe cmd topic           
    if(!ok) { sprintf(buffer,"MQTT subscribe ERROR %s",topic);logPrintln(LOG_ERROR,buffer); }
    else { sprintf(buffer,"MQTT subscribe %s ok:%d", topic,ok); logPrintln(LOG_INFO,buffer); }
    return ok;
}

//-----------------------------------------------------

void mqttPublishState(char *name,char *message) {
  if(!is(message) ||  mqttStatus != 2) { return ; }
  char *espStat=concat(mqttTopicStat,name,NULL);
  mqttPublish(espStat,message);  
  free(espStat);
}

//-------------------------------------------------------------------------------------
// Receive messages

void mqttReceive(char* topic, byte* payload, unsigned int length) {  

  if (mqttCmdEnable && strcmp(topic,mqttTopicCmd) == 0) {    
    char *msg=copy(NULL,(char*)payload,length);
    sprintf(buffer,"MQTT cmd '%s' %s %d", topic, msg,length); logPrintln(LOG_DEBUG,buffer);
    char *result=cmdLine(msg); 
    free(msg);
    mqttPublishState("cmd",result);

  } else if(attrMap.find(topic)!=-1) { 
    attrMap.replace(topic,(char*)payload,length);
    sprintf(buffer,"MQTT attrSet '%s'", topic); logPrintln(LOG_DEBUG,buffer);

  #if mqttDiscovery  
  } else if (strcmp(topic,mqttTopicReceive) == 0) { 
      char *msg=copy(NULL,(char*)payload,length);

      sprintf(buffer,"MQTT HA '%s'=%s", topic,msg); logPrintln(LOG_DEBUG,buffer);
      //char *result=cmdLine(msg); 
      //if(result!=NULL) { mqttPublish(mqttTopicStat,result); }
      free(msg);
  #endif

  } else { sprintf(buffer,"MQTT unkown topic '%s'", topic); logPrintln(LOG_DEBUG,buffer);}

}



//-------------------------------------------------------

#if mqttDiscovery

/* auto discover this as a light in HomeAssistant */
void mqttDiscover(char *type,char *name,boolean receiveOn) { 
    char *espStat=concat(mqttTopicStat,name,NULL);
    logPrintln(LOG_DEBUG,espStat);  
    char *topic=concat("homeassistant/",type,"/CmdOS/",eeBoot.espName,"/config",NULL);

    uint32_t chipid=espChipId();
    sprintf(buffer,"{\"name\":\"%s_%s\",\"uniq_id\":\"CmdOs%08X_%s\",\"avty_t\":\"%s\",\"stat_t\":\"%s\"",eeBoot.espName,name,chipid,name,mqttTopicOnline,espStat);
    if(receiveOn) { 
      char *espCmd=concat(mqttTopicReceive,name,NULL); 
      sprintf(buffer+strlen(buffer),",\"cmd_t\":\"%s\"",espCmd);
      free(espCmd);
    }
    sprintf(buffer+strlen(buffer),",\"dev\":{\"name\":\"%s\",\"ids\":\"CmdOs%08X\",\"configuration_url\":\"http://%s\",\"mf\":\"%s\",\"mdl\":\"%s\",\"sw\":\"%s\"}",eeBoot.espName,chipid,appIP.c_str(),APP_NAME_PREFIX,prgTitle,prgVersion);
    sprintf(buffer+strlen(buffer),"}");
    mqttPublish(topic,buffer);
    free(topic); free(espStat);

//    mqttPublish(mqttTopicStat, "");        // state        
}

#endif

//-------------------------------------------------------

boolean mqttRunning=false;

void mqttInit() {
  if(MODE_DEFAULT==EE_MODE_PRIVAT) { mqttSetUrl((char*)mqtt_default); } // my privat MQTT server)
  else if(!is(mqttServer) && is(eeBoot.mqtt)) { mqttSetUrl(eeBoot.mqtt);  }// set mqtt
  mqttClientName=eeBoot.espName;

  if(!is(mqttServer)) { logPrintln(LOG_SYSTEM,"MQTT error - mqttServer missing");  mqttConFail=3; return ; }  
  else if(mqttPort<1) { logPrintln(LOG_SYSTEM,"MQTT error - mqttPort missing");  mqttConFail=3; return ; }  
  else if(!is(eeBoot.espName)) { logPrintln(LOG_SYSTEM,"MQTT error - clientName missing");  mqttConFail=3; return ; }  

  if(!mqttSSL) { 
    mqttWifiClient=new WiFiClient();  
    mqttClient=new PubSubClient(*mqttWifiClient);
  }else {
    #ifdef ESP32
      mqttClientSSL=new NetworkClientSecure();  
    #elif defined(ESP8266)
      mqttClientSSL=new WiFiClientSecure();  
    #endif    
    mqttClient=new PubSubClient(*mqttClientSSL);
  }  

  mqttTopicOnline=concat(mqttPrefix,"/",eeBoot.espName,"/online",NULL); // availability/online
  mqttTopicStat=concat(mqttPrefix,"/",eeBoot.espName,"/stat_",NULL); 
  mqttTopicReceive=concat(mqttPrefix,"/",eeBoot.espName,"/control/",NULL);

  mqttClient->setBufferSize(512); // extends mqtt message size
  mqttClient->setCallback(mqttReceive);     
  mqttClient->setServer(mqttServer, mqttPort);

  mqttTopicCmd =concat(mqttTopicReceive,"cmd",NULL);
  mqttTopicResponse =concat(mqttTopicStat,"cmd",NULL);

  mqttRunning=true;
  *mqttTime=0; // start conection now
  if(!is(mqttUser) || !is(mqttPas)) {
    sprintf(buffer,"MQTT init %s:%d user: pas: client:%s", mqttServer, mqttPort,mqttClientName); logPrintln(LOG_INFO,buffer);
  }else { 
    sprintf(buffer,"MQTT init %s:%d user:%s pas:%s client:%s", mqttServer, mqttPort,mqttUser,mqttPas,mqttClientName); logPrintln(LOG_INFO,buffer);
  }
}


void mqttConnect() {    
    sprintf(buffer,"MQTT connecting... %s => %s", mqttClientName, mqttServer); logPrintln(LOG_DEBUG,buffer);    

    if (mqttClient->connect(mqttClientName,mqttUser,mqttPas,mqttTopicOnline, 0, true, "offline")) { // cennect with user and last will availability="offline"
      sprintf(buffer,"MQTT connected %s => %s", mqttClientName, mqttServer); logPrintln(LOG_INFO,buffer); 

      mqttStatus = 2;     
//      publishValue("status","connect");        
      
      char *cmdTopic=concat(mqttTopicReceive,"+",NULL); mqttSubscribe(cmdTopic); free(cmdTopic); // subscibe all cmds

      #if mqttDiscovery
        mqttDiscover("notify","state",false); // send mqtt homaAssistant state online Discover
        mqttDiscover("text","cmd",mqttCmdEnable); // send mqtt homaAssistant Discover
        mqttPublishState("state", "on");                   // discovery state
      #endif

      mqttClient->publish(mqttTopicOnline, "online");    // availability  online/offline
      mqttConFail=0;   // connected => reset fail

    } else {
      sprintf(buffer,"MQTT connection faild %d rc:%d - %s => %s", mqttConFail, mqttClient->state(),mqttClientName, mqttServer);  logPrintln(LOG_SYSTEM,buffer);      
      mqttConFail++; 
      mqttStatus = 0;
    }
}

void mqttDisconnect() { 
  if(mqttClient!=NULL && mqttClient->connected()) {
    mqttClient->publish(mqttTopicOnline, "offline");    // availability  online/offline
//    publishValue("status","disconnect");
    mqttClient->disconnect();     
  }
  mqttClient=NULL;
  mqttStatus = 0;
  mqttRunning=false;
}

void mqttOpen(boolean on) {
  if(on) { mqttInit(); } else { mqttDisconnect(); }
}
//-------------------------------------------------------------------------------------

void mqttSetup() {
  if(mqttEnable) { mqttInit(); }
}

void mqttLoop() {
  if(mqttRunning && is(mqttServer) && eeMode==EE_MODE_OK) {
    if(mqttClient==NULL) { mqttInit(); }
    else if (!mqttClient->connected()) { 
        if(mqttConFail<3 && isTimer(mqttTime, 1000)) { 
          mqttConnect(); // every second => reconnect mqtt
        }else if(mqttConFail>=3 && isTimer(mqttTime, 60000)) { 
          mqttConnect(); // every min => reconnect mqtt
        }
    }
    mqttClient->loop();   
  }  
}

#else
  void mqttSetUrl(char* mqttUrl) {}
  void mqttOpen(boolean on) {}
  void mqttInit() {}
  void mqttDisconnect() { }
  char* mqttSet(char* mqtt) { return "MQTT disabled"; }
  void mqttSetup() {}
  void mqttLoop() {}
  void mqttLog(char *message) {}
  boolean mqttPublish(char* topic,char *message) {} 
  void mqttAttr(char *topic,boolean on) {}
#endif
