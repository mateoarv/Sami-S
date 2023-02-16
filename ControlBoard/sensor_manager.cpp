#include "sensor_manager.h"
#include "HighPressureSensors.h"
#include "flow_sensors.h"
#include "utils.h"
#include "volume.h"
#include "Ventilation.h"

uint32_t SensorManager::gas_period = 50;
uint32_t SensorManager::lp_period = 50;
uint32_t SensorManager::temp_period = 5000;
uint32_t SensorManager::hp_period = 50;

// SensorManager::sensor_t SensorManager::sensors[(uint8_t)sensor_e::SENSOR_NUM];
SensorManager::sensor_t SensorManager::gas_sensors[GAS_SENSOR_NUM];
SensorManager::sensor_t SensorManager::flow_sensors[SENSOR_NUM_FLOW];
SensorManager::sensor_t SensorManager::hp_sensors[SENSOR_NUM_HP];
SensorManager::sensor_t SensorManager::total_flow;
SensorManager::sensor_t SensorManager::volumes[SENSOR_NUM_FLOW];
SensorManager::sensor_t SensorManager::total_volume;

bool SensorManager::_buffers_filled = false;
uint16_t SensorManager::delay_offset = 0;

sensor_delays_t SensorManager::delays = {
    .pressure = 0,
    .gas = {
        1701,  // IN
        4362,  // D1
        4566,  // D2
        3821,  // I1
        3836,  // I2
        6779,  // OUT
    },
};

void SensorManager::init() {
    const uint16_t extra = 400;
    // Determinar retrasos
    uint16_t* p = (uint16_t*)&delays;
    uint16_t max_delay = 0;
    for (uint8_t i = 0; i < 7; i++) {
        if (i) {
            p[i] -= extra;
        }
        max_delay = max(max_delay, p[i]);
    }
    printlnfd(Serial, "Max delay: %I16u", max_delay);

    //  Inicializar sensores
    for (uint8_t i = 0; i < SENSOR_NUM_HP; i++) {
        init_sensor_struct(hp_sensors[i], hp_period, delays.pressure, max_delay);
    }
    for (uint8_t i = 0; i < SENSOR_NUM_FLOW; i++) {
        init_sensor_struct(flow_sensors[i], lp_period, delays.pressure, max_delay);
        init_sensor_struct(volumes[i], lp_period, delays.pressure, max_delay);
    }
    init_sensor_struct(total_flow, lp_period, delays.pressure, max_delay);
    init_sensor_struct(total_volume, lp_period, delays.pressure, max_delay);
    for (uint8_t i = 0; i < GAS_SENSOR_NUM; i++) {
        init_sensor_struct(gas_sensors[i], gas_period, delays.gas[i / 2], max_delay);
    }
    // init_sensor_struct(gas_sensors[(uint8_t)gas_sensor_e::O2_IN], gas_period, 4300);
    // init_sensor_struct(gas_sensors[(uint8_t)gas_sensor_e::CO2_IN], gas_period, 4300);
}

/*float SensorManager::get_reading(sensor_e sensor) {
    assert(sensor != sensor_e::SENSOR_NUM);
    return sensors[(uint8_t)sensor].sensor_buff.get_reading();
}*/

float SensorManager::get_gas_reading(gas_sensor_e sensor) {
    return gas_sensors[(uint8_t)sensor].sensor_buff.get_reading(delay_offset);
}

float SensorManager::get_hp_reading(uint8_t i) {
    return hp_sensors[i].sensor_buff.get_reading(delay_offset);
}

float SensorManager::get_flow_reading(uint8_t i) {
    return flow_sensors[i].sensor_buff.get_reading(delay_offset);
}

float SensorManager::get_total_flow_reading() {
    return total_flow.sensor_buff.get_reading();
}

float SensorManager::get_volume_reading(uint8_t i) {
    return volumes[i].sensor_buff.get_reading();
}
float SensorManager::get_total_volume_reading() {
    return total_volume.sensor_buff.get_reading();
}

