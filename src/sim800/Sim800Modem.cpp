#include "Sim800Modem.h"

Sim800Modem::Sim800Modem(Stream &serial, int8_t onOffPin, int8_t statusPin, int8_t dtrPin, int8_t ctsPin) 
    : CellModem(serial, onOffPin, statusPin, dtrPin, ctsPin) 
{

}

Sim800Modem::~Sim800Modem() {}

bool Sim800Modem::enableDatetimeNetworkSync() {
    if(_getCLTS() == 1) {
        return true;
    }

    if(!_setCLTS(1)) {
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

bool Sim800Modem::disableDatetimeNetworkSync() {
    return _setCLTS(0);
}

bool Sim800Modem::_setCLTS(uint8_t mode) {
    sendATCommand(F("AT+CLTS="), mode);

    if(readResponse() == ATResponse::ResponseOK) {
        return true;
    }

    return false;
}

int8_t Sim800Modem::_getCLTS() {
    sendATCommand(F("AT+CLTS?"));

    unsigned int mode;
    if(readResponse<unsigned int, uint8_t>(_cltsParser, &mode, nullptr) == ATResponse::ResponseOK) {
        Serial.println("Mode: ");
        Serial.println(mode);
        
        return static_cast<int8_t>(mode);
    }

    Serial.println("No Mode");
    return -1;
}


ATResponse Sim800Modem::_cltsParser(ATResponse &response, const char * buffer, size_t size, unsigned int * mode, uint8_t * dummy) {
    if(!mode) {
        return ATResponse::ResponseError;
    }

    if(sscanf_P(buffer, PSTR("+CLTS: %u"), mode) == 1) {
        return ATResponse::ResponseEmpty;
    }

    return ATResponse::ResponseError;
}