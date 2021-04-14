#include <assert.h>
#include "app_camera.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "mbedtls/base64.h"

static const char *TAG = "app_camera";

esp_err_t initialize_camera()
{
  /* IO13, IO14 is designed for JTAG by default,
     * to use it as generalized input,
     * firstly declare it as pullup input */
  gpio_config_t conf;
  conf.mode = GPIO_MODE_INPUT;
  conf.pull_up_en = GPIO_PULLUP_ENABLE;
  conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  conf.intr_type = GPIO_INTR_DISABLE;
  conf.pin_bit_mask = 1LL << 13;
  gpio_config(&conf);
  conf.pin_bit_mask = 1LL << 14;
  gpio_config(&conf);

  camera_config_t camera_config = {
      .pin_pwdn = CAM_PIN_PWDN,
      .pin_reset = CAM_PIN_RESET,
      .pin_xclk = CAM_PIN_XCLK,
      .pin_sscb_sda = CAM_PIN_SIOD,
      .pin_sscb_scl = CAM_PIN_SIOC,

      .pin_d7 = CAM_PIN_D7,
      .pin_d6 = CAM_PIN_D6,
      .pin_d5 = CAM_PIN_D5,
      .pin_d4 = CAM_PIN_D4,
      .pin_d3 = CAM_PIN_D3,
      .pin_d2 = CAM_PIN_D2,
      .pin_d1 = CAM_PIN_D1,
      .pin_d0 = CAM_PIN_D0,
      .pin_vsync = CAM_PIN_VSYNC,
      .pin_href = CAM_PIN_HREF,
      .pin_pclk = CAM_PIN_PCLK,

      // XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
      .xclk_freq_hz = 20000000,
      .ledc_timer = LEDC_TIMER_0,
      .ledc_channel = LEDC_CHANNEL_0,

      .pixel_format = PIXFORMAT_JPEG, // YUV422,GRAYSCALE,RGB565,JPEG
      .frame_size = FRAMESIZE_QSXGA,  // QQVGA-QXGA Do not use sizes above QVGA when not JPEG

      .jpeg_quality = 10, // 0-63 lower number means higher quality
      .fb_count = 2       // if more than one, i2s runs in continuous mode. Use only with JPEG
  };

  // Initialize the camera
  esp_err_t err = esp_camera_init(&camera_config);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "Camera Init Failed");
    return err;
  }

  // See https://randomnerdtutorials.com/esp32-cam-ov2640-camera-settings for we can change for this sensor.
  sensor_t *s = esp_camera_sensor_get();
  s->set_vflip(s, 1);

  return ESP_OK;
}

esp_err_t get_base64_image(uint8_t **img, size_t img_buff_size, size_t *olen)
{
  assert(img != NULL);

  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb)
  {
    ESP_LOGE(TAG, "Camera Capture Failed");
    return ESP_FAIL;
  }

  *img = (uint8_t *)malloc(img_buff_size);

  int err = mbedtls_base64_encode(*img, img_buff_size, olen, fb->buf, fb->len);
  if (err != 0)
  {
    ESP_LOGE(TAG, "mbed_tls: Failed to encode image as base64");
    ESP_LOGE(TAG, "Buffer size needed for base64 encoding = %d \n", *olen);
    ESP_LOGE(TAG, "Buffer size allocated = %d \n", img_buff_size);

    return ESP_FAIL;
  }

  esp_camera_fb_return(fb);
  return ESP_OK;
}
