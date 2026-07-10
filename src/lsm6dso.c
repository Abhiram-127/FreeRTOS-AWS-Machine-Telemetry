#include "lsm6dso.h"
#include "sensor_data.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

extern sensor_data_t sensor;
extern SemaphoreHandle_t sensor_mutex;

static const char *TAG = "LSM6DSO";

#define PIN_NUM_MISO 13
#define PIN_NUM_MOSI 11
#define PIN_NUM_CLK  12
#define PIN_NUM_CS   10

#define SPI_CLOCK_SPEED_HZ 1000000

// Registers
#define LSM6DSO_WHO_AM_I   0x0F
#define LSM6DSO_CTRL1_XL   0x10
#define LSM6DSO_CTRL2_G    0x11
#define LSM6DSO_OUTX_L_G   0x22

static spi_device_handle_t spi_handle = NULL;

/****************************************************
 * SPI Initialization
 ****************************************************/
static esp_err_t spi_master_init(void)
{
    esp_err_t ret;

    spi_bus_config_t buscfg =
    {
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32
    };

    spi_device_interface_config_t devcfg =
    {
        .clock_speed_hz = SPI_CLOCK_SPEED_HZ,
        .mode = 0,
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 1
    };

    ret = spi_bus_initialize(SPI2_HOST,
                             &buscfg,
                             SPI_DMA_CH_AUTO);

    if (ret != ESP_OK &&
        ret != ESP_ERR_INVALID_STATE)
    {
        return ret;
    }

    ret = spi_bus_add_device(
            SPI2_HOST,
            &devcfg,
            &spi_handle);

    return ret;
}

/****************************************************
 * SPI Write
 ****************************************************/
static esp_err_t lsm6dso_write_register(uint8_t reg,
                                        uint8_t data)
{
    uint8_t tx[2] =
    {
        reg & 0x7F,
        data
    };

    spi_transaction_t t =
    {
        .length = 16,
        .tx_buffer = tx
    };

    return spi_device_polling_transmit(
                spi_handle,
                &t);
}

/****************************************************
 * SPI Read
 ****************************************************/
static esp_err_t lsm6dso_read_registers(
            uint8_t reg,
            uint8_t *data,
            size_t len)
{
    uint8_t tx[13] = {0};
    uint8_t rx[13] = {0};

    tx[0] = reg | 0x80;

    spi_transaction_t t =
    {
        .length = (len + 1) * 8,
        .tx_buffer = tx,
        .rx_buffer = rx
    };

    esp_err_t ret =
        spi_device_polling_transmit(
            spi_handle,
            &t);

    if(ret == ESP_OK)
    {
        memcpy(data,
               &rx[1],
               len);
    }

    return ret;
}

/****************************************************
 * Sensor Initialization
 ****************************************************/
esp_err_t lsm6dso_init(void)
{
    ESP_LOGI(TAG, "Initializing SPI...");

    ESP_ERROR_CHECK(spi_master_init());

    if(spi_handle == NULL)
    {
        ESP_LOGE(TAG,"SPI Handle NULL");
        return ESP_FAIL;
    }

    uint8_t who = 0;

    ESP_ERROR_CHECK(
        lsm6dso_read_registers(
            LSM6DSO_WHO_AM_I,
            &who,
            1));

    ESP_LOGI(TAG,
             "WHO_AM_I = 0x%02X",
             who);

    if(who != 0x6C)
    {
        ESP_LOGE(TAG,
                 "LSM6DSO Not Found");
        return ESP_FAIL;
    }

    ESP_ERROR_CHECK(
        lsm6dso_write_register(
            LSM6DSO_CTRL1_XL,
            0x40));

    ESP_ERROR_CHECK(
        lsm6dso_write_register(
            LSM6DSO_CTRL2_G,
            0x40));

    ESP_LOGI(TAG,
             "LSM6DSO Initialized");

    return ESP_OK;
}

/****************************************************
 * IMU Task
 ****************************************************/
void lsm6dso_read_task(void *pv)
{
    uint8_t raw[12];

    int16_t ax, ay, az;
    int16_t gx, gy, gz;

    static float prevMagnitude = 0.0f;

    while (1)
    {
        if (lsm6dso_read_registers(
                LSM6DSO_OUTX_L_G,
                raw,
                12) == ESP_OK)
        {
            gx = (raw[1] << 8) | raw[0];
            gy = (raw[3] << 8) | raw[2];
            gz = (raw[5] << 8) | raw[4];

            ax = (raw[7] << 8) | raw[6];
            ay = (raw[9] << 8) | raw[8];
            az = (raw[11] << 8) | raw[10];

            float magnitude = sqrtf(
                (float)ax * ax +
                (float)ay * ay +
                (float)az * az);

            float vibration = fabsf(magnitude - prevMagnitude);

            prevMagnitude = magnitude;

            xSemaphoreTake(sensor_mutex, portMAX_DELAY);

            sensor.accel_x = ax;
            sensor.accel_y = ay;
            sensor.accel_z = az;

            sensor.gyro_x = gx;
            sensor.gyro_y = gy;
            sensor.gyro_z = gz;

            sensor.vibration = vibration;

            // Adjust this threshold as needed
            sensor.alert = (vibration > 300);

            xSemaphoreGive(sensor_mutex);

            ESP_LOGI(TAG,
                "AX:%6d AY:%6d AZ:%6d | GX:%6d GY:%6d GZ:%6d | Vib:%7.2f Alert:%d",
                ax,
                ay,
                az,
                gx,
                gy,
                gz,
                vibration,
                sensor.alert);
        }
        else
        {
            ESP_LOGE(TAG, "SPI Read Failed");
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}