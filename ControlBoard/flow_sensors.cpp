#include "flow_sensors.h"
#include "PressureSensors.h"
#include "utils.h"
#include "error.h"
#include "processes.h"
#include "storage.h"
#include "UI.h"
#include "Ventilation.h"
FlowSensors::flow_sensor_t FlowSensors::sensors[FLOW_N_SENSORS] = {};
uint16_t FlowSensors::sample_period = 20;
sensor_cal_t FlowSensors::def_cal = {
    {0.00f, 46.52f, 142.94f, 251.70f, 362.48f, 511.02f, 1326.00f, 2221.60f, 3086.86f, 4030.06f, 5993.76f, 8716.94f, 11319.84f, 13043.80f, 14560.74f, 15530.54f, 0, 0, 0},
    {0.00f, 2.05f, 4.03f, 6.09f, 8.10f, 10.03f, 20.14f, 30.41f, 40.25f, 50.75f, 69.20f, 90.57f, 110.50f, 130.60f, 151.30f, 169.90f, 0, 0, 0},
    -1.24f};
float FlowSensors::cal_suggested_vals[FLOW_CALIB_SAMPLES] =
    {0, 2, 4, 6, 8, 10, 15, 20, 25, 30, 35, 40, 45, 55, 65, 75, 85, 100, 120};

bool FlowSensors::global_enabled = true;

bool FlowSensors::cal_rutine_running = false;
uint8_t FlowSensors::cal_rutine_sample_i = 0;
uint8_t FlowSensors::cal_rutine_sensor_i = 0;
uint32_t FlowSensors::cal_rutine_time = 0;
float FlowSensors::cal_measurement = 0;
bool FlowSensors::cal_measurement_ready = false;

float FlowSensors::total_flow = 0;
float FlowSensors::total_volume = 0;
float FlowSensors::total_comp_volume = 0;
bool FlowSensors::ins = false;

void FlowSensors::init() {
    // Leer calibración
    for (uint8_t i = 0; i < FLOW_N_SENSORS; i++) {
        Storage::read_flow_cal(i, sensors[i].cal);
        printfd(Serial, "%u: ", i);
        print_sensor_cal(i);
        sensors[i].ins_volume_mult = 1;
        sensors[i].max_vol_ins = -1000;
        sensors[i].min_vol_esp = 1000;
    }
}

float FlowSensors::read_flow(uint8_t sensor_i) {
    const float flow = interpolate(
        sensors[sensor_i].cal.in_vals,
        sensors[sensor_i].cal.out_vals,
        FLOW_CALIB_SAMPLES,
        (float)PressureSensors::read_pressure_raw(sensor_i),
        true);
    sensors[sensor_i].last_val = flow;
    return flow;
}

