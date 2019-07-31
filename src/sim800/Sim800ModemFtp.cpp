#include "Sim800ModemFtp.h"

Sim800ModemFtp::Sim800ModemFtp(Sim800Modem &modem) :
    CellModemFtp(modem)
{
    _modem->addUrcHandler(this);
}

bool Sim800ModemFtp::init(const char * server, const char * username, const char * password) {
    if(server == nullptr || strlen(server) <= 0) return false;

    _modem->sendATCommand(F("AT+FTPCID="), DEFAULT_BEARER_ID);

    if(_modem->readResponse() != ATResponse::ResponseOK) {
        return false;
    }

    _modem->sendATCommand(F("AT+FTPSERV=\""), server, '"');

    if(_modem->readResponse() != ATResponse::ResponseOK) {
        return false;
    }

    if(username != nullptr && strlen(username) > 0) {
        _modem->sendATCommand(F("AT+FTPUN=\""), username, '"');

        if(_modem->readResponse() != ATResponse::ResponseOK) {
            return false;
        }
    }

    if(password != nullptr && strlen(password) > 0) {
        _modem->sendATCommand(F("AT+FTPPW=\""), password, '"');

        if(_modem->readResponse() != ATResponse::ResponseOK) {
            return false;
        }
    }

    return true;
}

bool Sim800ModemFtp::_initFtpTransaction(const char * path, const char * filename) {
    if(filename == nullptr || strlen(filename) <= 0) return false;

    _modem->sendATCommand(F("AT+FTPPUTNAME=\""), filename, '"');

    if(_modem->readResponse() != ATResponse::ResponseOK) {
        return false;
    }

    if(path != nullptr && strlen(path) > 0) {
        _modem->sendATCommand(F("AT+FTPPUTPATH=\""), path, '"');

        if(_modem->readResponse() != ATResponse::ResponseOK) {
            return false;
        }
    }

    _modem->sendATCommand(F("AT+FTPPUT=1"));
    
    if(_modem->readResponse() != ATResponse::ResponseOK) {
        return false;
    }

    return _waitFtpPutUrc();
}

bool Sim800ModemFtp::_endFtpTransaction() {
    _modem->sendATCommand(F("AT+FTPPUT=2,0"));

    if(_modem->readResponse() != ATResponse::ResponseOK) return false;

    return _waitFtpPutUrc();
}

bool Sim800ModemFtp::send(const char * path, const char * filename, const uint8_t * buffer, size_t size) {
    if(!_initFtpTransaction(path, filename)) return false;

    size_t bytesSent = 0;
    bool success = true;

    while(success && bytesSent < size) {
        size_t remainingBytes = size - bytesSent;

        success = _sendChunk(buffer + bytesSent, _transactionLength > remainingBytes ? remainingBytes : _transactionLength, &bytesSent);
    }

    return _endFtpTransaction();
}

bool Sim800ModemFtp::send(const char * path, const char * filename, Stream &stream, size_t size) {
    if(!_initFtpTransaction(path, filename)) return false;

    size_t bytesSent = 0;
    bool success = true;
    Serial.println(size);
    while(success && bytesSent < size) {
        size_t remainingBytes = size - bytesSent;

        success = _sendChunk(stream, _transactionLength > remainingBytes ? remainingBytes : _transactionLength, &bytesSent);
    }

    return _endFtpTransaction();
}

bool Sim800ModemFtp::_sendChunk(const uint8_t * buffer, size_t sizeToSend, size_t * bytesSent) {
    _modem->sendATCommand(F("AT+FTPPUT=2,"), sizeToSend);

    if(!_waitFtpPutUrc()) return false;
    if(_transactionLength != sizeToSend) return false;

    _modem->getSerial()->write(buffer, sizeToSend);

    if(_modem->readResponse() != ATResponse::ResponseOK) return false;

    *bytesSent += sizeToSend;

    return _waitFtpPutUrc();
}

bool Sim800ModemFtp::_sendChunk(Stream &stream, size_t sizeToSend, size_t * bytesSent) {
    _modem->sendATCommand(F("AT+FTPPUT=2,"), sizeToSend);

    if(!_waitFtpPutUrc()) return false;
    if(_transactionLength != sizeToSend) return false;

    for(size_t i = 0; i < sizeToSend; ++i) {
        _modem->getSerial()->write(stream.read());
    }    

    if(_modem->readResponse() != ATResponse::ResponseOK) return false;

    *bytesSent += sizeToSend;

    return _waitFtpPutUrc();
}

bool Sim800ModemFtp::_waitFtpPutUrc(uint16_t timeout) {
    _ftpPutUrcReceived = false;
    unsigned long start = millis();

    while(!_ftpPutUrcReceived && !isTimedout(start, timeout)) {
        _modem->poll();        
    }

    return _ftpPutUrcReceived;
}


uint8_t Sim800ModemFtp::getLastTransactionError() const {
    return _transactionError;
}

ATResponse Sim800ModemFtp::handleUrcs() {
    // Posible responses are:
    // +FTPPUT: 1, <error>
    // +FTPPUT: 1,1,<maxlenght>
    // +FTPPUT: 2, <reqlength>
    char * ptr = _modem->getResponseBuffer();
        
    if(strncmp(ptr, "+FTPPUT:", 8) == 0) {
        _ftpPutUrcReceived = true;
        ptr = strchr(ptr, ':');

        ++ptr;  //Skip ':'

        uint8_t mode = strtoul(ptr, &ptr, 10);
        
        if(mode < 1 || mode > 2) return ATResponse::ResponseError;

        if(*ptr == ',') ++ptr;

        uint16_t aux = strtoul(ptr, &ptr, 10);

        if(mode == 1) {
            _transactionError = static_cast<uint8_t>(aux);
            
            if(aux != 1) {
                return ATResponse::ResponseError;
            }

            if(*ptr == ',') ++ptr;
            _transactionLength = strtoul(ptr, &ptr, 10);

        } else if(mode == 2) {
            _transactionLength = aux;
        }

        

        return ATResponse::UrcHandled;
    }

}