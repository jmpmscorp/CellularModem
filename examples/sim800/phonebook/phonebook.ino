#define CELLMODEM_MODEL SIM800
#define CELLMODEM_USE_PHONEBOOK
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


#define MAX_INDEX_LIST  30

CellularModem modem(modemSerial, MODEM_ON_OFF_PIN, MODEM_RESET_PIN, MODEM_STATUS_PIN); 
CellularModemPhonebook phonebook(modem);

char phoneNumber[20] = "";
char contactBuffer[20] = "";

const char * searchFilter = "+34";

void setup() {
 
  debugSerial.begin(SERIAL_BAUDRATE);
  modemSerial.begin(SERIAL_BAUDRATE);

  unsigned int now = millis();

  while(!Serial && millis() - now < 5000) {
    delay(10);
  }

  modem.init();
  
  modem.networkOn();


  if(phonebook.addContact("+34630123123")){
    debugSerial.println("Conctact saved!");
  }
  
  if(phonebook.readContact(1, phoneNumber, contactBuffer)) {
    debugSerial.print("Phone Number: ");
    debugSerial.println(phoneNumber);
    debugSerial.print("Contact: ");
    debugSerial.println(contactBuffer);
  }

  unsigned int indexContactFound;
  unsigned int totalOccurences; 

  int contactsWritten = phonebook.searchContacts(searchFilter, &indexContactFound, 1, &totalOccurences);
  debugSerial.print("Contacts Written: ");
  debugSerial.println(contactsWritten);
  debugSerial.print("Contact Index: ");
  debugSerial.println(indexContactFound);
  debugSerial.print("Total Occurences: ");
  debugSerial.println(totalOccurences);

  phonebook.removeContact(4);
  
  unsigned int indexContactsFound[3];
  contactsWritten = phonebook.searchContacts(searchFilter, indexContactsFound, 3, &totalOccurences);
  debugSerial.print("Contacts Written: ");
  debugSerial.println(contactsWritten);

  for(uint8_t i = 0; i < contactsWritten; ++i) {
    debugSerial.print("Contact Index: ");
    debugSerial.println(indexContactsFound[i]);
  }
  debugSerial.print("Total Occurences: ");
  debugSerial.println(totalOccurences);
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
