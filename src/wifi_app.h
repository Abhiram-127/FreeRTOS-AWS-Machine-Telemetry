#ifndef WIFI_APP_H
#define WIFI_APP_H

#include <stdbool.h>

/* Initialize WiFi */
void wifi_init(void);

/* Wait until WiFi is connected */
void wifi_wait_connected(void);

/* Returns true if connected */
bool wifi_is_connected(void);

#endif