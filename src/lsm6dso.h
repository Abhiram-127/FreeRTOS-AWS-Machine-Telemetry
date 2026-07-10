#ifndef LSM6DSO_H
#define LSM6DSO_H

#include "esp_err.h"

esp_err_t lsm6dso_init(void);

void lsm6dso_read_task(void *pvParameters);

#endif