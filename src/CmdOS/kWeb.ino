
//-----------------------------------------------------------------------------------
// web

#if webEnable

#include <Arduino.h>
#ifdef ESP32
  #include <AsyncTCP.h>
  #include <ESPmDNS.h>
#elif defined(ESP8266)
  #include <ESPAsyncTCP.h>
  #include <ESP8266mDNS.h>
#endif

AsyncAuthenticationMiddleware basicAuth;

boolean _webInit = false;

String webParam(AsyncWebServerRequest *request,String key) { 
  if (request->hasParam(key)) { return request->getParam(key)->value(); }
  else { return EMPTYSTRING; }
}

//-------------------------------------------------------------------------------------------------------------------

String pageHead(String html, String title) {
  html += "<html><head><title>" + title + "</title></head><body><b>" + prgTitle + " - " + title + "</b> | ";
  html += "[<a href='/app'>app</a>][<a href='/config'>config</a>][<a href='/file'>file</a>][<a href='/webserial'>console</a>][<a href='/cmd'>cmd</a>][<a href='/firmware'>firmware</a>]<hr>";
  return html;
}
String pageEnd(String html,String message) {
  if(is(message)) { html+="<script>alert('"+message+"');</script>"; }
  html += "</body></html>";
  return html;
}
String pageForm(String html, String title) {
  html += "<table><form method='GET'><tr><th colspan='2'>" + title+"</th></tr>";
  return html;
}

String pageFormEnd(String html) { 
  html+="</form></table>"; return html;
}

String pageInput(String html, const char *key, char *value) {
  String k = String(key);
  html += "<tr><th align='right'>"+k+"</th><td><input label='" + k + "' type='text' name='" + k + "' value='" + String(value) + "' size='64'/></td></tr>";
  return html;
}
String pageButton(String html, const char *key, const char *value) {
  html += "<tr><th></th><td><input type='submit' name='" + String(key) + "' value='" + String(value) + "'/></td>";
  return html;
}

String pageCmd(String html, const char *cmd, const char *name) {
  html+="[<a href='/cmd/"+String(cmd)+"'>"+String(name)+"</A>]";
  return html;
}

String pageUpload(String html, String title,String action) {
  html += "<form id='dz' method='POST' action='" + action + "' enctype='multipart/form-data' style='width:300px;height:150px;border: 1px dotted grey;text-align: center;'>";
  html += "<br><br><b>"+title+"</b><br>(drop or select file)<br><br><input id='fz' type='file' name='update'><input type='submit' value='ok'></form>";
  html += "<script>var ddz=document.getElementById('dz');";
  html += "function dragIt(e,color) { e.preventDefault();e.stopPropagation(); ddz.style.backgroundColor=color; }";
  html += "function dropit(e) { e.preventDefault();e.stopPropagation(); document.getElementById('fz').files=e.dataTransfer.files; ddz.submit(); }";
  html += "ddz.addEventListener('dragover', function (e) {dragIt(e,\"green\");}, false);";
  html += "ddz.addEventListener('drop', dropit, false);";
  html += "ddz.addEventListener('dragleave', function (e) {dragIt(e,\"white\");}, false);";
  html += "</script>";
  return html;
}

//-------------------------------------------------------------------------------------------------------------------

boolean isWebAccess(int level) {
  setAccess(true); // enable is admin in web access
  return true;
}

void webRoot(AsyncWebServerRequest *request) {
  isWebAccess(ACCESS_READ); 
  String html = "";
  html = pageHead(html, "Index");
  html = pageEnd(html,EMPTYSTRING);
  request->send(200, "text/html", html);
}

void webWifiReset(AsyncWebServerRequest *request) {
  String value=webParam(request,"reset");
  char* res=bootReset((char*)value.c_str());
  String html = "";
  html = pageHead(html, "Reset");
  html += "<b>"+String(res)+"</b>";
  html = pageForm(html, "Reset");
  html = pageInput(html, "reset", "");
  html = pageButton(html, "ok", "ok");
  html = pageFormEnd(html);

  html = pageEnd(html,EMPTYSTRING);
  request->send(200, "text/html", html);
}

//-------------------------------------------------------------------------------------------------------------------
// File Manager

