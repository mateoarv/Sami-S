#ifndef PROGRAM_H
#define PROGRAM_H

#include "includes.h"
#include "flow_controller.h"

#define OVERRIDE_CTRL_SERIAL 1

extern FlowController flow_cont_2;

class Program {
   public:
    static void start();
    static void process(void* obj);
    static void set_pump(uint8_t perc);
    static float set_air_f(float flow);
    static float set_N2_f(float flow);
    static float set_CO2_f(float flow);
    static float set_sen_f(float flow);
    static void set_sp_o2(float sp);
    static void close_gases();
    static void ftdi_mode();
    static void set_flow_plot(uint8_t i, bool state);
    static void set_volume_plot(uint8_t i, bool state);
    static void set_pressure_plot(uint8_t i, bool state);
    static void set_DAB_O2_plot(bool state);
    static void set_DAB_CO2_plot(bool state);
    static void set_plot(uint8_t i, bool state);
    static void set_plot_control(bool plot);
    static void set_controller(bool enabled);
    static void stop_all();

   private:
    typedef struct {
        uint8_t id;
        uint8_t type;
    } node_t;

    // static FlowController flow_cont_1;
    // static FlowController flow_cont_2;
    // static FlowController flow_cont_3;

    static const uint8_t IN_BUFF_LEN = 100;
    static char in_buff[IN_BUFF_LEN];
    static uint8_t in_buff_i;
    static uint8_t pump_perc;
    static float air_f_atp;
    static float air_f_stp;
    static float N2_f_atp;
    static float N2_f_stp;
    static float CO2_f_atp;
    static float CO2_f_stp;
    static float sen_f_atp;
    static float sen_f_stp;
    static float sen_f;
    static float sp_o2;
    static bool controlling;
    static bool plot_control;

    static void process_comm_cmd();
    static void print_flags(uint8_t sensor_type, uint8_t flags);
    static void get_flags_str(uint8_t sensor_type, uint8_t flags, char* dest);
    static void plot();
    static void control(bool reset_test);
    static float trim(float val, float a, float b);
    static float trim_flow(float val, float low_limit, float high_limit);
    static void fail();

    static void in_p_cal(bool reset);
    static void delay_cal(bool reset);
};

#endif