#define CELLULAR_MODEM_UBLOX
#include "CellularModem.h"

CellularModem modem(SerialGSM, GSM_RESETN, -1, GSM_DTR, -1); 
//CellularModem modem(Serial, -1, -1, -1, -1);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  SerialGSM.begin(115200);
  modem.begin();
  

  /*pinMode(GSM_DTR, OUTPUT);
  pinMode(GSM_RESETN, OUTPUT);

  SerialGSM.begin(115200);
  pinMode(GSM_RESETN, OUTPUT);
  digitalWrite(GSM_RESETN, HIGH);
  delay(100);
  digitalWrite(GSM_RESETN, LOW);
  
  //modem.on();*/
  
  modem.connect();
  
}

void loop() {
  // put your main code here, to run repeatedly:
  //modem.isAlive(1000);
  char buffer[20]="";
  modem.getOperatorName(buffer, sizeof(buffer));
  Serial.println(buffer);
  delay(15000);
  // Serial.println("Retry");
}
