#pragma once

#ifndef __CELLULAR_MODEM_H__
#define __CELLULAR_MODEM_H__

#define UBLOX 1
#define SIM800 2

#include "CellModemDatetime.h"


#ifndef CELLMODEM_MODEL
    #error "You should define a model to use"
#else
    #if CELLMODEM_MODEL == UBLOX
        #pragma message ("MODEL UBLOX")
        #include "ublox/UbloxModem.h"
        #include "CellModemSMS.h"

        typedef UbloxModem CellularModem;
        typedef CellModemSMS CellularModemSMS;
    #elif CELLMODEM_MODEL == SIM800
        #pragma message ("MODEL SIM800")
        #include "sim800/Sim800Modem.h"
        #include "CellModemSMS.h"

        typedef Sim800Modem CellularModem;
        typedef CellModemSMS CellularModemSMS;
    #else
        #error "Cellular Modem Error. You should define CELLMODEM_MODEL"
    #endif
#endif

#endif // __CELLULAR_MODEM__