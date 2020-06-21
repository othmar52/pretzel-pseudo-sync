/*
  thanks to: https://iot.fkainka.de/wifi-udp-y-netzwerk-aus-drei-pretzelboards-und-einem-laptop for the original code
  the server+client sketches has been modified to allow communication between server + 4 clients
  
  UDP Bidirectional Commmunication for Client 2 in a network of one server with three clients, which not all have to be present.
  No change necessary.
  For the definition of the AT commands, see for
  https://room-15.github.io/blog/2015/03/26/esp8266-at-command-reference/#at-commands
  The serial monitor has to be set to Baud 19200 and CR and LF active.
  Messages to be send or received by ESP or entered via Serial Monitor have the following syntax:
  <message Msg> : <char final receiver Id on send or original sender Id on receive>
  example: 100pv:0
*/

#include "syncconfig.h"
#include <SoftwareSerial.h>

//Ids for the devices (starting from 1, max 4 clients possible)
#define me          "1" //myself

SoftwareSerial esp8266(11, 12); // RX, TX



void setup() {
  Serial.begin(19200);
  esp8266.begin(19200);
  setupNetwork();
}

unsigned long currentMillis = 0;

void loop() {
  currentMillis = millis();
  loopSyncHandling();
  checkNetworkMessages();
}

void handleRecievedMessage(String msg) {
  handleRecievedMessageSync(msg);
  
}

String getDelimitedValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

int countDelimitersInString(String data, char delimiter)
{
  int delimiterAmount = 0;
  for(int i=0; i<=data.length()-1; i++){
    if(data.charAt(i)==delimiter){
        delimiterAmount++;
    }
  }
  return delimiterAmount;
}


//-------------------------------------------------Debug Functions------------------------------------------------------
void debug(String Msg)
{
  if (DEBUG)
  {
    Serial.println(Msg);
  }
}
