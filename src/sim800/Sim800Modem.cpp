#include "Sim800Modem.h"


Sim800Modem::Sim800Modem(Stream &serial, int8_t onOffPin, int8_t statusPin, int8_t dtrPin, int8_t ctsPin) 
    : CellModem(serial, onOffPin, statusPin, dtrPin, ctsPin) 
{
}

Sim800Modem::~Sim800Modem() {}

bool Sim800Modem::on() {

    if(isAlive(200)) {
        return true;
    }

    if(!isOn()) {
        if(_onOffPin > -1) {
            digitalWrite(_onOffPin, LOW);
            _modemDelay(100);
            digitalWrite(_onOffPin, HIGH);
        }

        if(_dtrPin > -1) {
            digitalWrite(_dtrPin, LOW);
            _modemDelay(100);
        }

        _onOffStatus = true;
    }
    
    bool timeout = true;
    
    for (uint8_t i = 0; i < 10; i++) {
        if (isAlive(500)) {
            timeout = false;
            break;
        }
    }

    if (timeout) {
        return false;
    }    
}

bool Sim800Modem::attachGPRS(const char * apn, const char * username, const char * password) {
    if(apn == nullptr) {
        return false;
    }

    if(isGPRSConnected()) {
        if(!detachGRPS()) {
            return false;
        }
    }

    sendATCommand(F("AT+SAPBR=3,"), DEFAULT_BEARER_ID, ",\"APN\",\"", apn, '"');

    if(readResponse() != ATResponse::ResponseOK) {
        return false;
    }

    if(username != nullptr && strlen(username) > 0) {
        sendATCommand(F("AT+SAPBR=3,"), DEFAULT_BEARER_ID, ",\"USER\",\"", username, '"');

        if(readResponse() != ATResponse::ResponseOK) {
            return false;
        }
    }

    if(password != nullptr && strlen(password) > 0) {
        sendATCommand(F("AT+SAPBR=3,"), DEFAULT_BEARER_ID, ",\"PWD\",\"", password, '"');

        if(readResponse() != ATResponse::ResponseOK) {
            return false;
        }
    }

    sendATCommand(F("AT+SAPBR=1,"), DEFAULT_BEARER_ID);

    if(readResponse(nullptr, 85000) == ATResponse::ResponseOK) {
        return true;
    }

    return false;
    
}

bool Sim800Modem::detachGRPS() {
    sendATCommand(F("AT+SAPBR=0,"), DEFAULT_BEARER_ID);

    if(readResponse(nullptr, 65000) == ATResponse::ResponseOK) {
        return true;
    }

    return false;
}

bool Sim800Modem::isGPRSConnected() {
    sendATCommand(F("AT+SAPBR=2,"), DEFAULT_BEARER_ID);

    unsigned int status = 0;

    if(readResponse<unsigned int, char>(_sapbrParser, &status, nullptr) == ATResponse::ResponseOK) {
        if(status == 1) {
            return true;
        }
    }

    return false;
}

ATResponse Sim800Modem::_sapbrParser(ATResponse &response, const char * buffer, size_t size, unsigned int * status, char * ip) {
    if(status == nullptr && ip == nullptr) return ATResponse::ResponseError;

    if(sscanf_P(buffer, PSTR("+SAPBR: %*d,%d"), status) == 1) {
        return ATResponse::ResponseEmpty;
    }

    return ATResponse::ResponseError;
}

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