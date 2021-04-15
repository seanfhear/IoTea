#include <string.h>
#include "app_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

#define WIFI_CONNECTED_BIT (1 << 0)
#define WIFI_FAIL_BIT (1 << 1)

static const char *TAG = "app_wifi";

static int s_retries_count = 0;

EventGroupHandle_t s_app_event_group;

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    esp_wifi_connect();
  else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
  {
    if (s_retries_count < CONFIG_ESP_MAXIMUM_RETRY)
    {
      ESP_LOGI(TAG, "Retrying to connect...");
      esp_wifi_connect();
      s_retries_count++;
    }
    else
      xEventGroupSetBits(s_app_event_group, WIFI_FAIL_BIT);
  }
  else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
  {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    s_retries_count = 0;
    // ESP_LOGI(TAG, "IP: %s", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
    xEventGroupSetBits(s_app_event_group, WIFI_CONNECTED_BIT);
  }
}

esp_err_t initialize_wifi()
{
  if (!strlen(CONFIG_ESP_WIFI_SSID))
  {
    ESP_LOGE(TAG, "SSID has not been provided so wifi will be turned off.");
    return ESP_FAIL;
  }

  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  s_app_event_group = xEventGroupCreate();

  esp_netif_init();

  ESP_ERROR_CHECK(esp_event_loop_create_default());

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

  wifi_config_t wifi_config = {
      .sta = {
          .ssid = CONFIG_ESP_WIFI_SSID,
          .password = CONFIG_ESP_WIFI_PASSWORD},
  };

  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

  ESP_LOGI(TAG, "Connecting to SSID:%s", CONFIG_ESP_WIFI_SSID);

  ESP_ERROR_CHECK(esp_wifi_start());

  EventBits_t bits = xEventGroupWaitBits(s_app_event_group,
                                         WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                         pdFALSE,
                                         pdFALSE,
                                         portMAX_DELAY);

  ret = ESP_OK;
  if (bits & WIFI_FAIL_BIT)
  {
    ESP_LOGE(TAG, "Failed to connect to a Wi-Fi. Are the SSID and password set correctly in the config?");
    ret = ESP_FAIL;
  }
  else if (bits & WIFI_CONNECTED_BIT)
    ESP_LOGI(TAG, "Connected to %s", CONFIG_ESP_WIFI_SSID);

  ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
  ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
  vEventGroupDelete(s_app_event_group);

  return ret;
}
