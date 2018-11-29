#pragma once

#ifndef __UBLOX_MODEM_H__
#define __UBLOX_MODEM_H__

#include "../CellModem.h"

class UbloxModem : public CellModem {    
    public:        
        UbloxModem(Stream &stream, int8_t onOffPin, int8_t statusPin ,int8_t dtrPin, int8_t ctsPin);
        ~UbloxModem();

    private:
        bool _sendInitializationCommands();        
};


#endif // __UBLOX_MODEM_H__