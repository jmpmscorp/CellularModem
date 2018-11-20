#pragma once

#ifndef __GPRS_MODEM_H__
#define __GPRS_MODEM_H__

#include "ATCellularModem.h"

class GPRSModem : public ATCellularModem {
    public:
        GPRSModem(Uart &serial, int8_t dtrPin, int8_t resetPin) : ATCellularModem(serial, dtrPin, resetPin) {}
        virtual void begin(uint32_t baudrate) = 0;
        virtual int networkOn() = 0;
        virtual int getSignalQuality(int8_t * rssi, int8_t * ber);
        virtual int attachGPRS(const char * apn, const char * user, const char * password) = 0;
        virtual int dettachGPRS() = 0;
    
    protected:
        int8_t _minSignalQuality = -93;
};

#endif // __GPRS_MODEM_H__