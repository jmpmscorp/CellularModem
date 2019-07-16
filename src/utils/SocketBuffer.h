#pragma once

#include <Arduino.h>

class SocketBuffer {
    public:
        SocketBuffer(const size_t size);

        size_t count() const;
        size_t size() const;
        void clear();
        //uint8_t & operator[](const size_t index);

        void write(uint8_t c);
        uint8_t read();
        uint8_t peek();
    
    private:
        uint8_t * _head;
        uint8_t * _tail;
        size_t _count;
        const size_t _size;
        uint8_t * _buffer;
        uint8_t * _bufferEnd;

        void _incPtr(uint8_t * ptr);
        void _decPtr(uint8_t * ptr);


};