void SensorManager::process(void* obj) {
    static uint32_t max_dt = 0;
    const float flow_thr = 2.0f;
    uint32_t t = 0;
    uint32_t t1 = Utils::tick();

    /*static uint32_t t_gas = t;
    static uint32_t t_flow = t;
    static uint32_t t_temp = t;
    static uint32_t t_high_press = t;*/

    // HP sensors
    t = millis();
    for (uint8_t i = 0; i < SENSOR_NUM_HP; i++) {
        if (t - hp_sensors[i].last_read_t >= hp_sensors[i].period) {
            hp_sensors[i].sensor_buff.feed(HighPressureSensors::read_pressure(i));
            hp_sensors[i].last_read_t = t;
        }
    }
    // Flow and volume sensors
    t = millis();
    float _total_flow = 0;
    bool flow_read = false;
    for (uint8_t i = 0; i < SENSOR_NUM_FLOW; i++) {
        if (t - flow_sensors[i].last_read_t >= flow_sensors[i].period) {
            flow_read = true;
            float flow = FlowSensors::read_flow(i);
            flow_sensors[i].sensor_buff.feed(flow);
            flow_sensors[i].last_read_t = t;
            _total_flow += flow;
        }
    }
    if (flow_read) {
        total_flow.sensor_buff.feed(_total_flow);
        total_flow.last_read_t = t;
        if (!Ventilation::is_ins()) {
            if (_total_flow >= flow_thr) {
                Ventilation::register_ins();
            }
        } else {
            if (_total_flow <= (-flow_thr)) {
                Ventilation::register_esp();
            }
        }
        Volume::new_flow_readings();
        total_volume.sensor_buff.feed(Volume::get_total_volume());
        total_volume.last_read_t = t;
        for (uint8_t i = 0; i < SENSOR_NUM_FLOW; i++) {
            volumes[i].sensor_buff.feed(Volume::get_volume(i));
            volumes[i].last_read_t = t;
        }
    }

    // Gas sensors TODO: Organizar
    t = millis();
    if (t - gas_sensors[0].last_read_t >= gas_sensors[0].period) {
        GasSensors::read_all();
        for (uint8_t i = 0; i < GAS_SENSOR_NUM; i++) {
            gas_sensors[i].sensor_buff.feed(GasSensors::get_reading((gas_sensor_e)i));
            gas_sensors[i].last_read_t = t;
        }
    }
    uint32_t dt = Utils::tock(t1, false);
    if (dt > max_dt) {
        max_dt = dt;
        printlnfd(Serial, "Delta: %I32u", max_dt);
    }
    /*if (t - t_gas >= gas_period) {
    GasSensors::read_all();
    for (uint8_t i = 0; i < GAS_SENSOR_NUM; i++) {
        // gas_sensors[i].gas_sensors[i].feed(GasSensors::get_reading((gas_sensor_e)i));
    }
    t_gas = millis();
    }
    if (t - t_flow >= flow_period) {
        t_flow = millis();
    }
    if (t - t_temp >= temp_period) {
        t_temp = millis();
    }
    if (t - t_high_press >= high_press_period) {
        t_high_press = millis();
    }*/
}

void SensorManager::set_delay_offset(uint16_t offset) {
    delay_offset = offset;
}

// TODO
bool SensorManager::buffers_filled() {
    return _buffers_filled;
}

void SensorManager::init_sensor_struct(sensor_t& sensor, uint32_t period, uint16_t del, uint16_t max_del) {
    del = (max_del - del);
    sensor.del = del;
    sensor.period = period;
    sensor.last_read_t = 0;
    const uint16_t samples = max(1, (uint16_t)round(((float)del) / ((float)period)));
    sensor.sensor_buff.allocate(samples);
    // printlnfd(Serial, "Samples: %I16u", samples);
}