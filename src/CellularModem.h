#pragma once

#ifndef __CELLULAR_MODEM_H__
#define __CELLULAR_MODEM_H__

#define UBLOX 1
#define SIM800 2

#include "CellModemDatetime.h"


#ifndef CELLMODEM_MODEL
    #error "You should define a model to use"
#else
    #pragma message ("Cellular Modem Library Version 1.1.0")
    #if CELLMODEM_MODEL == UBLOX
        #pragma message ("MODEL UBLOX")
        #include "ublox/UbloxModem.h"
        
        typedef UbloxModem CellularModem;

        #ifdef CELLMODEM_USE_SMS
            #include "CellModemSMS.h"
            typedef CellModemSMS CellularModemSMS;
        #endif

        #ifdef CELLMODEM_USE_PHONEBOOK
            #include "CellModemPhonebook.h"
            typedef CellModemPhonebook CellularModemPhonebook;
        #endif

        #ifdef CELLMODEM_USE_HTTP
            #include "ublox/UbloxModemFilesystem.h"
            #include "ublox/UbloxModemHttp.h"
            typedef UbloxModemHttp CellularModemHttp;
            typedef UbloxModemFilesystem CellularModemFilesystem;
        #endif
    
    #elif CELLMODEM_MODEL == SIM800
        #pragma message ("MODEL SIM800")
        #include "simcom/SimcomModem.h"
        typedef SimcomModem CellularModem;

        #ifdef CELLMODEM_USE_SMS
            #pragma message ("Using SMS Functionality");
            #include "CellModemSMS.h"
            typedef CellModemSMS CellularModemSMS;
        #endif

        #ifdef CELLMODEM_USE_PHONEBOOK
            #pragma message ("Using Phonebook Functionality");
            #include "CellModemPhonebook.h"
            typedef CellModemPhonebook CellularModemPhonebook;
        #endif

        #ifdef CELLMODEM_USE_FTP
            #pragma message ("Using FTP Functionality");
            #include "simcom/SimcomModemFtp.h"
            typedef SimcomModemFtp CellularModemFtp;
        #endif
    #else
        #error "Cellular Modem Error. You should define CELLMODEM_MODEL"
    #endif
#endif

#endif // __CELLULAR_MODEM__