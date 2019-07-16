#pragma once

#include <inttypes.h>
#include "CellModem.h"

class CellModemHttp {
    public:
        CellModemHttp(CellModem &modem);
        ~CellModemHttp();

        virtual bool init(const char * server, const uint16_t port = 80) = 0;
        virtual bool initSSL(const char * server, const uint16_t port = 443) = 0;

        virtual bool get(const char * path) = 0;
        virtual bool post(const char * path, const uint8_t * buffer, size_t len) = 0;
        virtual bool post(const char * path, Stream * stream) = 0;

    protected:
        CellModem * _modem;
    

};