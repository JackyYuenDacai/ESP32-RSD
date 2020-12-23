#include"main.h"
#ifndef NAS_QUEUE_H
#define NAS_QUEUE_H
#define NAS_QUEUE_MAX 32
struct nas_entry{
	char time_str[32];
	int milli_mul;
	int milli_index;
	float avg_amp;
	float max_amp;
	float min_amp;
};
extern int nas_queue_index;
extern struct nas_entry nas_queue_entity[NAS_QUEUE_MAX];
unsigned char nas_queue_push(struct nas_entry* entry);
unsigned char nas_queue_empty();
unsigned char nas_queue_is_empty();
struct nas_entry* nas_queue_pop();
struct nas_entry* nas_queue_top();

#endif