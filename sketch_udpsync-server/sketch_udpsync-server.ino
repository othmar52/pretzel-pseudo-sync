/*
  thanks to: https://iot.fkainka.de/wifi-udp-y-netzwerk-aus-drei-pretzelboards-und-einem-laptop for the original code
  the server+client sketches has been modified to allow communication between server + 4 clients

  UDP Bidirectional Commmunication for server with three clients, which not all have to be present.
  No change necessary.
  For the definition of the AT commands, see for
  https://room-15.github.io/blog/2015/03/26/esp8266-at-command-reference/#at-commands
  The serial monitor has to be set to Baud 19200 and CR and LF active.
  Messages to be send or received by ESP or entered via Serial Monitor have the following syntax:
  <message Msg> : <char original sender Id on send or final receiver Id on receive>
  example: 100pv:0
*/

#include "syncconfig.h"

#include <SoftwareSerial.h>

SoftwareSerial esp8266(11, 12); // RX, TX

void setup() {
  Serial.begin(19200);
  esp8266.begin(19200);
  setupNetwork();
}

void loop() {
  onboardLedIndicator();
  listenToNetworkMessages();
}

void debug(String Msg) {
  if (DEBUG) {
    Serial.println(Msg);
  }
}
