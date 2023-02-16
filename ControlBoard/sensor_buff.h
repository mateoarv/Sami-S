#ifndef SENSOR_BUFF_H
#define SENSOR_BUFF_H
#include "Arduino.h"
#include "buff_circ.h"

template <class T>
class SensorBuff {
   public:
    SensorBuff() {}
    SensorBuff(uint16_t len);
    ~SensorBuff();

    void feed(T val);
    T get_reading();
    T get_reading(uint16_t offset);
    void empty();
    void allocate(uint16_t len);
    void deallocate();
    bool is_filled();

   private:
    bool allocated = false;
    T* raw_buff;
    BuffCirc<T> buff_circ = BuffCirc<T>();

    void reallocate(uint16_t len);
};

#endif