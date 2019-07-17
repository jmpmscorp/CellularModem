#pragma once

#include "UbloxModem.h"
#include "../CellModemHttp.h"
#include "UbloxModemFileSystem.h"

class UbloxModemHttp : public CellModemHttp, public CellModemUrcHandler {
    public:

        UbloxModemHttp(UbloxModem &modem);
        UbloxModemHttp(UbloxModem &modem, UbloxModemFilesystem &filesystem);

        virtual bool init(const char * server, const uint16_t port = 80);
        virtual bool initSSL(const char * server, const uint16_t port = 443);

        virtual bool get(const char * path);
        virtual bool post(const char * path, const uint8_t * buffer, size_t len);
        virtual bool post(const char * path, Stream * stream);

        virtual ATResponse handleUrcs();

    private:
        bool _init(const char * server, const uint16_t port, const bool ssl);
        bool _initWriteTempFile(const char * buffer, const size_t size);
        bool _initWriteTempFile(Stream * stream, const size_t size);

        static ATResponse _uuhttpcrParser(ATResponse &response, const char * buffer, size_t size, int * result, uint8_t * dummy);
        static ATResponse _uhttperParser(ATResponse &response, const char * buffer, size_t size, int * errorClass, int * errorCode);
        
        UbloxModemFilesystem * _filesystem;

        bool _httpResult;
        bool _httpResultAvailable;
};