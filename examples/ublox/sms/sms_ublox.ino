#define CELLMODEM_UBLOX
#include "CellularModem.h"

CellularModem modem(SerialGSM, GSM_RESETN, -1, GSM_DTR, -1); 
CellularModemSMS sms(modem);

void onCMTI(char * onMemorySaved, uint16_t index) {
  Serial.print(onMemorySaved);
  Serial.print(",");
  Serial.println(index);
}

void onCMT(char * remotePhoneNumber, char * textMessage) {
  Serial.println();
  Serial.print(remotePhoneNumber);
  Serial.print(",");
  Serial.println(textMessage);
}


void setup() {
 
  Serial.begin(115200);
  SerialGSM.begin(115200);
  modem.init();
  
  sms.setCMTCallback(onCMT);
  sms.setCMTICallback(onCMTI);
  modem.connect();
  sms.setTextMode();
  sms.setNewSMSIndicator(1,2);
}

void loop() {
    modem.poll();
}