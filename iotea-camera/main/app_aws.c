#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

#include "app_aws.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "driver/sdmmc_host.h"

#include "nvs.h"
#include "nvs_flash.h"

#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"

static const char *TAG = "app_aws";

extern const uint8_t aws_root_ca_pem_start[] asm("_binary_aws_root_ca_pem_start");
extern const uint8_t aws_root_ca_pem_end[] asm("_binary_aws_root_ca_pem_end");
extern const uint8_t certificate_pem_crt_start[] asm("_binary_certificate_pem_crt_start");
extern const uint8_t certificate_pem_crt_end[] asm("_binary_certificate_pem_crt_end");
extern const uint8_t private_pem_key_start[] asm("_binary_private_pem_key_start");
extern const uint8_t private_pem_key_end[] asm("_binary_private_pem_key_end");

/**
 * @brief Default MQTT HOST URL is pulled from the aws_iot_config.h
 */
char HostAddress[255] = AWS_IOT_MQTT_HOST;

/**
 * @brief Default MQTT port is pulled from the aws_iot_config.h
 */
uint32_t port = AWS_IOT_MQTT_PORT;

void iot_subscribe_callback_handler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                                    IoT_Publish_Message_Params *params, void *pData)
{
  ESP_LOGI(TAG, "Message recieved in trigger topic");
}

void disconnectCallbackHandler(AWS_IoT_Client *pClient, void *data)
{
  ESP_LOGW(TAG, "MQTT Disconnect");
  IoT_Error_t rc = FAILURE;

  if (NULL == pClient)
    return;

  if (aws_iot_is_autoreconnect_enabled(pClient))
    ESP_LOGI(TAG, "Auto Reconnect is enabled, Reconnecting attempt will start now");
  else
  {
    ESP_LOGW(TAG, "Auto Reconnect not enabled. Starting manual reconnect...");
    rc = aws_iot_mqtt_attempt_reconnect(pClient);
    if (NETWORK_RECONNECTED == rc)
      ESP_LOGW(TAG, "Manual Reconnect Successful");
    else
      ESP_LOGW(TAG, "Manual Reconnect Failed - %d", rc);
  }
}

esp_err_t aws_mqtt_init(AWS_IoT_Client *client)
{
  IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;
  IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;

  ESP_LOGI(TAG, "AWS IoT SDK Version %d.%d.%d-%s", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);

  mqttInitParams.enableAutoReconnect = false; // We enable this later below
  mqttInitParams.pHostURL = HostAddress;
  mqttInitParams.port = port;

  mqttInitParams.pRootCALocation = (const char *)aws_root_ca_pem_start;
  mqttInitParams.pDeviceCertLocation = (const char *)certificate_pem_crt_start;
  mqttInitParams.pDevicePrivateKeyLocation = (const char *)private_pem_key_start;

  mqttInitParams.mqttCommandTimeout_ms = 20000;
  mqttInitParams.tlsHandshakeTimeout_ms = 5000;
  mqttInitParams.isSSLHostnameVerify = true;
  mqttInitParams.disconnectHandler = disconnectCallbackHandler;
  mqttInitParams.disconnectHandlerData = NULL;

  IoT_Error_t rc = aws_iot_mqtt_init(client, &mqttInitParams);
  if (SUCCESS != rc)
  {
    ESP_LOGE(TAG, "aws_iot_mqtt_init returned error : %d ", rc);
    return ESP_FAIL;
  }

  connectParams.keepAliveIntervalInSec = 10;
  connectParams.isCleanSession = true;
  connectParams.MQTTVersion = MQTT_3_1_1;
  connectParams.pClientID = CONFIG_AWS_IOT_CLIENT_ID;
  connectParams.clientIDLen = (uint16_t)strlen(CONFIG_AWS_IOT_CLIENT_ID);
  connectParams.isWillMsgPresent = false;

  ESP_LOGI(TAG, "Connecting to AWS...");
  do
  {
    rc = aws_iot_mqtt_connect(client, &connectParams);
    if (SUCCESS != rc)
    {
      ESP_LOGE(TAG, "Error(%d) connecting to %s:%d", rc, mqttInitParams.pHostURL, mqttInitParams.port);
      vTaskDelay(1000 / portTICK_RATE_MS);
    }
  } while (SUCCESS != rc);

  rc = aws_iot_mqtt_autoreconnect_set_status(client, true);
  if (SUCCESS != rc)
  {
    ESP_LOGE(TAG, "Unable to set Auto Reconnect to true - %d", rc);
    return ESP_FAIL;
  }

  return ESP_OK;
}

void aws_task(void *param)
{
  AWS_IoT_Client client;
  ESP_ERROR_CHECK(aws_mqtt_init(&client));

  char topic_name[100];
  sprintf(topic_name, "IoTea/%s/trigger", CONFIG_PLANT_NAME);

  const char *TOPIC = (const char *)&topic_name;
  const int TOPIC_LEN = strlen(TOPIC);

  ESP_LOGI(TAG, "Subscribing to %s", TOPIC);
  IoT_Error_t rc = aws_iot_mqtt_subscribe(&client, TOPIC, TOPIC_LEN, QOS0, iot_subscribe_callback_handler, NULL);
  if (SUCCESS != rc)
  {
    ESP_LOGE(TAG, "Error subscribing : %d ", rc);
    abort();
  }
  ESP_LOGI(TAG, "Successfully subscribed to topic");

  while ((NETWORK_ATTEMPTING_RECONNECT == rc || NETWORK_RECONNECTED == rc || SUCCESS == rc))
  {
    //Max time the yield function will wait for read messages
    rc = aws_iot_mqtt_yield(&client, 100);
    if (NETWORK_ATTEMPTING_RECONNECT == rc)
    {
      // If the client is attempting to reconnect we will skip the rest of the loop.
      continue;
    }
  }

  ESP_LOGE(TAG, "An error occurred in the main loop.");
  abort();
}

esp_err_t start_mqtt()
{
  xTaskCreatePinnedToCore(&aws_task, "aws_task", 9216, NULL, 5, NULL, 1);
  return ESP_OK;
}
