#define CELLMODEM_UBLOX
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


#define MAX_INDEX_LIST  30

CellularModem modem(modemSerial, MODEM_ON_OFF_PIN, MODEM_STATUS_PIN, MODEM_DTR_PIN, MODEM_CST_PIN); 
CellularModemSMS sms(modem);

char phoneNumber[20] = "";
char textBuffer[161] = "";

void onCMTI(char * onMemorySaved, uint16_t index) {
  debugSerial.print(onMemorySaved);
  debugSerial.print(",");
  debugSerial.println(index);
}

void onCMT(char * remotePhoneNumber, char * textMessage) {
  debugSerial.println();
  debugSerial.print(remotePhoneNumber);
  debugSerial.print(",");
  debugSerial.println(textMessage);
}


void setup() {
 
  debugSerial.begin(115200);
  modemSerial.begin(115200);

  unsigned int now = millis();

  while(!Serial && millis() - now < 5000) {
    delay(10);
  }

  modem.init();
  
  sms.setCMTCallback(onCMT);
  sms.setCMTICallback(onCMTI);
  modem.networkOn();
  sms.setTextMode();
  sms.setNewSMSIndicator(1,2);

  unsigned int indexList[MAX_INDEX_LIST ];
  unsigned int remainingSize;
  
  int result = sms.readList("ALL", indexList, MAX_INDEX_LIST, &remainingSize);
  debugSerial.println(result);
  if(result > 0) {
    debugSerial.print("Remaining Size: "); debugSerial.println(remainingSize);

    for(size_t i = 0; i < result; ++i) {
      debugSerial.println(indexList[i]);
    }
  }

  sms.remove(0,4);

  sms.send("+34650395489", "Hola");
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