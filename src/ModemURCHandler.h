#pragma once

#ifndef __MODEM_URC_HANDLER__
#define __MODEM_URC_HANDLER__

class ModemURCHandler {
    public:
        virtual void handleURC(const char * urc) = 0;
};

#endif // __MODEM_URC_HANDLER__