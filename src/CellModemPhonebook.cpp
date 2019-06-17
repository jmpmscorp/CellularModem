#include "CellModemPhonebook.h"

CellModemPhonebook::CellModemPhonebook(CellModem &modem) :
    _modem(&modem)
    {

    }

bool CellModemPhonebook::addContact(const char * phoneNumber, const char * contactName, uint8_t index) {
    if(!phoneNumber) {
        return false;
    }

    char indexBuffer[4] = "";

    if(index > 0) {
        itoa(index, indexBuffer, 10);
    }

    
    _modem->sendATCommand(F("AT+CPBW="), indexBuffer,",\"", 
        phoneNumber,"\",,\"", contactName != nullptr ? contactName : phoneNumber, '"');

    

    if(_modem->readResponse() == ATResponse::ResponseOK) {
        return true;
    }

    return false;
}


bool CellModemPhonebook::readContact(uint8_t index, char * numberBuffer, char * contactNameBuffer) {
    if(index <=0 ) {
        return false;
    }

    _modem->sendATCommand(F("AT+CPBR="), index);

    return _modem->readResponse<char, char>(_cpbrParser, numberBuffer, contactNameBuffer) == ATResponse::ResponseOK;
}

ATResponse CellModemPhonebook::_cpbrParser(ATResponse &response, const char * buffer, size_t size, char * numberBuffer, char * contactNameBuffer) {
    if(!numberBuffer || !contactNameBuffer) {
        return ATResponse::ResponseError;
    }

    if(sscanf_P(buffer, PSTR("+CPBR: %*d,\"%[^\"]\",%*d,\"%[^\"]"), numberBuffer, contactNameBuffer) == 2) {
        return ATResponse::ResponseEmpty;
    }

    return ATResponse::ResponseError;
}


int CellModemPhonebook::searchContacts(const char * toFindStr, unsigned int * indexesBuffer, size_t indexesBufferSize, unsigned int * totalOccurences) {
    if(indexesBuffer == nullptr) {
        return 0;
    }

    _modem->sendATCommand(F("AT+CPBF=\""), toFindStr, "\"");

    int freeBufferSize = indexesBufferSize;

    if(_modem->readResponse<unsigned int, int>(_cpbfParser, indexesBuffer, &freeBufferSize) == ATResponse::ResponseOK) {
        if(totalOccurences) {
            *totalOccurences = indexesBufferSize - freeBufferSize;
        }
        
        if(freeBufferSize < static_cast<int>(indexesBufferSize)) {
            uint16_t i = 0, j = indexesBufferSize - 1;

            while(i < j) {
                unsigned int temp = indexesBuffer[j];
                indexesBuffer[j] = indexesBuffer[i];
                indexesBuffer[i] = temp;
                ++i; --j; 
            }
        }

        return freeBufferSize > 0 ? indexesBufferSize - freeBufferSize : indexesBufferSize;
    }

    return 0;
}


ATResponse CellModemPhonebook::_cpbfParser(ATResponse &response, const char * buffer, size_t size, unsigned int * indexesBuffer, int * freeBufferSize) {
    // +CPBF: [index],"number",[type],"text"
    unsigned int aux;

    if(sscanf_P(buffer, PSTR("+CPBF: %d,"), &aux) == 1) {
        if(*freeBufferSize > 0) {
            *(indexesBuffer + *freeBufferSize - 1) = aux;
        }

        --(*freeBufferSize);
    }

    return ATResponse::ResponseMultilineParser;
}

bool CellModemPhonebook::removeContact(uint8_t index) {
    if(index <= 0) {
        return false;
    }

    _modem->sendATCommand(F("AT+CPBW= "), index);

    if(_modem->readResponse() == ATResponse::ResponseOK) {
        return true;
    }

    return false;
}
