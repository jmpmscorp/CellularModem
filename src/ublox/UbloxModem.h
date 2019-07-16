#pragma once

#ifndef __UBLOX_MODEM_H__
#define __UBLOX_MODEM_H__

#include "../CellModem.h"
#include "../CellModemClient.h"

#define MAX_CLIENTS_NUMBER  7

class UbloxModem : public CellModem {    
    public:        
        UbloxModem(Stream &stream, int8_t onOffPin, int8_t statusPin ,int8_t dtrPin, int8_t ctsPin);
        ~UbloxModem();

        virtual bool attachGPRS(const char * apn, const char * username, const char * password);
        virtual bool detachGRPS();
        virtual bool isGPRSConnected();

        bool enableDatetimeNetworkSync();
        bool disableDatetimeNetworkSync();

        CellModemClient * getCellModemClient(uint8_t id);
        
    private:
        bool _setCTZU(uint8_t mode);
        int8_t _getCTZU();
        bool _initializationProcess();   

        CellModemClient * _clients[MAX_CLIENTS_NUMBER];
    
        static ATResponse _ctzuParser(ATResponse& response, const char * buffer, size_t size, unsigned int * mode, uint8_t * dummy);     
        static ATResponse _upsndParser(ATResponse& response, const char * buffer, size_t size, uint8_t * value, uint8_t * dummy);     

        
        
};


#endif // __UBLOX_MODEM_H__