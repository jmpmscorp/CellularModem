#pragma once

#ifndef __UBLOX_MODEM_SMS__
#define __UBLOX_MODEM_SMS__

#include "../CellModemSMS.h"
#include "../CellModemUrcHandler.h"
#include "UbloxModem.h"

#define CELLMODEM_SMS_MAX_INDEXES_EVENTS 5

class UbloxModemSMS : public CellModemSMS, public CellModemUrcHandler {
    public:
        UbloxModemSMS(UbloxModem & modem);

        ATResponse handleUrcs();   
        
};

#endif // __UBLOX_MODEM_SMS__