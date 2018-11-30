#pragma once

#ifndef __CELLMODEM_SMS__
#define __CELLMODEM_SMS__

#include "CellModem.h"

typedef void ( * CMTICallback ) ( char * memoryBuffer, uint16_t index );
typedef void ( * CMTCallback ) ( char * phoneNumber, char * text);

class CellModemSMS {
    public:
        CellModemSMS(CellModem &modem);

        virtual bool send(char * phoneNumber, const char * text);
        virtual bool read(uint16_t index, char * phoneNumber, size_t phoneNumberSize, char * textBuffer, size_t textBufferSize);
        virtual int readList(const char * filter, unsigned int * indexList, size_t size, unsigned int * remainingSize);
        virtual bool remove(unsigned int index, uint8_t flag = 0);

        virtual bool setNewSMSIndicator(uint8_t mode, uint8_t mt);
        void setCMTCallback(CMTCallback cmtCallback);
        void setCMTICallback(CMTICallback cmtiCallback);
        virtual int8_t getMode();
        virtual bool setTextMode();
        virtual bool setPDUMode();

    protected:
        virtual bool _setMode(uint8_t mode);
        ATResponse _handleUrcs();
        CellModem * _modem;

        CMTCallback _cmtCallback = nullptr;
        CMTICallback _cmtiCallback = nullptr;

        static ATResponse _cmgfParser(ATResponse &response, const char * buffer, size_t size, uint8_t * mode, uint8_t * dummy);
        static ATResponse _cmgrParser(ATResponse &response, const char * buffer, size_t size, SafeCharBufferPtr * phoneNumber, SafeCharBufferPtr * textBuffer);
        static ATResponse _cmglParser(ATResponse &response, const char * buffer, size_t size, unsigned int * indexList, int * remainingSize);
    
        uint8_t _mt = 0;
};

#endif // __CELLMODEM_SMS__