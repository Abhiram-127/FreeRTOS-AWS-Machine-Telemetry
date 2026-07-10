#ifndef MQTT_APP_H
#define MQTT_APP_H

#include <stdbool.h>
#include "esp_err.h"
#include "mqtt_client.h"

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------
 * MQTT Topics
 *---------------------------------------------------------*/
#define MQTT_TOPIC_TELEMETRY    "industrial/MyESP32/telemetry"
#define MQTT_TOPIC_ALERT        "industrial/MyESP32/alert"
#define MQTT_TOPIC_COMMAND      "industrial/MyESP32/command"
#define MQTT_TOPIC_STATUS       "industrial/MyESP32/status"

/*----------------------------------------------------------
 * Public APIs
 *---------------------------------------------------------*/

/**
 * @brief Initialize and start the MQTT client.
 *
 * @return ESP_OK on success.
 */
esp_err_t mqtt_app_start(void);

/**
 * @brief MQTT publishing task.
 *
 * Reads the latest sensor values and publishes them
 * periodically to AWS IoT Core.
 */
void mqtt_publish_task(void *pvParameters);

/**
 * @brief Returns true if MQTT is connected.
 */
bool mqtt_is_connected(void);

/**
 * @brief Publish telemetry JSON immediately.
 *
 * Useful when another task wants to force a publish.
 */
esp_err_t mqtt_publish_telemetry(void);

/**
 * @brief Publish an alert message immediately.
 *
 * Used when high vibration is detected.
 */
esp_err_t mqtt_publish_alert(void);

/**
 * @brief Returns the internal MQTT client handle.
 */
esp_mqtt_client_handle_t mqtt_get_client(void);

#ifdef __cplusplus
}
#endif

#endif