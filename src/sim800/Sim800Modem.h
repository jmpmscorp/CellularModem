#pragma once

#ifndef __SIM800_MODEM_H__
#define __SIM800_MODEM_H__

#include "../CellModem.h"

#define DEFAULT_BEARER_ID   1

class Sim800Modem : public CellModem, public CellModemUrcHandler{
    public:
        Sim800Modem(Stream &stream, int8_t onOffPin, int8_t resetPin, int8_t statusPin ,int8_t dtrPin, int8_t ctsPin);
        ~Sim800Modem();

        virtual bool attachGPRS(const char * apn, const char * username, const char * password);
        virtual bool detachGRPS();
        virtual bool isGPRSConnected();


        virtual bool on();
        virtual bool enableDatetimeNetworkSync();
        virtual bool disableDatetimeNetworkSync();

        ATResponse handleUrcs();

    private:
        bool _gotCIEV = true;
        bool _gotSMSReady = true;

        bool _setCLTS(uint8_t mode);
        int8_t _getCLTS();
        bool _initializationProcess();

        static ATResponse _cltsParser(ATResponse &response, const char * buffer, size_t size, unsigned int * mode, uint8_t * dummy);
        static ATResponse _sapbrParser(ATResponse &response, const char * buffer, size_t size, unsigned int * status, char * ip);
};




#endif // __SIM800_MODEM_H__