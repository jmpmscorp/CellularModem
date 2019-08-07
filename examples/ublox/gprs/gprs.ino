#define CELLMODEM_MODEL UBLOX
#include "CellularModem.h"
#include "ArduinoHttpClient.h"

#define LOGGING true

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



CellularModem modem(modemSerial, MODEM_ON_OFF_PIN, MODEM_RESET_PIN, MODEM_STATUS_PIN); 
CellularModemClient client(modem);

const char * server = "antaresserver.dynu.net";
const int port = 80;

HttpClient http(client, server, port);

void setup() {
 
  debugSerial.begin(115200);
  modemSerial.begin(115200);

  unsigned int now = millis();

  modem.init();
  
  modem.networkOn();

  if(!modem.isGPRSConnected()) {
    modem.attachGPRS("orangeworld", nullptr, nullptr);

    int error = http.get("/videlsur");

    if(error != 0) {
      debugSerial.println("Failed to get resource");
    }

    int status = http.responseStatusCode();

    debugSerial.print("Response Code: ");
    debugSerial.println(status);

    while(http.headerAvailable()) {
      String headerName = http.readHeaderName();
      String headerValue = http.readHeaderValue();
      debugSerial.println("    " + headerName + " : " + headerValue);
    }
    
    http.stop();
  }
}

void loop() {
  /* static unsigned long millisTime = millis();
  
  modem.poll();

  if(millis() - millisTime > 10000) {
    gprs.isConnected();
    millisTime = millis();
  } */

  if (Serial.available()) {
    SerialGSM.write(Serial.read());
  }

  if (SerialGSM.available()) {
    Serial.write(SerialGSM.read());
  }
}
