#include <stdio.h>
#include "wifi_app.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_err.h"

#include "lsm6dso.h"
#include "dht_sensor.h"
#include "sensor_data.h"

#include "mqtt_app.h"

sensor_data_t sensor;
SemaphoreHandle_t sensor_mutex;

void app_main(void)
{
    sensor_mutex = xSemaphoreCreateMutex();

    wifi_init();
    wifi_wait_connected();

    ESP_ERROR_CHECK(lsm6dso_init());

    ESP_ERROR_CHECK(mqtt_app_start());

    xTaskCreate(lsm6dso_read_task,
                "imu_task",
                4096,
                NULL,
                5,
                NULL);

    xTaskCreate(dht_task,
                "dht_task",
                4096,
                NULL,
                4,
                NULL);

    xTaskCreate(mqtt_publish_task,
                "mqtt_task",
                6144,
                NULL,
                4,
                NULL);
}
