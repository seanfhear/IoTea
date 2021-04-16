#include <string.h>

#include "app_aws.h"
#include "app_camera.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "cJSON.h"
#include "aws_iot_config.h"
#include "aws_iot_mqtt_client_interface.h"

static const char *TAG = "app_aws";
static const char *IMAGE_CATEGORY = "image";

extern const uint8_t aws_root_ca_pem_start[] asm("_binary_aws_root_ca_pem_start");
extern const uint8_t aws_root_ca_pem_end[] asm("_binary_aws_root_ca_pem_end");
extern const uint8_t certificate_pem_crt_start[] asm("_binary_certificate_pem_crt_start");
extern const uint8_t certificate_pem_crt_end[] asm("_binary_certificate_pem_crt_end");
extern const uint8_t private_pem_key_start[] asm("_binary_private_pem_key_start");
extern const uint8_t private_pem_key_end[] asm("_binary_private_pem_key_end");

char HostAddress[255] = AWS_IOT_MQTT_HOST;
uint32_t port = AWS_IOT_MQTT_PORT;

void iot_subscribe_callback_handler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                                    IoT_Publish_Message_Params *params, void *pData)
{
  ESP_LOGI(TAG, "Recieved trigger message");

  cJSON *root = cJSON_Parse(params->payload);
  if (root == NULL)
  {
    ESP_LOGI(TAG, "Trigger is not for camera");
    return;
  }
  cJSON *category = cJSON_GetObjectItem(root, "category");
  if (category == NULL)
  {
    ESP_LOGI(TAG, "Trigger is not for camera");
    return;
  }
  if (strcmp(category->valuestring, IMAGE_CATEGORY) != 0)
  {
    ESP_LOGI(TAG, "Trigger is not for camera");
    return;
  }

  cJSON_Delete(root);

  uint8_t *img;
  // TODO: Figure out a way to dynamically set an appropriate buffersize?
  // Experimentally found 200,000 to be more than enough (usually used ~64,000 for encoded image).
  // Initially used (2560 * 1920) * sizeof(char) but this is way overkill.
  size_t img_buff_size = 200000;
  size_t olen = 0;

  esp_err_t err = get_base64_image(&img, img_buff_size, &olen);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "Camera capture failed");
    // TODO publish MQTT message for failure
  }
  else
  {
    ESP_LOGI(TAG, "Camera capture succeeded");
    ESP_LOGI(TAG, "Base64 encoded image is %d bytes long", olen);
  }

  IoT_Publish_Message_Params publishParams;
  publishParams.qos = QOS1;
  publishParams.payload = (void *)img;
  publishParams.isRetained = 0;

  publishParams.payloadLen = olen;
  IoT_Error_t rc = aws_iot_mqtt_publish(pClient, topicName, topicNameLen, &publishParams);
  if (rc == MQTT_REQUEST_TIMEOUT_ERROR)
  {
    ESP_LOGW(TAG, "QOS1 publish ack not received.");
    rc = SUCCESS;
  }
  else if (rc != SUCCESS)
    ESP_LOGE(TAG, "Failed to publish image rc:%d", rc);

  free(img);
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
  // Set AWS Client initialization parameters
  IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;

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

  // Initialize AWS MQTT
  IoT_Error_t rc = aws_iot_mqtt_init(client, &mqttInitParams);
  if (SUCCESS != rc)
  {
    ESP_LOGE(TAG, "aws_iot_mqtt_init returned error : %d ", rc);
    return ESP_FAIL;
  }

  // Set IoT Client connection parameters
  IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;

  connectParams.keepAliveIntervalInSec = 10;
  connectParams.isCleanSession = true;
  connectParams.MQTTVersion = MQTT_3_1_1;
  connectParams.pClientID = CONFIG_AWS_IOT_CLIENT_ID;
  connectParams.clientIDLen = (uint16_t)strlen(CONFIG_AWS_IOT_CLIENT_ID);
  connectParams.isWillMsgPresent = false;

  // Connect to AWS
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

  // Turn back on autoreconnect
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
  // Initialize AWS Client
  AWS_IoT_Client client;
  ESP_ERROR_CHECK(aws_mqtt_init(&client));

  // Subscribe to the trigger topic for PLANT_NAME
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

  // Loop continuously to listen for messages on the topic. Yields to provide time to process message
  while ((NETWORK_ATTEMPTING_RECONNECT == rc || NETWORK_RECONNECTED == rc || SUCCESS == rc))
  {
    rc = aws_iot_mqtt_yield(&client, 100);
    if (NETWORK_ATTEMPTING_RECONNECT == rc)
      continue;
  }

  // We shouldn't ever get here so panic if we do
  ESP_LOGE(TAG, "An error occurred in the main loop.");
  abort();
}

esp_err_t start_mqtt()
{
  // Create the task to initialize the AWS client and to connect to MQTT
  xTaskCreatePinnedToCore(&aws_task, "aws_task", 9216, NULL, 5, NULL, 1);
  return ESP_OK;
}
