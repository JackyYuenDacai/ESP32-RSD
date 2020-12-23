#include "sound_task.h"

#define UTOF_32(ab) ((float)ab) / ((float)5000)

uint32_t sound_samples[MAX_SAMPLES_BUF] = {0};
int sound_samples_start = 0;
int sound_samples_index = 0;
struct nas_entry *pack_into_entry(struct nas_entry *entry, struct tm *time, int milli_mul, int milli_index, float avg_amp, float max_amp, float min_amp)
{
	//22-01-2015 10:15:55 AM
	if (entry == NULL)
		return NULL;

	if (time->tm_hour == 12)
	{
		sprintf(entry->time_str, "%hu-%hu-%d%%20%hu:%hu:%hu%%20PM", time->tm_mday, time->tm_mon + 1, time->tm_year + 1900, time->tm_hour, time->tm_min, time->tm_sec);
	}
	else if (time->tm_hour > 12)
	{
		sprintf(entry->time_str, "%hu-%hu-%d%%20%hu:%hu:%hu%%20PM", time->tm_mday, time->tm_mon + 1, time->tm_year + 1900, time->tm_hour - 12, time->tm_min, time->tm_sec);
	}
	else
	{
		sprintf(entry->time_str, "%hu-%hu-%d%%20%hu:%hu:%hu%%20AM", time->tm_mday, time->tm_mon + 1, time->tm_year + 1900, time->tm_hour, time->tm_min, time->tm_sec);
	}

	ESP_LOGI(TAG, "%s", entry->time_str);

	entry->avg_amp = avg_amp;
	entry->max_amp = max_amp;
	entry->min_amp = min_amp;
	entry->milli_index = milli_index;
	entry->milli_mul = milli_mul;
	return entry;
}
void record_one_sample(uint32_t vol)
{
	sound_samples_index = (sound_samples_index + 1) % MAX_SAMPLES_BUF;
	sound_samples[sound_samples_index] = vol;
}
struct nas_entry *get_record_package(struct nas_entry *entry, struct tm *time)
{
	float sum = 0.0;
	float avg = 0.0;
	float max = -9999.9;
	float min = 9999.9;
	int count = 0;
	while (sound_samples_start != sound_samples_index)
	{
		sum += (float)sound_samples[sound_samples_start];
		if (max < (float)sound_samples[sound_samples_start])
			max = (float)sound_samples[sound_samples_start];
		if (min > (float)sound_samples[sound_samples_start])
			min = (float)sound_samples[sound_samples_start];
		sound_samples_start = (sound_samples_start + 1) % MAX_SAMPLES_BUF;
		count++;
	}
	avg = sum / ((float)count);
	return pack_into_entry(entry, time, SOUND_TASK_MS, count, avg, max, min);
}
void sound_task(void *pvParameters)
{
	//Characterize ADC
	int64_t last_mtime = 0;
	int64_t now_mtime = 0;
	adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
	esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, adc_chars);
	//print_char_val_type(val_type);

	//Continuously sample ADC1
	while (1)
	{
		uint32_t adc_reading = 0;
		//Multisampling
		for (int i = 0; i < NO_OF_SAMPLES; i++)
		{
			if (unit == ADC_UNIT_1)
			{
				adc_reading += adc1_get_raw((adc1_channel_t)channel);
			}
			else
			{
				int raw;
				adc2_get_raw((adc2_channel_t)channel, width, &raw);
				adc_reading += raw;
			}
		}
		adc_reading /= NO_OF_SAMPLES;
		//Convert adc_reading to voltage in mV
		uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
		record_one_sample(voltage);
		/*
		char print_buf[32];
		sprintf(print_buf, "R: %d\tV: %dmV\n", adc_reading, voltage);
		TFT_print(print_buf, CENTER, LASTY);
		ESP_LOGI(TAG,"R: %d\tV: %dmV\n", adc_reading, voltage);
		now_mtime = xx_time_get_time() % 1000;
		if(now_mtime<last_mtime){
			time_t now;
			struct tm timeinfo;
			time(&now);
			localtime_r(&now, &timeinfo);
			push_nas_queue(&timeinfo);
			sound_samples[(now_mtime % 1000)/10]=voltage;
		}else{
			sound_samples[(now_mtime % 1000)/10]=voltage;
		}
		last_mtime = now_mtime; */

		vTaskDelay(SOUND_TASK_MS / portTICK_PERIOD_MS);
	}
}

void sound_task_init()
{
	//Configure ADC
	if (unit == ADC_UNIT_1)
	{
		adc1_config_width(width);
		adc1_config_channel_atten(channel, atten);
	}
	else
	{
		adc2_config_channel_atten((adc2_channel_t)channel, atten);
	}
	xTaskCreate(&sound_task, "sound_task", 4096, NULL, 8, NULL);
}