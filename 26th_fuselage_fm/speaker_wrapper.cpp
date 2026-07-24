#include "speaker_wrapper.h"


void run_speaker(void){
    
    bool isInside = isInsideArea(data_air_gps_latitude_deg, data_air_gps_longitude_deg);
    
    float ttc = calc_time_to_reach(data_air_gps_latitude_deg,
        data_air_gps_longitude_deg,
        data_air_gps_groundspeed_ms,
        data_air_gps_heading_deg
    );

    float trusted_altitude;
    if (urm_is_reliable == true){
        trusted_altitude = data_under_urm_altitude_m;
    } else {
        trusted_altitude = filtered_bmp_altitude_m;
    }

    speaker(filtered_airspeed_ms, trusted_altitude, takeoff, isInside, ttc);

    // For Debug
    // Serial.print("isInside:  ");
    // Serial.println(isInside);
    // Serial.print("ttc:   ");
    // Serial.println(ttc);
}