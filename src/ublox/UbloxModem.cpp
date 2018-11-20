#include "UbloxModem.h"

UbloxModem::UbloxModem(Uart &serial, int8_t dtrPin, int8_t resetPin) : GPRSModem(serial, dtrPin, resetPin)
{

}

UbloxModem::~UbloxModem(){}


void UbloxModem::begin(unsigned long baudrate) {
    ATCellularModem::beginModem(baudrate);
}

int UbloxModem::on() {
    if(ATCellularModem::isAlive()) {
        return 0;
    }

    if(_resetPin > -1) {
        digitalWrite(_resetPin, HIGH);
        _delay(100);
        digitalWrite(_resetPin, LOW);
    }

    if(_dtrPin > -1) {
        digitalWrite(_dtrPin, LOW);
        _delay(100);
    }

    return isAlive(300);
}

int UbloxModem::off() {
    
}

int UbloxModem::reset() {
    sendATCommand(F("AT+CFUN=16"));

    return (waitForResponse(1000) == 1);
}

int UbloxModem::attachGPRS(const char * apn, const char * user, const char * password) {

}

int UbloxModem::dettachGPRS() {

}

int UbloxModem::networkOn() {

}

int UbloxModem::getSignalQuality(int8_t * rssi, int8_t * ber) {

}