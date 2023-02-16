#include "program.h"
#include "PressureSensors.h"
#include "comm.h"
#include "SerialCmds.h"
#include "GasSensors.h"
#include "UI.h"
#include "flow_sensors.h"
#include "HighPressureSensors.h"
#include "Ventilation.h"
#include "storage.h"
#include "global_values.h"
#include "utils.h"
#include "sensor_manager.h"

#define AIR_L_LIMIT 0.5f
#define AIR_H_LIMIT 4.0f
#define N2_L_LIMIT 0.09f
#define N2_H_LIMIT 3.6f
#define CO2_L_LIMIT 0.07f
#define CO2_H_LIMIT 3.6f
#define SENF_H_LIMIT 3.6f
#define SENF_L_LIMIT 0.07f
#define FLOW_SENSOR_I1 0
#define FLOW_SENSOR_I2 1
#define FLOW_SENSOR_D1 2
#define FLOW_SENSOR_D2 3

// FlowController Program::flow_cont_1(SerialCont1, 1);
// FlowController Program::flow_cont_2(SerialCont2, 2);
// FlowController Program::flow_cont_3(SerialCont3, 3);
FlowController flow_cont_2(SerialCont2, 2);
char Program::in_buff[IN_BUFF_LEN];
uint8_t Program::in_buff_i = 0;
uint8_t Program::pump_perc = 0;
float Program::air_f_atp = 0;
float Program::air_f_stp = 0;
float Program::N2_f_atp = 0;
float Program::N2_f_stp = 0;
float Program::CO2_f_atp = 0;
float Program::CO2_f_stp = 0;
float Program::sen_f_atp = 0;
float Program::sen_f_stp = 0;
float Program::sp_o2 = 0;
static void serial_override();
static bool plots[12];
static bool flow_plots[9];    // El último es el total
static bool volume_plots[9];  // El último es el total
static bool pressure_plots[2];
static bool ab_plots[2];

bool Program::controlling = false;
bool Program::plot_control = false;

void Program::start() {
    UI::init();
    SensorManager::init();
    Processes::add_process_fun(FlowController::static_process, &flow_cont_2);
    Processes::add_process_fun(Comm::process);
    Processes::add_process_fun(SerialCmds::process);
    // Processes::add_process_fun(FlowSensors::process);
    Processes::add_process_fun(GasSensors::process);

    // flow_cont_1.init();
    if (flow_cont_2.init()) {
        printlnd("Controlador de flujo: OK");
    } else {
        printlnd("Controlador de flujo: FAIL");
        fail();
    }

    // flow_cont_3.init();

    if (PressureSensors::init()) {
        printlnd("Sensores de flujo: OK");
    } else {
        printlnd("Sensores de flujo: FAIL");
        fail();
    }

    // GasSensors::init(100);
    FlowSensors::init();
    FlowSensors::set_sensor_enabled(FLOW_SENSOR_I1, true);
    FlowSensors::set_sensor_enabled(FLOW_SENSOR_I2, true);
    FlowSensors::set_sensor_enabled(FLOW_SENSOR_D1, true);
    FlowSensors::set_sensor_enabled(FLOW_SENSOR_D2, true);

    HighPressureSensors::load_zero();

    delay(100);
    printlnfd(Serial, "Temperature (C): %.2f", PressureSensors::read_temp(7));

    Processes::add_process_fun(SensorManager::process);
    Processes::add_process_fun(process);

    // query_nodes();
}

void Program::process(void* obj) {
    static uint32_t t0 = millis();
    static uint32_t t1 = millis();
    /*static const float c0 = 10.197162f;
    static const float c1 = (3.3f / (8191.0f * 0.057f * 5.0f)) * c0;
    static const float c2 = ((1.8f / 5.0f - 0.5f) / 0.057f) * c0;
    static float p1_zero = 0;
    static bool zero_set = false;*/
    if (UI::get_btn_3_pressed()) {
        // stop_all();
        // in_p_cal(true);
        delay_cal(true);
    }
    if (controlling) {
        control(false);
    }
    if (millis() - t1 >= 20) {
        // in_p_cal(false);
        delay_cal(false);
        t1 = millis();
    }

    if (millis() - t0 >= 100) {
        // Utils::tock(t0, "Plot dt: ");
        //  GasSensors::read_all();
        plot();
        /*const uint32_t r = analogRead(PIN_P1);
        const float v = r * 3.3f / 8191.0f;
        const float p1 = (float)analogRead(PIN_P1) * c1 + c2 - p1_zero;
        if (!zero_set) {
            p1_zero = p1;
            zero_set = true;
        }*/

        /*float p1 = HighPressureSensors::read_pressure(0);
        float p2 = HighPressureSensors::read_pressure(1);
        printlnfd(Serial, "P1: %.2f, P2: %.2f", p1, p2);*/

        t0 = millis();
    }
}

