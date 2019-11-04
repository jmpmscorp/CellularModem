#include "UbloxModemFilesystem.h"
#include <inttypes.h>

UbloxModemFilesystem::UbloxModemFilesystem(UbloxModem &modem) :
    _modem(&modem)
{

}

uint32_t UbloxModemFilesystem::getMaxFileSize() {
    _modem->sendATCommand(F("AT+ULSTFILE=1"));

    uint32_t filesize;
    if(_modem->readResponse<uint32_t, uint8_t>(_ulstfileParser, &filesize, nullptr, nullptr, 30000) == ATResponse::ResponseOK) {
        return filesize;
    } 

    return 0;
}

ATResponse UbloxModemFilesystem::_ulstfileParser(ATResponse &response, const char * buffer, size_t size, uint32_t * filesize, uint8_t * dummy) {
    if(filesize == nullptr) {
        return ATResponse::ResponseError;
    }

    if(sscanf_P(buffer, PSTR("+ULSTFILE: %" SCNu32), filesize) == 1) {
        return ATResponse::ResponseEmpty;
    }

    return ATResponse::ResponseError;
}

bool UbloxModemFilesystem::writeFile(const char * filename, const uint8_t * buffer, const size_t size) {
    _modem->sendATCommand(F("AT+UDWNFILE=\""), filename, "\",", size);

    if(_modem->readResponse(nullptr, 10000) == ATResponse::ResponsePrompt) {
        _modem->getSerial()->write(buffer, size);

        if(_modem->readResponse() == ATResponse::ResponseOK) {
            return true;
        }
    }    

    return false;
}


bool UbloxModemFilesystem::writeFile(const char * filename, Stream * stream, const size_t size) {
    _modem->sendATCommand(F("AT+UDWNFILE=\""), filename, "\",", size);

    if(_modem->readResponse(nullptr, 10000) == ATResponse::ResponsePrompt) {
        size_t written = 0;
        unsigned long start = _modem->getCustomMillis()();

        while(written < size && !isTimedout(start, 30000)) {
            if(stream->available()) {
                _modem->getSerial()->write(stream->read());
                ++written;
            }

            if(written % 1000 == 0) {
                _modem->getCustomDelay()(1);
            }
        }

        if(_modem->readResponse() == ATResponse::ResponseOK) {
            return true;
        }
    }   

    return false;
}

bool UbloxModemFilesystem::readFile(const char * filename, ResponseParserCallbackPtr readparser, void* param1, void* param2) {
    _modem->sendATCommand(F("AT+URDFILE=\""), filename, "\"");
    if(_modem->readResponse<void, void>(*readparser, param1, param2) == ATResponse::ResponseOK) {
        return true;

    }

    return false;
}

ATResponse UbloxModemFilesystem::_readfileParser(ATResponse &response, const char * buffer, size_t size, ResponseParserCallbackPtr * readparser, uint8_t * dummy) {
    // if(readparser) {
    //     (*readparser)();
    // }

    return ATResponse::ResponseMultilineParser;
}

bool UbloxModemFilesystem::existFile(const char * filename) {
    _modem->sendATCommand(F("AT+ULSTFILE=2,\""), filename,"\"");

    uint32_t filesize;
    if(_modem->readResponse<uint32_t, uint8_t>(_ulstfileParser, &filesize, nullptr) == ATResponse::ResponseOK) {
        if(filesize > 0) {
            return true;
        }
    }

    return false;
}

bool UbloxModemFilesystem::deleteFile(const char * filename) {
    _modem->sendATCommand(F("AT+UDELFILE=\""), filename, "\"");

    if(_modem->readResponse() == ATResponse::ResponseOK) {
        return true;
    }

    return false;
}