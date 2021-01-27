#pragma once

#include "UbloxModem.h"
#include "../CellModemHttp.h"
#include "UbloxModemFilesystem.h"



class UbloxModemHttp : public CellModemHttp, public CellModemUrcHandler {
    public:

        explicit UbloxModemHttp(UbloxModem &modem);
        explicit UbloxModemHttp(UbloxModem &modem, UbloxModemFilesystem &filesystem);

        virtual bool init(const char * server, const uint16_t port = 80) override;
        virtual bool initSSL(const char * server, const uint16_t port = 443) override;

        virtual bool get(const char * path, uint8_t * receiveBuffer, const size_t receiveLen, CellModemHttpHeader_t * header) override;
        
        virtual bool post(const char * path, const char * contentType, const uint8_t * sendBuffer, size_t sendLen, 
                        uint8_t * receiveBuffer = nullptr, const size_t receiveLen = 0, CellModemHttpHeader_t * header = nullptr) override;
        
        virtual bool post(const char * path, const char * contentType, Stream * stream, const size_t size,
                        uint8_t * receiveBuffer = nullptr, const size_t receiveLen = 0, CellModemHttpHeader_t * header = nullptr) override;

        virtual bool isResponseAvailable() const; 
        virtual bool getHttpResult() const;
        virtual bool readResponse(CellModemHttpHeader_t * header, char * bodyBuffer, size_t size);
        virtual CellModemHttpError_t readLastError() override;

        virtual ATResponse handleUrcs() override;

    private:
        typedef struct UbloxHttpResponseParser {
            CellModemHttpHeader_t * header;
            SafeCharBufferPtr_t * body;
            bool bodyStart;
        }UbloxHttpResponseParser_t;

        bool _init(const char * server, const uint16_t port, const bool ssl);
        bool _initWriteTempFile(const char * buffer, const size_t size);
        bool _initWriteTempFile(Stream * stream, const size_t size);
        bool _waitHttpResponse(uint8_t * receiveBuffer, const size_t receiveLen, CellModemHttpHeader_t * header);
        bool _endRequest(uint8_t * receiveBuffer, const size_t receiveLen, CellModemHttpHeader_t * header);

        static ATResponse _uuhttpcrParser(ATResponse &response, const char * buffer, size_t size, int * result, uint8_t * dummy);
        static ATResponse _uhttperParser(ATResponse &response, const char * buffer, size_t size, int * errorClass, int * errorCode);
        static ATResponse _responseParser(ATResponse &response, const char * buffer, size_t size, void * param1, void * param2);
        
        UbloxModemFilesystem * _filesystem;
        CellModemHttpError_t _httpError;

        bool _httpResult;
        bool _httpResultAvailable;
};