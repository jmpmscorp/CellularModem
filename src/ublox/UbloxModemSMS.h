#pragma once

#ifndef __UBLOX_MODEM_SMS__
#define __UBLOX_MODEM_SMS__

#include "../CellModemSMS.h"
#include "UbloxModem.h"

class UbloxModemSMS : public CellModemSMS {
    public:
        UbloxModemSMS(UbloxModem & modem);

        bool sendSMS(char * telNumber, const char * text);
        bool readSMS();

        int8_t getSMSMode();
        bool setTextMode();
        bool setPDUMode();
    
    private:
        bool _setSMSMode(uint8_t mode);
        static ATResponse _cmgfParser(ATResponse &response, const char * buffer, size_t size, uint8_t * mode, uint8_t * dummy);
};

#endif // __UBLOX_MODEM_SMS__