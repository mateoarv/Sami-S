#ifndef BUFFCIRC_H
#define BUFFCIRC_H
#include "Arduino.h"

template <class T>
class BuffCirc {
   private:
    T* buff;
    uint16_t i = 0, j = 0, max_len, len = 0;
    void imas();
    void jmas();
    void imen();
    void jmen();

   public:
    BuffCirc() : buff(NULL) {}
    BuffCirc(T* buff, uint16_t len);
    void set_buff(T* _buff, uint16_t len);
    void add_end(T val);
    void add_start(T val);
    T pop_start();
    T pop_end();
    T read_start();
    T read_end();
    T read_at(uint16_t pos);
    uint16_t get_len();
    bool is_full();
    void fill(T val);
    void empty();
};

#endif