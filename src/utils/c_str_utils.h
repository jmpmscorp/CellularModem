#pragma once

#ifndef __C_STR_UTILS__
#define __C_STR_UTILS__

#if defined(ARDUINO)
    #include <Arduino.h>
#endif

#include "stdint.h"
#include "ctype.h"
#include "string.h"

#ifndef __AVR__
    #define c_str_startWith_P c_str_startWith
    #define c_str_endsWith_P c_str_endsWith
#endif 

bool c_str_startWith(const char * source, const char * str);
bool c_str_endsWith(const char * source, const char * str);
void c_str_trim(char * source);

#ifdef __AVR__
    bool c_str_startWith_P(const char * source, const char * str);
    bool c_str_endsWith_P(const char * source, const char * str);
#endif
    
#endif // __C_STR_UTILS__