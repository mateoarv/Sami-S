#include "Arduino.h"
#include "buff_circ.h"
#include "error.h"

template <class T>
BuffCirc<T>::BuffCirc(T* buff, uint16_t len) {
    set_buff(buff, len);
}

template <class T>
void BuffCirc<T>::set_buff(T* _buff, uint16_t len) {
    buff = _buff;
    max_len = len;
    len = 0;
    i = 0;
    j = 0;
}

template <class T>
void BuffCirc<T>::fill(T val) {
    for (uint16_t i = 0; i < max_len; i++) {
        add_start(val);
    }
}

template <class T>
void BuffCirc<T>::empty() {
    len = 0;
    i = 0;
    j = 0;
}

template <class T>
void BuffCirc<T>::add_start(T val) {
    assert(buff);
    if (len > 0) {
        imen();
    }
    buff[i] = val;
    len = min(len + 1, max_len);
}

// Agrega y sobreescribe
template <class T>
void BuffCirc<T>::add_end(T val) {
    assert(buff);
    if (len > 0) {
        jmas();
    }
    buff[j] = val;
    len = min(len + 1, max_len);
}

template <class T>
T BuffCirc<T>::pop_start() {
    assert(buff);
    T val = 0;
    if (len > 0) {
        val = buff[i];
        len--;
        if (len > 0) {
            imas();
        }
    }
    return val;
}

template <class T>
T BuffCirc<T>::pop_end() {
    assert(buff);
    T val = 0;
    if (len > 0) {
        val = buff[j];
        len--;
        if (len > 0) {
            jmen();
        }
    }
    return val;
}

template <class T>
T BuffCirc<T>::read_start() {
    return len > 0 ? buff[i] : 0;
}

template <class T>
T BuffCirc<T>::read_end() {
    return len > 0 ? buff[j] : 0;
}

template <class T>
void BuffCirc<T>::jmas() {
    j++;
    if (j >= max_len) {
        j = 0;
    }
    if (i == j) {
        pop_start();
    }
}

template <class T>
void BuffCirc<T>::imas() {
    i++;
    if (i >= max_len) {
        i = 0;
    }
}

template <class T>
void BuffCirc<T>::imen() {
    i = i > 0 ? i - 1 : max_len - 1;
    if (i == j) {
        pop_end();
    }
}

template <class T>
void BuffCirc<T>::jmen() {
    j = j > 0 ? j - 1 : max_len - 1;
}

template <class T>
T BuffCirc<T>::read_at(uint16_t pos) {
    assert(buff);
    if (pos < len) {
        uint16_t z = i + pos;
        if (z >= max_len) {
            z -= max_len;
        }
        return buff[z];
    } else {
        return read_end();
    }
}

template <class T>
uint16_t BuffCirc<T>::get_len() {
    return len;
}

template <class T>
bool BuffCirc<T>::is_full() {
    return len == max_len;
}

template class BuffCirc<float>;
template class BuffCirc<unsigned long>;
template class BuffCirc<uint8_t>;