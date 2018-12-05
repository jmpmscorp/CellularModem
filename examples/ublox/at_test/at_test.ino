#define CELLMODEM_MODEL SIM800
#include "CellularModem.h"

CellularModem modem(Serial1, BEEDTR, BEECTS, -1, -1); 
//CellularModem modem(Serial, -1, -1, -1, -1);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(19200);
  Serial1.begin(19200);
  modem.init();
  

  /*pinMode(GSM_DTR, OUTPUT);
  pinMode(GSM_RESETN, OUTPUT);

  SerialGSM.begin(115200);
  pinMode(GSM_RESETN, OUTPUT);
  digitalWrite(GSM_RESETN, HIGH);
  delay(100);
  digitalWrite(GSM_RESETN, LOW);
  
  //modem.on();*/
  
  Serial.println(modem.networkOn());
  delay(2000);
  //modem.enableDatetimeNetworkSync();
  
}

void loop() {
  // put your main code here, to run repeatedly:
  //modem.isAlive(1000);º

  char time[22];

  modem.getDatetime(time, sizeof(time));

  Serial.println(time);

  delay(20000);
  // Serial.println("Retry");
}
