#ifndef GAS_SENSORS_H
#define GAS_SENSORS_H
#include "includes.h"
#include "comm.h"

#define GAS_SENSOR_NUM 12
#define GAS_P_CAL_SAMPLES 10

enum class gas_sensor_e : uint8_t {
    O2_IN = 0,   // 0000
    CO2_IN = 1,  // 0001
    // D1
    O2_D1 = 2,   // 0010
    CO2_D1 = 3,  // 0011
    // D2
    O2_D2 = 4,   // 0100
    CO2_D2 = 5,  // 0101
    // I1
    O2_I1 = 6,   // 0110
    CO2_I1 = 7,  // 0111
    // I2
    O2_I2 = 8,   // 1000
    CO2_I2 = 9,  // 1001

    O2_OUT = 10,   // 1010
    CO2_OUT = 11,  // 1011
};
enum class sensor_type_e : uint8_t {
    O2 = 0,
    CO2 = 1
};

typedef struct {
    float p[GAS_P_CAL_SAMPLES];
    float o2_factor[GAS_P_CAL_SAMPLES];
} gas_p_cal_t;

extern const char* sensor_names[12];

class GasSensors {
   public:
    static void init();
    static float get_reading(gas_sensor_e sensor);
    static void read_all();
    static float get_max_reading(gas_sensor_e sensor);
    static float get_min_reading(gas_sensor_e sensor);
    static bool is_warming();
    static uint8_t get_flags(gas_sensor_e sensor);
    static sensor_type_e get_sensor_type(gas_sensor_e sensor);
    static void O2_low_cal();
    static void O2_low_cal(gas_sensor_e sensor);
    static void O2_high_cal();
    static void O2_high_cal(gas_sensor_e sensor);
    static void CO2_low_cal();
    static void CO2_low_cal(gas_sensor_e sensor);
    static void reset_min_max(gas_sensor_e sensor);
    static void reset_min_max();
    static void reset_p_cal(gas_sensor_e sensor);
    static void process(void* obj);

   private:
    typedef struct {
        bool enabled;
        sensor_type_e type;
        float aux_reading;
        bool propagate_pending;
        float reading;
        float max_reading;
        float min_reading;
        uint8_t flags;
        uint32_t last_reading_t;
        gas_p_cal_t p_cal;
        bool p_cal_set;
    } sensor_t;

    static sensor_t sensors[GAS_SENSOR_NUM];
    static uint16_t period;
    static void query_nodes();
    static bool read_sensor(gas_sensor_e sensor, float& dest);
    static void propagate_readings();
    static bool ping_node(uint8_t node_id);
    static bool query_sensor_type(uint8_t node_id, sensor_type_e& dest);
    static bool wait_for_comm_resp(uint8_t sender_id, comm_cmd_t& dest);
    static bool wait_for_empty_comm_resp(uint8_t sender_id);
    static void load_p_cal();
};

#endif