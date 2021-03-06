#include "SocketBuffer.h"

SocketBuffer::SocketBuffer(const size_t size) 
    :
    _count(0),
    _size(size)    
{
    
}

void SocketBuffer::init() {
    if(_initialized) return;

    _buffer = (uint8_t *)malloc(_size * sizeof(uint8_t));
    _bufferEnd = _buffer + _size - 1;
    _head = _tail = _buffer;

    _initialized = true;
}

size_t SocketBuffer::count() const {
    return _count;
}

size_t SocketBuffer::size() const {
    return _size;
}

void SocketBuffer::clear() {
    Serial.println("Clear");
    memset(_buffer, 0, _size - 1);
    _head = _buffer;
    _tail = _buffer;
}

// uint8_t & SocketBuffer::operator[](const size_t index) {
//     if(index > _count - 1) return;


// }


void SocketBuffer::write(uint8_t c) {
    if(_count > 0) {
        _incPtr(&_tail);
    }
    
    *_tail = c;
    ++_count;

    if(_count > _size) {
        _count = _size;
        _incPtr(&_head);
    }

}

uint8_t SocketBuffer::read() {
    if(_count == 0) return 0;

    uint8_t c = *_head;

    --_count;

    if(_count > 0) {
        _incPtr(&_head);
    }
    
    return c;
}

uint8_t SocketBuffer::peek() {
    if(_count == 0) return 0;

    return *_head;
}


void SocketBuffer::_incPtr(uint8_t ** ptr) {
    ++(*ptr);

    if(*ptr > _bufferEnd) {
        *ptr = _buffer;
    }
}

void SocketBuffer::_decPtr(uint8_t ** ptr) {
    --(*ptr);

    if(*ptr < _buffer) {
        *ptr = _bufferEnd;
    }
}