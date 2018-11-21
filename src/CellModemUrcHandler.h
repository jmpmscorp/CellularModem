#pragma once

#ifndef __CELLMODEM_URC_HANDLER__
#define __CELLMODEM_URC_HANDLER__

#include "CellModemCommon.h"

class CellModemUrcHandler {
    public:
        virtual ATResponse handleUrcs() = 0;
};

#endif