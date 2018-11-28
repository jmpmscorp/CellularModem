#include "CellModemSMS.h"

CellModemSMS::CellModemSMS(CellModem &modem) :
    _modem(&modem) 
{
    
}

bool CellModemSMS::send(char * telNumber, const char * text) {

}

bool CellModemSMS::read(uint16_t index, char * phoneNumber, char * textBuffer) {
    _modem->sendATCommand(F("AT+CMGR="), index);

    return _modem->readResponse<char, char>(_cmgrParser, phoneNumber, textBuffer) == ATResponse::ResponseOK;
}

ATResponse CellModemSMS::_cmgrParser(ATResponse& response, const char* buffer, size_t size, char* phoneNumber, char* textBuffer) {
    if(!phoneNumber || !textBuffer) {
        return ATResponse::ResponseError;
    }

    if (sscanf_P(buffer, PSTR("+CMGR: \"%*[^\"]\",\"%[^\"]"), phoneNumber) == 1) {
        return ATResponse::ResponseEmpty;
    }
    else if ((buffer[size - 2] == '\r') && (buffer[size - 1] == '\n')) {
        for(size_t i = 0; i = size - 2; ++i) {
            if(buffer[i] && textBuffer[i]) {
                textBuffer[i] = buffer[i];
            }
            else {
                textBuffer[i - 1] = '\0';
                return ATResponse::ResponseEmpty;
            }
        }

        textBuffer[size - 2] = '\0';

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
            Serial.println("Aqui");
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

ATResponse CellModemSMS::_cmglParser(ATResponse &response, const char * buffer, size_t size, unsigned int * indexList, int * indexListSize) {
    unsigned int index;

    if(sscanf_P(buffer, PSTR("+CMGL: %u,"), &index) == 1) {
        if(*indexListSize > 0) {
            indexList[*indexListSize - 1] = index;
            //++indexList;
        }
        --(*indexListSize);
        
        //return ATResponse::ResponseEmpty;
    }

    return ATResponse::ResponseContinuosParser;
    //return ATResponse::ResponseError;
}


bool CellModemSMS::remove(unsigned int index) {
    _modem->sendATCommand(F("AT+CMGD="), index);

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

ATResponse CellModemSMS::_handleUrcs() {
    if(_mt > 0) {
        if(_mt == 1) {
            char memory[3] = "";
            int index = 0;
            if(sscanf_P(_modem->getResponseBuffer(), PSTR("+CMTI: \"%[^\"]\",%d"), memory, &index) == 2) {
                if(_cmtiCallback) {
                    _cmtiCallback(memory, static_cast<int16_t>(index));
                }
            }
        }
        else if (_mt == 2) {
            char phoneNumber[21] = "";
            if(sscanf_P(_modem->getResponseBuffer(), PSTR("+CMT: \"%[^\"]\",,\"%*[^\"]\""), phoneNumber) == 1) {
                _modem->readLine();
                if(_cmtCallback) {
                    _cmtCallback(phoneNumber, _modem->getResponseBuffer());
                }                
            }
        }    
    }
    
}





