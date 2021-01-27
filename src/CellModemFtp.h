#pragma once

#include "CellModem.h"

class CellModemFtp {
    public:
        explicit CellModemFtp(CellModem &modem);
        ~CellModemFtp();

        virtual bool init(const char * server, const char * username, const char * password) = 0;
        virtual bool send(const char * path, const char * filename, const uint8_t * buffer, size_t size) = 0;
        virtual bool send(const char * path, const char * filename, Stream &stream, size_t size) = 0;
    
    protected:
        CellModem * _modem;
};