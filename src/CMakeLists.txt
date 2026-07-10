idf_component_register(
    SRCS
        "main.c"
        "lsm6dso.c"
        "dht_sensor.c"
        "wifi_app.c"
        "mqtt_app.c"

    INCLUDE_DIRS
        "."

    EMBED_TXTFILES
        "../cert/amazon_ca.pem"
        "../cert/device_cert.pem"
        "../cert/private_key.pem"
)