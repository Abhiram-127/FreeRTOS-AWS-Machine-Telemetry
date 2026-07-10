#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include <stdbool.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

typedef struct
{
    float temperature;
    float humidity;

    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;

    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;

    float vibration;

    bool alert;

} sensor_data_t;

extern sensor_data_t sensor;

extern SemaphoreHandle_t sensor_mutex;

#endif