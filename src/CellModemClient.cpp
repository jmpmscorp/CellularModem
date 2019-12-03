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
    _socketBuffer.init();
    _socketBuffer.clear();

    uint8_t oldSocketId = _socketId;

    

    if(socketConnect(host, port, &_socketId, _ssl, 120000) == 1) {
        _socketConnected = true;
        return 1;
    }

    _socketConnected = false;

    return 0;
}

size_t CellModemClient::write(uint8_t c) {    
    return write(&c, 1);
}

size_t CellModemClient::write(const uint8_t *buf, size_t size) {
    //Serial.println("Call Write!");
    /*if(_socketAvailable > 0) {
        socketRead(_socketAvailable > 1024 ? 1024 : _socketAvailable, _socketId);
    }*/
    
    socketWrite(buf, size, _socketId);
    return size;
}

int CellModemClient::available() {
    // _socketAvailable = socketAvailable(_socketId);
    return _socketBuffer.count();
}

int CellModemClient::read() {
    uint8_t c;

    if(read(&c, 1) == 1) {
        return c;
    }

    return -1;
}

int CellModemClient::read(uint8_t *buf, size_t size) {
    int readCount = 0;
    size_t i = 0;

    /*uint16_t aux = socketAvailable(_socketId);
    if(aux > 0) {
        socketRead(_socketAvailable, _socketId);
    }*/
    _modem->poll(250);
    size_t lenRemaining = size;

    while(_socketBuffer.count() > 0 && lenRemaining > 0) {
        // *(buf + readCount) = _socketBuffer.read();
        uint8_t c = _socketBuffer.read();
        *(buf + readCount) = c;
        readCount++;
        lenRemaining--;
    }

    return readCount;
}

int CellModemClient::peek() {
    return _socketBuffer.peek();
}

void CellModemClient::flush() {
    return;
}

void CellModemClient::stop() {
    Serial.println("Stop Call!");
    _socketBuffer.clear();

    _socketConnected = false;
    
    socketDisconnect(_socketId);
}

uint8_t CellModemClient::connected() {
    return _socketConnected;
}