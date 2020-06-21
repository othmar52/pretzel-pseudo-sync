/*
  network related functions
*/

String me = "0";//own Id

const String myIp = String(HOST_IP_PATTERN) + "1";
String frId = me;//final receivers Id on receiving a message
String osId = me;//original senders Id on sending a message

bool ESPsga = true;//ESPsga (ESP self generated answer) means the incoming is supposed to be a ESP self generated answer, true for the beginning
String Msg;//net message



void setupNetwork() {
  esp8266.setTimeout(5000);
  debug(sendCom("AT+RST", "ready") ? "RESET OK" : "RESET FAIL");
  if (configAP()) {
    debug("AP ready");
  }
  if (configUDP()) {
    debug("UDP ready");
  }
}

void listenToNetworkMessages() {
  char InChar;
  String Sender = me;

  if (esp8266.available()) {
    if (ESPsga) {//if assumed, that it is ESP self generated answer, check it
      if (esp8266.find("+IPD,")) {
        delay(1);
        //read original sender Id: osId
        osId = (String)esp8266.parseInt();
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
        Msg += InChar;//add net message together, when inChar not ':'
      } else {
        //read final receiver Id: frId
        frId = (String)esp8266.parseInt();

        //Find out who was the original sender
        switch (osId.toInt()) {
          case 0: Sender = "Client 1";   break;
          case 1: Sender = "Client 2";   break;
          case 2: Sender = "Client 3";   break;
          case 3: Sender = "Client 4";   break;
          case 4: Sender = "Client 5";   break;
          default: Sender = "not valid sender " + String(osId.toInt());
        }
        //print 'Msg'
        if (frId != me)debug("Msg got: \"" + Msg + "\" from " + Sender);

        //if the message is for me, execute it, else send it to final receiver Id: frId
        if (frId == me) {
          debug("Msg for me:   \"" + Msg + "\"   from:   " + Sender);
          // sending immediate response
          handleRecievedMessageSync(Msg, osId.toInt()+1);
        }
        //when final receiver is existing, append to message Msg the original sender Id: osId and send it to final receiver
        else if (frId == "1" || frId == "2" || frId == "3" || frId == "4" || frId == "5") WifiSend(Msg + "BBBB:" + osId, frId);
        else debug("final receiver Id is not valid");

        //clear buffer because usable message ended
        esp8266.find('\n');
        //Message ended, ready for next receive, which is again supposed first to be a ESP self generated answer
        ESPsga  = true;
      }
    }
  } else if (Serial.available()) {
    //identify myself
    osId = me;
    //read message Msg

    Msg = Serial.readStringUntil(':');
    delay(1);
    //to do: parse Msg
    //when Msg does not end with "\r\n", there is more, identify the final receiver Id: frId, default it is me
    frId = me;
    //when Msg ends with "\r\n", get rid of it
    if (Msg.endsWith("\r\n")) {
      Msg = Msg.substring(0, Msg.length() - 2);
    }
    else frId = (String)(Serial.read() - '0');//read the final receiver Id: frId
    //if the message is for me execute it, else send it to final receiver Id: frId
    if (frId == me)
      debug("Msg for me: \"" + Msg + "\" from: my Serial Monitor");
    //when final receiver is existing, append to message Msg the original sender Id: osId and send it
    else if (frId == "1" || frId == "2" || frId == "3" || frId == "4" || frId == "5")
      WifiSend(Msg + ":" + osId, frId);
    else debug("final receiver Id is not valid " + frId);
    Serial.find('\n');//clear buffer
  } // else: computing and other code
}

//-----------------------------------------Config ESP8266------------------------------------

boolean configAP()
{
  boolean success = true;
  //set  AP mode (host)
  success &= (sendCom("AT+CWMODE=2", "OK"));
  //set AP to SSID & password, channel Id 5 and no encoding
  success &= (sendCom("AT+CWSAP=\""+ String(SSID)+"\",\""+ String(WLANPASSWD)+"\",5,0", "OK"));
  //Set ip addr of ESP8266 softAP
  success &= (sendCom("AT+CIPAP=\"192.168.4.1\"", "OK"));

  return success;
}

boolean configUDP()
{
  int myTxPortForClient, myRxPortForClient;
  String clientIp;
  boolean success = true;
  //set transfer mode to normal transmission.
  success &= (sendCom("AT+CIPMODE=0", "OK"));
  //set to multiple connections (up to four)
  success &= (sendCom("AT+CIPMUX=1", "OK"));
  for (int i=0; i<MAXCLIENTS; i++) {
    myTxPortForClient = HOST_START_PORT_TX + i + 1;
    myRxPortForClient = HOST_START_PORT_RX + i + 1;
    clientIp = String(HOST_IP_PATTERN + String(i + 2));
    //start connection in UDP with assigned IP addresses, send channel and receive channel
    debug ("client: " + String(i+1) + " IP:" + clientIp + " " + myTxPortForClient + " " + myRxPortForClient);
    success &= sendCom("AT+CIPSTART=" + String(i) + ",\"UDP\",\""+ clientIp +"\","+ myTxPortForClient +","+ myRxPortForClient +"", "OK"); //UDP Bidirectional and Broadcast to Packet sender or TWedge
   
  }
  return success;
}

//-----------------------------------------------Control ESP-----------------------------------------------------

boolean sendCom(String command, char respond[])
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
  bool sucess = sendCom("AT+CIPSEND=" + String(Id.toInt()-1) + "," + (String)(Msg.length()), "OK");
  delay(1);

  sucess &= sendCom(Msg, "OK");

  if (sucess)
    Serial.print("message send ok: \"" + Msg + "\"");
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
