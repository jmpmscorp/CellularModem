#include "ATCellularModem.h"

ATCellularModem::ATCellularModem(Uart &serial, int8_t dtrPin, int8_t resetPin) :
    _serial(&serial),
    _dtrPin(dtrPin),
    _resetPin(resetPin)
    
{
    memset(_responseBuffer, '\0', sizeof(_responseBuffer));
}

void ATCellularModem::beginModem(uint32_t baudrate) {
    /*_serial.begin(baudrate);

    if(_dtrPin > -1) {
        pinMode(_dtrPin, OUTPUT);
    }

    if(_resetPin > -1) {
        pinMode(_resetPin, OUTPUT);
    }*/
}

bool ATCellularModem::isAlive(uint16_t timeout) {
    unsigned long start = millis();

    while(millis() - start < timeout) {
        sendATCommand(F("AT"));

        if(waitForResponse() == 1) {
            return true;
        }

        _delay(100);
    }

    return false;
}

int8_t ATCellularModem::ready() {
    poll();

    return _ready;
}

void ATCellularModem::poll() {
    while(_serial->available() > 0) {
        char c = _serial->read();
        
        if(c <= 0) {
            continue;
        }
        //_responseBuffer[_pos - 1] = (char) _serial->read();
        
        _responseBuffer[_bytePos] = c;

        //Serial.print(c);
        
        
        if(_bytePos > 63 ) _bytePos = 0;
        
        for(size_t i = 0; i < strlen(_responseBuffer); i++) {
            Serial.print(_responseBuffer[i], HEX);
            Serial.print('-');
        }
        Serial.println();

        switch(_atCommandState) {
            case AT_COMMAND_IDLE:
            default: {
                if(c_str_startWith_P(_responseBuffer, PSTR("AT")) && c_str_endsWith_P(_responseBuffer, CRLF)) {
                    Serial.println("Received Response");
                    _atCommandState = AT_RECEIVING_RESPONSE;                    
                    _clearResponseBuffer();
                }
                else if ( c_str_endsWith_P(_responseBuffer, CRLF)) {
                    Serial.println("CRLF");
                    c_str_trim(_responseBuffer);

                    if(strlen(_responseBuffer) > 0) {
                        _lastResponseOrURCMillis = millis();
                    }

                    for(size_t i = 0; i < MAX_URC_HANDLERS; i++) {
                        if(_urcHandlers[i] != nullptr) {
                            _urcHandlers[i]->handleURC(_responseBuffer);
                        } 
                    }
                    _clearResponseBuffer();
                }

                break;
            }

            case AT_RECEIVING_RESPONSE: {
                if( c == '\n' ) {
                    _lastResponseOrURCMillis = millis();

                    if(strstr_P(_responseBuffer, OK_RESPONSE) != nullptr) {
                        Serial.println("Received OK");
                        _ready = 1;
                    }
                    else if(strstr_P(_responseBuffer, ERROR_RESPONSE) != nullptr) {
                        Serial.println("Received ERROR RESPONSE");
                        _ready = 2;
                    }
                    else if(strstr_P(_responseBuffer, PSTR("NO CARRIER\r\n")) != nullptr) {
                        Serial.println("Received NO CARRIER");
                        _ready = 3;
                    }

                    if(_ready != 0) {
                        _atCommandState = AT_COMMAND_IDLE;
                        _clearResponseBuffer();
                    }

                    return;
                }

                break;
            }
        }

        ++_bytePos;
    }
}

int8_t ATCellularModem::waitForResponse(uint16_t timeout) {
    unsigned long start = millis();

    while(millis() - start < timeout) {
        int r = ready();

        if( r != 0 ) {
            return r;
        }
    }

    _clearResponseBuffer();
}

int8_t ATCellularModem::waitForPrompt(uint16_t timeout) {
    unsigned long start = millis();

    while(millis() - start < timeout) {
        ready();

        if(c_str_endsWith_P(_responseBuffer, ">")) {
            return 1;
        }
    }

    return -1;
}

void ATCellularModem::_clearResponseBuffer() {
    Serial.println("Clear Buffer");
    memset(_responseBuffer, '\0', sizeof(_responseBuffer));
    _bytePos = 0;
}