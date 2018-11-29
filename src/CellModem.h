#pragma once

#ifndef __CELL_MODEM_H__
#define __CELL_MODEM_H__

#include <Arduino.h>
#include <Stream.h>
#include "stdint.h"
#include "utils/c_str_utils.h"
#include "CellModemCommon.h"
#include "CellModemUrcHandler.h"


#if defined(__AVR__)
    #include <avr/pgmspace.h>
    static const char AT_STR[] PROGMEM = "AT";
    static const char OK_RESPONSE[] PROGMEM = "OK";
    static const char ERROR_RESPONSE[] PROGMEM ="ERROR";
    static const char CME_ERROR_RESPONSE[] PROGMEM = "+CME ERROR:";
    static const char CMS_ERROR_RESPONSE[] PROGMEM = "+CMS ERROR:";
#else
    static const char AT_STR[] = "AT";
    static const char OK_RESPONSE[] = "OK";
    static const char ERROR_RESPONSE[] ="ERROR";
    static const char CME_ERROR_RESPONSE[] = "+CME ERROR:";
    static const char CMS_ERROR_RESPONSE[] = "+CMS ERROR:";
#endif

#define CELLMODEM_TERMINATOR "\r\n"
#define CELLMODEM_TERMINATOR_LENGTH (sizeof(CELLMODEM_TERMINATOR) - 1)       //Length of CRLF without '\0' terminator

#define CELLMODEM_DEFAULT_RESPONSE_BUFFER_SIZE 256
#define CELLMODEM_DEFAULT_TIMEOUT_MS 5000

#define CELLMODEM_MAX_URC_HANDLERS 10


class CellModem {
    friend class CellModemSMS;
    public:
        
        CellModem(Stream &serial, int8_t onOffPin, int8_t statusPin, int8_t dtrPin, int8_t ctsPin);

        virtual void init();

        virtual bool on();
        virtual bool isOn() const;
        virtual bool off() { return true; }
        
        virtual bool reset() { return true; }

        virtual bool networkOn(bool enableAutoregistration = true);
        virtual bool networkOn(const char * pin, bool enableAutoregistration = true);
        virtual bool isNetworkRegistered();
        virtual NetworkRegistrationStatus getNetworkRegistrationStatus();

        virtual bool setSIMPin(const char * pin);
        virtual SIMStatus getSIMStatus();

        virtual bool getOperatorName(char * buffer, size_t size);
        virtual bool getSignalQuality(int8_t * rssi, uint8_t * ber);        

        virtual bool getDatetime(char * buffer, size_t size);
        virtual bool enableDatetimeNetworkSync() = 0;
        virtual bool disableDatetimeNetworkSync() = 0;

        bool isAlive(uint16_t timeout = 300);
        void setMinRSSI(int8_t minSignalRSSI);

        ATResponse poll(const char * buffer, size_t size, uint32_t timeout);
        ATResponse poll(uint32_t timeout = 250);   

        void addUrcHandler(CellModemUrcHandler * urcHandler);
        void removeUrcHandler(CellModemUrcHandler * urcHandler);    

        char * getResponseBuffer() const;
        void setCustomDelay(delayFnPtr delayFn);

        template<typename T>
        void sendData(T arg) {
            _serial->print(arg);
        }

        template<typename T, typename... Args>
        void sendData(T first, Args... last) {
            _serial->print(first);            
            sendData(last...);
        }

        template<typename... Args>
        void sendATCommand(Args... cmd) {
            sendData(cmd...);
            _serial->println();
            _serial->flush();
        }

        ATResponse readResponse(char* buffer, size_t size, size_t* outSize, uint32_t timeout = CELLMODEM_DEFAULT_TIMEOUT_MS)
        {
            return readResponse(_responseBuffer, _responseBufferSize, NULL, NULL, NULL, outSize, timeout);
        };

