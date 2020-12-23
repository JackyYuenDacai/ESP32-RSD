#include "main.h"

void app_main(void)
{

	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");

	wifi_init_sta();
	mbedtls_ssl_config conf;
	mbedtls_ssl_config_init(&conf);
	esp_crt_bundle_attach(&conf);

	freewifi_init();
	time_task_init();
	nas_task_init();
	sound_task_init();
	reboot_task_init();
}
