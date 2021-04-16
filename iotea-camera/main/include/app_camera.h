#ifndef _APP_CAMERA_H_
#define _APP_CAMERA_H_

#include "esp_camera.h"

// ESP-EYE PIN Map
#define CAM_PIN_PWDN -1  // power down is not used
#define CAM_PIN_RESET -1 // software reset will be performed
#define CAM_PIN_XCLK 4
#define CAM_PIN_SIOD 18
#define CAM_PIN_SIOC 23

#define CAM_PIN_D7 36
#define CAM_PIN_D6 37
#define CAM_PIN_D5 38
#define CAM_PIN_D4 39
#define CAM_PIN_D3 35
#define CAM_PIN_D2 14
#define CAM_PIN_D1 13
#define CAM_PIN_D0 34

#define CAM_PIN_VSYNC 5
#define CAM_PIN_HREF 27
#define CAM_PIN_PCLK 25

esp_err_t initialize_camera();
esp_err_t get_base64_image(uint8_t **img, size_t img_buff_size, size_t *olen);

#endif