/*void FlowSensors::process(void* obj) {
    static uint32_t t0 = millis();
    static bool has_ins = false;
    static bool has_esp = false;
    const float flow_thr = 2.0f;
    if (!global_enabled) {
        return;
    }
    if (millis() - t0 >= sample_period) {
        // Utils::tock(t0, "Sensor process dt: ");
        total_flow = 0;
        total_volume = 0;
        total_comp_volume = 0;
        for (uint8_t i = 0; i < FLOW_N_SENSORS; i++) {
            if (sensors[i].enabled) {
                float p = (float)PressureSensors::read_pressure_raw(i);
                uint32_t new_t = millis();

                float new_val = interpolate(
                    sensors[i].cal.in_vals, sensors[i].cal.out_vals, FLOW_CALIB_SAMPLES, p, true);

                if (!sensors[i].ins) {  // Detectar ins
                    if (new_val >= flow_thr) {
                        has_ins = true;
                        sensors[i].ins = true;
                        if (has_ins && has_esp) {
                            // const float real_max_vol_ins = sensors[i].max_vol_ins / sensors[i].ins_volume_mult;
                            const float v_diff = sensors[i].max_vol_ins - sensors[i].min_vol_esp;
                            sensors[i].ins_volume_mult = v_diff / sensors[i].max_vol_ins;
                            // printlnfd(Serial, "min %.2f max %.2f mult %.2f", sensors[i].min_vol_esp,
                            // sensors[i].max_vol_ins, sensors[i].ins_volume_mult);
                            sensors[i].volume = 0;
                            sensors[i].comp_volume = 0;
                            sensors[i].max_vol_ins = 0;
                        }
                    }
                } else {  // Detectar esp
                    if (new_val <= (-flow_thr)) {
                        has_esp = true;
                        sensors[i].ins = false;
                        if (has_ins && has_esp) {
                            sensors[i].min_vol_esp = sensors[i].volume;
                        }
                    }
                }

                sensors[i].last_val = new_val;
                total_flow += new_val;
                float dv = (new_val / 60.0f) * (new_t - sensors[i].last_t);
                // sensors[i].volume += sensors[i].ins ? dv * sensors[i].ins_volume_mult : dv;
                sensors[i].volume += dv;
                if (sensors[i].ins) {
                    sensors[i].comp_volume = sensors[i].volume * sensors[i].ins_volume_mult;
                } else {
                    sensors[i].comp_volume = sensors[i].volume + sensors[i].max_vol_ins * (sensors[i].ins_volume_mult - 1.0f);
                    // sensors[i].comp_volume = sensors[i].volume - sensors[i].min_vol_esp;
                }
                sensors[i].comp_volume = max(0, sensors[i].comp_volume);

                sensors[i].max_vol_ins = max(sensors[i].max_vol_ins, sensors[i].volume);
                sensors[i].min_vol_esp = min(sensors[i].min_vol_esp, sensors[i].volume);
                total_volume += sensors[i].volume;
                total_comp_volume += sensors[i].comp_volume;

                sensors[i].last_t = new_t;
            }
            if (!ins) {
                if (total_flow >= flow_thr) {
                    ins = true;
                    Ventilation::register_ins();
                }
            } else {
                if (total_flow <= (-flow_thr)) {
                    ins = false;
                    Ventilation::register_esp();
                }
            }
        }
        t0 = millis();
    }
}*/

void FlowSensors::start_cal_rutine(uint8_t sensor_i, uint32_t time) {
    assert(!cal_rutine_running);

    cal_rutine_sample_i = 0;
    cal_rutine_running = true;
    cal_rutine_sensor_i = sensor_i;
    cal_rutine_time = time;
    cal_measurement = 0;
    cal_measurement_ready = false;
    printlnfd(Serial, "Flow calibration rutine started, send %u calibration values:", FLOW_CALIB_SAMPLES);
    print_suggested_val(0);
}

void FlowSensors::start_cal_measurement() {
    assert(cal_rutine_running);
    cal_measurement = get_raw_avg(cal_rutine_sensor_i, cal_rutine_time);
    cal_measurement_ready = true;
    printlnfd(Serial, "Cal measurement: %.2f", cal_measurement);
    UI::buzz_loud(3, 100);
}

void FlowSensors::set_measurement_out(float out_val) {
    assert(cal_rutine_running);
    assert(cal_measurement_ready);
    cal_measurement_ready = false;
    // float in_val = (float)get_raw_avg(cal_rutine_sensor_i, cal_rutine_time);
    sensor_cal_t& cal = sensors[cal_rutine_sensor_i].cal;
    if (out_val == 0) {
        cal.zero_offset = cal_measurement;
        printlnfd(Serial, "Zero offset set: %.2f", cal_measurement);
    }
    cal_measurement -= cal.zero_offset;
    cal.in_vals[cal_rutine_sample_i] = cal_measurement;
    cal.out_vals[cal_rutine_sample_i] = out_val;
    cal_rutine_sample_i++;
    if (cal_rutine_sample_i >= FLOW_CALIB_SAMPLES) {
        cal_rutine_running = false;
        printlnd("Flow calibration rutine finished:");
        printd("Out vals: ");
        for (uint8_t i = 0; i < FLOW_CALIB_SAMPLES; i++) {
            printlnfd(Serial, "%.2f ", cal.out_vals[i]);
        }
        printlnd();
        printd("In vals: ");
        for (uint8_t i = 0; i < FLOW_CALIB_SAMPLES; i++) {
            printlnfd(Serial, "%.2f ", cal.in_vals[i]);
        }
        printlnfd(Serial, "Zero offset: %.2f", cal.zero_offset);
        printlnd();
    } else {
        printlnfd(Serial, "Calibration value added,Out: %.2f In: %.2f %u remaining",
                  out_val, cal_measurement, FLOW_CALIB_SAMPLES - cal_rutine_sample_i);
        print_suggested_val(cal_rutine_sample_i);
    }
}