void webFileManagerRename(AsyncWebServerRequest *request, String name) {
  if(!isWebAccess(ACCESS_CHANGE)) { request->send(403, "text/html"); }
  String html = "";
  html = pageHead(html, "File Manager - Rename");
  html += "Rename <form method='get'><input type='hidden' name='name' value='" + name + "'/><input type='text' name='newname' value='" + name + "'/><input type='submit' name='doRename' value='ok'></form>";
  html = pageEnd(html,EMPTYSTRING);
  request->send(200, "text/html", html);
}

void webFileManagerEd(AsyncWebServerRequest *request, String name) {
  if(!isWebAccess(ACCESS_CHANGE)) { request->send(403, "text/html"); }
  String html = "";
  html = pageHead(html, "File Manager - Ed");
  html += "<form method='GET' action='?doSave=1'><input type='text' name='name' value='" + name + "'/><br>";
  html += "<textarea label='" + name + "' name='value' cols='80' rows='40'>";
  File ff = SPIFFS.open(name, "r");
  if (ff) { html += ff.readString(); }
  ff.close();
  html += "</textarea><br><input type='submit' name='doSave' value='save'>";
  html += "<input type='submit' name='doSaveAndRun' value='save and run'></form>";
  html = pageEnd(html,EMPTYSTRING);
  request->send(200, "text/html", html);
}

/* save file [ADMIN]  */
void webFileManagerSave(AsyncWebServerRequest *request, String name, String value) {  
  if(!isWebAccess(ACCESS_CHANGE)) { request->send(403, "text/html"); }
  
  File ff = SPIFFS.open(name, "w");
  if (value != NULL) { ff.print(value); }
  ff.close();
  sprintf(buffer, "save %s", name.c_str()); logPrintln(LOG_INFO,buffer);
}


/* upload file */
void webFileManagerUpload(AsyncWebServerRequest *request, String file, size_t index, uint8_t *data, size_t len, bool final) {
  if(!isWebAccess(ACCESS_CHANGE)) { request->send(403, "text/html"); }
  sprintf(buffer, "upload %s %d index:%d", file.c_str(), len, index);logPrintln(LOG_INFO,buffer);

  File ff;
  if (!index) {
    FILESYSTEM.remove(rootDir + file); // remove old file 
    ff = SPIFFS.open(rootDir + file, "w");
  } else {
    ff = SPIFFS.open(rootDir + file, "a");
  }

  for (size_t i = 0; i < len; i++) { ff.write(data[i]); }
  ff.close();
  if (final) {
    sprintf(buffer, "uploaded %s %d", file.c_str(), (index + len)); logPrintln(LOG_INFO,buffer);
    request->redirect("/file");
  }
}


void webFileManager(AsyncWebServerRequest *request) {
  String message;
  if (request->hasParam("del")) { fsDelete(webParam(request,"name")); }
  else if (request->hasParam("rename")) { webFileManagerRename(request, webParam(request,"name")); return; }
  else if (request->hasParam("doRename")) { fsRename(webParam(request,"name"), webParam(request,"newname")); }
  else if (request->hasParam("ed")) { webFileManagerEd(request, webParam(request,"name")); return; }
  else if (request->hasParam("doSave") || request->hasParam("doSaveAndRun")) { 
    webFileManagerSave(request, webParam(request,"name"), webParam(request,"value")); 
    if(request->hasParam("doSaveAndRun")) { cmdFile((char*)webParam(request,"name").c_str()); }
    webFileManagerEd(request, webParam(request,"name")); return;
  }
  else if (request->hasParam("doUploadUrl")) { message=fsDownload(webParam(request,"url"), webParam(request,"name"),-1);  }

  String html = "";
  html = pageHead(html, "File Manager");
  html += "[<a href='?ed=1&name=/new'>new</a>]<p>";
  File root = FILESYSTEM.open(rootDir,"r");
  File foundfile = root.openNextFile();
  while (foundfile) {
    String name = String(foundfile.name());
    html += "<li><a href='/res?name=" + rootDir + name + "'>" + name + "</a> (" + foundfile.size() + ")";
    html += " [<a href='?ed=1&name=" + rootDir + name + "'>edit</a>]";
    html += " [<a href='?del=1&name=" + rootDir + name + "'>delete</a>]";
    html += " [<a href='?rename=1&name=" + rootDir + name + "'>rename</a>]";
    html += " [<a href='/res?name=" + rootDir + name + "' download='" + name + "'>download</a>]";
    foundfile = root.openNextFile();
  }
  root.close();
  foundfile.close();
  html += "</ul><hr>";
  html = pageUpload(html, "Upload File","/doUpload");
  html += "<form method='GET'>Upload URL<input type='text' size='64' name='url'/><input type='submit' name='doUploadUrl' value='ok'/></form>";
  html = pageEnd(html,message);
  request->send(200, "text/html", html);
}

