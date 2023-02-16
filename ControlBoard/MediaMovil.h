#ifndef MEDIAMOVIL_H
#define MEDIAMOVIL_H
#include <stdint.h>
#include "buff_circ.h"

template <class T>
class MediaMovil {
   private:
    BuffCirc<T> *buffCirc;
    T suma = 0;
    float prom;

   public:
    MediaMovil(T *buff, uint8_t len);
    float agregarVal(T val);
    float getMedia();
    void fill(T val);
    uint8_t getLen();
};

#endif