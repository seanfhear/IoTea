# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES 
  esp32-camera 
  mbedtls
  nvs_flash
  esp-aws-iot
  json
)
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS 
  "main.c" 
  "app_camera.c" 
  "app_wifi.c"
  "app_aws.c"
)
set(COMPONENT_ADD_INCLUDEDIRS "include")

register_component()

target_add_binary_data(${COMPONENT_TARGET} "certs/aws-root-ca.pem" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "certs/certificate.pem.crt" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "certs/private.pem.key" TEXT)  