void Program::plot() {
    bool plotted = false;
    /*float p4 = PressureSensors::read_pressure(4);
    float p5 = PressureSensors::read_pressure(5);
    float p6 = PressureSensors::read_pressure(6);
    float p7 = PressureSensors::read_pressure(7);
    printfd(Serial, "P4: %.2f, P5: %.2f, P6: %.2f, P7: %.2f", p4, p5, p6, p7);
    plotted = true;*/
    if (!plot_control) {
        // Sensores O2 y CO2
        for (uint8_t i = 0; i < 12; i++) {
            if (plots[i]) {
                if (plotted) {
                    print(", ");
                }
                plotted = true;
                // float reading = 0;
                uint8_t type = (uint8_t)GasSensors::get_sensor_type((gas_sensor_e)i);
                uint8_t flags = GasSensors::get_flags((gas_sensor_e)i);
                // float reading = GasSensors::get_reading((gas_sensor_e)i);
                float reading = SensorManager::get_gas_reading((gas_sensor_e)i);
                // GasSensors::read_sensor((gas_sensor_e)i, reading);

                char flags_str[10];
                get_flags_str(type, flags, flags_str);
                printf(Serial, "%s(%s): %.2f", sensor_names[i], flags_str, reading);
            }
        }
        // Flujos
        for (uint8_t i = 0; i < 8; i++) {
            if (flow_plots[i]) {
                if (plotted) {
                    print(", ");
                }
                plotted = true;
                // printfd(Serial, "F%u: %.2f", i, FlowSensors::get_flow(i));
                printf(Serial, "F%u: %.2f", i, SensorManager::get_flow_reading(i));
            }
        }
        // Flujo total
        if (flow_plots[8]) {
            if (plotted) {
                print(", ");
            }
            plotted = true;
            // printf(Serial, "FTOT: %.2f", FlowSensors::get_total_flow());
            printf(Serial, "FTOT: %.2f", SensorManager::get_total_flow_reading());
        }
        // Volúmenes
        for (uint8_t i = 0; i < 8; i++) {
            if (volume_plots[i]) {
                if (plotted) {
                    print(", ");
                }
                plotted = true;
                // printf(Serial, "V%u: %.2f", i, FlowSensors::get_volume(i));
                printf(Serial, "V%u: %.2f", i, SensorManager::get_volume_reading(i));
            }
        }
        // Volumen total
        if (volume_plots[8]) {
            if (plotted) {
                print(", ");
            }
            plotted = true;
            // printf(Serial, "VTOT: %.2f", FlowSensors::get_total_volume());
            printf(Serial, "VTOT: %.2f", SensorManager::get_total_volume_reading());
        }
        // Presiones
        for (uint8_t i = 0; i < 2; i++) {
            if (pressure_plots[i]) {
                if (plotted) {
                    print(", ");
                }
                plotted = true;
                printf(Serial, "P%u: %.2f", i, SensorManager::get_hp_reading(i));
            }
        }
        // Delta AB
        if (ab_plots[0]) {
            if (plotted) {
                print(", ");
            }
            plotted = true;
            printf(Serial, "DAB_O2: %.1f", Ventilation::get_delta_ab_O2());
        }
        if (ab_plots[1]) {
            if (plotted) {
                print(", ");
            }
            plotted = true;
            printf(Serial, "DAB_CO2: %.1f", Ventilation::get_delta_ab_CO2());
        }
    }

    if (plotted) {
        println();
    }
}

