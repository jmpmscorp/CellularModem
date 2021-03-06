#include "CellModem.h"

#define debugPrintLn(...)                             \
    {                                                 \
        if (this->_debugSerial)                       \
            this->_debugSerial->println(__VA_ARGS__); \
    }
#define debugPrint(...)                             \
    {                                               \
        if (this->_debugSerial)                     \
            this->_debugSerial->print(__VA_ARGS__); \
    }

CellModem::CellModem(Stream &serial, int8_t onOffPin, int8_t resetPin, int8_t statusPin) : _serial(&serial),
                                                                                           _onOffPin(onOffPin),
                                                                                           _resetPin(resetPin),
                                                                                           _statusPin(statusPin)
{
}

void CellModem::init()
{
    if (_onOffPin > -1)
    {
        pinMode(_onOffPin, OUTPUT);
        digitalWrite(_onOffPin, LOW);
    }

    if (_resetPin > -1)
    {
        pinMode(_resetPin, OUTPUT);
        digitalWrite(_resetPin, LOW);
    }

    if (_statusPin > -1)
    {
        pinMode(_statusPin, INPUT_PULLUP);
    }

    _initResponseBuffer();
}

void CellModem::_initResponseBuffer()
{
    if (!_isResponseBufferInitialized)
    {
        _responseBuffer = static_cast<char *>(malloc(_responseBufferSize));

        _isResponseBufferInitialized = true;
    }
}

bool CellModem::on()
{
    if (isAlive(200))
    {
        return true;
    }

    if (!isOn())
    {
        if (_onOffPin > -1)
        {
            digitalWrite(_onOffPin, HIGH);
        }

        _onOffStatus = true;
    }

    bool timeout = true;

    for (uint8_t i = 0; i < 10; i++)
    {
        if (isAlive(500))
        {
            timeout = false;
            break;
        }
    }

    if (timeout)
    {
        return false;
    }

    return true;
}

bool CellModem::off()
{
    if (_onOffPin > 0)
    {
        digitalWrite(_onOffPin, LOW);
        _modemDelay(1000);
    }

    return true;
}

bool CellModem::forceReset()
{
    if (_resetPin > -1)
    {
        digitalWrite(_resetPin, HIGH);
        _modemDelay(1000);
        digitalWrite(_resetPin, LOW);
    }
    else if (_onOffPin > -1)
    {
        digitalWrite(_onOffPin, LOW);
        _modemDelay(1000);
        digitalWrite(_onOffPin, HIGH);
    }

    bool timeout = true;
    for (uint8_t i = 0; i < 10; i++)
    {
        if (isAlive(500))
        {
            timeout = false;
            break;
        }
    }

    if (timeout)
    {
        return false;
    }

    return true;
}

bool CellModem::isOn() const
{
    if (_statusPin > -1)
    {
        return static_cast<bool>(digitalRead(_statusPin));
    }
    else
    {
        return _onOffStatus;
    }
}

bool CellModem::reset()
{
    sendATCommand(F("AT+CFUN=16"));

    return readResponse() == ATResponse::ResponseOK;
}

void CellModem::setUartPins(UartPins_t uartPins)
{
    _uartPins.dtr = uartPins.dtr;
    _uartPins.dcd = uartPins.dcd;
    _uartPins.cts = uartPins.cts;
    _uartPins.rts = uartPins.rts;
    _uartPins.ri = uartPins.ri;

    if (_uartPins.dtr > -1)
    {
        pinMode(_uartPins.dtr, OUTPUT);
    }

    if (_uartPins.dcd > -1)
    {
        pinMode(_uartPins.dcd, INPUT_PULLUP);
    }

    if (_uartPins.cts > -1)
    {
        pinMode(_uartPins.cts, INPUT_PULLUP);
    }

    if (_uartPins.rts > -1)
    {
        pinMode(_uartPins.rts, OUTPUT);
    }

    if (_uartPins.ri > -1)
    {
        pinMode(_uartPins.ri, INPUT_PULLUP);
    }
}

UartPins_t CellModem::getUartPins() const
{
    return _uartPins;
}

uint8_t CellModem::getActiveLowPowerMode() const
{
    return _lowPowerMode;
}

/*******************************************************************
 * **************   NETWORK FUNCTIONS ******************************
 * *****************************************************************/
