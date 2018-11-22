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

        void begin();
        virtual bool on();
        virtual bool isOn() const;
        bool connect();
        int attachGPRS(const char * apn, const char * user, const char * password);
        int dettachGPRS();

        bool getOperatorName(char * buffer, size_t size);
        int getSignalQuality(int8_t * rssi, int8_t * ber);

    private:
        bool _sendInitializationCommands();

        static ATResponse _copsParser(ATResponse &response, const char* buffer, size_t size, char* operatorNameBuffer, size_t* operatorNameBufferSize);
        static ATResponse _copsParser(ATResponse &response, const char* buffer, size_t size, int* networkTechnology, uint8_t* dummy);
};


#endif // __UBLOX_MODEM_H__