#define CELLULAR_MODEM_UBLOX
#include "CellularModem.h"

UbloxModem modem(SerialGSM, GSM_DTR, GSM_RESETN); 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //modem.begin(9600);
  

  pinMode(GSM_DTR, OUTPUT);
  pinMode(GSM_RESETN, OUTPUT);

  SerialGSM.begin(115200);
  pinMode(GSM_RESETN, OUTPUT);
  digitalWrite(GSM_RESETN, HIGH);
  delay(100);
  digitalWrite(GSM_RESETN, LOW);
  
  //modem.on();
}

void loop() {
  // put your main code here, to run repeatedly:
  modem.isAlive(1000);
  
  delay(15000);
  // Serial.println("Retry");
}
