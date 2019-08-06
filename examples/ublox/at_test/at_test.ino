#define CELLMODEM_MODEL UBLOX
#include "CellularModem.h"


#ifdef ARDUINO_SAMD_MKRGSM1400
  #define modemSerial       SerialGSM
  #define MODEM_DTR_PIN     GSM_DTR
  #define MODEM_CST_PIN     -1
  #define MODEM_ON_OFF_PIN  -1
  #define MODEM_RESET_PIN   GSM_RESETN  
  #define MODEM_STATUS_PIN  -1

  #define debugSerial       Serial
#else
  #warning "User should define his own pins"
#endif



CellularModem modem(modemSerial, MODEM_ON_OFF_PIN, MODEM_RESET_PIN, MODEM_STATUS_PIN, MODEM_DTR_PIN, MODEM_CST_PIN); 


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
  modem.enableDatetimeNetworkSync();
  
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
