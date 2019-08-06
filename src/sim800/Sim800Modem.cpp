#include "Sim800Modem.h"

Sim800Modem::Sim800Modem(Stream &serial, int8_t onOffPin, int8_t resetPin, int8_t statusPin, int8_t dtrPin, int8_t ctsPin) 
    : CellModem(serial, onOffPin, resetPin, statusPin, dtrPin, ctsPin) 
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

bool Sim800Modem::_initializationProcess() {
    _gotCIEV = _getCLTS() == 1 ? false : true;
    _gotSMSReady = false;
    // Serial.println(_gotCIEV);
    addUrcHandler(this);

    unsigned long start = millis();

    do {
        poll();
    }while((!_gotSMSReady || !_gotCIEV) && !isTimedout(start, 10000));

    _gotCIEV = _gotSMSReady = true;
    removeUrcHandler(this);
    
    return true;
    
}

ATResponse Sim800Modem::handleUrcs() {
    if(strncmp_P(_responseBuffer, PSTR("SMS Ready"), 9) == 0) {
        _gotSMSReady = true;
    } else if(strncmp_P(_responseBuffer, PSTR("+CIEV"), 5) == 0) {
        _gotCIEV = true;
    }
}

int8_t Sim800Modem::_getCLTS() {
    sendATCommand(F("AT+CLTS?"));

    unsigned int mode;
    if(readResponse<unsigned int, uint8_t>(_cltsParser, &mode, nullptr) == ATResponse::ResponseOK) {        
        return static_cast<int8_t>(mode);
    }
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