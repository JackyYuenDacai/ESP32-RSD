#include "time_task.h"

//Mon, 21 Dec 2020 13:16:30 GMT

void time_task(void *pvp)
{
	char tm_buf[32] = {0};
	while (1)
	{
		time_t now;
		struct tm timeinfo;
		time(&now);
		localtime_r(&now, &timeinfo);
		strftime(tm_buf, 32, "%F %T %Z", &timeinfo);
		ESP_LOGI(TAG, "TIME :%s", tm_buf);
		vTaskDelay(10000 / portTICK_PERIOD_MS);
	}
}
void time_task_init()
{
	esp_newlib_time_init();
	xTaskCreate(&time_task, "time_sync_task", 2048, NULL, 7, NULL);
}