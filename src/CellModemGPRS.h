#pragma once

#include "CellModem.h"
#include "CellModemUrcHandler.h"

class CellModemGPRS : public CellModemUrcHandler {
    public:
        CellModemGPRS(CellModem &modem);

        virtual bool attach(const char * apn, const char * username, const char * password) = 0;
        virtual bool dettach() = 0;
        virtual bool isConnected() = 0;
    
    protected:
        CellModem * _modem;
};