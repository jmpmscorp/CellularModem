#define CELLMODEM_UBLOX
#include "CellularModem.h"

CellularModem modem(SerialGSM, GSM_RESETN, -1, GSM_DTR, -1); 
CellularModemSMS sms(modem);


void setup() {
 
  Serial.begin(115200);
  SerialGSM.begin(115200);
  modem.init();
  
  modem.connect();
  sms.setTextMode();
}

void loop() {
  
  Serial.println(sms.getSMSMode());
  sms.setPDUMode();
  delay(10000);
}