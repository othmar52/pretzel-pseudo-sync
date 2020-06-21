/*
  network related functions
*/



const String hostIp = String(HOST_IP_PATTERN) + "1";
const String myIp = String(HOST_IP_PATTERN) + String(atoi(me) + 1);
const String myRxPort = String(HOST_START_PORT_TX + atoi(me));
const String myTxPort = String(HOST_START_PORT_RX + atoi(me));


String frId = me;//final receivers Id on sending a message
String osId = me;//original senders Id on receiving a message
bool ESPsga = true;//ESPsga (ESP self generated answer) means the incoming is supposed to be a ESP self generated answer, true for the beginning
String Msg;//net message


void setupNetwork() {
  esp8266.setTimeout(5000);
  if (sendCom("AT+RST", "ready")) debug("RESET OK");
  else debug("RESET NOT OK");
  esp8266.setTimeout(1000);

  if ( ConfigESP ()) debug("CONFIG OK");
  else debug("CONFIG NOT OK");

  if (ConfigWifi ()) debug("WIFI OK");
  else debug("WIFI NOT OK");
  esp8266.setTimeout(1000);

  if (StartCon()) {
    debug("Connected");
  }  else debug("NOT CONNECTED");
}



void checkNetworkMessages() {
  char InChar;
  String Sender = me;

  if (esp8266.available()) {
    if (ESPsga) {//if assumed, that it is ESP self generated answer, check it
      if (esp8266.find("+IPD,")) {
        delay(1);
        //ignore the serverId
        esp8266.find(':');
        //clear the net message: Msg
        Msg = "";
        //Found "+IPD,", so we set ESPsga false to not read it again until next message or answer
        ESPsga = false;//it is not a ESP self generated answer, so it is a message
      } else
        //clear buffer, when "+IPD," not found
        esp8266.find('\n');
    } else {//it is clear, that a message has been found
      //reads one char of the message per looping from the buffer
      InChar = esp8266.read();

      if (!(InChar == ':')) {
        Msg += InChar;//add message together, when inChar not ':'
      } else {
        //read original sender Id: osId
        osId = (String)esp8266.parseInt();

        //Find out who was the original sender
        switch (osId.toInt()) {
          case 0: Sender = "Server"; break;
          case 1: Sender = "Client 1";   break;
          case 2: Sender = "Client 2";   break;
          case 3: Sender = "Client 3";   break;
          case 4: Sender = "Client 4";   break;
          case 5: Sender = "Client 5";   break;
          default: Sender = "not valid sender";
        }
        //print 'Msg'
        // debug("Msg got: \"" + Msg + "\" from " + Sender + "delta: " + String(millis() - Msg.toInt()));
        handleRecievedMessage(Msg);

        ///clear buffer because usable message ended
        esp8266.find('\n');
        //Message ended, ready for next receive, which is again supposed first to be a ESP self generated answer
        ESPsga  = true;
      }
    }
  } else if (Serial.available()) {
    // TODO: we do not any serial stuff, right?
    return;


    //default send to myself, final receiver, that's me
    frId = me;

    Msg = Serial.readStringUntil(':');
    delay(1);
    //to do: parse Msg
    //when Msg does not end with "\r\n", there is more, identify the final receiver Id: frId, default it is me
    //when Msg ends with "\r\n", get rid of it
    if (Msg.endsWith("\r\n")) {
      Msg = Msg.substring(0, Msg.length() - 2);
    }
    else frId = (String)(Serial.read() - '0');//read the final receiver Id: frId
    //if the message is for me execute it, else send it to the server
    if (frId == me)
      debug("Msg for me:  \"" + Msg + "\" from: my Serial Monitor");
    //when final receiver is existing, send it to the server
    else if (frId == "0" || frId == "1" || frId == "2" || frId == "3" || frId == "4" || frId == "5")
      WifiSend(Msg, frId);
    else debug("final receiver Id is not valid");
    Serial.find('\n');//clear buffer
    esp8266.write(Serial.read());
  } // else: computing and other code

}





//-----------------------------------------Config ESP8266------------------------------------
bool ConfigESP () {
  bool success = sendCom("AT+CIPMODE=0", "OK");
  success &= sendCom("AT+CIPMUX=1", "OK");
  //set to station (client) mode
  success &= sendCom("AT+CWMODE=1", "OK");
  //set IP addr of ESP8266 station (client)
  success &= sendCom("AT+CIPSTA=\""+ myIp +"\"", "OK");
  debug("my ip is " + myIp);
  return success;
}

bool ConfigWifi () {
  esp8266.setTimeout(3000);
  // connect to host's AP
  return (sendCom("AT+CWJAP=\""+ String(SSID)+"\",\""+ String(WLANPASSWD)+"\"", "OK"));
}

bool StartCon () {
  //start connection with own Id and servers IP addr, send channel and receive channel
  debug("host ip is " + hostIp);
  debug("my tx port " + myTxPort);
  debug("my rx port " + myRxPort);
  return sendCom(String("AT+CIPSTART=1,\"UDP\",\""+ hostIp +"\","+ myTxPort +","+ myRxPort +""), "OK");

}
//-----------------------------------------------Controll ESP-----------------------------------------------------
bool sendCom(String command, char respond[])
{
  esp8266.println(command);
  if (esp8266.findUntil(respond, "ERROR"))
  {
    return true;
  }
  debug("ESP SEND ERROR: " + command);
  return false;
}


//---------------------------------------------------------Sender--------------------------------------------------------
void WifiSend (String Msg, String Id) {
  //A client can only send a message directly to the Server, so the first id is everytime the id of the connection to the server
  bool sucess = sendCom("AT+CIPSEND=1," + (String)(Msg.length() + 2), "OK");
  delay(1);

  sucess &= sendCom(Msg + ":" + Id, "OK");
  return;
  if (sucess)
    Serial.print("Msg send ok: \"" + Msg + "\"");
    
  switch (Id.toInt()) {
    case 0: debug("   sent to Server"); break;
    case 1: debug("   sent to Client 1"); break;
    case 2: debug("   sent to Client 2"); break;
    case 3: debug("   sent to Client 3"); break;
    case 4: debug("   sent to Client 4"); break;
    case 5: debug("   sent to Client 5"); break;
    default: debug("   invalid ID");
  }
}
