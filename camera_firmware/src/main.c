#include "mgos.h"
#include <stdio.h>
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"

#include "esp_camera.h"

//ESP32-EYE
#define CAM_PIN_PWDN -1
#define CAM_PIN_RESET -1 //software reset will be performed
#define CAM_PIN_XCLK 4   //GPIO
#define CAM_PIN_SIOD 18  //GPIO
#define CAM_PIN_SIOC 23  //GPIO

#define CAM_PIN_D7 36   //GPIO
#define CAM_PIN_D6 37   //GPIO
#define CAM_PIN_D5 38   //GPIO (sensor VN)
#define CAM_PIN_D4 39   //GPIO (sensor VP)
#define CAM_PIN_D3 35   //GPIO
#define CAM_PIN_D2 14   //GPIO
#define CAM_PIN_D1 13   //GPIO
#define CAM_PIN_D0 34   //GPIO
#define CAM_PIN_VSYNC 5 //GPIO
#define CAM_PIN_HREF 27 //GPIO
#define CAM_PIN_PCLK 25 //GPIO

static camera_config_t camera_config = {
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

    //XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG, //YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size = FRAMESIZE_QXGA,   //QQVGA-QXGA Do not use sizes above QVGA when not JPEG

    .jpeg_quality = 10, //0-63 lower number means higher quality
    .fb_count = 2       //if more than one, i2s runs in continuous mode. Use only with JPEG
};

esp_err_t camera_init1()
{
  //power up the camera if PWDN pin is defined
  if (CAM_PIN_PWDN != -1)
  {
    //pinMode(CAM_PIN_PWDN, OUTPUT);
    //digitalWrite(CAM_PIN_PWDN, LOW);
    gpio_pad_select_gpio(CAM_PIN_PWDN);
    gpio_set_direction(CAM_PIN_PWDN, GPIO_MODE_OUTPUT);
    gpio_set_level(CAM_PIN_PWDN, 0);
  }

  //initialize the camera
  esp_err_t err = esp_camera_init(&camera_config);
  if (err != ESP_OK)
  {
    return err;
  }

  return ESP_OK;
}

void camera_app_main()
{
  printf("Begining of code\n");

  printf("Camera init...\n");
  esp_err_t res = camera_init1();
  if (res == ESP_OK)
  {
    printf("Camera init success!!\n");
  }
  else
  {
    printf("Camera init fail!!\n");
  }

  // for (int i = 10; i >= 0; i--)
  // {
  //   printf("Taking Picture in %d seconds...\n", i);
  //   vTaskDelay(1000 / portTICK_PERIOD_MS);
  // }
  // printf("Taking Picture Now!...\n");

  // camera_fb_t *fb = esp_camera_fb_get();
  // if (!fb)
  // {
  //   printf("Camera capture fail!!\n");
  // }
  // else
  // {
  //   printf("Camera capture success!!\n");
  // }

  // fflush(stdout);
}

enum mgos_app_init_result mgos_app_init(void)
{
  return MGOS_APP_INIT_SUCCESS;
}
