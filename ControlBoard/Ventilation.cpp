#include "Ventilation.h"
#include "flow_sensors.h"
#include "GasSensors.h"
#include "utils.h"

float Ventilation::delta_ab_O2 = 0;
float Ventilation::delta_ab_CO2 = 0;
float Ventilation::buff_filtro_ab_O2[AB_FILTER_LEN] = {};
float Ventilation::buff_filtro_ab_CO2[AB_FILTER_LEN] = {};
MediaMovil<float> Ventilation::filtro_ab_O2(buff_filtro_ab_O2, AB_FILTER_LEN);
MediaMovil<float> Ventilation::filtro_ab_CO2(buff_filtro_ab_CO2, AB_FILTER_LEN);
bool Ventilation::ins = false;

void Ventilation::register_ins() {
    ins = true;
    /*const float min_O2_IN = GasSensors::get_min_reading(gas_sensor_e::O2_IN);
    const float max_CO2_IN = GasSensors::get_max_reading(gas_sensor_e::CO2_IN);
    const float max_O2_AV = GasSensors::get_max_reading(gas_sensor_e::O2_OUT);
    const float min_CO2_AV = GasSensors::get_min_reading(gas_sensor_e::CO2_OUT);
    GasSensors::reset_min_max();

    filtro_ab_O2.agregarVal((max_O2_AV - min_O2_IN) * 7.6f);
    filtro_ab_CO2.agregarVal((max_CO2_IN - min_CO2_AV) * 7.6f);*/

    // delta_ab_O2 = (max_O2_AV - min_O2_IN) * 7.6f;
    // delta_ab_CO2 = (max_CO2_IN - min_CO2_AV) * 7.6f;
    //  printlnfd(Serial, "ABO2 %.2f ABCO2 %.2f", delta_ab_O2, delta_ab_CO2);
}

void Ventilation::register_esp() {
    ins = false;
    GasSensors::reset_min_max();
}

float Ventilation::get_delta_ab_O2() {
    return filtro_ab_O2.getMedia();
}
float Ventilation::get_delta_ab_CO2() {
    return filtro_ab_CO2.getMedia();
}

bool Ventilation::is_ins() {
    return ins;
}