bool CellModem::isAlive(uint16_t timeout)
{
    sendATCommand(F("AT"));
    if (readResponse(nullptr, 1000) == ATResponse::ResponseOK)
    {
        return true;
    }
    return false;
}

bool CellModem::_initializationProcess()
{
    sendATCommand(F("AT+CMEE=2"));

    if (readResponse() != ATResponse::ResponseOK)
    {
        return false;
    }

    return true;
}

bool CellModem::networkOn(bool enableAutoregistration)
{
    return networkOn(nullptr, enableAutoregistration);
}

bool CellModem::networkOn(const char *pin, bool enableAutoregistration)
{
    if (!isOn())
    {
        on();
    }

    _initializationProcess();

    //poll(10000);

    switch (getSIMStatus())
    {
    case SIMStatus::Ready:
    {
        break;
    }
    case SIMStatus::NeedsPin:
    {
        if (!setSIMPin(pin))
        {
            return false;
        }
        if (getSIMStatus() != SIMStatus::Ready)
        {
            return false;
        }
    }
    case SIMStatus::Missing:
    case SIMStatus::Unknown:
    default:
        return false;
    }

    if (enableAutoregistration)
    {
        if (!_enableAutoregistrationNetwork())
        {
            return false;
        }
    }

    if (!_waitForSignalQuality())
    {
        return false;
    }

    return true;
}

int8_t CellModem::_getAutoregistrationNetworkMode()
{
    sendATCommand(F("AT+COPS?"));

    unsigned int mode;
    if (readResponse<unsigned int, unsigned int>(_copsParser, &mode, NULL) != ATResponse::ResponseOK)
    {
        return -1;
    }

    return static_cast<int8_t>(mode);
}

bool CellModem::_enableAutoregistrationNetwork(uint32_t timeout)
{
    if (_getAutoregistrationNetworkMode() == 0)
    {
        return true;
    }

    sendATCommand(F("AT+COPS=0"));

    if (readResponse(NULL, 120000) == ATResponse::ResponseOK)
    {
        return true;
    }

    return false;
}

bool CellModem::getOperatorName(char *buffer, size_t size)
{
    if (buffer == nullptr)
    {
        return false;
    }

    if (size > 0)
    {
        *buffer = '\0';
    }
    else
    {
        return false;
    }

    sendATCommand(F("AT+COPS?"));

    if (readResponse<char, size_t>(_copsParser, buffer, &size) != ATResponse::ResponseOK)
    {
        return false;
    }

    return (buffer[0] != '\0');
}

ATResponse CellModem::_copsParser(ATResponse &response, const char *buffer, size_t size,
                                  char *operatorNameBuffer, size_t *operatorNameBufferSize)
{
    if (!operatorNameBuffer || !operatorNameBufferSize)
    {
        return ATResponse::ResponseError;
    }

    // TODO maybe limit length of string in format? needs converting int to string though
    if (sscanf_P(buffer, PSTR("+COPS: %*d,%*d,\"%[^\"]\",%*d"), operatorNameBuffer) == 1)
    {
        return ATResponse::ResponseEmpty;
    }
    if (sscanf_P(buffer, "+COPS: %*d,%*d,\"%[^\"]\"", operatorNameBuffer) == 1)
    {
        return ATResponse::ResponseEmpty;
    }
    int dummy;
    if (sscanf_P(buffer, "+COPS: %d", &dummy) == 1)
    {
        return ATResponse::ResponseEmpty;
    }

    return ATResponse::ResponseError;
}

ATResponse CellModem::_copsParser(ATResponse &response, const char *buffer, size_t size,
                                  unsigned int *mode, unsigned int *networkTechnology)
{
    if (!networkTechnology && !mode)
    {
        return ATResponse::ResponseError;
    }

    if (!networkTechnology)
    {
        if (sscanf_P(buffer, PSTR("+COPS: %u"), mode) == 1 ||
            sscanf_P(buffer, PSTR("+COPS: %d,%*d,\"%*[^\"]\",%*d"), mode) == 1)
        {
            return ATResponse::ResponseEmpty;
        }
        else
        {
            return ATResponse::ResponseError;
        }
    }

    if (sscanf_P(buffer, PSTR("+COPS: %u,%*d,\"%*[^\"]\",%u"), mode, networkTechnology) == 2)
    {
        return ATResponse::ResponseEmpty;
    }

    return ATResponse::ResponseError;
}

