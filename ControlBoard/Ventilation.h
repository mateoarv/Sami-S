#ifndef VENTILATION_H
#define VENTILATION_H
#include "Arduino.h"
#include "MediaMovil.h"

#define AB_FILTER_LEN 6

class Ventilation {
   public:
    static void register_ins();
    static void register_esp();
    static bool is_ins();
    static float get_delta_ab_O2();
    static float get_delta_ab_CO2();

   private:
    static float delta_ab_O2;
    static float delta_ab_CO2;
    static float buff_filtro_ab_O2[AB_FILTER_LEN];
    static float buff_filtro_ab_CO2[AB_FILTER_LEN];
    static MediaMovil<float> filtro_ab_O2;
    static MediaMovil<float> filtro_ab_CO2;
    static bool ins;
};

#endif