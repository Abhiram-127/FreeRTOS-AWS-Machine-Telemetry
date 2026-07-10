#include "dht_sensor.h"
#include "sensor_data.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "dht.h"

#define DHT_PIN GPIO_NUM_4

static const char *TAG = "DHT11";

extern sensor_data_t sensor;
extern SemaphoreHandle_t sensor_mutex;

void dht_task(void *pvParameters)
{
    float temperature, humidity;

    ESP_LOGI(TAG, "Starting DHT11 Task");

    vTaskDelay(pdMS_TO_TICKS(2000));

    while (1)
    {
        esp_err_t result =
            dht_read_float_data(DHT_TYPE_DHT11,
                                DHT_PIN,
                                &humidity,
                                &temperature);

        if (result == ESP_OK)
        {
            xSemaphoreTake(sensor_mutex, portMAX_DELAY);

            sensor.temperature = temperature;
            sensor.humidity = humidity;

            xSemaphoreGive(sensor_mutex);

            ESP_LOGI(TAG,
                     "Temp: %.1f C  Humidity: %.1f%%",
                     temperature,
                     humidity);
        }
        else
        {
            ESP_LOGE(TAG,
                     "DHT Read Failed : %s",
                     esp_err_to_name(result));
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}