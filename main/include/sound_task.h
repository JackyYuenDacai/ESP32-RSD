#include"main.h"

#ifndef SOUND_TASK_H
#define SOUND_TASK_H

#define SOUNDSENSOR_GPIO 35
#define MAX_SAMPLES_BUF 200
#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   64          //Multisampling
#define SOUND_TASK_MS	20
#define MIN_GAP 20

void sound_task(void*pvParameters);
struct nas_entry* get_record_package(struct nas_entry* entry,struct tm* time);
void sound_task_init();

extern int sound_samples_start;
extern int sound_samples_index;

static esp_adc_cal_characteristics_t *adc_chars;
#if CONFIG_IDF_TARGET_ESP32
static const adc_channel_t channel = ADC_CHANNEL_7;     //GPIO34 if ADC1, GPIO14 if ADC2
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;
#elif CONFIG_IDF_TARGET_ESP32S2
static const adc_channel_t channel = ADC_CHANNEL_67     // GPIO7 if ADC1, GPIO17 if ADC2
static const adc_bits_width_t width = ADC_WIDTH_BIT_13;
#endif
static const adc_atten_t atten = ADC_ATTEN_DB_0;
static const adc_unit_t unit = ADC_UNIT_1;

#endif