//-------------------------------------------------------------------------------------------------------------------
// OTA Web Update

#if updateEnable

  #ifdef ESP32
    #include <Update.h>
  #elif defined(ESP8266)
    #include <Updater.h>
  #endif

  size_t content_len;

  void webUpdate(AsyncWebServerRequest *request) {
    String html = "";
    html = pageHead(html, "Firmware Update");
    html += "<form method='POST' action='/doUpdate' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
    html = pageEnd(html,EMPTYSTRING);
    request->send(200, "text/html", html);
  }

  void webDoUpdate(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!updateEnable) {
      request->send(200, "text/html", "ota disabled");
      return;
    }
    //  if(!request->authenticate(http_username, http_password)) { return request->requestAuthentication(); }
    if (!index) {
      logPrintln(LOG_SYSTEM,"Update");
      content_len = request->contentLength();
      #ifdef ESP8266
        Update.runAsync(true);
        if (filename.indexOf("filesystem") > -1) {            
          if (!Update.begin(content_len, U_FS)) { Update.printError(Serial); }
        }else {
          uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
          if (!Update.begin(maxSketchSpace, U_FLASH)) { Update.printError(Serial); }
        }
      #else
        int cmd = (filename.indexOf("spiffs") > -1) ? U_SPIFFS  : U_FLASH;  // if filename includes spiffs, update the spiffs partition
        if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd)) { Update.printError(Serial); }
      #endif
    }

    if (Update.write(data, len) != len) {
      Update.printError(Serial);
  #ifdef ESP8266
    } else {
      sprintf(buffer, "Progress: %d%%", (Update.progress() * 100) / Update.size()); logPrintln(LOG_SYSTEM,buffer);
  #endif
    }

    if (final) {
      AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Please wait while the device reboots");
      response->addHeader("Refresh", "20");
      response->addHeader("Location", "/");
      request->send(response);
      if (!Update.end(true)) {
        Update.printError(Serial);
      } else {
        espRestart("Update complete");
      }
    }
  }

  void webProgress(size_t prg, size_t sz) {
    sprintf(buffer, "Progress: %d%%", (prg * 100) / content_len);
    logPrintln(LOG_SYSTEM,buffer);
  }

#else
  void webUpdate(AsyncWebServerRequest *request) { request->send(200, "text/html", "webupdate not implemented"); }
  void webDoUpdate(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final) {}
  void webProgress(size_t prg, size_t sz) {}
#endif

//-------------------------------------------------------------------------------------------------------------------
// web serial console

#if webSerialEnable

  #include <WebSerialLite.h>

  void recvMsg(uint8_t *data, size_t len){
    String line = "";
    for (int i = 0; i < len; i++) { line += char(data[i]); }
    char ca[len + 1];
    for (int i = 0; i < len; i++) { ca[i] = data[i]; }
    ca[len] = '\0';
    char* ret = cmdLine(ca);  // exec cmd
    if(is(ret)) { webLogLn(toString(ret)); }
  }

  void webLogLn(String msg) {
    if (webEnable && _webInit) { WebSerial.println(msg); }
  }

  void webSerialSetup() {
    WebSerial.begin(&webServer);
    WebSerial.onMessage(recvMsg);
//TODO    if (is(eeBoot.espPas)) { WebSerial.setAuthentication(user_admin, eeBoot.espPas); }  // webSerial auth
    sprintf(buffer, "WebSerial started /webserial"); logPrintln(LOG_DEBUG,buffer);
  }

#else
  void webSerialSetup() {}
  void webLogLn(String msg) {}
#endif

//-------------------------------------------------------------------------------------------------------------------
// auth

