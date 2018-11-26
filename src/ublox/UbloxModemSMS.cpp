#include "UbloxModemSMS.h"

UbloxModemSMS::UbloxModemSMS(UbloxModem &modem) :
    CellModemSMS(modem) {
    _modem->addUrcHandler(this);
}

bool UbloxModemSMS::sendSMS(char * telNumber, const char * text) {

}

bool UbloxModemSMS::readSMS() {

}

int8_t UbloxModemSMS::getSMSMode() {
    _modem->sendATCommand(F("AT+CMGF?"));
    
    uint8_t mode;
    if(_modem->readResponse<uint8_t, uint8_t>(_cmgfParser, &mode, NULL) != ATResponse::ResponseOK) {
        return -1;
    }

    return static_cast<int8_t>(mode);
}

bool UbloxModemSMS::setCNMI(uint8_t mode, uint8_t mt) {
    _modem->sendATCommand(F("AT+CNMI="), mode, ',', mt);

    if(_modem->readResponse() == ATResponse::ResponseOK) {
        _mt = mt;
        return true;
    }

    return false;
}

bool UbloxModemSMS::setTextMode() {
    return _setSMSMode(1);
}

bool UbloxModemSMS::setPDUMode() {
    return _setSMSMode(0);
}

ATResponse UbloxModemSMS::handleUrcs() {
    if(_mt == 1) {
        if(strstr(_modem->getResponseBuffer(), "+CMTI:")) {
            Serial.println("CMTI Arrived");
        }
    }
    else if (_mt == 2) {
        if(strstr(_modem->getResponseBuffer(), "+CMT:")) {
            Serial.println("CMT Arrived");
        }
    }    
}

bool UbloxModemSMS::_setSMSMode(uint8_t mode) {
    _modem->sendATCommand(F("AT+CMGF="), mode);

    if(_modem->readResponse() == ATResponse::ResponseOK) {
        return true;
    }

    return false;
}

ATResponse UbloxModemSMS::_cmgfParser(ATResponse &response, const char * buffer, size_t size, uint8_t * mode, uint8_t * dummy) {
    if(!mode) {
        return ATResponse::ResponseError;
    }

    int modeAux;
    if(sscanf_P(buffer, PSTR("+CMGF: %d"), &modeAux) == 1) {
        *mode = static_cast<uint8_t>(modeAux);
        return ATResponse::ResponseEmpty;
    }

    return ATResponse::ResponseError;
}