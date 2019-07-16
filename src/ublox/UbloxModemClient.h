#pragma once

#include "UbloxModem.h"
#include "../CellModemClient.h"
#include "../CellModemUrcHandler.h"

class UbloxModemClient : public CellModemClient, public CellModemUrcHandler {
    public:
        UbloxModemClient(UbloxModem& modem, bool ssl = false);
        ~UbloxModemClient();

        ATResponse handleUrcs();

    protected:
        virtual int socketConnect(const char * host, uint8_t port, uint8_t * socketId, bool ssl = false, unsigned int timeout = 30000);
        virtual int socketAvailable(uint8_t socketId);
        virtual int socketRead(size_t len, uint8_t socketId);
        virtual int socketWrite(const void * buf, size_t len, uint8_t socketId);
        virtual bool socketDisconnect(uint8_t socketId);
        virtual bool isSocketConnected(uint8_t socketId);

    private:
        static ATResponse _usocrParser(ATResponse &response, const char * buffer, size_t size, uint8_t * socketId, uint8_t * dummy);
        static ATResponse _usoctlParser(ATResponse &response, const char * buffer, size_t size, uint8_t * status, uint8_t * dummy);
        static ATResponse _usordParser(ATResponse &response, const char * buffer, size_t size, int * len, uint8_t * dummt);
        static ATResponse _usordParser(ATResponse &response, const char * buffer, size_t size, SocketBuffer * socketBuffer, size_t *len);
        static ATResponse _usowrParser(ATResponse &response, const char * buffer, size_t size, int * sent, uint8_t * dummy);

};