void Program::in_p_cal(bool reset) {
    const float p_step = 2.0f;
    const uint8_t max_steps = 30;

    static float vals[max_steps][4];  // p, %O2, %CO2, samples
    static uint8_t step_i = 0;
    static bool running = false;
    static bool ramp_started = false;
    if (reset) {
        step_i = 0;
        running = true;
        ramp_started = false;
        for (uint8_t i = 0; i < max_steps; i++) {
            vals[i][0] = 0.0f;
            vals[i][1] = 0.0f;
            vals[i][2] = 0.0f;
            vals[i][3] = 0.0f;
        }
        GasSensors::reset_p_cal(gas_sensor_e::O2_IN);
        vals[0][1] = GasSensors::get_reading(gas_sensor_e::O2_IN);
        vals[0][2] = GasSensors::get_reading(gas_sensor_e::CO2_IN);
        vals[0][0] = HighPressureSensors::read_pressure(0);
        // vals[0][0] = 0.0f;
        vals[0][3] = 1;
        printlnd("In cal started");
        return;
    }
    if (running) {
        float o2 = GasSensors::get_reading(gas_sensor_e::O2_IN);
        float co2 = GasSensors::get_reading(gas_sensor_e::CO2_IN);
        float p = HighPressureSensors::read_pressure(0);
        // find step
        uint8_t new_step_i = 0;
        while ((p < new_step_i * p_step) || (p >= (new_step_i + 1) * p_step)) {
            new_step_i++;
            if (new_step_i >= max_steps) {
                return;
            }
        }
        if (new_step_i > step_i) {
            step_i = new_step_i;
            ramp_started = true;
            assert(step_i < max_steps);
            vals[step_i][0] += p;
            vals[step_i][1] += o2;
            vals[step_i][2] += co2;
            vals[step_i][3]++;
            printlnfd(Serial, "Step: %u (%u)", (uint8_t)(step_i * p_step), (uint8_t)vals[step_i][3]);
        } else if (ramp_started && new_step_i == 0) {
            running = false;
            printlnd("In cal results (P, %O2, %CO2):");
            gas_p_cal_t cal;
            for (uint8_t i = 0; i < GAS_P_CAL_SAMPLES; i++) {
                if (vals[i][3]) {
                    vals[i][0] /= vals[i][3];
                    vals[i][1] /= vals[i][3];
                    vals[i][2] /= vals[i][3];
                    vals[i][3] /= vals[i][3];
                    float o2_factor = vals[0][1] / vals[i][1];
                    float co2_factor = vals[0][2] / vals[i][2];
                    cal.p[i] = vals[i][0];
                    cal.o2_factor[i] = o2_factor;
                    printlnfd(Serial, "%.2f %.2f %.2f %.2f %.2f", vals[i][0], vals[i][1], vals[i][2], o2_factor, co2_factor);
                }
            }
            Storage::write_gas_p_cal((uint8_t)gas_sensor_e::O2_IN, cal);
        }
    }
}

void Program::delay_cal(bool reset) {
    const uint8_t gas_sensor_i = (uint8_t)gas_sensor_e::O2_OUT;
    const uint8_t flow_sensor_i = 0;
    const uint32_t perc_stab_time = 10000;
    const uint32_t purge_t = 15000;
    const uint32_t pump_stab_t = 20000;
    const uint8_t samples = 5;
    const float flow_thr = 0.5f;
    const float perc_thr = 1.5f;

    static uint8_t state = 0;
    static float start_perc = 100;
    static float max_perc = 0;
    static float max_flow = 0;
    static uint32_t t0 = 0;
    static uint32_t t_start = 0;
    static uint8_t sample_i = 0;
    static uint32_t dt = 0;

    if (reset) {
        sample_i = 0;
        dt = 0;
        state = 1;
        printlnfd(Serial, "Delay cal started, samples: %I8u", samples);
        return;
    }

    if (state == 1) {  // Reset vals
        start_perc = 100;
        max_perc = 0;
        max_flow = -100;
        t0 = millis();
        printlnfd(Serial, "\nSample #%I8u", (uint8_t)(sample_i + 1));
        printlnd("-Purging N2...");
        set_air_f(5);
        state++;
    }
    if (state == 2) {  // Purgar N2
        if (millis() - t0 >= purge_t) {
            set_air_f(0);
            printlnd("-Waiting pump estabilization...");
            t0 = millis();
            state++;
        }
    }
    if (state == 3) {  // Esperar estabilización de bomba
        if (millis() - t0 >= pump_stab_t) {
            printlnd("-Measuring deltas...");
            state++;
            t0 = millis();
        }
    }
    if (state == 4) {  // Medir mínimo % y máximo flow
        float reading = GasSensors::get_reading((gas_sensor_e)gas_sensor_i);
        max_flow = max(max_flow, FlowSensors::get_flow(flow_sensor_i));
        if (reading > max_perc) {
            max_perc = reading;
            t0 = millis();
        }
        if (reading < start_perc) {
            start_perc = reading;
            t0 = millis();
        } else if (millis() - t0 >= perc_stab_time) {
            set_N2_f(4.5f);  // Aplicar O2
            printlnfd(Serial, "  -d%%: [%.2f, %.2f]", start_perc, max_perc);
            printlnd("-N2 applied, waiting for flow...");
            state++;
        }
    } else if (state == 5) {  // Esperar cambio en flujo
        if (FlowSensors::get_flow(flow_sensor_i) >= (max_flow + flow_thr)) {
            t_start = millis();
            printlnd("-Flow detected, waiting for sensor change...");
            state++;
        }
    } else if (state == 6) {  // Esperar cambio en %
        const float reading = GasSensors::get_reading((gas_sensor_e)gas_sensor_i);
        if (reading <= (start_perc - perc_thr)) {
            const uint32_t sample_dt = millis() - t_start;
            dt += sample_dt;
            set_N2_f(0);
            sample_i++;
            if (sample_i < samples) {
                printlnfd(Serial, "-Change detected: %.2f, sample delay: %I32u", reading, sample_dt);
                state = 1;
            } else {
                dt /= samples;
                printlnfd(Serial, "Final delay: %I32u", dt);
                state = 0;
                UI::buzz(3, 100);
            }
        }
    }
}

