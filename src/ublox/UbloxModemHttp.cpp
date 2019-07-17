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

bool UbloxModemHttp::get(const char * path) {

    _modem->sendATCommand(F("AT+UHTTPC="), DEFAULT_HTTP_PROFILE, ",1,\"", path,"\",\"", READ_TEMP_FILE, "\"");

    if(_modem->readResponse() != ATResponse::ResponseOK) {
        return false;
    }

    _httpResultAvailable = false;
    unsigned long start = millis();

    do {
        _modem->poll();    
    }while(!_httpResultAvailable && !isTimedout(start, 20000));

    if(_httpResultAvailable) {
        if(!_httpResult) {
            int errorClass, errorCode;

            _modem->sendATCommand(F("AT+UHTTPER="), DEFAULT_HTTP_PROFILE);

            if(_modem->readResponse<int, int>(_uhttperParser, &errorClass, &errorCode) == ATResponse::ResponseOK) {
                Serial.print("Error Class: "); Serial.println(errorClass);
                Serial.print("Error Code: "); Serial.println(errorCode);
            }
        }

        _modem->sendATCommand(F("AT+URDFILE=\""), READ_TEMP_FILE, '"');

        if(_modem->readResponse() != ATResponse::ResponseOK) {
            return false;
        }
    }

    return true;
}

bool UbloxModemHttp::post(const char * path, const uint8_t * buffer, size_t len) {
    if(!_filesystem) return false;

    if(_filesystem->existFile(WRITE_TEMP_FILE)) {
        _filesystem->deleteFile(WRITE_TEMP_FILE);
    }

    uint32_t filesize = _filesystem->getMaxFileSize();
    // auto toWrite = len < filesize ? len : filesize;
    // Serial.print("To Write: "); Serial.println(toWrite);


    _filesystem->writeFile(WRITE_TEMP_FILE, buffer, len < filesize ? len : filesize);
    _filesystem->readFile(WRITE_TEMP_FILE, nullptr);
    //_modem->sendATCommand(F("AT+UHTTPC="), DEFAULT_HTTP_PROFILE, ",1,\"", path,"\",\"", READ_TEMP_FILE, "\"");
}

bool UbloxModemHttp::post(const char * path, Stream * stream) {
    return true;
}

bool UbloxModemHttp::_initWriteTempFile(const char * buffer, const size_t size) {
    // Get de filesize limit
    _modem->sendATCommand(F("AT+ULSTFILE=1"));


}

ATResponse UbloxModemHttp::_uuhttpcrParser(ATResponse &response, const char * buffer, size_t size, int * result, uint8_t * dummy) {
    if(!result) ATResponse::ResponseError;

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

        ATResponse::UrcHandled;
    }
}