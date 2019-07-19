#include "UbloxModemHttp.h"

#define DEFAULT_HTTP_PROFILE    0
#define WRITE_TEMP_FILE         "writeTemp.ffs"
#define READ_TEMP_FILE          "readTemp.ffs"

UbloxModemHttp::UbloxModemHttp(UbloxModem &modem) :
    CellModemHttp(modem)
{
    _modem->addUrcHandler(this);
}

UbloxModemHttp::UbloxModemHttp(UbloxModem &modem, UbloxModemFilesystem &filesystem) :
    CellModemHttp(modem),
    _filesystem(&filesystem)
{
    _modem->addUrcHandler(this);
}

bool UbloxModemHttp::init(const char * server, const uint16_t port) {
    return _init(server, port, false);
}

bool UbloxModemHttp::initSSL(const char * server, const uint16_t port) {
    return _init(server, port, true);
}

bool UbloxModemHttp::_init(const char * server, const uint16_t port, const bool ssl) {
    _modem->sendATCommand(F("AT+UHTTP="), DEFAULT_HTTP_PROFILE);

    if(_modem->readResponse() != ATResponse::ResponseOK) {
        return false;
    }

    _modem->sendATCommand(F("AT+UHTTP="), DEFAULT_HTTP_PROFILE, ",1,\"", server, "\"");

    if(_modem->readResponse() != ATResponse::ResponseOK) {
        return false;
    }

    _modem->sendATCommand(F("AT+UHTTP="), DEFAULT_HTTP_PROFILE, ",5,", port);

    if(_modem->readResponse() != ATResponse::ResponseOK) {
        return false;
    }

    if(ssl) {
        _modem->sendATCommand(F("AT+UHTTP="), DEFAULT_HTTP_PROFILE, ",6,1,0");

        if(_modem->readResponse() != ATResponse::ResponseOK) {
            return false;
        }
    }

    return true;
}

bool UbloxModemHttp::isResponseAvailable() const {
    return _httpResultAvailable;
}

bool UbloxModemHttp::get(const char * path, uint8_t * receiveBuffer, const size_t receiveLen, CellModemHttpHeader_t * header) {

    _modem->sendATCommand(F("AT+UHTTPC="), DEFAULT_HTTP_PROFILE, ",1,\"", path,"\",\"", READ_TEMP_FILE, "\"");

    if(_modem->readResponse() != ATResponse::ResponseOK) {
        return false;
    }

    _httpResultAvailable = false;

    if(receiveBuffer == nullptr && header == nullptr) {
        return true;
    }

    return _waitHttpResponse(receiveBuffer, receiveLen, header);
}

bool UbloxModemHttp::post(const char * path, const char * contentType, const uint8_t * sendBuffer, size_t sendLen, 
                        uint8_t * receiveBuffer, const size_t receiveLen, CellModemHttpHeader_t * header) {
    
    if(!_initWriteTempFile(WRITE_TEMP_FILE, sendLen)) return false;

    uint32_t filesize = _filesystem->getMaxFileSize();

    if(!_filesystem->writeFile(WRITE_TEMP_FILE, sendBuffer, sendLen < filesize ? sendLen : filesize)) {
        return false;
    }

    _httpResultAvailable = false;
    _modem->sendATCommand(F("AT+UHTTPC="), DEFAULT_HTTP_PROFILE,",4,\"", path, "\",\"", READ_TEMP_FILE, "\",\"", WRITE_TEMP_FILE, "\",", 6, ",\"", contentType, "\"");
    

    if(_modem->readResponse() != ATResponse::ResponseOK) {
        return false;
    }


    if(receiveBuffer == nullptr && header == nullptr) {
        return true;
    }

    return _waitHttpResponse(receiveBuffer, receiveLen, header);
}

bool UbloxModemHttp::post(const char * path, const char * contentType, Stream * stream, const size_t size) {
    if(!_initWriteTempFile(WRITE_TEMP_FILE, size)) return false;

    uint32_t filesize = _filesystem->getMaxFileSize();

    return _filesystem->writeFile(WRITE_TEMP_FILE, stream, size < filesize ? size : filesize);
}

bool UbloxModemHttp::_initWriteTempFile(const char * buffer, const size_t size) {
    if(!_filesystem) return false;

    if(_filesystem->existFile(WRITE_TEMP_FILE)) {
        _filesystem->deleteFile(WRITE_TEMP_FILE);
    }

    if(_filesystem->existFile(READ_TEMP_FILE)) {
        _filesystem->deleteFile(READ_TEMP_FILE);
    }

    return true;
}

