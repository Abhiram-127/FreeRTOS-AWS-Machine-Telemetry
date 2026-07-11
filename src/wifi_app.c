#include "wifi_app.h"

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"

#define WIFI_SSID      "wifi name"
#define WIFI_PASSWORD  "password"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG = "WIFI_APP";

static EventGroupHandle_t wifi_event_group;

static int retry_count = 0;
#define MAXIMUM_RETRY 5

static bool wifi_connected = false;

/******************************************************
 * Event Handler
 ******************************************************/
static void wifi_event_handler(
        void *arg,
        esp_event_base_t event_base,
        int32_t event_id,
        void *event_data)
{
    if (event_base == WIFI_EVENT &&
        event_id == WIFI_EVENT_STA_START)
    {
        ESP_LOGI(TAG, "Connecting to WiFi...");
        esp_wifi_connect();
    }

    else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        wifi_connected = false;

        if (retry_count < MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            retry_count++;

            ESP_LOGW(TAG,
                     "Retrying WiFi (%d/%d)",
                     retry_count,
                     MAXIMUM_RETRY);
        }
        else
        {
            xEventGroupSetBits(
                wifi_event_group,
                WIFI_FAIL_BIT);

            ESP_LOGE(TAG,
                     "Failed to connect.");
        }
    }

    else if (event_base == IP_EVENT &&
             event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event =
            (ip_event_got_ip_t *)event_data;

        ESP_LOGI(TAG,
                 "Connected. IP: " IPSTR,
                 IP2STR(&event->ip_info.ip));

        retry_count = 0;

        wifi_connected = true;

        xEventGroupSetBits(
            wifi_event_group,
            WIFI_CONNECTED_BIT);
    }
}
/******************************************************
 * Initialize WiFi
 ******************************************************/
void wifi_init(void)
{
    esp_err_t ret;

    /* Initialize NVS */
    ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &wifi_event_handler,
            NULL,
            NULL));

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &wifi_event_handler,
            NULL,
            NULL));

    wifi_config_t wifi_config =
    {
        .sta =
        {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    ESP_ERROR_CHECK(
        esp_wifi_set_mode(WIFI_MODE_STA));

    ESP_ERROR_CHECK(
        esp_wifi_set_config(
            WIFI_IF_STA,
            &wifi_config));

    ESP_ERROR_CHECK(
        esp_wifi_start());

    ESP_LOGI(TAG,
             "WiFi initialization complete.");
}

/******************************************************
 * Wait until WiFi connects
 ******************************************************/
void wifi_wait_connected(void)
{
    EventBits_t bits =
        xEventGroupWaitBits(
            wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    if(bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG,
                 "WiFi Connected.");
    }
    else if(bits & WIFI_FAIL_BIT)
    {
        ESP_LOGE(TAG,
                 "Unable to connect.");
    }
}

/******************************************************
 * Returns connection status
 ******************************************************/
bool wifi_is_connected(void)
{
    return wifi_connected;
}
