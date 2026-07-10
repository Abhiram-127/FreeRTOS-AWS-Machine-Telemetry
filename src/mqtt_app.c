#include "mqtt_app.h"
#include "sensor_data.h"

#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "mqtt_client.h"

extern sensor_data_t sensor;
extern SemaphoreHandle_t sensor_mutex;

/*----------------------------------------------------------
 * Embedded Certificates
 *---------------------------------------------------------*/
extern const uint8_t amazon_ca_pem_start[] asm("_binary_amazon_ca_pem_start");
extern const uint8_t amazon_ca_pem_end[]   asm("_binary_amazon_ca_pem_end");

extern const uint8_t device_cert_pem_start[] asm("_binary_device_cert_pem_start");
extern const uint8_t device_cert_pem_end[]   asm("_binary_device_cert_pem_end");

extern const uint8_t private_key_pem_start[] asm("_binary_private_key_pem_start");
extern const uint8_t private_key_pem_end[]   asm("_binary_private_key_pem_end");

/*----------------------------------------------------------*/

#define MQTT_BROKER_URI \
"mqtts://a1eqw6eenb1q6w-ats.iot.us-east-1.amazonaws.com:8883"

static const char *TAG = "MQTT_APP";

static esp_mqtt_client_handle_t mqtt_client = NULL;

static bool mqtt_connected = false;

/*----------------------------------------------------------
 * MQTT Event Handler
 *---------------------------------------------------------*/

static void mqtt_event_handler(void *handler_args,
                               esp_event_base_t base,
                               int32_t event_id,
                               void *event_data)
{
    esp_mqtt_event_handle_t event =
            (esp_mqtt_event_handle_t) event_data;

    switch((esp_mqtt_event_id_t)event_id)
    {

    case MQTT_EVENT_CONNECTED:

        ESP_LOGI(TAG,"MQTT Connected");

        mqtt_connected = true;

        esp_mqtt_client_subscribe(
                mqtt_client,
                MQTT_TOPIC_COMMAND,
                1);

        break;

    case MQTT_EVENT_DISCONNECTED:

        ESP_LOGW(TAG,"MQTT Disconnected");

        mqtt_connected = false;

        break;

    case MQTT_EVENT_SUBSCRIBED:

        ESP_LOGI(TAG,"Subscribed");

        break;

    case MQTT_EVENT_DATA:

        ESP_LOGI(TAG,
                 "Topic: %.*s",
                 event->topic_len,
                 event->topic);

        ESP_LOGI(TAG,
                 "Data : %.*s",
                 event->data_len,
                 event->data);

        /*
            Future:

            ON
            OFF
            CHANGE_THRESHOLD
            OTA
        */

        break;

    case MQTT_EVENT_ERROR:

        ESP_LOGE(TAG,"MQTT Error");

        break;

    default:

        break;
    }
}

/*----------------------------------------------------------
 * Connection Status
 *---------------------------------------------------------*/

bool mqtt_is_connected(void)
{
    return mqtt_connected;
}

esp_mqtt_client_handle_t mqtt_get_client(void)
{
    return mqtt_client;
}
/*----------------------------------------------------------
 * MQTT Start
 *---------------------------------------------------------*/

esp_err_t mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg =
    {
        .broker.address.uri = MQTT_BROKER_URI,

        .broker.verification.certificate =
            (const char *)amazon_ca_pem_start,

        .credentials.authentication.certificate =
            (const char *)device_cert_pem_start,

        .credentials.authentication.key =
            (const char *)private_key_pem_start,
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);

    if (mqtt_client == NULL)
    {
        ESP_LOGE(TAG, "Failed to initialize MQTT client");
        return ESP_FAIL;
    }

    ESP_ERROR_CHECK(
        esp_mqtt_client_register_event(
            mqtt_client,
            ESP_EVENT_ANY_ID,
            mqtt_event_handler,
            NULL));

    ESP_ERROR_CHECK(
        esp_mqtt_client_start(mqtt_client));

    ESP_LOGI(TAG, "MQTT Client Started");

    return ESP_OK;
}

/*----------------------------------------------------------
 * Publish Telemetry
 *---------------------------------------------------------*/

esp_err_t mqtt_publish_telemetry(void)
{
    if (!mqtt_connected)
    {
        return ESP_FAIL;
    }

    char payload[512];

    xSemaphoreTake(sensor_mutex, portMAX_DELAY);

    snprintf(payload,
             sizeof(payload),

             "{"
             "\"device_id\":\"MyESP32\","
             "\"temperature\":%.1f,"
             "\"humidity\":%.1f,"
             "\"accel_x\":%d,"
             "\"accel_y\":%d,"
             "\"accel_z\":%d,"
             "\"gyro_x\":%d,"
             "\"gyro_y\":%d,"
             "\"gyro_z\":%d,"
             "\"vibration\":%.2f,"
             "\"alert\":%s"
             "}",

             sensor.temperature,
             sensor.humidity,

             sensor.accel_x,
             sensor.accel_y,
             sensor.accel_z,

             sensor.gyro_x,
             sensor.gyro_y,
             sensor.gyro_z,

             sensor.vibration,

             sensor.alert ? "true" : "false");

    xSemaphoreGive(sensor_mutex);

    int msg_id =
        esp_mqtt_client_publish(
            mqtt_client,
            MQTT_TOPIC_TELEMETRY,
            payload,
            0,
            1,
            0);

    ESP_LOGI(TAG,
             "Telemetry Published (msg=%d)",
             msg_id);

    return ESP_OK;
}

/*----------------------------------------------------------
 * Publish Alert
 *---------------------------------------------------------*/

esp_err_t mqtt_publish_alert(void)
{
    if (!mqtt_connected)
    {
        return ESP_FAIL;
    }

    char payload[256];

    snprintf(payload,
             sizeof(payload),

             "{"
             "\"device_id\":\"MyESP32\","
             "\"event\":\"HIGH_VIBRATION\","
             "\"vibration\":%.2f"
             "}",

             sensor.vibration);

    int msg_id =
        esp_mqtt_client_publish(
            mqtt_client,
            MQTT_TOPIC_ALERT,
            payload,
            0,
            1,
            0);

    ESP_LOGW(TAG,
             "Alert Published (msg=%d)",
             msg_id);

    return ESP_OK;
}
/*----------------------------------------------------------
 * MQTT Publish Task
 *---------------------------------------------------------*/

void mqtt_publish_task(void *pvParameters)
{
    bool previous_alert_state = false;

    ESP_LOGI(TAG, "MQTT Publish Task Started");

    while (1)
    {
        /* Wait until MQTT is connected */
        if (!mqtt_connected)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        /* Publish telemetry every 2 seconds */
        mqtt_publish_telemetry();

        /* Publish alert only on rising edge */
        xSemaphoreTake(sensor_mutex, portMAX_DELAY);

        bool current_alert = sensor.alert;

        xSemaphoreGive(sensor_mutex);

        if (current_alert && !previous_alert_state)
        {
            ESP_LOGW(TAG, "High vibration detected!");

            mqtt_publish_alert();
        }

        previous_alert_state = current_alert;

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}