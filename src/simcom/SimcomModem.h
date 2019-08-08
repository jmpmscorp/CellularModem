#pragma once

#include "../CellModem.h"

#define DEFAULT_BEARER_ID   1

class SimcomModem : public CellModem, public CellModemUrcHandler{
    public:
        SimcomModem(Stream &stream, int8_t onOffPin, int8_t resetPin, int8_t statusPin);
        ~SimcomModem();

        virtual bool softwareOff();

        virtual bool setLowPowerMode(uint8_t mode);
        virtual bool enableLowPowerMode();
        virtual bool disableLowPowerMode();

        virtual bool attachGPRS(const char * apn, const char * username, const char * password);
        virtual bool detachGRPS();
        virtual bool isGPRSConnected();

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
        static ATResponse _cpowdParser(ATResponse &response, const char * buffer, size_t size, uint8_t * dummy1, uint8_t * dummy2);
        static ATResponse _sapbrParser(ATResponse &response, const char * buffer, size_t size, unsigned int * status, char * ip);
};