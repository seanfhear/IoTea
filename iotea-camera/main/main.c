#include "esp_log.h"
#include "esp_err.h"
#include "app_camera.h"
#include "app_wifi.h"
#include "app_aws.h"

static const char *TAG = "main";

void app_main(void)
{
    esp_err_t err = initialize_wifi();
    if (err != ESP_OK)
        ESP_LOGE(TAG, "WiFi init failed with error 0x%x", err);

    // WiFi needs to be connected for MQTT to work
    assert(err == ESP_OK);

    err = initialize_camera();
    if (err != ESP_OK)
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);

    err = start_mqtt();
    if (err != ESP_OK)
        ESP_LOGE(TAG, "AWS MQTT failed to start");

    // Ensure nothing is broken
    ESP_ERROR_CHECK(err);
}
