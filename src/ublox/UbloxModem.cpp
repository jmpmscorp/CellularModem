#include "UbloxModem.h"
#include <stdio.h>


UbloxModem::UbloxModem(Stream &serial, int8_t onOffPin, int8_t statusPin, int8_t dtrPin, int8_t ctsPin) 
    : CellModem(serial, onOffPin, statusPin, dtrPin, ctsPin)
{

}

UbloxModem::~UbloxModem(){}


void UbloxModem::init() {
    CellModem::init();
}

bool UbloxModem::on() {
    return CellModem::on();
}

bool UbloxModem::isOn() const{
    return CellModem::isOn();
}


bool UbloxModem::_sendInitializationCommands() {
    ATResponse response;

    sendATCommand(F("AT+CMEE=2"));

    if(readResponse() != ATResponse::ResponseOK) {
        return false;
    }
}

bool UbloxModem::connect() {
    return connect(NULL);
}

bool UbloxModem::connect(const char * pin) {
    if(!isOn()) {
        on();
    }

    sendATCommand(F("AT+UMWI=0"));
    if(readResponse() != ATResponse::ResponseOK) {
        return false;
    }

    if(!_sendInitializationCommands()) {
        return false;
    }

    switch (getSimStatus()) {
        case SIMStatus::Ready: {
            break;
        }
        case SIMStatus::NeedsPin: {
            if(!setSimPin(pin)) {
                return false;
            }
            if(getSimStatus() != SIMStatus::Ready) {
                return false;
            }
        }
        case SIMStatus::Missing:
        case SIMStatus::Unknown:
        default:
            return false;
    }

    if( !_enableAutoregistrationNetwork()) {
        return false;
    }

    if(!_waitForSignalQuality()) {
        return false;
    }

    return true;
}

SIMStatus UbloxModem::getSimStatus() {
    SIMStatus simStatus;
    uint8_t retries = 0;

    do {
        if(retries > 0) {
            _modemDelay(250);
        }

        sendATCommand("AT+CPIN?");
        if (readResponse<SIMStatus, uint8_t>(_cpinParser, &simStatus, NULL) == ATResponse::ResponseOK) {
            return simStatus;
        }

        ++retries;
    } while(retries < 5);
    

    return SIMStatus::Missing;
}

bool UbloxModem::setSimPin(const char * pin) {
    if(!pin) {
        return false;
    }

    sendATCommand(F("AT+CPIN=\""), pin, F("\""));

    if(readResponse() == ATResponse::ResponseOK) {
        return true;
    }
    else {
        return false;
    }
}

int UbloxModem::_getAutoregistrationNetworkMode() {
    sendATCommand(F("AT+COPS?"));

    int mode;
    if(readResponse<int, uint8_t>(_copsParser, &mode, NULL) != ATResponse::ResponseOK) {
        return -1;
    }
    
    return mode;
}

bool UbloxModem::_enableAutoregistrationNetwork(uint32_t timeout) {
    if(_getAutoregistrationNetworkMode() == 0) {
        return true;
    }

    uint32_t start = millis();
    while(!isTimedout(start, timeout)) {
        sendATCommand(F("AT+COPS=0"));

        if(readResponse(NULL, 1000) == ATResponse::ResponseOK) {
            _modemDelay(1000);
            
            char nameBuffer[30];
            if(getOperatorName(nameBuffer, sizeof(nameBuffer))) {
                return true;
            }
        }

        _modemDelay(1000);
    }

    return false;

    return false;
}


bool UbloxModem::getOperatorName(char * buffer, size_t size) {
    if(buffer == nullptr) {
        return false;
    }

    if(size > 0) {
        *buffer ='\0';
    }
    else {
        return false;
    }

    sendATCommand(F("AT+COPS?"));

    if (readResponse<char, size_t>(_copsParser, buffer, &size) != ATResponse::ResponseOK) {
        return false;
}

    return (buffer[0] != '\0');
}



