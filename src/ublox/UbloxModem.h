#pragma once

#ifndef __UBLOX_MODEM_H__
#define __UBLOX_MODEM_H__

#include "../CellModem.h"
#include "../CellModemClient.h"

#define MAX_CLIENTS_NUMBER  7

class UbloxModem : public CellModem {    
    public:        
        UbloxModem(Stream &stream, int8_t onOffPin, int8_t resetPin, int8_t statusPin);
        ~UbloxModem();

        virtual bool softwareOff() override;

        virtual bool setLowPowerMode(uint8_t mode) override;
        virtual bool enableLowPowerMode() override;
        virtual bool disableLowPowerMode() override;

        virtual bool attachGPRS(const char * apn, const char * username, const char * password) override;
        virtual bool detachGRPS() override;
        virtual bool isGPRSConnected() override;

        bool enableDatetimeNetworkSync() override;
        bool disableDatetimeNetworkSync() override;

        CellModemClient * getCellModemClient(uint8_t id);
        
    private:
        bool _setCTZU(uint8_t mode);
        int8_t _getCTZU();
        bool _initializationProcess() override;   

        CellModemClient * _clients[MAX_CLIENTS_NUMBER];
    
        static ATResponse _ctzuParser(ATResponse& response, const char * buffer, size_t size, unsigned int * mode, uint8_t * dummy);     
        static ATResponse _upsndParser(ATResponse& response, const char * buffer, size_t size, uint8_t * value, uint8_t * dummy);     

        
        
};


#endif // __UBLOX_MODEM_H__