#pragma once

#ifndef __CELLMODEM_PHONEBOOK_H__
#define __CELLMODEM_PHONEBOOK_H__

#include "CellModem.h"

class CellModemPhonebook {
    public:
        CellModemPhonebook(CellModem &modem);

        bool addContact(const char * phoneNumber, const char * contactName, uint8_t index = 0);
        bool readSingleContact(uint8_t index, char * numberBuffer, char * contactNameBuffer);
        bool findContact(const char * toFindStr, char * numberBuffer, char * contactNameBuffer, uint8_t * numberType);
        bool removeContact(const char * contactName);
        bool removeContact(uint8_t index);
    
    protected:
        CellModem *_modem;

        static ATResponse _cpbrParser(ATResponse &response, const char * buffer, size_t size, char * numberBuffer, char * contactNameBuffer);
};


#endif // __CELLMODEM_PHONEBOOK_H__