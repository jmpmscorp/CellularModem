#pragma once

#ifndef __CELLMODEM_DATETIME_H__
#define __CELLMODEM_DATETIME_H__

#include "inttypes.h"


class CellModemDatetime {
    public:
        CellModemDatetime(const char * cclkStr);

        uint16_t year() { return _yOff + 2000; }
        uint8_t month() { return _m + 1; }
        uint8_t day() { return _d + 1; }
        uint8_t hour() { return _hh; }
        uint8_t minute() { return _mm; }
        uint8_t second() { return _ss; }
        uint8_t timezone() { return _tz; }

        // 32-bit number of seconds since Unix epoch (1970-01-01)
        uint32_t getUnixEpoch() const;
        // 32-bit number of seconds since Y2K epoch (2000-01-01)
        uint32_t getY2KEpoch() const;

    private:
        uint8_t _conv1d(const char * txt);
        uint8_t _conv2d(const char * txt);

        uint8_t _yOff;          // Year value minus 2000
        uint8_t _m;             // month (0..11)
        uint8_t _d;             // day (0..30)
        uint8_t _hh;            // hour (0..23)
        uint8_t _mm;            // minute (0..59)
        uint8_t _ss;            // second (0..59)
        int8_t _tz;            // timezone (multiple of 15 minutes)
};


#endif  // __CELLMODEM_DATETIME_H__