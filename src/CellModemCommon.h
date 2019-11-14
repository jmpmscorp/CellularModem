#pragma once

#ifndef __CELLMODEM_COMMON__
#define __CELLMODEM_COMMON__

#ifdef CELLMODEM_USE_FREERTOS
    #include "FreeRTOS_SAMD21.h"
#endif


enum class ATResponse : uint8_t {
    ResponseNotFound,
    ResponseOK,
    ResponseError,
    ResponsePrompt,
    ResponseTimeout,
    ResponseEmpty,
    ResponseMultilineParser,
    UrcHandled
};

enum class NetworkRegistrationStatus : uint8_t {
    NotRegistered,
    Registered,
    Searching,
    Denied,
    Unknown,
    Roaming
};

enum class SIMStatus : uint8_t {
    Unknown,
    Missing,
    NeedsPin,
    Ready
};

enum class NetworkTechnology : uint8_t {
    GSM,
    GSM_COMPACT,
    UTRAN
};

#ifndef __AVR__
    #define sscanf_P sscanf
#endif

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

typedef void( *delayFnPtr )(unsigned long milliseconds);
typedef unsigned long ( *millisFnPtr ) ();
typedef ATResponse ( * ResponseParserCallbackPtr ) (ATResponse &response, const char * buffer, size_t size, void * param1, void * param2);

typedef struct SafeCharBufferPtr
{
    char * bufferPtr;
    size_t size;
}SafeCharBufferPtr_t;


typedef struct {
    int8_t dtr;
    int8_t dcd;
    int8_t cts;
    int8_t rts;
    int8_t ri;
}UartPins_t;

static inline bool isTimedout(uint32_t from, uint32_t nr_ms) __attribute__((always_inline));

#ifdef CELLMODEM_USE_FREERTOS
    static inline bool isTimedout(uint32_t from, uint32_t nr_ms) {
        uint32_t ms_now = (uint32_t)( xTaskGetTickCount() * portTICK_PERIOD_MS);
        return ms_now - from > nr_ms;
        // return  (uint32_t)( xTaskGetTickCount() * portTICK_PERIOD_MS ) - from  > nr_ms;
    }
#else
    static inline bool isTimedout(uint32_t from, uint32_t nr_ms){
        return (millis() - from) > nr_ms;
    }
#endif

#endif // __CELLMODEM_COMMON__