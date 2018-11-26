#pragma once

#ifndef __CELLULAR_MODEM_H__
#define __CELLULAR_MODEM_H__

#if defined(CELLMODEM_UBLOX)
    #include "ublox/UbloxModem.h"
    #include "ublox/UbloxModemSMS.h"

    typedef UbloxModem CellularModem;
    typedef UbloxModemSMS CellularModemSMS;
#else
    #error "Cellular Modem Error. You should define MODEL"
#endif

#endif // __CELLULAR_MODEM__