float Program::trim(float val, float a, float b) {
    if (val < a) {
        return a;
    }
    if (val > b) {
        return b;
    }
    return val;
}

float Program::trim_flow(float flow, float low_limit, float high_limit) {
    if (flow > high_limit) {
        return high_limit;
    } else if (flow <= 0) {
        return 0;
    } else if (flow < low_limit) {
        const float mid = low_limit / 2.0f;
        return flow >= mid ? low_limit : 0;
    } else {
        return flow;
    }
}

void Program::control(bool reset_test) {
    static uint32_t t0 = millis();
    static uint32_t t1 = millis();
    float p_o2_out = 0;
    float p_co2_out = 0;
    static float p_o2_out_avg = 0;
    static float p_co2_out_avg = 0;
    static uint32_t avg_count = 0;
    static float p_o2_max = 0;
    static float p_o2_min = 100;
    static float p_co2_max = 0;
    static float p_co2_min = 100;

    const bool test = false;
    const uint8_t n_test_samples = 8;
    const uint8_t n_test_cycles = 6;
    static uint8_t sample_i = 0;
    static uint8_t cycle_i = 0;

    static float results_p_o2_max[n_test_samples];
    static float results_p_o2_min[n_test_samples];
    static float results_p_o2_mid[n_test_samples];
    static float results_p_co2_max[n_test_samples];
    static float results_p_co2_min[n_test_samples];
    static float results_p_co2_mid[n_test_samples];
    static float results_f_out_real[n_test_samples];

    if (reset_test) {
        printlnd("Test reseted");
        t0 = millis();
        t1 = millis();
        p_o2_max = 0;
        p_o2_min = 100;
        p_co2_max = 0;
        p_co2_min = 100;

        sample_i = 0;
        cycle_i = n_test_cycles - 1;
        return;
    }

    if (millis() - t0 >= 25) {
        // p_o2_out = GasSensors::get_reading(gas_sensor_e::O2_OUT);
        // p_co2_out = GasSensors::get_reading(gas_sensor_e::CO2_OUT);
        p_o2_out = SensorManager::get_gas_reading(gas_sensor_e::O2_OUT);
        p_co2_out = SensorManager::get_gas_reading(gas_sensor_e::CO2_OUT);
        p_o2_out_avg += p_o2_out;
        p_co2_out_avg += p_co2_out;
        avg_count += 1;
        t0 = millis();

        p_o2_max = max(p_o2_max, p_o2_out);
        p_o2_min = min(p_o2_min, p_o2_out);
        p_co2_max = max(p_co2_max, p_co2_out);
        p_co2_min = min(p_co2_min, p_co2_out);
    }
    if (millis() - t1 >= 5000) {
        cycle_i++;

        p_o2_out_avg /= (avg_count * 100.0f);
        p_co2_out_avg /= (avg_count * 100.0f);
        // p_o2_out = trim(p_o2_out_avg, 0, 100);
        // p_co2_out = trim(p_co2_out_avg, 0, 100);

        float p_o2_mid = (p_o2_max - p_o2_min) / 2.0f + p_o2_min;
        float p_co2_mid = (p_co2_max - p_co2_min) / 2.0f + p_co2_min;
        // p_o2_out = trim(p_o2_mid / 100.0f, 0, 100);
        // p_co2_out = trim(p_co2_min / 100.0f, 0, 100);
        p_o2_out = trim(p_o2_out_avg, 0, 100);
        p_co2_out = trim(p_co2_out_avg, 0, 100);

        // float last_f_out = sen_f + air_f;
        //  float f_out = trim(sp_o2 / p_o2_out, 0, 4.2f);
        const float AIR_L_LIMIT_ATP = STPD0_to_ATPD(AIR_L_LIMIT);
        const float N2_L_LIMIT_ATP = STPD0_to_ATPD(N2_L_LIMIT);
        const float N2_H_LIMIT_ATP = STPD0_to_ATPD(N2_H_LIMIT);
        const float CO2_L_LIMIT_ATP = STPD0_to_ATPD(CO2_L_LIMIT);
        const float CO2_H_LIMIT_ATP = STPD0_to_ATPD(CO2_H_LIMIT);
        float f_out_air = trim_flow(sp_o2 / p_o2_out - sen_f_atp, AIR_L_LIMIT_ATP, N2_H_LIMIT_ATP);
        float f_out_real = f_out_air + sen_f_atp;

        // float f_o2_out = last_f_out * p_o2_out;
        // float f_co2_out = last_f_out * p_co2_out;
        float f_o2_out = f_out_real * p_o2_out;
        float f_co2_out = f_out_real * p_co2_out;
        float f_co2_in = trim_flow(f_o2_out + f_co2_out, CO2_L_LIMIT_ATP, CO2_H_LIMIT_ATP);
        float f_n2_in_th = f_out_real - f_co2_in;
        float f_n2_in = trim_flow(f_n2_in_th, N2_L_LIMIT_ATP, N2_H_LIMIT_ATP);
        if (f_n2_in_th != f_n2_in) {
            // Poner el CO2 en 0
            f_co2_in = 0;
            if (f_out_real > N2_H_LIMIT_ATP) {
                printlnd("CASE 1");
                // Reducir f_out_air y f_out_real
                f_out_air = N2_H_LIMIT_ATP - sen_f_atp;
                f_out_real = N2_H_LIMIT_ATP;

                f_o2_out = f_out_real * p_o2_out;
                f_co2_out = f_out_real * p_co2_out;
            } else {
                printlnd("CASE 2");
            }
            f_n2_in = f_out_real;
        }

        // f_out_air = 0.59f;
        // f_co2_in = 0.12f;
        // f_n2_in = 0.67f;
        if (plot_control) {
            printlnf(Serial, "O2_TOT: %.2f, CO2_TOT: %.2f", p_o2_out * 100, p_co2_out * 100);
        } else {
            /*printlnf(Serial, "%%O2: %.2f %%CO2: %.2f", p_o2_out * 100, p_co2_out * 100);
            printlnfd(Serial, "F_OUT_REAL: %.2f F_CO2_IN: %.2f F_N2_IN: %.2f", f_out_real, f_co2_in, f_n2_in);
            printlnfd(Serial, "F_O2_OUT: %.2f F_CO2_OUT: %.2f", f_o2_out, f_co2_out);*/
        }

        set_air_f(f_out_air);
        set_CO2_f(f_co2_in);
        set_N2_f(f_n2_in);
        float diff = air_f_atp + sen_f_atp - CO2_f_atp - N2_f_atp;
        if (diff != 0) {
            printlnfd(Serial, "DIFF ERR: %f", diff);
        }

        if (test && sample_i < n_test_samples && cycle_i == n_test_cycles) {
            printlnfd(Serial, "Sample %u taken", sample_i + 1);
            cycle_i = 0;
            results_p_o2_max[sample_i] = p_o2_max;
            results_p_o2_min[sample_i] = p_o2_min;
            results_p_o2_mid[sample_i] = p_o2_mid;
            results_p_co2_max[sample_i] = p_co2_max;
            results_p_co2_min[sample_i] = p_co2_min;
            results_p_co2_mid[sample_i] = p_co2_mid;
            results_f_out_real[sample_i] = f_out_real;

            sample_i++;
            if (sample_i == n_test_samples) {
                for (uint8_t i = 0; i < 12; i++) {
                    set_plot(i, false);
                }
                printlnd("\nTEST COMPLETE:");
                printlnd("O2 min O2 mid O2 max CO2 min CO2 mid CO2 max F_OUT_REAL");
                for (uint8_t i = 0; i < n_test_samples; i++) {
                    printlnfd(Serial, "%.2f %.2f %.2f %.2f %.2f %.2f %.2f",
                              results_p_o2_min[i], results_p_o2_mid[i], results_p_o2_max[i],
                              results_p_co2_min[i], results_p_co2_mid[i], results_p_co2_max[i],
                              results_f_out_real[i]);
                }
                UI::buzz(2, 100);
            }
        }
        // printlnd();
        p_o2_out_avg = 0;
        p_co2_out_avg = 0;
        p_o2_max = 0;
        p_co2_max = 0;
        p_co2_min = 100;
        p_o2_min = 100;
        avg_count = 0;
        t1 = millis();
    }

    /*GasSensors::read_sensor(gas_sensor_e::O2_OUT, p_o2_out);
    GasSensors::read_sensor(gas_sensor_e::CO2_OUT, p_co2_out);
    p_o2_out /= 100.0f;
    p_co2_out /= 100.0f;
    p_o2_out = trim(p_o2_out, 0, 100);
    p_co2_out = trim(p_co2_out, 0, 100);*/
    // float current_f_out = air_f + sen_f;
}

