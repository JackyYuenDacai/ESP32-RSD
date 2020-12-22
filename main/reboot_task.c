#include"reboot_task.h"

void reboot_task(void*pv){

	for(int i=0;i<120;i++){
		for(int j=0;j<60;j++){
			vTaskDelay(1000 / portTICK_PERIOD_MS);
		}
		if(i%5==0){
			ESP_LOGI(TAG,"%d mins left to reboot.",120-i);
		}
	}
	esp_restart();
}
void reboot_task_init(){
	xTaskCreate(&reboot_task, "reboot_task", 2048, NULL, 10, NULL);
}