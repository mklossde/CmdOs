
//-----------------------------------------------------------------------------
// FILESYSTEM SPIFFS / LittleFS

#if enableFs    
  #ifdef ESP32  
    #include <SPIFFS.h>  
    #define FILESYSTEM SPIFFS
  #elif defined(ESP8266)
//    #include <SPIFFS.h>
//    #define FILESYSTEM U_FS
    #include <LittleFS.h>  
    #define FILESYSTEM LittleFS
  #endif

  String rootDir="/";

  /* delete file in SPIFFS [ADMINI] */ 
  boolean fsDelete(String file) {     
    if(!is(file)) { return false; }
    else if(!file.startsWith(rootDir)) { file=rootDir+file; } 
    boolean ok=FILESYSTEM.remove(file);  
    sprintf(buffer,"fsDel '%s' ok:%d",file.c_str(),ok);logPrintln(LOG_INFO,buffer);  
    return true;
  }
  boolean fsRename(String oldFile,String newFile) { 
    if(!is(oldFile) || !is(newFile)) { return false; }
    else if(!oldFile.startsWith(rootDir)) { oldFile=rootDir+oldFile; } 
    else if(!newFile.startsWith(rootDir)) { newFile=rootDir+newFile; } 
    boolean ok=FILESYSTEM.rename(oldFile,newFile);  
    sprintf(buffer,"fsRename '%s' => '%s' OK:%d",oldFile.c_str(),newFile.c_str(),ok);logPrintln(LOG_INFO,buffer);  
    return ok;
  }
  
  /* create SPIFFS file and write p1 into file */
  boolean fsWrite(String file,char *p1) { 
    if(!is(file)) { return false; }
    else if(!file.startsWith(rootDir)) { file=rootDir+file; }
    File ff = FILESYSTEM.open(file, "w");
    if(!ff){ return false; }
    int len=strlen(p1);
    if(p1!=NULL && len>0) { ff.print(p1); }
    ff.close();
    sprintf(buffer,"fsWrite '%s %d",file.c_str(),len);logPrintln(LOG_INFO,buffer); 
    return true;
  }

  /* create SPIFFS file and write p1 into file */
  boolean fsWriteBin(String file,uint8_t *p1,int len) { 
    if(!is(file)) { return false; }
    else if(!file.startsWith(rootDir)) { file=rootDir+file; }    
    File ff = FILESYSTEM.open(file, "w");
    if(!ff){ return false; }
    ff.write(p1,len);
    ff.close();
    sprintf(buffer,"fsWriteBin '%s %d",file,len);logPrintln(LOG_INFO,buffer); 
    return true;
  }


  /* read file as char array 
        char *data;  
        data = fsRead(name); 
        delete[] data;
  */
  char* fsRead(String file) {  
    if(!is(file)) { return NULL; }
    else if(!file.startsWith(rootDir)) { file=rootDir+file; }
    File ff = FILESYSTEM.open(file, "r");  
    if(ff==NULL) { sprintf(buffer,"fsRead unkown '%s'",file.c_str());logPrintln(LOG_INFO,buffer);   return NULL; } 
    size_t fileSize= ff.size();

    char *charArray = newChar(fileSize + 1);
    ff.readBytes(charArray, fileSize);
    charArray[fileSize] = '\0';
    ff.close();

    sprintf(buffer,"fsRead '%s' %d",file.c_str(),fileSize);logPrintln(LOG_INFO,buffer);  
    return charArray;
  }


  /* read file as bin 
        size_t dataSize = 0; // gif data size
        uint8_t *data = fsReadBin(name, dataSize); 
        delete[] data;
*/
  uint8_t* fsReadBin(String file, size_t& fileSize) {
    if(!is(file)) { return NULL; }
    else if(!file.startsWith(rootDir)) { file=rootDir+file; }
    File ff = FILESYSTEM.open(file, "r");  
    if(ff==NULL) { sprintf(buffer,"fsReadBin unkown '%s'",file.c_str());logPrintln(LOG_INFO,buffer);   return NULL; } 
    fileSize= ff.size();

    uint8_t *byteArray = new uint8_t[fileSize];
    ff.read(byteArray, fileSize);

    ff.close();
    sprintf(buffer,"fsReadBin '%s' %d",file.c_str(),fileSize);logPrintln(LOG_INFO,buffer);  
    return byteArray;
  }


  int fsSize(String file) { 
    if(!is(file)) { return -1; }
    else if(!file.startsWith(rootDir)) { file=rootDir+file; }
    File ff = FILESYSTEM.open(file,"r");
    if(ff==NULL) { sprintf(buffer,"missing fsSize %s",file.c_str());logPrintln(LOG_INFO,buffer); return -1; } 
    int len=ff.size();
    ff.close();
    return len;
  }

  /* show a file */
  void fsCat(String file) { 
    if(!is(file)) { return ; }
    else if(!file.startsWith(rootDir)) { file=rootDir+file; }
    File ff = FILESYSTEM.open(file, "r");
    if(ff==NULL) { logPrintln(LOG_INFO,"missing");  } 
    char buffer[50];
    while (ff.available()) {
      int l = ff.readBytes(buffer, sizeof(buffer));
      buffer[l] = '\0';  
      logPrintln(LOG_INFO,buffer);
    }
    ff.close();
  }

  /* list files in SPIFFS of dir (null=/) */
  char* fsDir(String find) {
    if(!isAccess(ACCESS_READ))  { return "NO ACCESS fsDir"; }
    sprintf(buffer,"Files:\n");
    File root = FILESYSTEM.open(rootDir,"r");
    File foundfile = root.openNextFile();
    while (foundfile) {
      String file=foundfile.name();
      if(!is(find) || file.indexOf(find)!=-1) { 
        sprintf(buffer+strlen(buffer),"%s (%d)\n",file,foundfile.size());        
      }
      foundfile = root.openNextFile();
    }
    root.close();
    foundfile.close();
    return buffer; 
  }

  /* list number of files in SPIFFS of dir (null=/) */
  int fsDirSize(String find) {
    int count=0;
    File root = FILESYSTEM.open(rootDir,"r");
    File foundfile = root.openNextFile();
    while (foundfile) {
      String file=foundfile.name();
      if(!is(find) || file.indexOf(find)!=-1) { count++; }
      foundfile = root.openNextFile();
    }
    root.close();
    foundfile.close();
    return count; 
  }

  /* get file-name match filter, in dir at index (e.g. .gif,0 => first gif-file) 
      type<=0 => name of file
      type=1 => size of file
  */
  char* fsFile(String find,int count,int type) {
    File root = FILESYSTEM.open(rootDir,"r");
    File foundfile = root.openNextFile();
    while (foundfile) {
      String file=foundfile.name();
      if(!is(find) || file.indexOf(find)!=-1) { 
        if(count--<=0) { 
          if(type<=0) { sprintf(buffer,"%s",(char*)file.c_str()); return buffer;  }
          else if(type==1) { sprintf(buffer,"%d",foundfile.size()); return buffer;  }
          else { return "unkown type"; }
        }
      }
      foundfile = root.openNextFile();
    }
    root.close();
    foundfile.close();
    return EMPTY; 
  }

  /* format SPIFFS */
  void fsFormat() {
    sprintf(buffer,"FS formating..."); logPrintln(LOG_DEBUG,buffer); 
    if (SPIFFS.format()) { sprintf(buffer,"FS format DONE"); logPrintln(LOG_SYSTEM,buffer);  }
    else { sprintf(buffer,"FS format FAILD"); logPrintln(LOG_ERROR,buffer); }    
  }

  #if netEnable
    #include <WiFiClient.h>
  #ifdef ESP32
    #include <HTTPClient.h>
  #elif defined(ESP8266)
    #include <ESP8266HTTPClient.h>    
  #endif

    // e.g. https://www.w3.org/Icons/64x64/home.gif
    char* fsDownload(String url,String name,int reload) {
      if(!is(url,0,250)) { return "missing url"; }

      HTTPClient http;
      if(name==NULL) { name=url.substring(url.lastIndexOf('/')); }
      if(!name.startsWith("/")) { name="/"+name; }

      // check redownload 
      if(fsSize(name)>0) {
        if(reload==-1) { sprintf(buffer,"download foundOld '%s'",name); return buffer; }
      }

      #ifdef ESP32
        http.begin(url); 
      #elif defined(ESP8266)
        WiFiClient client;
        http.begin(client,url); 
      #endif      
      
      int httpCode = http.GET();
      int size = http.getSize();
      if(size>MAX_DONWLOAD_SIZE) { http.end(); return "download maxSize error"; }

      FILESYSTEM.remove(name);  // remove old file
      if (httpCode == 200) {
        sprintf(buffer,"fs downloading '%s' size %d to '%s'", url.c_str(), size,name.c_str());logPrintln(LOG_INFO,buffer);
        File ff = FILESYSTEM.open(name, "w"); 
        http.writeToStream(&ff);
        ff.close();

        sprintf(buffer,"fs download '%s' size %d to '%s'", url.c_str(), size,name.c_str());logPrintln(LOG_INFO,buffer);
        http.end();
        return "download ok";
      } else {
        sprintf(buffer,"fs download '%s' error %d",name.c_str(),httpCode);logPrintln(LOG_INFO,buffer);
        http.end();
        return "download error";
      }

    } 

    /* do rest call and return result */
    char* rest(String url) {
      if(!isAccess(ACCESS_READ))  { return "NO ACCESS"; }
      if(!is(url,0,250)) { return "missing url"; }

      HTTPClient http;
      #ifdef ESP32
        http.begin(url); 
      #elif defined(ESP8266)
        WiFiClient client;
        http.begin(client,url); 
      #endif    
      int httpCode = http.GET();

      if (httpCode == 200) {
        int size = http.getSize();
        if(size>bufferMax-1) { http.end(); return "response size error"; }

        String payload = http.getString();
        http.end();
        return (char*)payload.c_str();

      } else {
        sprintf(buffer,"rest '%s' error %d",url.c_str(),httpCode);
        http.end();
        return buffer;
      }  
    }

  #else 
    char* fsDownload(String url,String name,int reload) { return UNKOWN; }
    char* rest(String url) { return UNKOWN; }  
  #endif


