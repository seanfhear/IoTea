#include "esp_log.h"
#include "app_camera.h"

static const char *TAG = "main";

esp_err_t camera_capture()
{
    //acquire a frame
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb)
    {
        ESP_LOGE(TAG, "Camera Capture Failed");
        return ESP_FAIL;
    }
    //replace this with your own function
    // process_image(fb->width, fb->height, fb->format, fb->buf, fb->len);

    //return the frame buffer back to the driver for reuse
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

    err = camera_capture();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera capture failed");
    }

    ESP_LOGE(TAG, "Success!!");
}