bool UbloxModem::getSignalQuality(int8_t * rssi, uint8_t * ber) {
    sendATCommand(F("AT+CSQ"));

    int csqRaw = 0;
    int berRaw = 0;

    if (readResponse<int, int> (_csqParser, &csqRaw, &berRaw) == ATResponse::ResponseOK) {
        *rssi = (csqRaw == 99) ? 0 : static_cast<int8_t>(-113 + 2 * csqRaw);
        *ber = (berRaw == 99) ? 0 : static_cast<uint8_t>(berRaw);

        return true;
    }

    return false;
}

bool UbloxModem::_waitForSignalQuality(uint32_t timeout) {
    uint32_t start = millis();

    int8_t rssi; uint8_t ber;

    uint8_t delayCount = 1;
    while(!isTimedout(start, timeout)) {
        if(getSignalQuality(&rssi, &ber)) {
            if(rssi != 0 && rssi >= _minRSSI) {
                return true;
            }
        }

        _modemDelay(delayCount * 1000);

        if(delayCount < 5) {
            ++delayCount;
        }
    }

    return false;
}


int UbloxModem::attachGPRS(const char * apn, const char * user, const char * password) {

}

int UbloxModem::dettachGPRS() {

}


// Parse the result from AT+COPS? and when we want to see the operator name.
// The manual says to expect:
//   +COPS: <mode>[,<format>,<oper>[,<AcT>]]
//   OK
ATResponse UbloxModem::_copsParser(ATResponse &response, const char* buffer, size_t size,
        char* operatorNameBuffer, size_t* operatorNameBufferSize)
{
    if (!operatorNameBuffer || !operatorNameBufferSize) {
        return ATResponse::ResponseError;
    }

    // TODO maybe limit length of string in format? needs converting int to string though
    if (sscanf_P(buffer, PSTR("+COPS: %*d,%*d,\"%[^\"]\",%*d"), operatorNameBuffer) == 1) {
        return ATResponse::ResponseEmpty;
    }
    if (sscanf_P(buffer, "+COPS: %*d,%*d,\"%[^\"]\"", operatorNameBuffer) == 1) {
        return ATResponse::ResponseEmpty;
    }
    int dummy;
    if (sscanf_P(buffer, "+COPS: %d", &dummy) == 1) {
        return ATResponse::ResponseEmpty;
    }

    return ATResponse::ResponseError;
}

ATResponse UbloxModem::_copsParser(ATResponse &response, const char* buffer, size_t size,
    int * mode, uint8_t* networkTechnology)
{
    if(!networkTechnology && !mode) {
        return ATResponse::ResponseError;
    }

    if(!networkTechnology) {
        if(sscanf_P(buffer, PSTR("+COPS: %d"), mode) == 1 || 
            sscanf_P(buffer, PSTR("+COPS: %d,%*d,\"%*[^\"]\",%*d"), mode) == 1) 
        {
            return ATResponse::ResponseEmpty;
        }
        else {
            return ATResponse::ResponseError;
        }
    }

    if(sscanf_P(buffer, PSTR("+COPS: %d,%*d,\"%*[^\"]\",%d"), mode, networkTechnology) == 2) {
        return ATResponse::ResponseEmpty;
    }

    return ATResponse::ResponseError;
}

ATResponse UbloxModem::_cpinParser(ATResponse& response, const char * buffer, size_t size, SIMStatus * simStatus, uint8_t * dummy) {
    if (!simStatus) {
        return ATResponse::ResponseError;
    }

    char status[16];
    if (sscanf_P(buffer, "+CPIN: %" STR(sizeof(status)-1) "s", status) == 1) {
        if (c_str_startWith("READY", status)) {
            *simStatus = SIMStatus::Ready;
        }
        else {
            *simStatus = SIMStatus::NeedsPin;
        }

        return ATResponse::ResponseEmpty;
    }
    
    return ATResponse::ResponseError;
}


ATResponse UbloxModem::_csqParser(ATResponse& response, const char* buffer, size_t size, int * rssi, int * ber) {
    if (!rssi || !ber) {
        return ATResponse::ResponseError;
    }

    if (sscanf_P(buffer, PSTR("+CSQ: %d,%d"), rssi, ber) == 2) {
        return ATResponse::ResponseEmpty;
    }

    return ATResponse::ResponseError;
}


