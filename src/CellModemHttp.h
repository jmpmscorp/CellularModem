#pragma once

#include <inttypes.h>
#include "CellModem.h"

enum class CellModemHttpProtocol : uint8_t {
    UNKNOWN,
    HTTP1_1,
    HTTP2
};

typedef struct CellModemHttpHeader_t {
    CellModemHttpProtocol protocol;
    uint16_t status;
}CellModemHttpHeader_t;

typedef struct CellModemHttpError_t {
    uint16_t errorClass;
    uint16_t errorCode;
}CellModemHttpError_t;

class CellModemHttp {
    public:
        explicit CellModemHttp(CellModem &modem);
        ~CellModemHttp();

        virtual bool init(const char * server, const uint16_t port = 80) = 0;
        virtual bool initSSL(const char * server, const uint16_t port = 443) = 0;

        virtual bool get(const char * path, uint8_t * receiveBuffer, const size_t receiveLen, CellModemHttpHeader_t * header) = 0;
        
        virtual bool post(const char * path, const char * contentType, const uint8_t * sendBuffer, size_t sendLen, 
                        uint8_t * receiveBuffer, const size_t receiveLen, CellModemHttpHeader_t * header) = 0;
        
        virtual bool post(const char * path, const char * contentType, Stream * sendStream, const size_t size,
                        uint8_t * receiveBuffer, const size_t receiveLen, CellModemHttpHeader_t * header) = 0;

        virtual CellModemHttpError_t readLastError() = 0;

    protected:
        CellModem * _modem;
    

};