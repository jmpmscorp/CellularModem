#define CELLMODEM_MODEL SIM800
#define CELLMODEM_USE_PHONEBOOK
#include "CellularModem.h"

#if defined(ARDUINO_AVR_SODAQ_MBILI)
  #define modemSerial       Serial1
  #define MODEM_DTR_PIN     -1
  #define MODEM_CTS_PIN     BEECTS
  #define MODEM_ON_OFF_PIN  BEEDTR
  #define MODEM_STATUS_PIN  -1

  #define debugSerial       Serial
  #define SERIAL_BAUDRATE   19200 
#else
  #warning "User should define his own pins"
#endif


#define MAX_INDEX_LIST  30

CellularModem modem(modemSerial, MODEM_ON_OFF_PIN, MODEM_STATUS_PIN, MODEM_DTR_PIN, MODEM_CTS_PIN); 
CellularModemPhonebook phonebook(modem);

char phoneNumber[20] = "";
char contactBuffer[20] = "";

void setup() {
 
  debugSerial.begin(SERIAL_BAUDRATE);
  modemSerial.begin(SERIAL_BAUDRATE);

  unsigned int now = millis();

  while(!Serial && millis() - now < 5000) {
    delay(10);
  }

  modem.init();
  
  modem.networkOn();

  
  phonebook.readSingleContact(1, phoneNumber, contactBuffer);
    debugSerial.print("Phone Number: ");
    debugSerial.println(phoneNumber);
    debugSerial.print("Contact: ");
    debugSerial.println(contactBuffer);
  
  
}

void loop() {
    modem.poll();

    
    /*if (sms.read(12, phoneNumber, textBuffer)) {
      debugSerial.println(phoneNumber);
      debugSerial.println(textBuffer);

      if(sms.remove(12)) {
        debugSerial.println("Success Remove");
      }
    }*/
}
