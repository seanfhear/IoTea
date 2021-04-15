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
    err = start_mqtt();
    if (err != ESP_OK)
        ESP_LOGE(TAG, "AWS MQTT failed to start");

    err = initialize_camera();
    if (err != ESP_OK)
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);

    uint8_t *img;
    // TODO: Figure out a way to dynamically set an appropriate buffersize?
    // Experimentally found 500,000 to be more than enough (usually used ~64,000 for encoded image).
    // Initially used (2560 * 1920) * sizeof(char) but this is way overkill.
    size_t img_buff_size = 500000;
    size_t olen = 0;

    err = get_base64_image(&img, img_buff_size, &olen);
    if (err != ESP_OK)
        ESP_LOGE(TAG, "Camera capture failed");

    free(img);

    // Ensure nothing is broken
    ESP_ERROR_CHECK(err);
}
