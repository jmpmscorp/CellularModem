#include "UbloxModem.h"
#include <stdio.h>

#define DEFAULT_GPRS_PROFILE    0

UbloxModem::UbloxModem(Stream &serial, int8_t onOffPin, int8_t statusPin, int8_t dtrPin, int8_t ctsPin) 
    : CellModem(serial, onOffPin, statusPin, dtrPin, ctsPin)
{

}

UbloxModem::~UbloxModem(){}


bool UbloxModem::_initializationProcess() {
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


bool UbloxModem::attachGPRS(const char * apn, const char * username, const char * password) {
    if(!apn) {
        return false;
    }

    sendATCommand(F("AT+UPSD="), DEFAULT_GPRS_PROFILE, ",1,\"", apn, "\"");

    if(readResponse() != ATResponse::ResponseOK) {
        return false;
    }

    if(username && *username) {
        sendATCommand(F("AT+UPSD="), DEFAULT_GPRS_PROFILE, ",2,\"", username, "\"");

        if(readResponse() != ATResponse::ResponseOK) {
            return false;
        }
    }

    if(password && *password) {
        sendATCommand(F("AT+UPSD="), DEFAULT_GPRS_PROFILE, ",3,\"", password, "\"");

        if(readResponse() != ATResponse::ResponseOK) {
            return false;
        }
    }

    //DHCP
    sendATCommand(F("AT+UPSD="), DEFAULT_GPRS_PROFILE,F(",7,\"0.0.0.0\""));

    if(readResponse() != ATResponse::ResponseOK) {
        return false;
    }

    sendATCommand(F("AT+UPSD="), DEFAULT_GPRS_PROFILE, F(",6,3"));

    if(readResponse() != ATResponse::ResponseOK) {
        return false;
    }

    sendATCommand(F("AT+UPSDA="), DEFAULT_GPRS_PROFILE, ",3");

    if(readResponse(nullptr, 20000) != ATResponse::ResponseOK) {
        return false;
    }

    return true;
}

bool UbloxModem::detachGRPS() {
    return true;
}

bool UbloxModem::isGPRSConnected() {
    uint8_t value = 0;

    sendATCommand(F("AT+UPSND="), DEFAULT_GPRS_PROFILE,",8");

    if(readResponse<uint8_t, uint8_t>(_upsndParser, &value, nullptr) == ATResponse::ResponseOK) {
        return value == 1;
    }

    return false;
}

CellModemClient * UbloxModem::getCellModemClient(uint8_t id) {
    if(id < MAX_CLIENTS_NUMBER - 1) {
        return _clients[id];
    }
}

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

ATResponse UbloxModem::_upsndParser(ATResponse& response, const char * buffer, size_t size, uint8_t * value, uint8_t * dummy) {
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

