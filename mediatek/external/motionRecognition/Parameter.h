#ifndef PARAMETER_H
#define PARAMETER_H

namespace Parameter
{
    int sensor_data_track_period_ms();
    int sensor_data_track_time_pers();
    int buffered_collected_sensor_data_seconds();
    int buffered_collected_sensor_data_size();
    float motion_window_seconds();
    int motion_window_size();
}


#endif // PARAMETER_H
