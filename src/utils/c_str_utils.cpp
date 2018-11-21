#include "c_str_utils.h"

bool c_str_startWith(const char * source, const char * str) {
    if(source == nullptr || str == nullptr) {
        return -1;
    }
    return strncmp(source, str, strlen(str)) == 0;
}

bool c_str_endsWith(const char * source, const char * str) {
    if(source == nullptr || str == nullptr) {
        return -1;
    }

    size_t strLength = strlen(str);
    size_t sourceLength = strlen(source);

    return strncmp(source + sourceLength - strLength, str, strLength) == 0; 
}

void c_str_trim(char * source) {
    if(source == nullptr) {
        return;
    }

    const char * firstNonSpace = source;

    while (firstNonSpace != nullptr && isspace(*firstNonSpace)) {
        ++firstNonSpace;
    }    

    size_t length = strlen(firstNonSpace) + 1;

    memmove(source, firstNonSpace, length);

    char * endPtr = source + strlen(source);

    while(source < endPtr && isspace(*endPtr)) {
        --endPtr;
    }

    *endPtr = '\0';
}

#if defined(__AVR__)
bool c_str_startWith_P(const char * source, const char * str) {
    if(source == nullptr || str == nullptr) {
        return -1;
    }
    
    /*Serial.print("Starts: ");
    Serial.println(strlen(source));
    Serial.println((strncmp_P(source, str, strlen_P(str)) == 0));*/
    return strncmp_P(source, str, strlen_P(str)) == 0;
}

bool c_str_endsWith_P(const char * source, const char * str) {
    if(source == nullptr || str == nullptr) {
        return -1;
    }

    size_t strLength = strlen_P(str);
    size_t sourceLength = strlen(source);

    return strncmp_P(source + sourceLength - strLength, str, strLength) == 0; 
}
#endif