bool CellModem::isNetworkRegistered()
{
    NetworkRegistrationStatus status = getNetworkRegistrationStatus();

    if (status == NetworkRegistrationStatus::Registered || status == NetworkRegistrationStatus::Roaming)
    {
        return true;
    }

    return false;
}

NetworkRegistrationStatus CellModem::getNetworkRegistrationStatus()
{
    sendATCommand(F("AT+CREG?"));

    unsigned int status;
    if (readResponse<unsigned int, uint8_t>(_cregParser, &status, NULL) == ATResponse::ResponseOK)
    {
        switch (status)
        {
        case 0:
            return NetworkRegistrationStatus::NotRegistered;
        case 1:
            return NetworkRegistrationStatus::Registered;
        case 2:
            return NetworkRegistrationStatus::Searching;
        case 3:
            return NetworkRegistrationStatus::Denied;
        case 5:
            return NetworkRegistrationStatus::Roaming;
        case 4:
        default:
            return NetworkRegistrationStatus::Unknown;
        }
    }

    return NetworkRegistrationStatus::Unknown;
}

ATResponse CellModem::_cregParser(ATResponse &response, const char *buffer, size_t size,
                                  unsigned int *networkRegistrationStatus, uint8_t *dummy)
{
    if (!networkRegistrationStatus)
    {
        return ATResponse::ResponseError;
    }

    if (sscanf_P(buffer, PSTR("+CREG: %*u,%u"), networkRegistrationStatus) == 1)
    {
        return ATResponse::ResponseEmpty;
    }

    return ATResponse::ResponseError;
}

bool CellModem::_waitForSignalQuality(uint32_t timeout)
{
    uint32_t start = _modemMillis();

    int8_t rssi;
    uint8_t ber;

    uint8_t delayCount = 1;
    while (!isTimedout(start, timeout))
    {
        if (getSignalQuality(&rssi, &ber))
        {
            if (rssi != 0 && rssi >= _minRSSI)
            {
                return true;
            }
        }

        _modemDelay(delayCount * 1000);

        if (delayCount < 5)
        {
            ++delayCount;
        }
    }

    return false;
}

bool CellModem::getSignalQuality(int8_t *rssi, uint8_t *ber)
{
    sendATCommand(F("AT+CSQ"));

    int csqRaw = 0;
    int berRaw = 0;

    if (readResponse<int, int>(_csqParser, &csqRaw, &berRaw) == ATResponse::ResponseOK)
    {
        *rssi = (csqRaw == 99) ? 0 : static_cast<int8_t>(-113 + 2 * csqRaw);
        *ber = (berRaw == 99) ? 0 : static_cast<uint8_t>(berRaw);

        return true;
    }

    return false;
}

ATResponse CellModem::_csqParser(ATResponse &response, const char *buffer, size_t size, int *rssi, int *ber)
{
    if (!rssi || !ber)
    {
        return ATResponse::ResponseError;
    }

    if (sscanf_P(buffer, PSTR("+CSQ: %d,%d"), rssi, ber) == 2)
    {
        return ATResponse::ResponseEmpty;
    }

    return ATResponse::ResponseError;
}

SIMStatus CellModem::getSIMStatus()
{
    SIMStatus simStatus;
    uint8_t retries = 0;

    do
    {
        if (retries > 0)
        {
            _modemDelay(250);
        }

        sendATCommand("AT+CPIN?");
        if (readResponse<SIMStatus, uint8_t>(_cpinParser, &simStatus, NULL) == ATResponse::ResponseOK)
        {
            return simStatus;
        }

        ++retries;
    } while (retries < 5);

    return SIMStatus::Missing;
}

bool CellModem::setSIMPin(const char *pin)
{
    if (!pin)
    {
        return false;
    }

    sendATCommand(F("AT+CPIN=\""), pin, F("\""));

    if (readResponse() == ATResponse::ResponseOK)
    {
        return true;
    }
    else
    {
        return false;
    }
}

ATResponse CellModem::_cpinParser(ATResponse &response, const char *buffer, size_t size, SIMStatus *simStatus, uint8_t *dummy)
{
    if (!simStatus)
    {
        return ATResponse::ResponseError;
    }

    char status[16];
    if (sscanf_P(buffer, PSTR("+CPIN: %" STR(sizeof(status) - 1) "s"), status) == 1)
    {
        if (c_str_startWith("READY", status))
        {
            *simStatus = SIMStatus::Ready;
        }
        else
        {
            *simStatus = SIMStatus::NeedsPin;
        }

        return ATResponse::ResponseEmpty;
    }

    return ATResponse::ResponseError;
}

