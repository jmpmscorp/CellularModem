#pragma once

#include "../CellModemFtp.h"
#include "../CellModemUrcHandler.h"
#include "SimcomModem.h"

class SimcomModemFtp : public CellModemFtp, public CellModemUrcHandler {
    public:
        explicit SimcomModemFtp(SimcomModem &modem);

        virtual bool init(const char * server, const char * username, const char * password) override;
        virtual bool send(const char * path, const char * filename, const uint8_t * buffer, size_t size) override;
        virtual bool send(const char * path, const char * filename, Stream &stream, size_t size) override;

        virtual uint8_t getLastTransactionError() const;

        virtual ATResponse handleUrcs() override;
    
    private:
        bool _initFtpTransaction(const char * path, const char * filename);
        bool _endFtpTransaction();
        bool _sendChunk(const uint8_t* buffer, size_t sizeToSend, size_t * bytesSent);
        bool _sendChunk(Stream &stream, size_t sizeToSend, size_t * bytesSent);
        bool _waitFtpPutUrc(uint32_t timeout = 75000);

        uint8_t _transactionError = 1;
        uint16_t _transactionLength = 0;

        bool _ftpPutUrcReceived = false;
};