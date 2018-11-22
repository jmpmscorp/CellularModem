#include "UbloxModem.h"
#include <stdio.h>


UbloxModem::UbloxModem(Stream &serial, int8_t onOffPin, int8_t statusPin, int8_t dtrPin, int8_t ctsPin) 
    : CellModem(serial, onOffPin, statusPin, dtrPin, ctsPin)
{

}

UbloxModem::~UbloxModem(){}


void UbloxModem::begin() {
    CellModem::begin();
}

bool UbloxModem::on() {
    return CellModem::on();
}

bool UbloxModem::isOn() const{
    return CellModem::isOn();
}

bool UbloxModem::connect() {
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
}

int UbloxModem::attachGPRS(const char * apn, const char * user, const char * password) {

}

int UbloxModem::dettachGPRS() {

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
    if (sscanf(buffer, "+COPS: %*d,%*d,\"%[^\"]\"", operatorNameBuffer) == 1) {
        return ATResponse::ResponseEmpty;
    }
    int dummy;
    if (sscanf(buffer, "+COPS: %d", &dummy) == 1) {
        return ATResponse::ResponseEmpty;
    }

    return ATResponse::ResponseError;
}

ATResponse UbloxModem::_copsParser(ATResponse &response, const char* buffer, size_t size,
    int* networkTechnology, uint8_t* dummy)
{
    if(!networkTechnology) {
        return ATResponse::ResponseError;
    }

    if(sscanf_P(buffer, PSTR("+COPS: %*d,%*d,\"%*[^\"]\",%d"), networkTechnology) == 1) {
        return ATResponse::ResponseEmpty;
    }

    return ATResponse::ResponseError;
}

int UbloxModem::getSignalQuality(int8_t * rssi, int8_t * ber) {

}

bool UbloxModem::_sendInitializationCommands() {
    ATResponse response;

    sendATCommand(F("AT+CMEE=2"));

    if(readResponse() != ATResponse::ResponseOK) {
        return false;
    }

}