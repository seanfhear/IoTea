# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES 
  esp32-camera 
  mbedtls
  nvs_flash
)
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS 
  "main.c" 
  "app_camera.c" 
  "app_wifi.c"
)
set(COMPONENT_ADD_INCLUDEDIRS "include")

register_component()
