#include "UbloxModem.h"
#include <stdio.h>


UbloxModem::UbloxModem(Stream &serial, int8_t onOffPin, int8_t statusPin, int8_t dtrPin, int8_t ctsPin) 
    : CellModem(serial, onOffPin, statusPin, dtrPin, ctsPin)
{

}

UbloxModem::~UbloxModem(){}


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