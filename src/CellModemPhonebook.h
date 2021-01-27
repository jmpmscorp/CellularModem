#pragma once

#ifndef __CELLMODEM_PHONEBOOK_H__
#define __CELLMODEM_PHONEBOOK_H__

#include "CellModem.h"

class CellModemPhonebook {
    public:
        explicit CellModemPhonebook(CellModem &modem);

        bool addContact(const char * phoneNumber, const char * contactName = nullptr, uint8_t index = 0);
        bool readContact(uint8_t index, char * numberBuffer, char * contactNameBuffer);
        int searchContacts(const char * toFindStr, unsigned int * indexesBuffer, size_t indexesBufferSize, unsigned int * totalOccurences = nullptr);
        bool removeContact(uint8_t index);
    
    protected:
        CellModem *_modem;

        static ATResponse _cpbrParser(ATResponse &response, const char * buffer, size_t size, char * numberBuffer, char * contactNameBuffer);
        static ATResponse _cpbfParser(ATResponse &response, const char * buffer, size_t size, unsigned int * indexesBuffer, int * freeBufferSize);
};


#endif // __CELLMODEM_PHONEBOOK_H__