#include "CellModemHttp.h"

CellModemHttp::CellModemHttp(CellModem &modem) :
    _modem(&modem)
    {

    }

CellModemHttp::~CellModemHttp() {
    _modem = nullptr;
}