//----------------------------------------------

String fsToSize(const size_t bytes) {
  if (bytes < 1024) return String(bytes) + " B";
  else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + " KB";
  else if (bytes < (1024 * 1024 * 1024)) return String(bytes / 1024.0 / 1024.0) + " MB";
  else return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
}

/* filesystem setup */
void fsSetup() {
  if(!enableFs) { return ; }
  #ifdef ESP32
    if (!FILESYSTEM.begin(true)) {    // if you have not used SPIFFS before on a ESP32, it will show this error. after a reboot SPIFFS will be configured and will happily work.
      espRestart("FILESYSTEM ERROR: Cannot mount");
    }
  #endif
  if(!FILESYSTEM.begin()){
    logPrintln(LOG_SYSTEM,"FILESYSTEM Mount Failed");
  } else {
    #ifdef ESP32
      sprintf(buffer,"SPIFFS Free:%s Used:%s Total:%s",
        fsToSize((FILESYSTEM.totalBytes() - FILESYSTEM.usedBytes())),fsToSize(FILESYSTEM.usedBytes()),fsToSize(FILESYSTEM.totalBytes()));logPrintln(LOG_INFO,buffer);
    #else
//TODO show nonne spiff fs ?     
      sprintf(buffer,"FS");logPrintln(LOG_INFO,buffer);
    #endif
  }
}

#else
  boolean fsDelete(String file) { return false; }
  boolean fsWrite(String file,char *p1) { return false; }
  boolean fsRename(String oldFile,String newFile) { return false; }
  char* fsRead(String file) { return NULL; }
  int8_t* fsReadBin(String file, size_t& fileSize) { return NULL; }
  int fsSize(String file) { return -1; }
  void fsCat(String file) {}
  char* fsDir(String find) { return UNKOWN;}  
  int fsDirSize(String find) { return 0; }
  char* fsDownload(String url,String name,int reaload) { return UNKOWN; }
  char* rest(String url) { return UNKOWN; }  
  char* fsToSize(const size_t bytes) { return UNKOWN; }  
  void fsSetup() {}
  void fsFormat() {}
  char* fsFile(String find,int count,int type) { return EMPTY; }
  
#endif

