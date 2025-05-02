
//--------------------------------------------------------------------------------
// telnet

#if telnetEnable

  int telnetPort=23;
//  WiFiServer telnetServer(telnetPort);  // Create server object on port 23
//  WiFiClient telnetClient=NULL;
  NetworkServer telnetServer(telnetPort);  // Create server object on port 23
  NetworkClient telnetClient=NULL;

/*
//TODO #include <SHA.h>
  #include <base64.h>

//  boolean wsHandshakeDone = false;

  String computeAcceptKey(String key) {
    // Magic string to be appended as per WebSocket protocol
    key += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    // SHA1 hash
    uint8_t sha1Hash[20];
//TODO    sha1(key.c_str(), key.length(), sha1Hash);

    // Base64 encode
    return base64::encode(sha1Hash, 20);
  }

  void telnetWsHandshake(String key) {
//    if (!wsHandshakeDone) {
//      wsHandshakeDone = true;

      telnetClient.println("HTTP/1.1 101 Switching Protocols");
      telnetClient.println("Upgrade: websocket");
      telnetClient.println("Connection: Upgrade");
      telnetClient.println("Sec-WebSocket-Accept: " + computeAcceptKey(key));
      telnetClient.println();

Serial.println("WebSocket Handshake completed");
  }

  char* telnetReadLine() {
    inIndex=0; boolean end=false;
    while(telnetClient.connected() && !end) {
      if(telnetClient.available()) {
          char c = telnetClient.read();
          if(c == '\r') {}
          else if (c != '\n' &&  inIndex < maxInData-1) { inData[inIndex++] = c; } // read char 
          else { end=true; }
        }
    }
    if(inIndex==0) { return NULL; }    
    inData[inIndex++] = '\0';
    return inData;
  }

  void telnetHttpRequest(char *request) {
Serial.print("REQUEST:");Serial.println(request);        
    char *line=telnetReadLine();
    while(is(line)) {
      line=telnetReadLine();
Serial.print("HEAD:");Serial.println(line);      
    }
Serial.print("BODY:");
    while(telnetClient.connected() && telnetClient.available()) { char c = telnetClient.read(); Serial.print(c); } // simple read available body
Serial.println("REQUEST END");     

  }
*/

  void telnetClientLoop() {
    if(!telnetClient) { // Check for incoming clients       
      telnetClient = telnetServer.accept(); 
      if(telnetClient) {
        sprintf(buffer,"telnetClient connect",telnetPort);logPrintln(LOG_DEBUG,buffer);
        inIndex=0;
      }
    }  
    
    if (!telnetClient) { return ; } // no client
    else if(!telnetClient.connected()) { // client close connection
      logPrintln(LOG_DEBUG,"telnetClient disconnect");
      telnetClient.stop(); telnetClient=NULL;

    }else if(telnetClient.available()) {
      char c = telnetClient.read();      
      if (c != '\n' && c != '\r' && inIndex < maxInData-1) { inData[inIndex++] = c; } // read char 
      else if(inIndex>0) { // RETURN or maxlength 
        inData[inIndex++] = '\0';
        if(equals(inData, "exit")) { 
          logPrintln(LOG_DEBUG,"telnetClient exit");
          telnetClient.stop(); telnetClient=NULL; 
//TODO HTTP        }else if(strncmp(inData, "GET",  3)==0) { telnetHttpRequest(inData); 
       } else {
          char* ret=cmdLine(inData);
          if(is(ret)) { telnetClient.println(ret); }
          inIndex = 0;
        }
      } 
    }
  }

  // log to telnet client
  void telnetLog(char *text) { if(telnetClient) { telnetClient.println(text); } }

  //------------------------------------

  void telnetSetup() {
    sprintf(buffer,"telnetServer start port:%d",telnetPort);logPrintln(LOG_INFO,buffer);    
    telnetServer.begin();
  }

  void telnetLoop() {
    telnetClientLoop();
  }

#else 
  void telnetSetup() {}
  void telnetLoop() {}
  void telnetLog(char *text) {}
#endif

