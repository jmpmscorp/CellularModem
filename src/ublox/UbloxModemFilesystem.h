#pragma once

#include "UbloxModem.h"

typedef void (*ReadFileParserCb)();

class UbloxModemFilesystem {
    public:
        UbloxModemFilesystem(UbloxModem &modem);

        uint32_t getMaxFileSize();
        bool writeFile(const char * filename, const uint8_t * buffer, const size_t size);
        bool writeFile(const char * filename, Stream * stream, const size_t size);
        bool readFile(const char * filename, ReadFileParserCb * readCb);
        bool existFile(const char * filename);
        bool deleteFile(const char * filename);
        
    private:
        static ATResponse _ulstfileParser(ATResponse &response, const char * buffer, size_t size, uint32_t * filesize, uint8_t * dummy);
        static ATResponse _readfileParser(ATResponse &response, const char * buffer, size_t size, ReadFileParserCb * readparser, uint8_t * dummy); 
        UbloxModem * _modem;
};