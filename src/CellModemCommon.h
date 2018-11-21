#pragma once

#ifndef __CELLMODEM_COMMON__
#define __CELLMODEM_COMMON__

enum class ATResponse : uint8_t {
    ResponseNotFound,
    ResponseOK,
    ResponseError,
    ResponsePrompt,
    ResponseTimeout,
    ResponseEmpty
};

typedef void( *delayFnPtr )(unsigned long milliseconds);
typedef ATResponse ( * ResponseParserCallbackPtr ) (ATResponse &response, const char * buffer, size_t size, void * param1, void * param2);

#endif // __CELLMODEM_COMMON__