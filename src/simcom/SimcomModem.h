#pragma once

#include "../CellModem.h"

#define DEFAULT_BEARER_ID   1

class SimcomModem : public CellModem, public CellModemUrcHandler{
    public:
        SimcomModem(Stream &stream, int8_t onOffPin, int8_t resetPin, int8_t statusPin);
        ~SimcomModem();

        virtual bool softwareOff() override;

        virtual bool setLowPowerMode(uint8_t mode) override;
        virtual bool enableLowPowerMode() override;
        virtual bool disableLowPowerMode() override;

        virtual bool attachGPRS(const char * apn, const char * username, const char * password) override;
        virtual bool detachGRPS() override;
        virtual bool isGPRSConnected() override;

        virtual bool enableDatetimeNetworkSync() override;
        virtual bool disableDatetimeNetworkSync() override;

        ATResponse handleUrcs() override;

    private:
        bool _gotCIEV = true;
        bool _gotSMSReady = true;

        bool _setCLTS(uint8_t mode);
        int8_t _getCLTS();
        bool _initializationProcess() override;

        static ATResponse _cltsParser(ATResponse &response, const char * buffer, size_t size, unsigned int * mode, uint8_t * dummy);
        static ATResponse _cpowdParser(ATResponse &response, const char * buffer, size_t size, uint8_t * dummy1, uint8_t * dummy2);
        static ATResponse _sapbrParser(ATResponse &response, const char * buffer, size_t size, unsigned int * status, char * ip);
};