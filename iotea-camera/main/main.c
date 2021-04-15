#include "esp_log.h"
#include "app_camera.h"
#include "app_wifi.h"

static const char *TAG = "main";

void app_main(void)
{
    initialize_wifi();
    esp_err_t err;
    // esp_err_t err = initialize_wifi();
    // if (err != ESP_OK)
    //     ESP_LOGE(TAG, "WiFi init failed with error 0x%x", err);

    err = initialize_camera();
    if (err != ESP_OK)
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);

    uint8_t *img;
    // TODO: Figure out a way to dynamically set an appropriate buffersize?
    // Experimentally found 100,000 to be more than enough (usually used ~64,000 for encoded image).
    // Initially used (2560 * 1920) * sizeof(char) but this is way overkill.
    size_t img_buff_size = 100000;
    size_t olen = 0;

    err = get_base64_image(&img, img_buff_size, &olen);
    if (err != ESP_OK)
        ESP_LOGE(TAG, "Camera capture failed");

    free(img);

    if (err != ESP_OK)
        ESP_LOGE(TAG, "Something broke :/");
    else
        ESP_LOGI(TAG, "Success!!");
}