bool CellModem::getDatetime(char *buffer, size_t size)
{
    sendATCommand(F("AT+CCLK?"));

    size_t auxSize = size;
    if (readResponse<char, size_t>(_cclkParser, buffer, &auxSize) == ATResponse::ResponseOK)
    {
        return true;
    }

    return false;
}

/**
 * Reponse is +CCLK: "yy/MM/dd,hh:mm:ss+TZ"
 * Maximum buffer size needed is 21 = 20 + '\0'
 * */
ATResponse CellModem::_cclkParser(ATResponse &response, const char *buffer, size_t size, char *cclkBuffer, size_t *cclkBufferSize)
{
    if (!cclkBuffer)
    {
        return ATResponse::ResponseError;
    }

    if (strncmp_P(buffer, PSTR("+CCLK:"), 6) == 0)
    {
        char *ptr = strchr(buffer, '"');
        ++ptr;
        strncpy(cclkBuffer, ptr, *cclkBufferSize >= 20 ? 20 : *cclkBufferSize);
        if (*cclkBufferSize >= 21)
        {
            cclkBuffer[20] = '\0';
        }
        else
        {
            cclkBuffer[*cclkBufferSize - 1] = '\0';
        }

        return ATResponse::ResponseEmpty;
    }

    return ATResponse::ResponseError;
}

void CellModem::setMinRSSI(int8_t minRSSI)
{
    _minRSSI = minRSSI;
}

/*******************************************************************
 * **************   INTERNAL UTILITY FUNCTIONS *********************
 * *****************************************************************/
void CellModem::setDebugSerial(Stream &serial)
{
    _debugSerial = &serial;
}

char *CellModem::getResponseBuffer() const
{
    return _responseBuffer;
}

void CellModem::setCustomMillis(millisFnPtr millisFn)
{
    if (millisFn)
    {
        _modemMillis = millisFn;
    }
}

millisFnPtr CellModem::getCustomMillis()
{
    return _modemMillis;
}

void CellModem::setCustomDelay(delayFnPtr delayFn)
{
    if (delayFn)
    {
        _modemDelay = delayFn;
    }
}

delayFnPtr CellModem::getCustomDelay() const
{
    return _modemDelay;
}

void CellModem::addUrcHandler(CellModemUrcHandler *urcHandler)
{
    uint8_t i = 0;

    while (i < CELLMODEM_MAX_URC_HANDLERS)
    {
        if (_urcHandlers[i] == nullptr)
        {
            _urcHandlers[i] = urcHandler;
            return;
        }
        ++i;
    }
}

void CellModem::removeUrcHandler(CellModemUrcHandler *urcHandler)
{
    uint8_t i = 0;

    while (i < CELLMODEM_MAX_URC_HANDLERS)
    {
        if (_urcHandlers[i] == urcHandler)
        {
            _urcHandlers[i] = nullptr;
            return;
        }
        ++i;
    }
}

ATResponse CellModem::poll(uint32_t timeout)
{
    return readResponse(NULL, timeout);
}

