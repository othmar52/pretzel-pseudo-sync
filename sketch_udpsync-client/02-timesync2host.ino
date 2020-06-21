

// we permanently measure the time [ms] for each request/response
// so we need to a few variables to calculate best approximation to hosts clock
#define MAXREQLOOP 12 // the amount of request(times) that gets stored

uint32_t requestSendTime[MAXREQLOOP];
uint32_t requestRecieveTime[MAXREQLOOP];
uint32_t requestHostsTime[MAXREQLOOP];
uint8_t myRequestId = 0; // helper var for ping pong to host. so we can measure time between request/response

int32_t timeDeltaToHost = 0;
const uint8_t neededDelimiterAmount = 3; // amount of delimiters needed in response


// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated

uint16_t pollInterval = 3000;           // interval at which to blink (milliseconds)
uint16_t pollIntervalOffset = 0;           // interval at which to blink (milliseconds)


uint8_t ledSyncInterval = 1;           // [seconds] for showing onboard LED in sync to host clock. set to 0 to deactivate
uint16_t ledDuration = 200;             // [ms] to wait before turning the led off again
const int ledPin =  LED_BUILTIN;       // the number of the LED pin
int ledState = LOW;                    // ledState used to set the LED


void loopSyncHandling() {
  onboardLedIndicator();
  if (currentMillis - previousMillis >= pollInterval && getCurrentHostMillis()%pollInterval + pollIntervalOffset > pollIntervalOffset) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    debug("now polling");
    pollHost();
  }
}

void onboardLedIndicator() {
  if(ledSyncInterval == 0) {
    // deactivated
    return;  
  }
  ledDuration = (getCurrentHostMillis()%(ledSyncInterval*4000) < ledSyncInterval*1000) ? 500 : 100;
  // debug("led check delayed?");
  uint16_t theRest = getCurrentHostMillis()%(ledSyncInterval*1000);
  if(ledState == HIGH && theRest >= ledDuration) {
    //debug("led off " + String(theRest));
    ledState = LOW;
    digitalWrite(ledPin, ledState);
    return;
  }
  if(ledState == LOW && theRest < ledDuration) {
    //debug("led on " + String(theRest));
    ledState = HIGH;
    digitalWrite(ledPin, ledState);
  }
}

uint32_t getCurrentHostMillis() {
  return currentMillis + timeDeltaToHost;
}

void pollHost() {
  uint32_t now = millis();
  requestSendTime[myRequestId] = now;
  WifiSend(String(myRequestId), "0");
  myRequestId++;
  if (myRequestId == MAXREQLOOP) {
    myRequestId = 0;
  }
}

void handleRecievedMessageSync(String msg) {
  if(countDelimitersInString(msg, ';') != neededDelimiterAmount) {
    debug("ignoring response due to delimiter amount mismatch");
    return;
  }
  uint8_t reqId = getDelimitedValue(msg, ';', 0).toInt();
  uint32_t now = millis();
  requestRecieveTime[reqId] = now;
  requestHostsTime[reqId] = getDelimitedValue(msg, ';', 1).toInt();
  pollInterval = getDelimitedValue(msg, ';', 2).toInt();
  pollIntervalOffset = getDelimitedValue(msg, ';', 3).toInt();
  recalculateTimeDeltaToHost();
}

void recalculateTimeDeltaToHost() {
  // debug( "duration:" + String(requestRecieveTime[reqId.toInt()] -  ));
  bool haveData = false;
  uint16_t reqDuration;
  uint8_t bestIndexForCalculation = 0;
  uint8_t shortestReqTime = 1000;


  uint8_t lastRequestId = (myRequestId == 0) ? MAXREQLOOP - 1 : myRequestId - 1;
  //debug("recalculateTimeDeltaToHost with last reqId=" + String(lastRequestId));

  // 2 loops to priorise the most recent entry in case we have the same request time [ms]
  for (int i = lastRequestId; i >= 0; i--) {
    //debug("loop 1. lastReq i=" + String(i));
    reqDuration = requestRecieveTime[i] - requestSendTime[i];
    if (reqDuration >= shortestReqTime || reqDuration < 1 || reqDuration >= 1000) {
      continue;
    }
    haveData = true;
    shortestReqTime = reqDuration;
    bestIndexForCalculation = i;

    //debug(String(reqDuration));
  }


  for (int i = MAXREQLOOP - 1; i > lastRequestId; i--) {
    //debug("loop 2. lastReq i=" + String(i));
    reqDuration = requestRecieveTime[i] - requestSendTime[i];
    if (reqDuration >= shortestReqTime || reqDuration < 1 || reqDuration >= 1000) {
      continue;
    }
    haveData = true;
    shortestReqTime = reqDuration;
    bestIndexForCalculation = i;

    //debug(String(reqDuration));
  }
  if (haveData == false) {
    debug("no data available");
    return;
  }
  timeDeltaToHost = requestHostsTime[bestIndexForCalculation] - requestRecieveTime[bestIndexForCalculation] + (shortestReqTime / 2);
  // debug("the delta to host is now:\t" + String(timeDeltaToHost));
  debug("hostmilli:\t" + String(getCurrentHostMillis()) + "\tmymilli:\t" + String(currentMillis) + "\tdelta:\t" + String(timeDeltaToHost));
}
