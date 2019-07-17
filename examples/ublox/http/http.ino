#define CELLMODEM_MODEL UBLOX
#define CELLMODEM_USE_HTTP
#include "CellularModem.h"


#ifdef ARDUINO_SAMD_MKRGSM1400
  #define modemSerial       SerialGSM
  #define MODEM_DTR_PIN     GSM_DTR
  #define MODEM_CST_PIN     -1
  #define MODEM_ON_OFF_PIN  GSM_RESETN
  #define MODEM_STATUS_PIN  -1

  #define debugSerial       Serial
#else
  #warning "User should define his own pins"
#endif



CellularModem modem(modemSerial, MODEM_ON_OFF_PIN, MODEM_STATUS_PIN, MODEM_DTR_PIN, MODEM_CST_PIN); 

#if CELLMODEM_MODEL == UBLOX
  CellularModemFilesystem filesystem(modem);
  CellularModemHttp http(modem, filesystem);
#else
  CellularModemHttp hptt(modem);
#endif

const char * server = "doleaguaycontrol.es";
const int port = 80;


void setup() {
 
  debugSerial.begin(115200);
  modemSerial.begin(115200);

  unsigned int now = millis();

  modem.init();
  
  modem.networkOn();

  if(!modem.isGPRSConnected()) {
    modem.attachGPRS("orangeworld", nullptr, nullptr);

    http.initSSL(server);
    
    //http.get("/");
    http.post("/", (uint8_t *)"hola", 4);

    filesystem.deleteFile("writeTemp.ffs");
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
