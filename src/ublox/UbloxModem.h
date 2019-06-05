#pragma once

#ifndef __UBLOX_MODEM_H__
#define __UBLOX_MODEM_H__

#include "../CellModem.h"

class UbloxModem : public CellModem {    
    public:        
        UbloxModem(Stream &stream, int8_t onOffPin, int8_t statusPin ,int8_t dtrPin, int8_t ctsPin);
        ~UbloxModem();

        bool enableDatetimeNetworkSync();
        bool disableDatetimeNetworkSync();

    private:
        bool _setCTZU(uint8_t mode);
        int8_t _getCTZU();
        bool _initializationProcess();   

        static ATResponse _ctzuParser(ATResponse& response, const char * buffer, size_t size, unsigned int * mode, uint8_t * dummy);     
};


#endif // __UBLOX_MODEM_H__