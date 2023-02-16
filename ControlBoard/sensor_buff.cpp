#include "sensor_buff.h"
#include "error.h"

template <class T>
SensorBuff<T>::SensorBuff(uint16_t len) {
    allocate(len);
}

template <class T>
void SensorBuff<T>::feed(T val) {
    assert(allocated);
    buff_circ.add_end(val);
}

template <class T>
T SensorBuff<T>::get_reading() {
    assert(allocated);
    return buff_circ.read_start();
}

template <class T>
T SensorBuff<T>::get_reading(uint16_t offset) {
    assert(allocated);
    return buff_circ.read_at(offset);
}

template <class T>
void SensorBuff<T>::empty() {
    assert(allocated);
    buff_circ.empty();
}

template <class T>
bool SensorBuff<T>::is_filled() {
    assert(allocated);
    return buff_circ.is_full();
}

template <class T>
void SensorBuff<T>::allocate(uint16_t len) {
    if (allocated) {
        reallocate(len);
        return;
    } else {
        raw_buff = (T*)calloc(len, sizeof(T));
        assert(raw_buff);
        buff_circ.set_buff(raw_buff, len);
        allocated = true;
    }
}

template <class T>
void SensorBuff<T>::reallocate(uint16_t len) {
    deallocate();
    allocate(len);
    buff_circ.set_buff(raw_buff, len);
}

template <class T>
void SensorBuff<T>::deallocate() {
    assert(allocated);
    free(raw_buff);
    buff_circ.set_buff(NULL, 0);
    allocated = false;
}

template <class T>
SensorBuff<T>::~SensorBuff() {
    if (allocated) {
        deallocate();
    }
}

template class SensorBuff<float>;