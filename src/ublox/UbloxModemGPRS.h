#pragma once

#include "../CellModemGPRS.h"
#include "UbloxModem.h"

class UbloxModemGPRS : public CellModemGPRS {
    public:
        
        UbloxModemGPRS(UbloxModem &modem);
        virtual bool attach(const char * apn, const char * username, const char * password);
        virtual bool dettach();
        virtual bool isConnected();

        ATResponse handleUrcs();
    
    private:

        static ATResponse _upsndParser(ATResponse& response, const char * buffer, size_t size, uint8_t * value, uint8_t * dummy);     
};