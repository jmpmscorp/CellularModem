#define CELLMODEM_MODEL SIM800
#define CELLMODEM_USE_FTP
#include "CellularModem.h"

#include "SD.h"

#if defined(ARDUINO_AVR_SODAQ_MBILI)
  #define modemSerial       Serial1
  
  #define MODEM_ON_OFF_PIN  BEEDTR
  #define MODEM_RESET_PIN   -1
  #define MODEM_STATUS_PIN  BEECTS

  #define debugSerial       Serial
  #define SERIAL_BAUDRATE   19200 

  #define SD_CS             11 
#else
  #warning "User should define his own pins"
#endif

CellularModem modem(modemSerial, MODEM_ON_OFF_PIN, MODEM_RESET_PIN, MODEM_STATUS_PIN); 
CellularModemFtp ftp(modem);

const char * apn = "orangeworld";
const char * username = "";
const char * password = "";

const char * ftpServer = "www.doleaguaycontrol.es";
const char * ftpUser = "ftpuser";
const char * ftpPassword = "Oregon07";

void setup() {
  // put your setup code here, to run once:
  modemSerial.begin(19200);
  debugSerial.begin(19200);

  if(!SD.begin(SD_CS)) {
    debugSerial.println("Can't open SD!!"); 
    while(1);
  }

  if(!SD.exists("test")) {
    debugSerial.println("Found test.txt!!");
  } else {
    debugSerial.println("Cant't found test.txt!!");
  }

  File file = SD.open("test.txt");
  if(!file) {
    debugSerial.println("Can't open file!!");
    while(1);
  }
      
  modem.init();
  
  if(modem.networkOn()) {
    if(modem.attachGPRS(apn, username, password)) {
      ftp.init(ftpServer, ftpUser, ftpPassword);

      // const char * test = "Hola test!";
      // ftp.send("/", "test.txt", test, strlen(test));

      if(ftp.send("/", "test.txt", file, file.size())) {
        debugSerial.println("Sent Success!!");
      } else {
        debugSerial.println("Sent Unsuccess!!");
      }

      file.close();
      
      
    }
  }

  debugSerial.println("End");
}

void loop() {
  // put your main code here, to run repeatedly:

}
