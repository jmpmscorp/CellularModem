#pragma once

#ifndef __CELLMODEM_SMS__
#define __CELLMODEM_SMS__

#include "CellModem.h"

class CellModemSMS {
    public:
        CellModemSMS(CellModem &modem) : _modem(&modem) {}

        virtual bool sendSMS(char * telNumber, const char * text) = 0;
        virtual bool readSMS() = 0;

        virtual bool setCNMI(uint8_t mode, uint8_t mt);

        virtual int8_t getSMSMode() = 0;
        virtual bool setTextMode() = 0;
        virtual bool setPDUMode() = 0;
    protected:
        CellModem * _modem;
};

#endif // __CELLMODEM_SMS__