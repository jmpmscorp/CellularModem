#include "UbloxModemSMS.h"

UbloxModemSMS::UbloxModemSMS(UbloxModem &modem) :
    CellModemSMS(modem) {
    _modem->addUrcHandler(this);
}

ATResponse UbloxModemSMS::handleUrcs() {
    return CellModemSMS::handleUrcs();    
}