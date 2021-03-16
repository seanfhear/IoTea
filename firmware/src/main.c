#include "mgos.h"
#include "mgos_wifi.h"
#include "esp_wpa2.h"
#include "esp_wifi.h"

#define EAP_ID "ldap_username_without@tcd.ie"
#define EAP_PASSWORD "password"
#define SSID "TCDwifi"

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        printf("Connecting to wifi!\n");
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        printf("Obtained a wifi IP!\n");
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      printf("Disconnected from the wifi :(\n");
      esp_wifi_connect();
      printf("Reconnecting!\n");
      break;
    default:
        break;
    }
    return ESP_OK;
}

enum mgos_app_init_result mgos_app_init(void) {
  //char *msg = "";
  //struct mgos_config_wifi_sta cfg = { .enable = 1, .ssid = "TCDwifi", .pass = "password", .user = "user", .nameserver = "134.226.251.200" };

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_event_loop_init(event_handler, NULL);

  esp_wifi_init(&cfg);

  wifi_config_t wifi_config = {
      .sta = {
          .ssid = SSID,
      },
  };

  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
  esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_ID, strlen(EAP_ID));
  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_ID, strlen(EAP_ID));
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD));
  esp_wifi_sta_wpa2_ent_enable();
  esp_wifi_start();

  esp_wifi_connect();

/*
  // validate config
  bool valid = mgos_wifi_validate_sta_cfg(&cfg, &msg);
  printf("Config Valid %s\n", valid ? "true" : "false");
  printf("Error message: %s\n", msg);

  // Setup station
  bool setup = mgos_wifi_setup_sta(&cfg);
  printf("Wifi Setup completed %s\n:", setup ? "true" : "false");

  // Connect to previously setup station
  bool connect = mgos_wifi_connect();
  printf("WIFI connected %s\n:", connect ? "true" : "false");
*/
  return MGOS_APP_INIT_SUCCESS;
}