void Program::set_plot(uint8_t i, bool state) {
    plots[i] = state;
}

void Program::set_flow_plot(uint8_t i, bool state) {
    flow_plots[i] = state;
}
void Program::set_pressure_plot(uint8_t i, bool state) {
    assert(i <= 1);
    pressure_plots[i] = state;
}
void Program::set_volume_plot(uint8_t i, bool state) {
    volume_plots[i] = state;
}

void Program::set_DAB_O2_plot(bool state) {
    ab_plots[0] = state;
}
void Program::set_DAB_CO2_plot(bool state) {
    ab_plots[1] = state;
}

void Program::ftdi_mode() {
    printlnd("\nFTDI MODE");
    while (true) {
        if (Serial.available()) {
            SerialExt.print((char)Serial.read());
        }
        if (SerialExt.available()) {
            printd((char)SerialExt.read());
        }
    }
}

void Program::print_flags(uint8_t sensor_type, uint8_t flags) {
    char str[10];
    get_flags_str(sensor_type, flags, str);
    printd(str);
}
void Program::get_flags_str(uint8_t sensor_type, uint8_t flags, char* dest) {
    *dest = '\0';
    if (sensor_type) {  // CO2
        if (flags & 0b1) {
            strcat(dest, "E");
        }
        if (flags & 0b10) {
            strcat(dest, "B");
        }
        if (flags & 0b100) {
            strcat(dest, "S");
        }
        if (flags & 0b1000) {
            strcat(dest, "W");
        }
    } else {  // O2
        if (flags & 0b1) {
            strcat(dest, "B");
        }
        if (flags & 0b10) {
            strcat(dest, "C");
        }
        if (flags & 0b100) {
            strcat(dest, "E");
        }
        if (flags & 0b1000) {
            strcat(dest, "S");
        }
        if (flags & 0b10000) {
            strcat(dest, "X");
        }
    }
}

