#pragma once

#ifndef __UBLOX_MODEM_H__
#define __UBLOX_MODEM_H__

#include "../GPRSModem.h"

class UbloxModem : public GPRSModem {

    public:
        UbloxModem(Uart &stream, int8_t dtrPin, int8_t resetPin);
        ~UbloxModem();

        void begin(uint32_t baudrate);
        int on();
        int off();
        int reset();

        int attachGPRS(const char * apn, const char * user, const char * password);
        int dettachGPRS();

        virtual int networkOn();
        virtual int getSignalQuality(int8_t * rssi, int8_t * ber);


};


#endif // __UBLOX_MODEM_H__