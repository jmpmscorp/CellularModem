#pragma once

#ifndef __UBLOX_MODEM_H__
#define __UBLOX_MODEM_H__

#include "../CellModem.h"

#ifndef __AVR__
    #define sscanf_P sscanf
#endif

class UbloxModem : public CellModem {

    public:
        UbloxModem(Stream &stream, int8_t onOffPin, int8_t statusPin ,int8_t dtrPin, int8_t ctsPin);
        ~UbloxModem();

        void init();
        virtual bool on();
        virtual bool isOn() const;

        SIMStatus getSimStatus();
        bool setSimPin(const char * pin);
        bool connect();
        bool connect(const char * pin);


        int attachGPRS(const char * apn, const char * user, const char * password);
        int dettachGPRS();

        bool getOperatorName(char * buffer, size_t size);
        bool getSignalQuality(int8_t * rssi, uint8_t * ber);

    private:
        bool _sendInitializationCommands();
        bool _enableAutoregistrationNetwork(uint32_t timeout = 4*60*1000); // 4 minutes
        int _getAutoregistrationNetworkMode();
        bool _waitForSignalQuality(uint32_t timeout = 60 * 1000);  // 60 seconds

        static ATResponse _copsParser(ATResponse &response, const char * buffer, size_t size, char * operatorNameBuffer, size_t * operatorNameBufferSize);
        static ATResponse _copsParser(ATResponse &response, const char * buffer, size_t size, int * networkTechnology, uint8_t * dummy);
        static ATResponse _cpinParser(ATResponse& response, const char * buffer, size_t size, SIMStatus * simStatusResult, uint8_t * dummy);
        static ATResponse _csqParser(ATResponse& response, const char* buffer, size_t size, int * rssi, int * ber);
};


#endif // __UBLOX_MODEM_H__