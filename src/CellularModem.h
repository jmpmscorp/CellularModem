#pragma once

#ifndef __CELLULAR_MODEM_H__
#define __CELLULAR_MODEM_H__

#include "CellModemDatetime.h"

#if defined(CELLMODEM_UBLOX)
    #include "ublox/UbloxModem.h"
    #include "CellModemSMS.h"

    typedef UbloxModem CellularModem;
    typedef CellModemSMS CellularModemSMS;
#else
    #error "Cellular Modem Error. You should define MODEL"
#endif

#endif // __CELLULAR_MODEM__