static void serial_override() {
    Stream& cont_port = SerialCont2;
    while (1) {
        if (Serial.available()) {
            cont_port.print((char)Serial.read());
        }
        if (cont_port.available()) {
            printd((char)cont_port.read());
        }
    }
}

void Program::process_comm_cmd() {
    static comm_cmd_t cmd = {};
    if (!Comm::get_cmd(cmd)) {
        // TODO
        return;
    }
    printlnfd(Serial, "Comm msg type: %u", cmd.msg_type);
}

void Program::set_pump(uint8_t perc) {
    if (perc <= 100) {
        analogWrite(PIN_MOT_1, map(perc, 0, 100, 0, 255));
        pump_perc = perc;
    }
}

float Program::set_air_f(float flow_atp) {
    const float flow_stp = ATPD_to_STPD0(flow_atp);
    const float real_flow = trim_flow(flow_stp, AIR_L_LIMIT, AIR_H_LIMIT);
    if (flow_stp != real_flow) {
        const float new_flow_atp = STPD0_to_ATPD(real_flow);
        printlnfd(Serial, "Air flow trimmed: %.2f -> %.2f", flow_atp, new_flow_atp);
        flow_atp = new_flow_atp;
    }
    flow_cont_2.write_SP_rate_STPD0(1, real_flow);
    air_f_atp = flow_atp;
    air_f_stp = real_flow;
    return flow_atp;
}