ATResponse CellModem::readResponse(char *buffer, size_t size,
                                   ResponseParserCallbackPtr parserCallback, void *callbackParam1, void *callbackParam2,
                                   size_t *outSize, uint32_t timeout)
{
    ATResponse response = ATResponse::ResponseNotFound;
    uint32_t start = _modemMillis();
    do
    {
        int count = readLine(buffer, size, 250);

        if (count < 0)
        {
            continue;
        }

        if (outSize)
        {
            *outSize = count;
        }

        for (size_t i = 0; i < CELLMODEM_MAX_URC_HANDLERS; ++i)
        {
            if (_urcHandlers[i] != nullptr)
            {
                ATResponse res = _urcHandlers[i]->handleUrcs();
                if (res == ATResponse::UrcHandled)
                {
                    continue;
                }
            }
        }

        if (c_str_startWith_P(buffer, AT_STR))
        {
            continue;
        }

        if (c_str_startWith_P(buffer, OK_RESPONSE))
        {
            return ATResponse::ResponseOK;
        }

        if (c_str_startWith_P(buffer, ERROR_RESPONSE) ||
            c_str_startWith_P(_responseBuffer, CME_ERROR_RESPONSE) ||
            c_str_startWith_P(_responseBuffer, CME_ERROR_RESPONSE))
        {
            return ATResponse::ResponseError;
        }

        if (c_str_startWith_P(buffer, SOCKET_PROMPT_RESPONSE) ||
            c_str_startWith_P(buffer, SMS_PROMPT_RESPONSE) ||
            c_str_startWith_P(buffer, FILE_PROMPT_RESPONSE))
        {
            return ATResponse::ResponsePrompt;
        }

        if (parserCallback)
        {
            ATResponse callbackResponse = parserCallback(response, buffer, count, callbackParam1, callbackParam2);
            if (callbackResponse != ATResponse::ResponseEmpty && callbackResponse != ATResponse::ResponseMultilineParser && callbackResponse != ATResponse::ResponseNotFound)
            {
                return callbackResponse;
            }

            if (callbackResponse != ATResponse::ResponseMultilineParser && callbackResponse != ATResponse::ResponseNotFound)
            {
                parserCallback = nullptr;
            }
        }

        if (response != ATResponse::ResponseNotFound && response != ATResponse::ResponseMultilineParser)
        {
            return response;
        }
        _modemDelay(10);

    } while (!isTimedout(start, timeout));

    if (outSize)
    {
        *outSize = 0;
    }

    return ATResponse::ResponseTimeout;
}

// Returns a character from the modem stream if read within _timeout ms or -1 otherwise.
int CellModem::readByte(uint32_t timeout) const
{
    int c;
    uint32_t _startMillis = _modemMillis();
    do
    {
        c = _serial->read();
        if (c >= 0)
        {
            debugPrint((char)c);
            return c;
        }

    } while (_modemMillis() - _startMillis < timeout);

    return -1; // -1 indicates timeout
}

// Fills the given "buffer" with characters read from the modem stream up to "length"
// maximum characters and until the "terminator" character is found or a character read
// times out (whichever happens first).
// The buffer does not contain the "terminator" character or a null terminator explicitly.
// Returns the number of characters written to the buffer, not including null terminator.
size_t CellModem::readBytesUntil(char terminator, char *buffer, size_t length, uint32_t timeout)
{
    if (length < 1)
    {
        return 0;
    }

    size_t index = 0;

    while (index < length)
    {
        int c = readByte(timeout);

        if (c < 0 || c == terminator)
        {
            break;
        }

        *buffer++ = static_cast<char>(c);
        index++;
    }

    if (index < length)
    {
        *buffer = '\0';
    }

    // TODO distinguise timeout from empty string?
    // TODO return error for overflow?
    return index; // return number of characters, not including null terminator
}

// Fills the given "buffer" with up to "length" characters read from the modem stream.
// It stops when a character read times out or "length" characters have been read.
// Returns the number of characters written to the buffer.
size_t CellModem::readBytes(uint8_t *buffer, size_t length, uint32_t timeout)
{
    size_t count = 0;

    while (count < length)
    {
        int c = readByte(timeout);

        if (c < 0)
        {
            break;
        }

        *buffer++ = static_cast<uint8_t>(c);
        count++;
    }

    // TODO distinguise timeout from empty string?
    // TODO return error for overflow?
    return count;
}

// Reads a line (up to the SODAQ_GSM_TERMINATOR) from the modem stream into the "buffer".
// The buffer is terminated with null.
// Returns the number of bytes read, not including the null terminator.
size_t CellModem::readLine(char *buffer, size_t size, uint32_t timeout)
{
    // Use size-1 to leave room for a string terminator

    size_t len = readBytesUntil(CELLMODEM_TERMINATOR[CELLMODEM_TERMINATOR_LENGTH - 1], buffer, size - 1, timeout);

    // check if the terminator is more than 1 characters, then check if the first character of it exists
    // in the calculated position and terminate the string there
    if ((CELLMODEM_TERMINATOR_LENGTH > 1) && (buffer[len - (CELLMODEM_TERMINATOR_LENGTH - 1)] == CELLMODEM_TERMINATOR[0]))
    {
        len -= CELLMODEM_TERMINATOR_LENGTH - 1;
    }

    // terminate string, there should always be room for it (see size-1 above)
    buffer[len] = '\0';

    return len;
}

size_t CellModem::readLine()
{
    return readLine(_responseBuffer, _responseBufferSize);
}

size_t CellModem::readLine(uint32_t timeout)
{
    return readLine(_responseBuffer, _responseBufferSize, timeout);
}