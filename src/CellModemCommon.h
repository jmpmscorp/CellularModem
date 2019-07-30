#pragma once

#ifndef __CELLMODEM_COMMON__
#define __CELLMODEM_COMMON__

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
typedef ATResponse ( * ResponseParserCallbackPtr ) (ATResponse &response, const char * buffer, size_t size, void * param1, void * param2);

typedef struct SafeCharBufferPtr
{
    char * bufferPtr;
    size_t size;
}SafeCharBufferPtr_t;


static inline bool isTimedout(uint32_t from, uint32_t nr_ms) __attribute__((always_inline));
static inline bool isTimedout(uint32_t from, uint32_t nr_ms)
{
    return (millis() - from) > nr_ms;
}

#endif // __CELLMODEM_COMMON__