#pragma once

#ifndef __CELLMODEM_SMS__
#define __CELLMODEM_SMS__

#include "CellModem.h"

typedef void ( * CMTICallback ) ( char * memoryBuffer, uint16_t index );
typedef void ( * CMTCallback ) ( char * phoneNumber, char * text);

class CellModemSMS {
    friend class CellModem;
    public:
        CellModemSMS(CellModem &modem) : _modem(&modem) {}

        virtual bool sendSMS(char * phoneNumber, const char * text) = 0;
        virtual bool readSMS() = 0;

        virtual bool setNewSMSIndicator(uint8_t mode, uint8_t mt) = 0;
        void setCMTCallback(CMTCallback cmtCallback) { _cmtCallback = cmtCallback; }
        void setCMTICallback(CMTICallback cmtiCallback) { _cmtiCallback = cmtiCallback; }
        void setCMTIRemotePhoneNumberBuffer(char * phoneNumberBuffer) { _cmtRemotePhoneNumber = phoneNumberBuffer; }
        virtual int8_t getSMSMode() = 0;
        virtual bool setTextMode() = 0;
        virtual bool setPDUMode() = 0;

    protected:
        CellModem * _modem;

        CMTCallback _cmtCallback = nullptr;
        CMTICallback _cmtiCallback = nullptr;

        bool _cmtReceived = false;
        char * _cmtRemotePhoneNumber;
};

#endif // __CELLMODEM_SMS__