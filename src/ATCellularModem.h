#pragma once

#ifndef __AT_CELLULAR_MODEM_H__
#define __AT_CELLULAR_MODEM_H__

#include <Stream.h>
#include "stdint.h"
#include "ModemURCHandler.h"
#include "utils/c_str_utils.h"

#if defined(__AVR__)
    #include <avr/pgmspace.h>
    const char CRLF[] PROGMEM = "\r\n";
    const char OK_RESPONSE[] PROGMEM = "OK\r\n";
    const char ERROR_RESPONSE[] PROGMEM ="ERROR\r\n";
#else
    static const char CRLF[]= "\r\n";
    static const char OK_RESPONSE[] = "OK\r\n";
    static const char ERROR_RESPONSE[] ="ERROR\r\n";
#endif

#include <Arduino.h>

typedef void( *delayFnPtr )(unsigned long milliseconds);

class ATCellularModem {
    public:
        ATCellularModem(Uart &serial, int8_t dtrPin, int8_t resetPin);

        
        virtual int on() = 0;
        virtual int off() = 0;
        virtual int reset() = 0;

        virtual bool isAlive(uint16_t timeout = 300);

        int8_t ready();
        void poll();       

        void setCustomDelay(delayFnPtr delayFn);

        template<typename T>
        void sendData(T arg) {
            //Serial.print(arg);
            _serial->print(arg);
            //SerialGSM.print(arg);
        }

        template<typename T, typename... Args>
        void sendData(T first, Args... last) {
            //Serial.print(first);
            _serial->print(first);
            //SerialGSM.print(first);
            sendData(last...);
        }

        template<typename... Args>
        void sendATCommand(Args... cmd) {
            sendData(cmd..., CRLF);
            _serial->flush();
            _ready = 0;
        }

        int8_t waitForResponse(uint16_t timeout = 100);
        int8_t waitForPrompt(uint16_t timeout = 500);

        void addURCHandler(ModemURCHandler * handler);
        void removeURCHandler(ModemURCHandler * halder);
        
    protected:
        void beginModem(uint32_t baudrate);
        delayFnPtr _delay = delay;
        unsigned long _lastResponseOrURCMillis;
    char _responseBuffer[64];
        

        //char _responseBuffer[64];

        Uart * _serial;
        int8_t _dtrPin = -1;
        int8_t _resetPin = -1;
        int8_t _ready = 1;
    
    private:
        void _clearResponseBuffer();

        
        uint8_t _bytePos = 0;

        enum {
            AT_COMMAND_IDLE,
            AT_RECEIVING_RESPONSE
        } _atCommandState = AT_COMMAND_IDLE;

        #ifndef MAX_URC_HANDLERS
            #define MAX_URC_HANDLERS 10
        #endif

        ModemURCHandler * _urcHandlers[MAX_URC_HANDLERS];
};

#endif // __CELLULAR_MODEM_H__