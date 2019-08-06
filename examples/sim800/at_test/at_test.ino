#define CELLMODEM_MODEL SIM800
#include "CellularModem.h"

#if defined(ARDUINO_AVR_SODAQ_MBILI)
  #define modemSerial       Serial1
  #define MODEM_DTR_PIN     -1
  #define MODEM_CTS_PIN     -1
  #define MODEM_ON_OFF_PIN  BEEDTR
  #define MODEM_RESET_PIN   -1
  #define MODEM_STATUS_PIN  BEECTS

  #define debugSerial       Serial
  #define SERIAL_BAUDRATE   19200 
#else
  #warning "User should define his own pins"
#endif

CellularModem modem(modemSerial, MODEM_ON_OFF_PIN, MODEM_RESET_PIN, MODEM_STATUS_PIN, MODEM_DTR_PIN, MODEM_CTS_PIN); 


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
  static unsigned long milliseconds = millis();
  char time[22];
  
  while( millis() - milliseconds < 20000) {
    modem.poll();
    
  }

  milliseconds = millis();
  modem.getDatetime(time, sizeof(time));

  Serial.println(time);

  //delay(20000);
  // Serial.println("Retry");
}
