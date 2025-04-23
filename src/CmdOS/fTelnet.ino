
/*
#if netEnable
  #include <SHA.h>
  #include <base64.h>

  WiFiServer telnetServer(23);  // Create server object on port 23
  WiFiClient telnetClient=NULL;

  String computeAcceptKey(String request) {
    String key = "";
    int startIndex = request.indexOf("Sec-WebSocket-Key: ");
    if (startIndex != -1) {
      startIndex += 19;
      int endIndex = request.indexOf("\r\n", startIndex);
      if (endIndex != -1) {
        key = request.substring(startIndex, endIndex);
        key.trim();
      }
    }

    // Magic string to be appended as per WebSocket protocol
    key += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    // SHA1 hash
    uint8_t sha1Hash[20];
    sha1(key.c_str(), key.length(), sha1Hash);

    // Base64 encode
    return base64::encode(sha1Hash, 20);
  }


  void telnetClientLoop() {
    if(telnetClient==null) {telnetClient = server.available(); }  // Check for incoming clients
    if (!telnetClient) { return ; }

    Serial.println("New client connected");
    boolean wsHandshakeDone = false;

    while (telnetClient.connected()) {
      if (telnetClient.available()) {
        String request = telnetClient.readStringUntil('\r');
        telnetClient.readStringUntil('\n'); // Consume newline

        Serial.println(request);

        if (!wsHandshakeDone && request.indexOf("GET / HTTP/1.1") >= 0) {
          wsHandshakeDone = true;

          telnetClient.println("HTTP/1.1 101 Switching Protocols");
          telnetClient.println("Upgrade: websocket");
          telnetClient.println("Connection: Upgrade");
          telnetClient.println("Sec-WebSocket-Accept: " + computeAcceptKey(request));
          telnetClient.println();
          Serial.println("WebSocket Handshake completed");
        } 
        else if (wsHandshakeDone) {
          // Simple echo back the message received after handshake
          Serial.println("Handling WebSocket data");
          
          if (telnetClient.available()) {
            uint8_t data[telnetClient.available()];
            telnetClient.readBytes(data, sizeof(data));
            Serial.println("Received data: ");
            Serial.write(data, sizeof(data));
            Serial.println();

            // Just a simple echo here
            telnetClient.write(0x81);  // Text frame
            telnetClient.write(sizeof(data));  // Length of data
            telnetClient.write(data, sizeof(data));
          }
        }
      }

      // Close connection
      telnetClient.stop(); telnetClient=NULL;
      Serial.println("Client disconnected");
    }
  }



  //------------------------------------

  void telnetSetup() {
    server.begin();

  }

  void telnetLoop() {
    
  }

#else 
  void telnetSetup() {}
  void telnetLoop() {}
#endif
*/

