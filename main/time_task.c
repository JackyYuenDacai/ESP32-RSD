#include "time_task.h"

const char *ntpServer = "stdtime.gov.hk";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

//Mon, 21 Dec 2020 13:16:30 GMT
const char* wd_abrev[7]={
	"Sun","Mon","Tue","Wed","Thu","Fri","Sat"
};
const char* mt_abrev[12]={
	"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
};
uint8_t parse_time2tm(struct tm* time,char* time_string){
	char wd_buf[16]={0};
	char md_buf[8]={0};
	char mt_buf[16]={0};
	char yr_buf[8]={0};
	char arbi_buf[6]={0};
	char tz_buf[8]={0};
	int i=0,j=0,len=0;
	if(time==NULL)return -1;
	while(time_string[i++]!='\0'){if(i>=40)return -1;}
	if((len=strlen(time_string))<=1) return -1;


	i=0;j=0;
	while(time_string[i]!=','&&i<len)
		wd_buf[j++]=time_string[i++];

	i++;j=0;
	while(time_string[i]==' '&&i<len)i++;
	while(time_string[i]!=' '&&i<len)
		md_buf[j++]=time_string[i++];
	time->tm_mday = atoi(md_buf);

	i++;j=0;
	while(time_string[i]==' '&&i<len)i++;
	while(time_string[i]!=' '&&i<len)
		mt_buf[j++]=time_string[i++];

	i++;j=0;
	while(time_string[i]==' '&&i<len)i++;
	while(time_string[i]!=' '&&i<len)
		yr_buf[j++]=time_string[i++];
	time->tm_year = atoi(yr_buf)-1900;
	i++;j=0;
	while(time_string[i]==' '&&i<len)i++;
	while(time_string[i]!=':'&&i<len)
		arbi_buf[j++]=time_string[i++];
	arbi_buf[j]=0;
	time->tm_hour = atoi(arbi_buf);

	i++;j=0;
	while(time_string[i]==' '&&i<len)i++;
	while(time_string[i]!=':'&&i<len)
		arbi_buf[j++]=time_string[i++];
	arbi_buf[j]=0;
	time->tm_min = atoi(arbi_buf);

	i++;j=0;
	while(time_string[i]==' '&&i<len)i++;
	while(time_string[i]!=' '&&i<len)
		arbi_buf[j++]=time_string[i++];
	arbi_buf[j]=0;
	time->tm_sec = atoi(arbi_buf);

	//Parse month
	for(i=0;i<12;i++){
		if(strstr(mt_buf,mt_abrev[i])!=NULL){
			time->tm_mon=i;
			break;
		}
	}

	//Parse weekday
	for(i=0;i<7;i++){
		if(strstr(wd_buf,wd_abrev[i])!=NULL){
			time->tm_wday=i;
			break;
		}
	}

	//ESP_LOGI(TAG,"TBUF: %s %s %s %s",wd_buf,md_buf,mt_buf,yr_buf);
	ESP_LOGI(TAG,"TPAR: %d %d %d %d %d %d",time->tm_mday,time->tm_mon,time->tm_year+1900,time->tm_hour,time->tm_min,time->tm_sec);
	return 0;
}

static void time_task(void *pvp)
{
	char tm_buf[32] = {0};
	while (1)
	{
		time_t now;
		struct tm timeinfo;
		time(&now);
		localtime_r(&now, &timeinfo);
		strftime(tm_buf,32,"%F %T %Z",&timeinfo);
		ESP_LOGI(TAG,"TIME :%s",tm_buf);
		vTaskDelay(10000 / portTICK_PERIOD_MS);
	}
}
void time_task_init()
{
	esp_newlib_time_init();
	xTaskCreate(&time_task, "time_sync_task", 2048, NULL, 7, NULL);
}