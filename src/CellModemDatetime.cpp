#include "CellModemDatetime.h"

#ifdef __AVR__
    #include <avr/pgmspace.h>
    static const uint8_t daysInMonth [] PROGMEM = { 31,28,31,30,31,30,31,31,30,31,30,31 };
#else
    static const uint8_t daysInMonth [] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
#endif


CellModemDatetime::CellModemDatetime(const char * cclk) {
    if(*cclk == '\"') cclk++;
    _yOff = _conv2d(cclk);
    cclk += 3;
    _m = _conv2d(cclk) - 1;                // Month is 0 based
    cclk += 3;
    _d = _conv2d(cclk) - 1;                // Day is 0 based
    cclk += 3;
    _hh = _conv2d(cclk);
    cclk += 3;
    _mm = _conv2d(cclk);
    cclk += 3;
    _ss = _conv2d(cclk);
    cclk += 2;
    uint8_t isNeg = *cclk == '-';
    ++cclk;
    _tz = _conv2d(cclk);
    if (isNeg) {
        _tz = -_tz;
    }
}


uint32_t CellModemDatetime::getY2KEpoch() const
{
  uint32_t ts;
  uint16_t days = _d + (365 * _yOff) + ((_yOff + 3) / 4);
  // Add the days of the previous months in this year.
  for (uint8_t i = 0; i < _m; ++i) {
    #ifdef __AVR__
        days += pgm_read_byte(daysInMonth + i);
    #else
        days += daysInMonth[i];
    #endif
  }

  if ((_m > 2) && ((_yOff % 4) == 0)) {
    ++days;
  }

  ts = ((uint32_t)days * 24) + _hh;
  ts = (ts * 60) + _mm;
  ts = (ts * 60) + _ss;

  ts = ts - (_tz * 15 * 60);

  return ts;
}

/*
 * \brief Compute the UNIX Epoch from the date and time
 */
uint32_t CellModemDatetime::getUnixEpoch() const
{
  return getY2KEpoch() + 946684800;
}

/*
 * \brief Convert a single digit to a number
 */
uint8_t CellModemDatetime::_conv1d(const char * txt)
{
  uint8_t val = 0;
  if (*txt >= '0' && *txt <= '9') {
    val = *txt - '0';
  }
  return val;
}

/*
 * \brief Convert two digits to a number
 */
uint8_t CellModemDatetime::_conv2d(const char * txt)
{
  uint8_t val = _conv1d(txt) * 10;
  txt++;
  val += _conv1d(txt);
  return val;
}
