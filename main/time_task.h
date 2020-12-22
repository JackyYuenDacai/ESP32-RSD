#include "main.h"
#ifndef TIME_TASK_H
#define TIME_TASK_H


static void time_task(void *pvp);
void time_task_init();
uint8_t parse_time2tm(struct tm* time,char* time_string);
#endif