float Program::set_N2_f(float flow_atp) {
    const float flow_stp = ATPD_to_STPD0(flow_atp);
    const float real_flow = trim_flow(flow_stp, N2_L_LIMIT, N2_H_LIMIT);
    if (flow_stp != real_flow) {
        const float new_flow_atp = STPD0_to_ATPD(real_flow);
        printlnfd(Serial, "N2 flow trimmed: %.2f -> %.2f", flow_atp, new_flow_atp);
        flow_atp = new_flow_atp;
    }
    flow_cont_2.write_SP_rate_STPD0(2, real_flow);
    N2_f_atp = flow_atp;
    N2_f_stp = real_flow;
    return flow_atp;
}

float Program::set_CO2_f(float flow_atp) {
    const float flow_stp = ATPD_to_STPD0(flow_atp);
    const float real_flow = trim_flow(flow_stp, CO2_L_LIMIT, CO2_H_LIMIT);
    if (flow_stp != real_flow) {
        const float new_flow_atp = STPD0_to_ATPD(real_flow);
        printlnfd(Serial, "CO2 flow trimmed: %.2f -> %.2f", flow_atp, new_flow_atp);
        flow_atp = new_flow_atp;
    }
    flow_cont_2.write_SP_rate_STPD0(3, real_flow);
    CO2_f_atp = flow_atp;
    CO2_f_stp = real_flow;
    return flow_atp;
}

float Program::set_sen_f(float flow_atp) {
    const float flow_stp = ATPD_to_STPD0(flow_atp);
    const float real_flow = trim_flow(flow_stp, SENF_L_LIMIT, SENF_H_LIMIT);
    if (flow_stp != real_flow) {
        const float new_flow_atp = STPD0_to_ATPD(real_flow);
        printlnfd(Serial, "SEN flow trimmed: %.2f -> %.2f", flow_atp, new_flow_atp);
        flow_atp = new_flow_atp;
    }
    flow_cont_2.write_SP_rate_STPD0(4, real_flow);
    sen_f_atp = flow_atp;
    sen_f_stp = real_flow;
    return flow_atp;
}

void Program::set_sp_o2(float sp) {
    sp_o2 = sp;
}

void Program::set_controller(bool enabled) {
    if ((!controlling) && enabled) {
        control(true);
    }
    controlling = enabled;
    digitalWrite(PIN_LED_2, controlling);
}

void Program::set_plot_control(bool plot) {
    plot_control = plot;
}

void Program::close_gases() {
    set_air_f(0);
    set_N2_f(0);
    set_CO2_f(0);
    set_sen_f(0);
}

void Program::stop_all() {
    set_controller(false);
    close_gases();
    set_pump(0);
    UI::buzz(3, 100);
}

void aFailed(uint8_t* file, uint32_t line) {
    uint32_t t0 = millis();
    while (1) {
        digitalWrite(PIN_LED_3, HIGH);
        delay(100);
        digitalWrite(PIN_LED_3, LOW);
        delay(100);
        if (millis() - t0 > 1000) {
            printlnfd(Serial, "ASSERT FAILED ON %s LINE %u", file, line);
            printlnd();
            t0 = millis();
        }
    }
}

void Program::fail() {
    uint32_t t0 = millis();
    while (1) {
        digitalWrite(PIN_LED_3, HIGH);
        delay(100);
        digitalWrite(PIN_LED_3, LOW);
        delay(100);
        if (millis() - t0 > 1000) {
            println("PROGRAM FAILED");
            t0 = millis();
        }
    }
}