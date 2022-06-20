#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_log.h"

#include "status.h"
#include "ble_server.h"
#include "wifi.h"

void app_main() {
  // Initialize NVS
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
      // NVS partition was truncated and needs to be erased
      // Retry nvs_flash_init
      ESP_ERROR_CHECK(nvs_flash_erase());
      err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);
  // Initialize BLE and WIFI controllers
  ble_controllers_init();
  wifi_controllers_init();
  while (1)
  {
    switch_status();
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}