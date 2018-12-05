#pragma once

#ifndef __SIM800_MODEM_H__
#define __SIM800_MODEM_H__

#include "../CellModem.h"

class Sim800Modem : public CellModem {
    public:
        Sim800Modem(Stream &stream, int8_t onOffPin, int8_t statusPin ,int8_t dtrPin, int8_t ctsPin);
        ~Sim800Modem();

        bool enableDatetimeNetworkSync();
        bool disableDatetimeNetworkSync();

    private:
        bool _setCLTS(uint8_t mode);
        int8_t _getCLTS();

        static ATResponse _cltsParser(ATResponse &response, const char * buffer, size_t size, unsigned int * mode, uint8_t * dummy);

};




#endif // __SIM800_MODEM_H__