void webWifiSet(AsyncWebServerRequest *request) {

  String v=webParam(request,"espName"); 
  if(is(v,1,20)) {  v.toCharArray(eeBoot.espName, sizeof(eeBoot.espName)); }
  v=webParam(request,"espPas"); if(is(v,1,20)) {  v.toCharArray(eeBoot.espPas, sizeof(eeBoot.espPas)); }
  v=webParam(request,"espBoard"); if(is(v,1,20)) {  v.toCharArray(eeBoot.espBoard, sizeof(eeBoot.espBoard)); }
  v=webParam(request,"wifi_ssid"); if(is(v,1,32)) {  v.toCharArray(eeBoot.wifi_ssid, sizeof(eeBoot.wifi_ssid)); }
  v=webParam(request,"wifi_pas"); if(is(v,1,32)) {  v.toCharArray(eeBoot.wifi_pas, sizeof(eeBoot.wifi_pas)); }
  v=webParam(request,"wifi_ntp"); if(is(v,1,32)) {  v.toCharArray(eeBoot.wifi_ntp, sizeof(eeBoot.wifi_ntp)); }
  v=webParam(request,"mqtt"); if(is(v,1,64)) {  v.toCharArray(eeBoot.mqtt, sizeof(eeBoot.mqtt)); }

  logPrintln(LOG_INFO,bootInfo());
}

void webWifi(AsyncWebServerRequest *request) {
  String message;
  if (request->hasParam("ok")) { webWifiSet(request); message="set"; }
  else if (request->hasParam("save")) { bootSave(); message="saved";  }
  else if (request->hasParam("reset")) { webWifiReset(request); return ;  }
  else if (request->hasParam("restart")) { espRestart("web restart"); }

  String html = "";
  html = pageHead(html, "wifi");
  html+= "[<a href='/config'>network</a>][<a href='/appSetup'>app</a>]";
  html = pageForm(html, "Wifi config");
  html = pageInput(html, "espName", eeBoot.espName);
  html = pageInput(html, "espPas", ""); // eeBoot.espPas
  html = pageInput(html, "espBoard", eeBoot.espBoard);
  html = pageInput(html, "wifi_ssid", eeBoot.wifi_ssid);
  html = pageInput(html, "wifi_pas", ""); // eeBoot.wifi_pas
  html = pageInput(html, "wifi_ntp", eeBoot.wifi_ntp);
  html = pageInput(html, "mqtt", eeBoot.mqtt);
  html = pageButton(html, "ok", "ok");
  html = pageFormEnd(html);
  html+= "<hr>";
  html = pageCmd(html, "restart", "restart");
  html = pageCmd(html, "save", "save");
//  html = pageCmd(html, "reset", "reset");
  html += "[<a href='?reset=0'>reset</a>]";
  
  //  html=pageFormEnd(html,"ok");
  html = pageEnd(html,message);
  request->send(200, "text/html", html);
}

//-------------------------------------------------------------------------------------------------------------------

/* call cmd from web (e.g. "/cmd/wifi KEY PAS" or "/cmd/wifi?wlan=KEY&password=PAS" or "/cmd?cmd=wifi&wlan=KEY&password=PAS")
  ! parameter will addad to cmd on given order => ?one=1&b=3&c=2 => 1 3 2 !
*/
void webCmd(AsyncWebServerRequest *request) {
  String cmd;
  // get cmd from path
  String path = request->url();
  int index=path.indexOf('/',1);
  if(index!=-1) { cmd=path.substring(index+1); }
  // get cmd from parma
  if(cmd==NULL && request->hasParam("cmd")) { cmd=request->getParam("cmd")->value(); } // get attr (&cmd=CMD)
  if(cmd==NULL) {  
    String html=""; html=pageHead(html,"cmd");
    html+="<form>CMD:<input type='text' size='100' name='cmd' value=''/><input type='submit' name='ok' value='ok'></form>";
    html=pageEnd(html,EMPTYSTRING);
    request->send(200, "text/html", html); 
    return ; 
  }

  // add all params to cmd 
  int params = request->params();
  for(int i=0;i<params;i++){
    const AsyncWebParameter* p = request->getParam(i);
    String key=p->name();
    if(!key.equals("cmd") && !key.equals("ok")){ cmd+=" "+p->value(); } 
  }
  
  char *inData=(char*)cmd.c_str(); 
  String ret=cmdLine(inData);

//  String html=""+ret;
  request->send(200, "text/plain", ret);
}

//-------------------------------------------------------------------------------------------------------------------