float FlowSensors::get_raw_avg(uint8_t sensor_i, uint32_t time) {
    printlnd("Reading flow avg...");
    int16_t max_p = 0;
    int16_t min_p = 0;
    uint32_t t0 = millis();
    float sum = 0;
    uint16_t n = 0;
    max_p = PressureSensors::read_pressure_raw(sensor_i);
    min_p = max_p;
    while (millis() - t0 < time) {
        // const uint32_t t0 = Utils::tick();
        Processes::delay(20);
        // Utils::tock(t0, "Raw avg dt: ");
        //  delay(20);
        int16_t p = PressureSensors::read_pressure_raw(sensor_i);
        max_p = max(max_p, p);
        min_p = min(min_p, p);
        sum += (float)p;
        n++;
    }
    printlnfd(Serial, "Max: %i Min: %i Diff: %i", max_p, min_p, max_p - min_p);
    return sum / n;
}

void FlowSensors::restore_def_cal() {
    for (uint8_t i = 0; i < FLOW_N_SENSORS; i++) {
        restore_def_cal(i);
    }
}

void FlowSensors::restore_def_cal(uint8_t sensor_i) {
    sensors[sensor_i].cal = def_cal;
    printlnfd(Serial, "Flow sensor #%u default calibration restored.", sensor_i);
}

void FlowSensors::save_sensor_cal() {
    for (uint8_t i = 0; i < FLOW_N_SENSORS; i++) {
        save_sensor_cal(i);
    }
}

void FlowSensors::save_sensor_cal(uint8_t sensor_i) {
    Storage::write_flow_cal(sensor_i, sensors[sensor_i].cal);
    printlnfd(Serial, "Flow sensor #%u calibration saved.", sensor_i);
}

void FlowSensors::print_sensor_cal(uint8_t sensor_i) {
    const sensor_cal_t& cal = sensors[sensor_i].cal;
    printlnfd(Serial, "\nSensor #%u calibration values: (Out In)", sensor_i);
    for (uint8_t i = 0; i < FLOW_CALIB_SAMPLES; i++) {
        printlnfd(Serial, "%.2f  %.2f", cal.out_vals[i], cal.in_vals[i]);
    }
    printlnfd(Serial, "Zero Offset: %.2f", cal.zero_offset);
}

void FlowSensors::print_suggested_val(uint8_t i) {
    printlnfd(Serial, "Suggested flow #%u: %.2f", i, cal_suggested_vals[i]);
}

void FlowSensors::set_sensor_enabled(uint8_t sensor_i, bool enabled) {
    sensors[sensor_i].enabled = enabled;
}

float FlowSensors::get_flow(uint8_t sensor_i) {
    return sensors[sensor_i].last_val;
}

float FlowSensors::get_total_flow() {
    return total_flow;
}

float FlowSensors::get_volume(uint8_t sensor_i) {
    /*if (sensors[sensor_i].ins) {
        return sensors[sensor_i].volume * sensors[sensor_i].ins_volume_mult;
    } else {
        // return sensors[sensor_i].volume + sensors[sensor_i].max_vol_ins * (sensors[sensor_i].ins_volume_mult - 1.0f);
        return sensors[sensor_i].volume - sensors[sensor_i].min_vol_esp;
    }*/
    return sensors[sensor_i].comp_volume;
}

float FlowSensors::get_total_volume() {
    return total_comp_volume;
}

bool FlowSensors::get_ins() {
    return ins;
}

void FlowSensors::set_sample_period(uint16_t ms) {
    assert(ms);
    sample_period = ms;
}

void FlowSensors::set_global_enabled(bool enabled) {
    global_enabled = enabled;
}
