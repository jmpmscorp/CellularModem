#pragma once

#ifndef __UBLOX_MODEM_SMS__
#define __UBLOX_MODEM_SMS__

#include "../CellModemSMS.h"
#include "../CellModemUrcHandler.h"
#include "UbloxModem.h"

class UbloxModemSMS : public CellModemSMS, public CellModemUrcHandler {
    public:
        UbloxModemSMS(UbloxModem & modem);

        bool sendSMS(char * telNumber, const char * text);
        bool readSMS();

        bool setCNMI(uint8_t mode = 1, uint8_t mt = 0);
        int8_t getSMSMode();
        bool setTextMode();
        bool setPDUMode();

        ATResponse handleUrcs();
    private:
        bool _setSMSMode(uint8_t mode);

        uint8_t _mt = 0;

        static ATResponse _cmgfParser(ATResponse &response, const char * buffer, size_t size, uint8_t * mode, uint8_t * dummy);
};

#endif // __UBLOX_MODEM_SMS__