#pragma once

#ifndef __CELLULAR_MODEM_H__
#define __CELLULAR_MODEM_H__

#if defined(CELLULAR_MODEM_UBLOX)
    #include "ublox/UbloxModem.h"
    typedef UbloxModem CellularModem;
#else
    #error "Cellular Modem Error. You should define MODEL"
#endif

#endif // __CELLULAR_MODEM__