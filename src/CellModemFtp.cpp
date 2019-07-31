#include "CellModemFtp.h"

CellModemFtp::CellModemFtp(CellModem &modem) :
    _modem(&modem)
{

}

CellModemFtp::~CellModemFtp() {
    _modem = nullptr;
}