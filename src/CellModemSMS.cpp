#include "CellModemSMS.h"

#define CTRL_Z '\x1A'

CellModemSMS::CellModemSMS(CellModem &modem) :
    _modem(&modem) 
{
    _modem->addUrcHandler(this);
}

bool CellModemSMS::send(char * phoneNumber, const char * text) {
    _modem->sendATCommand(F("AT+CMGS=\""), phoneNumber, F("\""));

    if(_modem->readResponse() == ATResponse::ResponsePrompt) {
        _modem->sendData(text, CTRL_Z);

        ATResponse response;
        uint32_t start = millis();

        do{
            response = _modem->readResponse();
            _modem->getCustomDelay()(10);
        }while(response == ATResponse::ResponseTimeout && !isTimedout(start, 60000));

        if(response == ATResponse::ResponseOK) {
            return true;
        }
        else {
            return false;
        }
    }

    return false;
}

bool CellModemSMS::read(uint16_t index, char * phoneNumber, size_t phoneNumberSize,
     char * textBuffer, size_t textBufferSize) 
{
    _modem->sendATCommand(F("AT+CMGR="), index);

    SafeCharBufferPtr safePhoneNumber = { phoneNumber, phoneNumberSize};
    SafeCharBufferPtr safeTextBuffer = { textBuffer, textBufferSize };

    return _modem->readResponse<SafeCharBufferPtr, SafeCharBufferPtr>(_cmgrParser, &safePhoneNumber, &safeTextBuffer) == ATResponse::ResponseOK;
}

ATResponse CellModemSMS::_cmgrParser(ATResponse& response, const char* buffer, size_t size, SafeCharBufferPtr * safePhoneNumber, SafeCharBufferPtr * safeTextBuffer) {
    if(!safePhoneNumber || !safeTextBuffer) {
        return ATResponse::ResponseError;
    }

    if (sscanf_P(buffer, PSTR("+CMGR: \"%*[^\"]\",\"%[^\"]"), safePhoneNumber->bufferPtr) == 1) {
        response = ATResponse::ResponseMultilineParser;
        return ATResponse::ResponseMultilineParser;
    }
    //else if ((buffer[size - 2] == '\r') && (buffer[size - 1] == '\n')) {
    else if (response == ATResponse::ResponseMultilineParser) {
        uint16_t len = strlen(buffer);

        strncpy(safeTextBuffer->bufferPtr, buffer, 
            len >= safeTextBuffer->size ? safeTextBuffer->size - 1 : len);
        
        if(strlen(buffer) >= safeTextBuffer->size) {
            safeTextBuffer->bufferPtr[safeTextBuffer->size -1] = '\0';
        }
        else {
            safeTextBuffer->bufferPtr[len] = '\0';
        }
        
        return ATResponse::ResponseEmpty;
    }

    return ATResponse::ResponseError;
}

int CellModemSMS::readList(const char * filter, unsigned int * indexList, size_t indexListSize, unsigned int * remainingSize) {
    _modem->sendATCommand(F("AT+CMGL=\""), filter, F("\""));

    if(!indexList) {
        return -1;
    }

    int auxSize = indexListSize;

    if(_modem->readResponse<unsigned int, int>(_cmglParser, indexList, &auxSize) == ATResponse::ResponseOK) {        
        if(remainingSize) {
            if(auxSize < 0) {
                *remainingSize = auxSize * (-1);
            }
            else {
                *remainingSize = 0;
            }
        }

        if(auxSize < static_cast<int>(indexListSize)) {
            uint16_t i = 0, j = indexListSize - 1;

            while(i < j) {
                unsigned int temp = indexList[j];
                indexList[j] = indexList[i];
                indexList[i] = temp;
                ++i; --j; 
            }
        }

        return auxSize > 0 ? indexListSize - auxSize : indexListSize;
    }

    return -1;
}

/**
 * Parser to CMGL command.
 * Indexes returned from +CMGL response are stored in the indexList buffer
 * If there are more SMSs stored than the indexList size, indexes are not stored
 * but they are advices in remainingSize, where it indicates the non-stored number of indexes.
 * 
 * indexes are stored in reverse direction, so they should be reversed to read as normal.
 * This reversion is done in readList function
 * */
ATResponse CellModemSMS::_cmglParser(ATResponse &response, const char * buffer, size_t size, unsigned int * indexList, int * indexListSize) {
    unsigned int index;

    if(sscanf_P(buffer, PSTR("+CMGL: %u,"), &index) == 1) {
        if(*indexListSize > 0) {
            indexList[*indexListSize - 1] = index;
        }
        --(*indexListSize);
    }

    return ATResponse::ResponseMultilineParser;
    //return ATResponse::ResponseError;
}


bool CellModemSMS::remove(unsigned int index, uint8_t flag) {
    if( flag < 0 || flag > 4) {
        return false;
    }

    if( flag == 0) {
        _modem->sendATCommand(F("AT+CMGD="), index);
    }
    else {
        _modem->sendATCommand(F("AT+CMGD="), index, ",", flag);
    }

    return _modem->readResponse() == ATResponse::ResponseOK;
}

bool CellModemSMS::setNewSMSIndicator(uint8_t mode, uint8_t mt) {
    _modem->sendATCommand(F("AT+CNMI="), mode, ',', mt);

    if(_modem->readResponse() == ATResponse::ResponseOK) {
        _mt = mt;
        return true;
    }

    return false;
}

int8_t CellModemSMS::getMode() {
    _modem->sendATCommand(F("AT+CMGF?"));
    
    uint8_t mode;
    if(_modem->readResponse<uint8_t, uint8_t>(_cmgfParser, &mode, NULL) != ATResponse::ResponseOK) {
        return -1;
    }

    return static_cast<int8_t>(mode);
}

bool CellModemSMS::setTextMode() {
    return _setMode(1);
}

bool CellModemSMS::setPDUMode() {
    return _setMode(0);
}

bool CellModemSMS::_setMode(uint8_t mode) {
    _modem->sendATCommand(F("AT+CMGF="), mode);

    if(_modem->readResponse() == ATResponse::ResponseOK) {
        return true;
    }

    return false;
}

ATResponse CellModemSMS::_cmgfParser(ATResponse &response, const char * buffer, size_t size, uint8_t * mode, uint8_t * dummy) {
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


void CellModemSMS::setCMTCallback(CMTCallback cmtCallback) { 
    _cmtCallback = cmtCallback; 
}

void CellModemSMS::setCMTICallback(CMTICallback cmtiCallback) { 
    _cmtiCallback = cmtiCallback; 
}

ATResponse CellModemSMS::handleUrcs() {
    if(_mt > 0) {
        if(_mt == 1) {
            char memory[3] = "";
            int index = 0;
            if(sscanf_P(_modem->getResponseBuffer(), PSTR("+CMTI: \"%[^\"]\",%d"), memory, &index) == 2) {
                if(_cmtiCallback) {
                    _cmtiCallback(memory, static_cast<int16_t>(index));
                }
            }
            
            return ATResponse::UrcHandled;
        }
        else if (_mt == 2) {
            char phoneNumber[21] = "";
            if(sscanf_P(_modem->getResponseBuffer(), PSTR("+CMT: \"%[^\"]\",,\"%*[^\"]\""), phoneNumber) == 1) {
                _modem->readLine();
                if(_cmtCallback) {
                    _cmtCallback(phoneNumber, _modem->getResponseBuffer());
                }                
            }

            return ATResponse::UrcHandled;
        }    
    }
    
    return ATResponse::ResponseEmpty;
}





