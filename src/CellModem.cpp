#include "CellModem.h"

CellModem::CellModem(Stream &serial, int8_t onOffPin, int8_t statusPin, int8_t dtrPin, int8_t ctsPin) :
    _serial(&serial),
    _onOffPin(onOffPin),
    _statusPin(statusPin),
    _dtrPin(dtrPin),
    _ctsPin(ctsPin)    
{
}

void CellModem::init() {
    if(_onOffPin > -1) {
        pinMode(_onOffPin, OUTPUT);
    }

    if(_statusPin > -1) {
        pinMode(_statusPin, INPUT);
    }

    if(_dtrPin > -1) {
        pinMode(_dtrPin, OUTPUT);
    }

    if(_ctsPin > -1) {
        pinMode(_ctsPin, INPUT);
    }

    _initResponseBuffer();
}

void CellModem::_initResponseBuffer() {
    if(!_isResponseBufferInitialized) {
        _responseBuffer = static_cast<char *>(malloc(_responseBufferSize));

        _isResponseBufferInitialized = true;
    }
}

bool CellModem::on() {

    if(isAlive(200)) {
        return true;
    }

    if(!isOn()) {
        if(_onOffPin > -1) {
            digitalWrite(_onOffPin, HIGH);
            _modemDelay(100);
            digitalWrite(_onOffPin, LOW);
        }

        if(_dtrPin > -1) {
            digitalWrite(_dtrPin, LOW);
            _modemDelay(100);
        }

        _onOffStatus = true;
    }
    
    bool timeout = true;
    Serial.println("On");
    for (uint8_t i = 0; i < 10; i++) {
        if (isAlive(500)) {
            timeout = false;
            break;
        }
    }

    if (timeout) {
        return false;
    }

    return isOn();

}

bool CellModem::isOn() const {
    if(_statusPin > -1) {
        return static_cast<bool>(digitalRead(_statusPin));
    } 
    else {
        return _onOffStatus;
    }
}

bool CellModem::isAlive(uint16_t timeout) {
    unsigned long start = millis();
    
    sendATCommand(F("AT"));
        
    return (readResponse(NULL, 500) == ATResponse::ResponseOK);
}

void CellModem::setMinRSSI(int8_t minRSSI) {
    _minRSSI = minRSSI;
}

char * CellModem::getResponseBuffer() const {
    return _responseBuffer;
}

void CellModem::addUrcHandler(CellModemUrcHandler * urcHandler) {
    uint8_t i = 0;

    while( i < CELLMODEM_MAX_URC_HANDLERS) {
        if(_urcHandlers[i] == nullptr) {
            _urcHandlers[i] = urcHandler;
            return;
        }
        ++i;
    }
}

void CellModem::removeUrcHandler(CellModemUrcHandler * urcHandler) {
    uint8_t i = 0;

    while( i < CELLMODEM_MAX_URC_HANDLERS) {
        if(_urcHandlers[i] == urcHandler) {
            _urcHandlers[i] = nullptr;
            return;
        }
        ++i;
    }
}

ATResponse CellModem::poll(uint32_t timeout) {
    readResponse(NULL, timeout);
}

