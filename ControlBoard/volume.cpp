#include "volume.h"
#include "sensor_manager.h"
#include "flow_sensors.h"
#include "Ventilation.h"

Volume::volume_data_t Volume::volumes[SENSOR_NUM_FLOW];
float Volume::total_volume = 0;
float Volume::total_comp_volume = 0;

void Volume::new_flow_readings() {
    static uint32_t last_t = millis();
    static bool last_ins = false;

    bool ins_change = (Ventilation::is_ins() != last_ins);
    last_ins = Ventilation::is_ins();

    uint32_t dt = millis() - last_t;
    last_t = millis();

    total_volume = 0;
    total_comp_volume = 0;
    for (uint8_t i = 0; i < SENSOR_NUM_FLOW; i++) {
        if (ins_change) {
            if (last_ins) {
                const float v_diff = volumes[i].max_vol_ins - volumes[i].min_vol_esp;
                volumes[i].ins_volume_mult = v_diff / volumes[i].max_vol_ins;
                volumes[i].volume = 0;
                volumes[i].comp_volume = 0;
                volumes[i].max_vol_ins = 0;
            } else {
                volumes[i].min_vol_esp = volumes[i].volume;
            }
        }

        float dv = FlowSensors::get_flow(i) * ((float)dt) / 60.0f;
        volumes[i].volume += dv;
        if (last_ins) {
            volumes[i].comp_volume = volumes[i].volume * volumes[i].ins_volume_mult;
        } else {
            volumes[i].comp_volume = volumes[i].volume + volumes[i].max_vol_ins * (volumes[i].ins_volume_mult - 1.0f);
        }
        volumes[i].comp_volume = max(0, volumes[i].comp_volume);

        volumes[i].max_vol_ins = max(volumes[i].max_vol_ins, volumes[i].volume);
        volumes[i].min_vol_esp = min(volumes[i].min_vol_esp, volumes[i].volume);

        total_volume += volumes[i].volume;
        total_comp_volume += volumes[i].comp_volume;
        FlowSensors::get_flow(i);
    }
}

float Volume::get_volume(uint8_t i) {
    return volumes[i].comp_volume;
}

float Volume::get_total_volume() {
    return total_comp_volume;
}