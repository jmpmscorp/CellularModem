#include "CellModemGPRS.h"

CellModemGPRS::CellModemGPRS(CellModem &modem) :
    _modem(&modem) 
{
    _modem->addUrcHandler(this);
}