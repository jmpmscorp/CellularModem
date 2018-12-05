#pragma once

#ifndef __CELLULAR_MODEM_H__
#define __CELLULAR_MODEM_H__

#include "CellModemDatetime.h"


#ifndef CELLMODEM_MODEL
    #error "You should define a model to use"
#else
    #if CELLMODEM_MODEL == UBLOX
        #include "ublox/UbloxModem.h"
        #include "CellModemSMS.h"

        typedef UbloxModem CellularModem;
        typedef CellModemSMS CellularModemSMS;
    #else
        #error "Cellular Modem Error. You should define CELLMODEM_MODEL"
    #endif
#endif

#endif // __CELLULAR_MODEM__