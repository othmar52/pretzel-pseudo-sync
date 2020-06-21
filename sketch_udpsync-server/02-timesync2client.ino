

uint32_t clientLastRequestTimes[MAXCLIENTS];
const uint8_t kickClientAfter = 20;   // [seconds] of no poll. make sure clientRequestInterval is shorter (depends on number of connected clients)

const uint16_t incomingRequestInterval = 3000; // [ms] between poll of clients
uint16_t clientRequestInterval = 1000; // [ms] between poll of a single client (depends on number of connected clients)


uint8_t ledSyncInterval = 1;           // [seconds] for showing onboard LED in sync to host clock. set to 0 to deactivate
uint16_t ledDuration = 100;             // [ms] to wait before turning the led off again
const int ledPin =  LED_BUILTIN;       // the number of the LED pin
int ledState = LOW;                    // ledState used to set the LED


/**
 * returns the amount of clients that polled within the last <kickClientAfter> seconds
 */
uint8_t connectedClients() {
  uint8_t amount = 0;
  unsigned long now = millis();
  for(int i = 0; i < MAXCLIENTS; i++) {
    if(clientLastRequestTimes[i] > now - kickClientAfter*1000) {
      amount++;
      continue;
    }
    clientLastRequestTimes[i] = 0;
  }
  clientRequestInterval = (amount == 0) ? incomingRequestInterval : incomingRequestInterval*amount;
  return amount;
}


void handleRecievedMessageSync(String msg, int clientId) {
  clientLastRequestTimes[clientId] = millis();
  debug("amount of connected clients: " + String(connectedClients()));
  uint16_t clientsOffset = (connectedClients() == 0) ? 0 : (getClientPosition(clientId) * clientRequestInterval)/connectedClients();
  // reqIdFromClient ; host-time ; reqIntervalForClient ; offset for clientsReqInterval ; 
  WifiSend(Msg + ";" + String(millis()) + ";" + String(clientRequestInterval)  + ";" + String(clientsOffset) + ":0", String(clientId));
}



void onboardLedIndicator() {
  if(ledSyncInterval == 0) {
    // deactivated
    return;  
  }
  ledDuration = (millis()%(ledSyncInterval*4000) < ledSyncInterval*1000) ? 500 : 100;

  // debug("led check delayed?");
  uint16_t theRest = millis()%(ledSyncInterval*1000);
  if(ledState == HIGH && theRest >= ledDuration) {
    //debug("led off " + String(theRest) + "\t" + String(ledDuration));
    ledState = LOW;
    digitalWrite(ledPin, ledState);
    return;
  }
  if(ledState == LOW && theRest < ledDuration) {
    //debug("led on " + String(theRest) + "\t" + String(ledDuration));
    ledState = HIGH;
    digitalWrite(ledPin, ledState);
  }
}



/**
 * returns the amount of clients that polled within the last <kickClientAfter> seconds
 */
uint8_t getClientPosition(int clientId) {
  uint8_t theClientPosition = 0;
  for(int i = 0; i < MAXCLIENTS; i++) {
    if(clientLastRequestTimes[i] == 0) {
      continue;
    }
    if(i == clientId) {
      return theClientPosition;
    }
    theClientPosition++;
  }
  return theClientPosition;
}