/* setup remote device by return "setup wifi_ssid wifi_pas NAME espPas" */
void webSetupDevice(AsyncWebServerRequest *request) {
  if(setupDevice>0) {
    String name=webParam(request,"name");
    char *setupName=NULL; if(name!=NULL) { setupName=(char*)name.c_str(); } 
    sprintf(buffer,"setup %s %s \"%s\" %s %s",to(eeBoot.wifi_ssid),to(eeBoot.wifi_pas),to(setupName),to(eeBoot.espPas),
      to(eeBoot.mqtt));
    request->send(200, "text/plain", buffer);     
    if(setupDevice<255) { setupDevice--;}
    sprintf(buffer,"webSetupDevice %s setupDevice:%d",to(setupName),setupDevice); logPrintln(LOG_INFO,buffer); 
    if(setupDevice==0) { 
//      wifiInit(); 
      WiFi.softAPdisconnect (true);
    } // switch back to normal wifi
  }
}  



//-------------------------------------------------------------------------------------------------------------------
// auth

/* show/download file */
void webRes(AsyncWebServerRequest *request) {
  String name=webParam(request,"name");
  if(!is(name)) { request->send(403, "text/html"); }
//  else if(onlyImage && (!name.endsWith(".gif") || !name.endsWith(".jpg"))) {  request->send(403, "text/html","not image"); }
  else { request->send(SPIFFS, name);  }
}

//-------------------------------------------------------------------------------------------------------------------


void webSetup() {
  if (!webEnable || _webInit) {
    _webInit = false;
    return;
  }

  // enable auth
  basicAuth.setUsername(user_admin);
  basicAuth.setPassword(eeBoot.espPas);
  basicAuth.setRealm("MyApp");
  basicAuth.setAuthFailureMessage("Authentication failed");
  basicAuth.setAuthType(AsyncAuthType::AUTH_BASIC);
  basicAuth.generateHash();

  webServer.on("/config", HTTP_GET, [](AsyncWebServerRequest *request) { webWifi(request);})
    .addMiddleware(&basicAuth);

  #if webSerialEnable
    // web serial console
    webSerialSetup();
  #endif

  // OTA
  if (updateEnable) {
    webServer.on("/firmware", HTTP_GET, [](AsyncWebServerRequest *request) {
            webUpdate(request);
          })
        .addMiddleware(&basicAuth);
//TODO    if(eeBoot.accessTyper!=ACCESS_ALL) {  .addMiddleware(&basicAuth); }
      
    webServer.on(
            "/doUpdate", HTTP_POST,
            [](AsyncWebServerRequest *request) {},
            [](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data,
               size_t len, bool final) {
              webDoUpdate(request, filename, index, data, len, final);
            })
      .addMiddleware(&basicAuth);
    ;
    sprintf(buffer, "WebUpdate started /update");
    logPrintln(LOG_DEBUG,buffer);

#if otaEnable && defined(ESP32) 
    Update.onProgress(webProgress);
#endif

  }

  //File Manager
  webServer.on("/file", HTTP_GET, [](AsyncWebServerRequest *request) {
          webFileManager(request);
        })
    .addMiddleware(&basicAuth);
  
  webServer.on(
          "/doUpload", HTTP_POST,
          [](AsyncWebServerRequest *request) {},[](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data,size_t len, bool final) {
            webFileManagerUpload(request, filename, index, data, len, final);
          })
    .addMiddleware(&basicAuth);
  

  // cmd
  webServer.on("/cmd", HTTP_GET, [](AsyncWebServerRequest *request) { webCmd(request); })
    .addMiddleware(&basicAuth);

  // resources
  webServer.on("/res", HTTP_GET, [](AsyncWebServerRequest *request) { webRes(request); })
    .addMiddleware(&basicAuth);

  // web setupdevice
  webServer.on("/setupDevice", HTTP_GET, [](AsyncWebServerRequest *request) { webSetupDevice(request); });

  //root
  webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request) { webRoot(request); });
  webServer.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404);
  });

  // app
  webApp();


  webServer.begin();


  _webInit = true;
  sprintf(buffer, "WEB started %s:%d", WiFi.localIP().toString().c_str(), _webPort); logPrintln(LOG_INFO,buffer);
}

void webLoop() {
  #if webSerialEnable
    if (webEnable && _webInit) {
/*
      webSerial.loop();
*/      
    }
  #endif
}


#else 
  void webLoop() {}  
  void webSetup() {}
#endif

