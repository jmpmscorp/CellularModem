#include "UbloxModemGPRS.h"

#define DEFAULT_PROFILE     0

UbloxModemGPRS::UbloxModemGPRS(UbloxModem &modem) :
    CellModemGPRS(modem) 
{
    _modem->addUrcHandler(this);
}

bool UbloxModemGPRS::attach(const char * apn, const char * username, const char * password) {
    if(!apn) {
        return false;
    }

    _modem->sendATCommand(F("AT+UPSD="), DEFAULT_PROFILE, ",1,\"", apn, "\"");

    if(_modem->readResponse() != ATResponse::ResponseOK) {
        return false;
    }

    if(username && *username) {
        _modem->sendATCommand(F("AT+UPSD="), DEFAULT_PROFILE, ",2,\"", username, "\"");

        if(_modem->readResponse() != ATResponse::ResponseOK) {
            return false;
        }
    }

    if(password && *password) {
        _modem->sendATCommand(F("AT+UPSD="), DEFAULT_PROFILE, ",3,\"", password, "\"");

        if(_modem->readResponse() != ATResponse::ResponseOK) {
            return false;
        }
    }

    //DHCP

    _modem->sendATCommand(F("AT+UPSD="), DEFAULT_PROFILE,F(",7,\"0.0.0.0\""));

    if(_modem->readResponse() != ATResponse::ResponseOK) {
        return false;
    }

    _modem->sendATCommand(F("AT+UPSD="), DEFAULT_PROFILE, F(",6,3"));

    if(_modem->readResponse() != ATResponse::ResponseOK) {
        return false;
    }

    _modem->sendATCommand(F("AT+UPSDA="), DEFAULT_PROFILE, ",3");

    if(_modem->readResponse(nullptr, 20000) != ATResponse::ResponseOK) {
        return false;
    }

    return true;
}


bool UbloxModemGPRS::dettach() {
    return true;
}


bool UbloxModemGPRS::isConnected() {
    uint8_t value = 0;

    _modem->sendATCommand(F("AT+UPSND="), DEFAULT_PROFILE,",8");

    if(_modem->readResponse<uint8_t, uint8_t>(_upsndParser, &value, nullptr) == ATResponse::ResponseOK) {
        return value == 1;
    }

    return false;
}


ATResponse UbloxModemGPRS::_upsndParser(ATResponse& response, const char * buffer, size_t size, uint8_t * value, uint8_t * dummy) {
    if(!value) {
        return ATResponse::ResponseError;
    }

    int val;

    if(sscanf_P(buffer, PSTR("+UPSND: %*d,%*d,%d"), &val) == 1) {
        *value = val;

        return ATResponse::ResponseEmpty;
    }

    return ATResponse::ResponseError;
}

ATResponse UbloxModemGPRS::handleUrcs() {
    return ATResponse::ResponseOK;
}