        ATResponse readResponse(char* buffer, size_t size,
                ResponseParserCallbackPtr parserMethod, void* callbackParameter, void* callbackParameter2 = NULL,
                size_t* outSize = NULL, uint32_t timeout = CELLMODEM_DEFAULT_TIMEOUT_MS);
        
        ATResponse readResponse(size_t* outSize = NULL, uint32_t timeout = CELLMODEM_DEFAULT_TIMEOUT_MS)
        {
            return readResponse(_responseBuffer, _responseBufferSize, NULL, NULL, NULL, outSize, timeout);
        };

        ATResponse readResponse(ResponseParserCallbackPtr parserMethod, void* callbackParameter,
            void* callbackParameter2 = NULL, size_t* outSize = NULL, uint32_t timeout = CELLMODEM_DEFAULT_TIMEOUT_MS)
        {
            return readResponse(_responseBuffer, _responseBufferSize,
                    parserMethod, callbackParameter, callbackParameter2,
                    outSize, timeout);
        };

        template<typename T1, typename T2>
        ATResponse readResponse(ATResponse (*parserMethod)(ATResponse& response, const char* parseBuffer, size_t size, T1* param1, T2* param2),
            T1* callbackParameter, T2* callbackParameter2,
            size_t* outSize = NULL, uint32_t timeout = CELLMODEM_DEFAULT_TIMEOUT_MS)
        {
            return readResponse(_responseBuffer, _responseBufferSize, (ResponseParserCallbackPtr)parserMethod, 
                (void*)callbackParameter, (void*)callbackParameter2, outSize, timeout);
        };

            
        
        size_t readLine();
        size_t readLine(uint32_t);
        size_t readLine(char * buffer, size_t length, uint32_t timeout = 1000);
        
    protected:
        int readByte(uint32_t timeout = 1000) const;
        size_t readBytes(uint8_t * buffer, size_t length, uint32_t timeout = 1000);
        size_t readBytesUntil(char terminator, char * buffer, size_t length, uint32_t timeout = 1000);
        
        virtual bool _enableAutoregistrationNetwork(uint32_t timeout = 4*60*1000);   // 4 minutes

        delayFnPtr _modemDelay = delay;
        unsigned long _lastResponseOrURCMillis;
        
        uint16_t _responseBufferSize = CELLMODEM_DEFAULT_RESPONSE_BUFFER_SIZE;
        char * _responseBuffer;
        
        Stream * _serial;
        int8_t _dtrPin = -1;
        int8_t _ctsPin = -1;
        int8_t _onOffPin = -1;
        int8_t _statusPin = -1;

        int8_t _minRSSI = -93;
    
    private:
        void _initResponseBuffer();

        virtual bool _sendInitializationCommands();
        virtual int8_t _getAutoregistrationNetworkMode();        
        virtual bool _waitForSignalQuality(uint32_t timeout = 60 * 1000);  // 60 seconds
        

        bool _isResponseBufferInitialized = false;

        CellModemUrcHandler * _urcHandlers[CELLMODEM_MAX_URC_HANDLERS];
        bool _onOffStatus = false;


        static ATResponse _copsParser(ATResponse &response, const char * buffer, size_t size, char * operatorNameBuffer, size_t * operatorNameBufferSize);
        static ATResponse _copsParser(ATResponse &response, const char * buffer, size_t size, unsigned int * mode, unsigned int * networkTechnology);
        static ATResponse _cpinParser(ATResponse& response, const char * buffer, size_t size, SIMStatus * simStatusResult, uint8_t * dummy);
        static ATResponse _csqParser(ATResponse& response, const char* buffer, size_t size, int * rssi, int * ber);
        static ATResponse _cregParser(ATResponse &response, const char* buffer, size_t size, unsigned int * networkRegistrationStatus, uint8_t * dummy);
        static ATResponse _cclkParser(ATResponse &response, const char * buffer, size_t size, char * cclkBuffer, size_t * cclkBufferSize);
};

#endif // __CELL_MODEM_H__