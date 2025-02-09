

void cmdOSSetup() {
  if(serialEnable) { 
    delay(1); Serial.begin(115200); 
    delay(1); Serial.println("----------------------------------------------------------------------------------------------------------------------");
  }
  eeSetup();
  ledSetup();  
  swSetup(); 
  bootSetup();
  
  fsSetup();

  if(isModeNoSystemError()) {
    wifiSetup();  
    otaSetup();
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
  }

  if(isModeOk()) {
    mqttLoop();
    webLoop();
    timeLoop();
  }
  delay(0);
}