ATResponse CellModem::readResponse(char* buffer, size_t size,
        ResponseParserCallbackPtr parserCallback, void* callbackParam1, void* callbackParam2,
        size_t* outSize, uint32_t timeout)
{
    ATResponse response = ATResponse::ResponseNotFound;
    uint32_t start = millis();

    do {
        int count = readLine(buffer, size, 500);

        if ( count < 0) {
            continue;
        }

        if ( outSize ) {
            *outSize = count;
        }

        for (size_t i = 0; i < CELLMODEM_MAX_URC_HANDLERS; ++i) {
            if( _urcHandlers[i] != nullptr ) {
                _urcHandlers[i] -> handleUrcs();
            }
        }

        if(c_str_startWith_P(_responseBuffer, AT_STR)) {
            continue;
        }

        if(c_str_startWith_P(_responseBuffer, OK_RESPONSE)) {
            return ATResponse::ResponseOK;
        }

        if(c_str_startWith_P(_responseBuffer, ERROR_RESPONSE) ||
            c_str_startWith_P(_responseBuffer, CME_ERROR_RESPONSE) ||
            c_str_startWith_P(_responseBuffer, CME_ERROR_RESPONSE))
        {
            return ATResponse::ResponseError;
        }

        if(parserCallback) {
            ATResponse callbackResponse = parserCallback(response, buffer, count, callbackParam1, callbackParam2);
            if(callbackResponse != ATResponse::ResponseEmpty && callbackResponse != ATResponse::ResponseContinuosParser ) {
                return callbackResponse;
            }

            if(callbackResponse != ATResponse::ResponseContinuosParser) {
                parserCallback = nullptr;
            }
            
        }

        if(response != ATResponse::ResponseNotFound) {
            return response;
        }

        delay(10);
    } while (!isTimedout(start, timeout));

    if(outSize) {
        *outSize = 0;
    }

    return ATResponse::ResponseTimeout;
}


// Returns a character from the modem stream if read within _timeout ms or -1 otherwise.
int CellModem::readByte(uint32_t timeout) const
{
    int c;
    uint32_t _startMillis = millis();

    do {
        c = _serial->read();
        if (c >= 0) {
            Serial.print((char)c);
            return c;
        }
    } while (millis() - _startMillis < timeout);

    return -1; // -1 indicates timeout
}

// Fills the given "buffer" with characters read from the modem stream up to "length"
// maximum characters and until the "terminator" character is found or a character read
// times out (whichever happens first).
// The buffer does not contain the "terminator" character or a null terminator explicitly.
// Returns the number of characters written to the buffer, not including null terminator.
size_t CellModem::readBytesUntil(char terminator, char* buffer, size_t length, uint32_t timeout) {
    if (length < 1) {
        return 0;
    }

    size_t index = 0;
    
    while (index < length) {
        int c = readByte(timeout);

        
        if (c < 0 || c == terminator) {
            break;
        }
        
        *buffer++ = static_cast<char>(c);        
        index++;
    }

    if (index < length) {
        *buffer = '\0';
    }

    // TODO distinguise timeout from empty string?
    // TODO return error for overflow?
    return index; // return number of characters, not including null terminator
}

// Fills the given "buffer" with up to "length" characters read from the modem stream.
// It stops when a character read times out or "length" characters have been read.
// Returns the number of characters written to the buffer.
size_t CellModem::readBytes(uint8_t* buffer, size_t length, uint32_t timeout) {
    size_t count = 0;

    while (count < length) {
        int c = readByte(timeout);

        if (c < 0) {
            break;
        }
        
        *buffer++ = static_cast<uint8_t>(c);
        count++;
    }

    // TODO distinguise timeout from empty string?
    // TODO return error for overflow?
    return count;
}

// Reads a line (up to the SODAQ_GSM_TERMINATOR) from the modem stream into the "buffer".
// The buffer is terminated with null.
// Returns the number of bytes read, not including the null terminator.
size_t CellModem::readLine(char* buffer, size_t size, uint32_t timeout) {
    // Use size-1 to leave room for a string terminator

    size_t len = readBytesUntil(CELLMODEM_TERMINATOR[CELLMODEM_TERMINATOR_LENGTH - 1], buffer, size - 1, timeout);
    
    
    // check if the terminator is more than 1 characters, then check if the first character of it exists 
    // in the calculated position and terminate the string there
    if ((CELLMODEM_TERMINATOR_LENGTH > 1) && (buffer[len - (CELLMODEM_TERMINATOR_LENGTH - 1)] == CELLMODEM_TERMINATOR[0])) {
        len -= CELLMODEM_TERMINATOR_LENGTH - 1;
    }

    // terminate string, there should always be room for it (see size-1 above)
    buffer[len] = '\0';

    return len;
}

size_t CellModem::readLine() {
    readLine(_responseBuffer, _responseBufferSize);
}

size_t CellModem::readLine(uint32_t timeout) {
    readLine(_responseBuffer, _responseBufferSize, timeout);
}