
//--------------------------------------------------------------------------------------
// Setup/Loop

void cmdOSSetup() {
  if(serialEnable) { 
    delay(1); Serial.begin(115200); 
    delay(1); Serial.println("----------------------------------------------------------------------------------------------------------------------");
  }
  freeHeapMax=ESP.getFreeHeap(); // remeber max freeHeap
  logPrintln(LOG_INFO,espInfo()); // ESP infos
  eeSetup();
  ledSetup();  
  swSetup(); 
  bootSetup();
  
  fsSetup();

  if(isModeNoSystemError()) {
    wifiSetup();  
    otaSetup();
    telnetSetup();
  }

  if(isModeOk()) {
    mqttSetup();  
    timeSteup();  
  }
}

void cmdOSLoop() {
  eeLoop();
  ledLoop(); 
  swLoop(); 
  cmdLoop();

  timeLoop();

  if(isModeNoSystemError()) {
    wifiLoop();
    otaLoop();
    telnetLoop();
  }

  if(isModeOk()) {
    mqttLoop();
    webLoop();
    timeLoop();
  }
  delay(0);
}
