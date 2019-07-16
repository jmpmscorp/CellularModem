#include "CellModemClient.h"

CellModemClient::CellModemClient(CellModem& modem, bool ssl):
    _modem(&modem),
    _ssl(ssl)
{

}

CellModemClient::~CellModemClient() {}


int CellModemClient::connect(IPAddress ip, uint16_t port) {
    return -1;
}

int CellModemClient::connect(const char *host, uint16_t port) {
    _socketBuffer.clear();

    uint8_t oldSocketId = _socketId;

    if(socketConnect(host, port, &_socketId, _ssl, 120000) == 0) {
        _socketConnected = true;
        return 0;
    }

    _socketConnected = false;

    return 1;
    /*_modem->sendATCommand(F("AT+USOCR=6"));

    if(_modem->readResponse<uint8_t, uint8_t>(_usocrParser, &_socketId, nullptr, nullptr, 20000) != ATResponse::ResponseOK) {
        return false;
    }

    _modem->sendATCommand(F("AT+USOCO="), _socketId, ",\"", host, "\",", port);

    if(_modem->readResponse() != ATResponse::ResponseOK ) {
        return false;
    }

    return true;
    */
}

size_t CellModemClient::write(uint8_t c) {    
    return write(&c, 1);
}

size_t CellModemClient::write(const uint8_t *buf, size_t size) {
    //Serial.println("Call Write!");
    if(_socketAvailable > 0) {
        socketRead(_socketAvailable > 1024 ? 1024 : _socketAvailable, _socketId);
    }
    socketWrite(buf, size, _socketId);
    return size;
}

int CellModemClient::available() {
    Serial.println("Call Available");
    _socketAvailable = socketAvailable(_socketId);
    Serial.print("Socket Buffer Count:");
    Serial.println(_socketBuffer.count());
    Serial.print("Socket Available: ");
    Serial.println(_socketAvailable);
    return _socketBuffer.count() + _socketAvailable;
}

int CellModemClient::read() {
    uint8_t c;

    if(read(&c, 1) == 1) {
        return c;
    }

    return -1;
}

int CellModemClient::read(uint8_t *buf, size_t size) {
    //wakeUpXbee();
    int readCount = 0;
    size_t i = 0;
    Serial.println("Call Read buf");
    Serial.print("Size: ");
    Serial.println(size);
    socketRead(size, _socketId);

    return readCount;
}

int CellModemClient::peek() {
    return _socketBuffer.peek();
}

void CellModemClient::flush() {
    return;
}

void CellModemClient::stop() {
    _socketBuffer.clear();

    _socketConnected = false;
    
    socketDisconnect(_socketId);
    /* _modem->sendATCommand(F("AT+USOCL="), _socketId);

    _modem->readResponse();
    */
}

uint8_t CellModemClient::connected() {
    return _socketConnected;
}