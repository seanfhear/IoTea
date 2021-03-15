#include "mgos.h"
#include "mgos_wifi.h"

enum mgos_app_init_result mgos_app_init(void) {
  char* msg ="";
  struct mgos_config_wifi_sta cfg;
  // If you change this enable to 1, it core dumps,honestly there's 3 different places that say 3 different things about what this enable does :/, Otherwise everything works fine and then final connect call just fails :( )
  cfg.enable = 0;
  cfg.ssid = "TCDwifi";
  cfg.pass = "pw";
  cfg.user = "user";

  // validate config
  bool valid  =  mgos_wifi_validate_sta_cfg(&cfg,&msg);
  printf("Config Valid %s\n", valid ? "true" : "false");
  printf("Error message: %s\n", msg);

  // Setup station
  bool setup = mgos_wifi_setup_sta(&cfg);
  printf("Wifi Setup completed %s\n:", setup ? "true" : "false");

  // Connect to previously setup station
  bool connect = mgos_wifi_connect();
  printf("WIFI connected %s\n:", connect ? "true" : "false");

  return MGOS_APP_INIT_SUCCESS;
}
