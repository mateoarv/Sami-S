#ifndef VOLUME_H
#define VOLUME_H
#include "Arduino.h"
#include "global_config.h"

class Volume {
   public:
    static void new_flow_readings();
    static float get_volume(uint8_t i);
    static float get_total_volume();

   private:
    typedef struct {
        float volume;
        float comp_volume;
        float max_vol_ins;
        float min_vol_esp;
        float ins_volume_mult;
    } volume_data_t;
    static volume_data_t volumes[SENSOR_NUM_FLOW];
    static float total_volume;
    static float total_comp_volume;
};

#endif