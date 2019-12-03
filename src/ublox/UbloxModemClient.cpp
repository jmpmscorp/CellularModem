#include "UbloxModemClient.h"

UbloxModemClient::UbloxModemClient(UbloxModem& modem, bool ssl):
    CellModemClient(modem, ssl)
{
    _modem->addUrcHandler(this);
}

UbloxModemClient::~UbloxModemClient() {}

bool UbloxModemClient::setTCPMode(uint8_t mode) {
    _modem->sendATCommand(F("AT+UDCONF=1,"), mode);

    if(_modem->readResponse() != ATResponse::ResponseOK) {
        return false;
    }

    return true;
}

int UbloxModemClient::socketConnect(const char * host, uint16_t port, uint8_t * socketId, bool ssl, unsigned int timeout) {
    _modem->sendATCommand(F("AT+USOCR=6"));

    if(_modem->readResponse<uint8_t, uint8_t>(_usocrParser, socketId, nullptr, nullptr, (uint32_t)timeout) != ATResponse::ResponseOK) {
        return 0;
    }

    _modem->sendATCommand(F("AT+USOCO="), *socketId, ",\"", host, "\",", port);

    if(_modem->readResponse(nullptr, 60000) != ATResponse::ResponseOK ) {
        return 0;
    }

    return 1;
}

ATResponse UbloxModemClient::_usocrParser(ATResponse &response, const char * buffer, size_t size, uint8_t * socketId, uint8_t * dummy) {
    if(sscanf_P(buffer, PSTR("+USOCR: %d"), &socketId) == 1) {
        return ATResponse::ResponseEmpty;
    }

    return ATResponse::ResponseError;
}

int UbloxModemClient::socketAvailable(uint8_t socketId) {
    _modem->sendATCommand(F("AT+USORD="), socketId, ",0");
    
    int len;
    if(_modem->readResponse<int, uint8_t>(_usordParser, &len, nullptr) == ATResponse::ResponseOK) {
        return len;
    }

    //_socketConnected = isSocketConnected(socketId);

    return 0;
}

ATResponse UbloxModemClient::_usordParser(ATResponse &response, const char * buffer, size_t size, int * len, uint8_t * dummy) {
    if(!len) {
        return ATResponse::ResponseError;
    }


    if(sscanf_P(buffer, PSTR("+USORD: %*d,%d"), len) == 1) {
        return ATResponse::ResponseEmpty;
    }

    return ATResponse::ResponseError;
}

int UbloxModemClient::socketRead(size_t len, uint8_t socketId) {
    Serial.println("Socket Read!");
    _modem->sendATCommand(F("AT+USORD="), socketId, ",", len);

    size_t auxLen;

    if(_modem->readResponse<SocketBuffer, size_t>(_usordParser, &_socketBuffer, &auxLen, nullptr, 25000) != ATResponse::ResponseOK) {
        auxLen = 0;
    }

    _socketAvailable = socketAvailable(socketId);

    if(_socketAvailable > 0) {
        socketRead(_socketAvailable, _socketId);
    }

    return auxLen;
}

ATResponse UbloxModemClient::_usordParser(ATResponse &response, const char * buffer, size_t size, SocketBuffer * socketBuffer, size_t * len) {
    if(!socketBuffer || !len) {
        return ATResponse::ResponseError;
    }

    int socketId, count;

    if(sscanf_P(buffer, PSTR("+USORD: %d,%d,"), &socketId, &count) == 2) {
        char * ptr = strchr(buffer, '"');
        Serial.println(count);
        if(ptr) {
            ++ptr;
            int i = 0;

            while(ptr && *ptr != '"' && i < count && socketBuffer->count() < socketBuffer->size()) {
                socketBuffer->write((uint8_t)*ptr);
                ++i;
                ++ptr;
            }
        }

        return ATResponse::ResponseEmpty;
    }

    return ATResponse::ResponseMultilineParser;
}

int UbloxModemClient::socketWrite(const void* buf, size_t len, uint8_t socketId) {
    _modem->sendATCommand(F("AT+USOWR="), socketId, ',', len);

    if(_modem->readResponse() == ATResponse::ResponsePrompt) {
        delayFnPtr customDelay = _modem->getCustomDelay();
        customDelay(50);

        Stream * serial = _modem->getSerial();

        serial->write((uint8_t *)buf, len);
        //serial->flush();

        int sent;
       
        if(_modem->readResponse<int, uint8_t>(_usowrParser, &sent, nullptr, nullptr, 30000) == ATResponse::ResponseOK) {
            return sent;
        }
    }   

    return 0;
}

ATResponse UbloxModemClient::_usowrParser(ATResponse &response, const char * buffer, size_t size, int * sent, uint8_t * dummy) {
    if(!sent) return ATResponse::ResponseError;
    
    if(sscanf_P(buffer, PSTR("+USOWR: %*d,%d"), sent) == 1) {
        return ATResponse::ResponseEmpty;
    }

    return ATResponse::ResponseNotFound;
}

bool UbloxModemClient::isSocketConnected(uint8_t socketId) {
    _modem->sendATCommand(F("AT+USOCTL="), socketId, ",10");

    uint8_t status;
    if(_modem->readResponse<uint8_t, uint8_t>(_usoctlParser, &status, nullptr) == ATResponse::ResponseOK) {
        if(status > 0) {
            return true;
        } else {
            return false;
        }
    }

    return false;
}

ATResponse UbloxModemClient::_usoctlParser(ATResponse &response, const char * buffer, size_t size, uint8_t * status, uint8_t * dummy) {
    if(!status) return ATResponse::ResponseError;

    if(sscanf_P(buffer, PSTR("+USOCTL: %*d,%*d,%d"), &status) == 1) {
        return ATResponse::ResponseEmpty;
    }

    return ATResponse::ResponseError;
}

bool UbloxModemClient::socketDisconnect(uint8_t socketId) {
    _modem->sendATCommand(F("AT+USOCL="), socketId);

    if(_modem->readResponse() == ATResponse::ResponseOK) {
        return true;
    }

    return false;
}


ATResponse UbloxModemClient::handleUrcs() {
    int param1, param2;
    if(sscanf_P(_modem->getResponseBuffer(), PSTR("+UUSORD: %d,%d"), &param1, &param2) == 2) {
        Serial.println("Aqui URC");
        if(_socketId == param1) {
            Serial.println("Reading!");
            //_socketAvailable += param2;
            socketRead(param2, param1);
            return ATResponse::UrcHandled;
        }
    }

    return ATResponse::ResponseEmpty;
}