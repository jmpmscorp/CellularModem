#include "CellModemPhonebook.h"

CellModemPhonebook::CellModemPhonebook(CellModem &modem) :
    _modem(&modem)
    {

    }

bool CellModemPhonebook::addContact(const char * phoneNumber, const char * contactName, uint8_t index = 0) {
    if(!phoneNumber) {
        return false;
    }

    _modem->sendATCommand(F("AT+CPBW="), index > 0 ? index : '\0', 
        phoneNumber,",,", contactName != nullptr ? contactName : phoneNumber);

    if(_modem->readResponse() == ATResponse::ResponseOK) {
        return true;
    }

    return false;
}


bool CellModemPhonebook::readSingleContact(uint8_t index, char * numberBuffer, char * contactNameBuffer) {
    if(index <=0 ) {
        return false;
    }

    _modem->sendATCommand(F("AT+CPBR="), index);

    return _modem->readResponse<char, char>(_cpbrParser, numberBuffer, contactNameBuffer) == ATResponse::ResponseOK;
}


bool CellModemPhonebook::findContact(const char * toFindStr, char * numberBuffer, char * contactNameBuffer, uint8_t * numberType) {

}


bool CellModemPhonebook::removeContact(const char * contactName) {

}

bool CellModemPhonebook::removeContact(uint8_t index) {

}

ATResponse CellModemPhonebook::_cpbrParser(ATResponse &response, const char * buffer, size_t size, char * numberBuffer, char * contactNameBuffer) {
    if(!numberBuffer || !contactNameBuffer) {
        return ATResponse::ResponseError;
    }

    if(sscanf_P(buffer, PSTR("+CPBR: %*d,\"%[^\"]\",%*d,\"%[^\"]"), numberBuffer, contactNameBuffer) == 2) {
        Serial.println("Aqui");
        return ATResponse::ResponseEmpty;
    }

    return ATResponse::ResponseError;
}