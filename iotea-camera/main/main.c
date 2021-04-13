#include <assert.h>
#include "esp_log.h"
#include "app_camera.h"
#include "mbedtls/base64.h"

static const char *TAG = "main";

esp_err_t camera_capture(uint8_t **img, size_t img_buff_size)
{
    assert(img != NULL);

    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb)
    {
        ESP_LOGE(TAG, "Camera Capture Failed");
        return ESP_FAIL;
    }

    *img = (uint8_t *)malloc(img_buff_size);
    size_t olen;

    int err = mbedtls_base64_encode(*img, img_buff_size, &olen, fb->buf, fb->len);
    if (err != 0)
    {
        ESP_LOGE(TAG, "mbed_tls: Failed to encode image as base64");
        ESP_LOGE(TAG, "Buffer size needed for base64 encoding = %d \n", olen);
        ESP_LOGE(TAG, "Buffer size allocated = %d \n", img_buff_size);

        return ESP_FAIL;
    }

    esp_camera_fb_return(fb);
    return ESP_OK;
}

void app_main(void)
{
    esp_err_t err = initialize_camera();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera init failed");
    }

    uint8_t *img;
    // TODO: Figure out a way to dynamically set an appropriate buffersize?
    // Experimentally found 100,000 to be more than enough (usually used ~64,000
    // for encoded image). Initially used line below but this is way overkill.
    // size_t img_buff_size = (2560 * 1920) * sizeof(char);
    size_t img_buff_size = 100000;

    err = camera_capture(&img, img_buff_size);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera capture failed");
    }

    free(img);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Something broke :/");
    }
    else
    {
        ESP_LOGI(TAG, "Success!!");
    }
}
