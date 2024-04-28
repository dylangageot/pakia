#pragma once

template <typename T, int SIZE> struct circular_buffer {

    bool read(T *read_value) {
        if (_head != _tail) {
            *read_value = *_head;
            ++_head;
            _head = (_head == (_buffer + SIZE)) ? _buffer : _head;
            return true;
        } else {
            // buffer is empty
            return false;
        }
    }

    bool write(T *value_to_write) {
        T* future_tail = _tail + 1;
        future_tail = (future_tail == (_buffer + SIZE)) ? _buffer : future_tail;
        if (future_tail == _head) {
            *future_tail = *value_to_write;
            _tail = future_tail;
            return true;
        } else {
            // buffer is full
            return false;
        }
    }

  private:
    T _buffer[SIZE];
    T* _tail = _buffer;
    T* _head = _buffer;
};