#include "UbloxModem.h"
#include <stdio.h>


UbloxModem::UbloxModem(Stream &serial, int8_t onOffPin, int8_t statusPin, int8_t dtrPin, int8_t ctsPin) 
    : CellModem(serial, onOffPin, statusPin, dtrPin, ctsPin)
{

}

UbloxModem::~UbloxModem(){}

bool UbloxModem::enableDatetimeNetworkSync() {
    if(_getCTZU() == 1) {
        return true;
    }

    if(!_setCTZU(1)) {
        return false;
    }

    sendATCommand(F("AT+COPS=2"));

    if(readResponse() != ATResponse::ResponseOK) {
        return false;
    }

    if(!_enableAutoregistrationNetwork()) {
        return false;
    }

    return true;
}

bool UbloxModem::disableDatetimeNetworkSync() {
    return _setCTZU(0);
}

bool UbloxModem::_setCTZU(uint8_t mode) {
    sendATCommand(F("AT+CTZU="), mode);

    if(readResponse() == ATResponse::ResponseOK) {
        return true;
    }

    return false;
}

int8_t UbloxModem::_getCTZU() {
    sendATCommand(F("AT+CTZU?"));

    unsigned int mode;
    if(readResponse<unsigned int, uint8_t>(_ctzuParser, &mode, nullptr) == ATResponse::ResponseOK) {
        return static_cast<int8_t>(mode);
    }

    return -1;
}

ATResponse UbloxModem::_ctzuParser(ATResponse& response, const char * buffer, size_t size, 
    unsigned int * mode, uint8_t * dummy)
{
    if(!mode) {
        return ATResponse::ResponseError;
    }

    if(sscanf_P(buffer, PSTR("+CTZU: %u"), mode) == 1) {
        return ATResponse::ResponseEmpty;
    }

    return ATResponse::ResponseError;
}

bool UbloxModem::_sendInitializationCommands() {
    sendATCommand(F("AT+CMEE=2"));

    if(readResponse() != ATResponse::ResponseOK) {
        return false;
    }

    sendATCommand(F("AT+UMWI=0"));

    if(readResponse() != ATResponse::ResponseOK) {
        return false;
    }

    return true;
}