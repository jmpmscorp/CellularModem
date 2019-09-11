#include "SimcomModem.h"

SimcomModem::SimcomModem(Stream &serial, int8_t onOffPin, int8_t resetPin, int8_t statusPin) 
    : CellModem(serial, onOffPin, resetPin, statusPin) 
{
}

SimcomModem::~SimcomModem() {}

bool SimcomModem::softwareOff() {
    sendATCommand(F("AT+CPOWD=1"));

    if(readResponse<uint8_t, uint8_t>(_cpowdParser, nullptr, nullptr, nullptr, 60000) == ATResponse::ResponseOK) {
        off();
        return true;
    }

    return false;
}

ATResponse SimcomModem::_cpowdParser(ATResponse &response, const char * buffer, size_t size, uint8_t * dummy1, uint8_t * dummy2) {
    if(strstr_P(buffer, PSTR("NORMAL POWER DOWN"))) {
        return ATResponse::ResponseOK;
    }

    return ATResponse::ResponseMultilineParser;
}

bool SimcomModem::setLowPowerMode(uint8_t mode) {
    sendATCommand(F("AT+CSCLK="), mode);

    if(readResponse() == ATResponse::ResponseOK) {
        _lowPowerMode = mode;
        return true;
    }

    return false;

}

bool SimcomModem::enableLowPowerMode() {
    if(_lowPowerMode == 1) {
        if(_uartPins.dtr > -1) {
            digitalWrite(_uartPins.dtr, HIGH);
            return true;
        }

        return false;
    }

    return true;
}

bool SimcomModem::disableLowPowerMode() {
    if(_lowPowerMode == 1) {
        if(_uartPins.dtr > -1) {
            digitalWrite(_uartPins.dtr, LOW);
            return true;
        }

        return false;
    }

    return true;

}

bool SimcomModem::attachGPRS(const char * apn, const char * username, const char * password) {
    if(apn == nullptr) return false;

    if(isGPRSConnected()) {
        detachGRPS();
    }

    sendATCommand(F("AT+SAPBR=3,"), DEFAULT_BEARER_ID, ",\"APN\",\"", apn, '"');

    if(readResponse() != ATResponse::ResponseOK) return false;

    if(username != nullptr) {
        sendATCommand(F("AT+SAPBR=3,"), DEFAULT_BEARER_ID, ",\"USER\",\"", username, '"');

        if(readResponse() != ATResponse::ResponseOK) return false;
    }

    if(password != nullptr) {
        sendATCommand(F("AT+SAPBR=3,"), DEFAULT_BEARER_ID, ",\"PWD\",\"", password, '"');

        if(readResponse() != ATResponse::ResponseOK) return false;
    }

    sendATCommand(F("AT+SAPBR=1,"), DEFAULT_BEARER_ID);

    if(readResponse(nullptr, 85000) == ATResponse::ResponseOK) return true;

    return false;
}

bool SimcomModem::detachGRPS() {
    sendATCommand(F("AT+SAPBR=0,"), DEFAULT_BEARER_ID);

    if(readResponse(nullptr, 65000) == ATResponse::ResponseOK) {
        return true;
    }

    return false;
}

bool SimcomModem::isGPRSConnected() {
    sendATCommand(F("AT+SAPBR=2,"), DEFAULT_BEARER_ID);

    unsigned int status = 0;

    if(readResponse<unsigned int, char>(_sapbrParser, &status, nullptr) == ATResponse::ResponseOK) {
        return status == 1;
    }

    return false;
}

ATResponse SimcomModem::_sapbrParser(ATResponse &response, const char * buffer, size_t size, unsigned int * status, char * ip) {
    if(status == nullptr) return ATResponse::ResponseError;

    if(sscanf_P(buffer, PSTR("+SAPBR: %*d,%d"), status) == 1) {
        return ATResponse::ResponseEmpty;
    }

    return ATResponse::ResponseNotFound;
}



bool SimcomModem::enableDatetimeNetworkSync() {
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

bool SimcomModem::disableDatetimeNetworkSync() {
    return _setCLTS(0);
}

bool SimcomModem::_setCLTS(uint8_t mode) {
    sendATCommand(F("AT+CLTS="), mode);

    if(readResponse() == ATResponse::ResponseOK) {
        return true;
    }

    return false;
}

bool SimcomModem::_initializationProcess() {
    _gotCIEV = _getCLTS() == 1 ? false : true;
    _gotSMSReady = false;
    // Serial.println(_gotCIEV);
    addUrcHandler(this);

    unsigned long start = millis();

    do {
        poll();
        _modemDelay(1);
    }while((!_gotSMSReady || !_gotCIEV) && !isTimedout(start, 20000));

    _gotCIEV = _gotSMSReady = true;
    removeUrcHandler(this);
    
    return true;
    
}



int8_t SimcomModem::_getCLTS() {
    sendATCommand(F("AT+CLTS?"));

    unsigned int mode;
    if(readResponse<unsigned int, uint8_t>(_cltsParser, &mode, nullptr) == ATResponse::ResponseOK) {        
        return static_cast<int8_t>(mode);
    }
    return -1;
}


ATResponse SimcomModem::_cltsParser(ATResponse &response, const char * buffer, size_t size, unsigned int * mode, uint8_t * dummy) {
    if(!mode) {
        return ATResponse::ResponseError;
    }

    if(sscanf_P(buffer, PSTR("+CLTS: %u"), mode) == 1) {
        return ATResponse::ResponseEmpty;
    }

    return ATResponse::ResponseError;
}

ATResponse SimcomModem::handleUrcs() {
    if(strncmp_P(_responseBuffer, PSTR("SMS Ready"), 9) == 0) {
        _gotSMSReady = true;
        return ATResponse::UrcHandled;
    } else if(strncmp_P(_responseBuffer, PSTR("+CIEV"), 5) == 0) {
        _gotCIEV = true;
        return ATResponse::UrcHandled;
    }

    return ATResponse::ResponseEmpty;
}