bool UbloxModemHttp::_waitHttpResponse(uint8_t * receiveBuffer, const size_t receiveLen, CellModemHttpHeader_t * header) {
    unsigned long start = millis();

    do {
        _modem->poll(); 
        _modem->getCustomDelay()(10);   
    }while(!_httpResultAvailable && !isTimedout(start, 30000));
    
    if(_httpResultAvailable) {
        int errorClass, errorCode;
        if(!_httpResult) {      

            _modem->sendATCommand(F("AT+UHTTPER="), DEFAULT_HTTP_PROFILE);

            if(_modem->readResponse<int, int>(_uhttperParser, &errorClass, &errorCode) == ATResponse::ResponseOK) {
                
            }

            return false;
        } else {
            readResponse(header, (char *)receiveBuffer, receiveLen);
        }
        
        _httpError.errorClass = errorClass;
        _httpError.errorCode = errorCode;
    } 

    return true;
}

bool UbloxModemHttp::readResponse(CellModemHttpHeader_t * header, char * bodyBuffer, size_t len) {
    uint32_t contentLenght = 0;
    SafeCharBufferPtr_t safeCharBuffer = {bodyBuffer, len};
    if(safeCharBuffer.bufferPtr != nullptr) {
        memset(safeCharBuffer.bufferPtr, '\0', len);
    }
    UbloxHttpResponseParser_t parserStruct = {header, &safeCharBuffer};
    return _filesystem->readFile(READ_TEMP_FILE, _responseParser, &parserStruct, &contentLenght);
}

ATResponse UbloxModemHttp::_responseParser(ATResponse &response, const char * buffer, size_t size, void * param1, void * param2) {    
    UbloxHttpResponseParser_t * parserStruct = (UbloxHttpResponseParser_t *)param1;

    if(parserStruct == nullptr) return ATResponse::ResponseEmpty;

    if(!parserStruct->bodyStart) {
        if(parserStruct->header != nullptr) {
            if(strncmp_P(buffer, PSTR("+URDFILE"), 8) == 0) {
                char *ptr = strstr_P(buffer, PSTR("HTTP/1.1"));

                if(ptr != nullptr) {
                    ptr = ptr + 9; //Skip HTTP/1.1;
                    parserStruct->header->protocol = CellModemHttpProtocol::HTTP1_1;
                    parserStruct->header->status = strtol(ptr, &ptr, 10);
                    return ATResponse::ResponseMultilineParser;
                }
            }
        }

        if(param2 != nullptr) {
            if(strncasecmp_P(buffer, PSTR("Content-Length"), 14) == 0) {
                char *ptr = strchr(buffer, ':');

                if(ptr != nullptr) {
                    ++ptr; // Skip whitespace
                    *(uint32_t *)param2 = strtol(ptr, &ptr, 10);
                    return ATResponse::ResponseMultilineParser;
                }
            }
        }
    }
    
    
    if(parserStruct->bodyStart && parserStruct->body->bufferPtr != nullptr) {
        uint32_t * contentLength = (uint32_t *)param2;
        if(strlen(parserStruct->body->bufferPtr) >= *contentLength || *contentLength <= 0) {
            return ATResponse::ResponseEmpty;
        }

        
        uint16_t freeSpace = parserStruct->body->size - strlen(parserStruct->body->bufferPtr) - 1;
        uint16_t bufferLen = strlen(buffer);

        if(c_str_endsWith(buffer, "\"")) {
            --bufferLen;    // Skip last " from modem response
        }

        strncat(parserStruct->body->bufferPtr, buffer, freeSpace > bufferLen ? bufferLen : freeSpace);

    }

    if(parserStruct != nullptr && !parserStruct->bodyStart && strlen(buffer) == 0) {
        parserStruct->bodyStart = true;
    }

    return ATResponse::ResponseMultilineParser;
}

CellModemHttpError_t UbloxModemHttp::readLastError() {
    return _httpError;
}

ATResponse UbloxModemHttp::_uuhttpcrParser(ATResponse &response, const char * buffer, size_t size, int * result, uint8_t * dummy) {
    if(result == nullptr) return ATResponse::ResponseError;
    

    if(sscanf_P(buffer, PSTR("+UUHTTPCR: %*d,%*d,%d"), result) == 1) {
        return ATResponse::ResponseEmpty;
    }

    return ATResponse::ResponseError;
}

ATResponse UbloxModemHttp::_uhttperParser(ATResponse &response, const char * buffer, size_t size, int * errorClass, int * errorCode) {
    if(!errorClass || !errorCode) return ATResponse::ResponseError;

    if(sscanf_P(buffer, PSTR("+UHTTPER: %*d,%d,%d"), errorClass, errorCode) == 2) {
        return ATResponse::ResponseEmpty;
    }

    return ATResponse::ResponseError;
}

ATResponse UbloxModemHttp::handleUrcs() {
    int result = 0;
    if(sscanf_P(_modem->getResponseBuffer(), PSTR("+UUHTTPCR: %*d,%*d,%d"),&result) == 1) {
        _httpResultAvailable = true;
        _httpResult = result == 0 ? false : true;
        
        return ATResponse::UrcHandled;
    }
    
    return ATResponse::ResponseEmpty;
}