
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

char* mqttCmdTopic; // topic for cmd messages (e.g. device/esp/EspBoot00DC9235/cmd)
char* mqttResponseTopic; // topic for resposne of cmd messages (e.g. device/esp/EspBoot00DC9235/result) 

WiFiClient *mqttWifiClient=NULL;
NetworkClientSecure *mqttClientSSL=NULL; // WiFiClientSecure / NetworkClientSecure
PubSubClient *mqttClient=NULL;

static char* mqttTopic=new char[64]; // buffer of topic
static char* mqttMessage=new char[1024]; // buffer of message

//-------------------------------------------------------------------------------------
// mqtt-url:  mqtt://USER:PAS@SERVER:PORT or mqtts://USER:PAS@SERVER:PORT /

/* get mqtt infos */
char* mqttInfo() {
  if(!is(eeBoot.mqtt) || !is(mqttUser) || !is(mqttServer) ) { return "mqtt not defined"; }
  char *type="mqtt"; if(mqttSSL) { type="mqtts"; }  
  if(is(mqttUser)) {
    sprintf(buffer,"MQTT status:%d  %s://%s:%d@%s:%d (ee:%s)",
      mqttStatus,type,to(mqttUser),is(mqttPas),to(mqttServer),mqttPort,to(eeBoot.mqtt)); return buffer;
  }else { sprintf(buffer,"MQTT status:%d  %s://%s:%d (ee:%s)",mqttStatus,type,to(mqttServer),mqttPort,to(eeBoot.mqtt)); return buffer;}
}

/* split mqtt-url */
void mqttSetUrl(char* mqttUrl) {
  mqttUser=NULL; mqttPas=NULL; mqttServer=NULL; mqttPort=1833;  
  if(!is(mqttUrl,1,127)) { logPrintln(LOG_ERROR,"MQTT missing/wrong"); return ; }

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
   delete[] mqtt;
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

void mqttLog(char *message) {
  if(eeBoot.mqttLogEnable) {
      if (mqttStatus != 2) { return ; }
      sprintf(mqttTopic,"%s/%s%/log",mqttPrefix,mqttClientName);
      mqttClient->publish(mqttTopic, message);
  }
}

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

void publishResponse(char *id,char *result) {
  if (mqttStatus != 2) { return ; }
  if(result!=NULL && sizeof(result)>0) {
    if(id!=NULL) { sprintf(mqttMessage,"%s:%s", id,result); }
    else { sprintf(mqttMessage,result); }
    boolean ok=mqttClient->publish(mqttResponseTopic, mqttMessage);
    sprintf(buffer,"MQTT publish %s => %s ok:%d", mqttResponseTopic,mqttMessage,ok);  logPrintln(LOG_DEBUG,buffer);
  }
}

void publishTopic(char* topic,char *message) {
  if (mqttStatus != 2) { return ; }
  boolean ok=mqttClient->publish(topic, message);
  sprintf(buffer,"MQTT publish %s => %s ok:%d", mqttTopic,message,ok);  logPrintln(LOG_DEBUG,buffer);
}

/** subcribe topic to attr **/
void mqttAttr(char *topic,boolean on) {
  if(mqttStatus != 2) { return ; }
  char* t=copy(topic);
  sprintf(buffer,"MQTT attr via topic %s",topic);
  if(on) { 
    attrMap.replace(t,(char*)"",0); boolean ok=mqttClient->subscribe(t); 
    sprintf(buffer,"MQTT subsrcibe '%s' attr:%s", topic,topic,ok); logPrintln(LOG_DEBUG,buffer);
  } else { 
    boolean ok=mqttClient->unsubscribe(t); attrMap.del(t); 
    sprintf(buffer,"MQTT unsubsrcibe '%s' attr:%s ok:%d", topic,topic,ok); logPrintln(LOG_DEBUG,buffer);
  } 
}

//-------------------------------------------------------------------------------------
// Receive messages

void mqttReceive(char* topic, byte* payload, unsigned int length) {  
  if (strcmp(topic,mqttCmdTopic) == 0) {    
    char *msg=copy(NULL,(char*)payload,length);
    sprintf(buffer,"MQTT cmd '%s' %s %d", topic, msg,length); logPrintln(LOG_DEBUG,buffer);
    char *result=cmdLine(msg); 
    free(msg);
    if(result!=NULL) {
      char *id=NULL;    
      publishResponse(id,result);
    }

  } else if(attrMap.find(topic)!=-1) { 
    attrMap.replace(topic,(char*)payload,length);
    sprintf(buffer,"MQTT attrSet '%s'", topic); logPrintln(LOG_DEBUG,buffer);

  } else { sprintf(buffer,"MQTT unkown topic '%s'", topic); logPrintln(LOG_DEBUG,buffer);}

}

//-------------------------------------------------------

boolean mqttRunning=false;

void mqttInit() {
  mqttClientName=eeBoot.espName;

  if(!is(mqttServer)) { logPrintln(LOG_SYSTEM,"MQTT error - mqttServer missing");  mqttConFail=3; return ; }  
  else if(mqttPort<1) { logPrintln(LOG_SYSTEM,"MQTT error - mqttPort missing");  mqttConFail=3; return ; }  
  else if(!is(eeBoot.espName)) { logPrintln(LOG_SYSTEM,"MQTT error - clientName missing");  mqttConFail=3; return ; }  

  if(!mqttSSL) { 
    mqttWifiClient=new WiFiClient();  
    mqttClient=new PubSubClient(*mqttWifiClient);
  }else {
    mqttClientSSL=new NetworkClientSecure();  
    mqttClient=new PubSubClient(*mqttClientSSL);
  }  

  mqttClient->setCallback(mqttReceive);     
  mqttClient->setServer(mqttServer, mqttPort);

  mqttCmdTopic = copy(to(mqttPrefix, "/", mqttClientName, "/cmd"));
  mqttResponseTopic = copy(to(mqttPrefix, "/", mqttClientName, "/result"));

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
    if (mqttClient->connect(mqttClientName,mqttUser,mqttPas)) { // cennect 
      sprintf(buffer,"MQTT connected %s => %s", mqttClientName, mqttServer); logPrintln(LOG_INFO,buffer); 


      boolean ok=mqttClient->subscribe(mqttCmdTopic); // subscribe cmd topic   
      sprintf(buffer,"MQTT subscribe %s ok:%d", mqttCmdTopic,ok); logPrintln(LOG_INFO,buffer);
      mqttStatus = 2;     
      publishValue("status","connect");        
      mqttConFail=0;   // connected => reset fail

    } else {
      sprintf(buffer,"MQTT connection faild %d rc:%d - %s => %s", mqttConFail, mqttClient->state(),mqttClientName, mqttServer);  logPrintln(LOG_SYSTEM,buffer);      
      mqttConFail++; 
      mqttStatus = 0;
    }
}

void mqttDisconnect() { 
  if(mqttClient!=NULL && mqttClient->connected()) {
    publishValue("status","disconnect");
    mqttClient->disconnect();     
  }
  mqttClient=NULL;
  mqttStatus = 0;
  mqttRunning=false;
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
  void mqttInit() {}
  void mqttDisconnect() { }
  char* mqttSet(char* mqtt) { return NULL; }
  void mqttSetup() {}
  void mqttLoop() {}
  void mqttLog(char *message) {}
  void publishTopic(char* topic,char *message) {} 
#endif
