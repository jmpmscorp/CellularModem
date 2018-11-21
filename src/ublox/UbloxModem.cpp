#include "UbloxModem.h"

UbloxModem::UbloxModem(Uart &serial, int8_t onOffPin, int8_t statusPin, int8_t dtrPin, int8_t ctsPin) 
    : CellModem(serial, onOffPin, statusPin, dtrPin, ctsPin)
{

}

UbloxModem::~UbloxModem(){}


void UbloxModem::begin(unsigned long baudrate) {
    CellModem::begin(baudrate);
}

int UbloxModem::attachGPRS(const char * apn, const char * user, const char * password) {

}

int UbloxModem::dettachGPRS() {

}


int UbloxModem::getSignalQuality(int8_t * rssi, int8_t * ber) {

}

ATResponse UbloxModem::_sendInitializationCommands() {
    ATResponse response;

    sendATCommand(F("AT+CMEE=2"));

}