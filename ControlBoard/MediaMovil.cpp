#include "MediaMovil.h"
template <class T>
MediaMovil<T>::MediaMovil(T *buff, uint8_t len) {
    buffCirc = new BuffCirc<T>(buff, len);
    // BuffCirc<T> _buffCirc(buff, len);
    // buffCirc = &_buffCirc;
}

template <class T>
float MediaMovil<T>::agregarVal(T val) {
    suma += val;
    if (buffCirc->is_full()) {
        suma -= buffCirc->pop_start();
    }
    buffCirc->add_end(val);
    if (buffCirc->get_len() != 0) {
        prom = ((float)suma) / buffCirc->get_len();
    }
    return prom;
}

template <class T>
void MediaMovil<T>::fill(T val) {
    for (uint8_t i = 0; i < buffCirc->get_len(); i++) {
        agregarVal(val);
    }
}

template <class T>
uint8_t MediaMovil<T>::getLen() {
    return buffCirc->get_len();
}

template <class T>
float MediaMovil<T>::getMedia() {
    return prom;
}

template class MediaMovil<float>;