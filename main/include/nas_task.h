#include"main.h"
#ifndef NAS_TASK_H
#define NAS_TASK_H

extern const char *NAS_REQUEST_FORMAT ;
extern const char *NAS_HOST;
extern char NAS_URL[180];
void nas_task(void*pvp);
void nas_task_init();
#endif