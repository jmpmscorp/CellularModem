#pragma once

#ifndef __UBLOX_MODEM_H__
#define __UBLOX_MODEM_H__

#include "../CellModem.h"

class UbloxModem : public CellModem {

    public:
        UbloxModem(Uart &stream, int8_t onOffPin, int8_t statusPin ,int8_t dtrPin, int8_t ctsPin);
        ~UbloxModem();

        void begin(unsigned long baudrate);
        
        int attachGPRS(const char * apn, const char * user, const char * password);
        int dettachGPRS();

        virtual int getSignalQuality(int8_t * rssi, int8_t * ber);

    private:
        ATResponse _sendInitializationCommands();
};


#endif // __UBLOX_MODEM_H__