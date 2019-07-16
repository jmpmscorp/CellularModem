#pragma once

#include "CellModem.h"
#include "utils/SocketBuffer.h"
#include "Client.h"

#define DEFAULT_SOCKET_BUFFER_SIZE  512

class CellModemClient : public Client {
    public:
        CellModemClient(CellModem& modem, bool ssl = false);
        ~CellModemClient();

        virtual int connect(IPAddress ip, uint16_t port);
        virtual int connect(const char *host, uint16_t port);
        virtual size_t write(uint8_t c);
        virtual size_t write(const uint8_t *buf, size_t size);
        virtual int available();
        virtual int read();
        virtual int read(uint8_t *buf, size_t size);
        virtual int peek();
        virtual void flush();
        virtual void stop();
        virtual uint8_t connected();
        virtual operator bool() {return connected(); }

    protected:
        virtual int socketConnect(const char * host, uint8_t port, uint8_t * socketId, bool ssl = false, unsigned int timeout = 30000) = 0;
        virtual int socketAvailable(uint8_t socketId) = 0;
        virtual int socketRead(size_t len, uint8_t socketId) = 0;
        virtual int socketWrite(const void * buf, size_t len, uint8_t socketId) = 0;
        virtual bool socketDisconnect(uint8_t socketId) = 0;
        virtual bool isSocketConnected(uint8_t socketId) = 0;
        
        CellModem * _modem;
        SocketBuffer _socketBuffer = SocketBuffer(CELLMODEM_DEFAULT_RESPONSE_BUFFER_SIZE);
        
        bool _socketConnected;
        uint16_t _socketAvailable;
        uint8_t _socketId;
        
    private:
        
        bool _ssl;

        

};