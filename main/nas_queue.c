#include "nas_queue.h"

int nas_queue_index = -1;
struct nas_entry nas_queue_entity[NAS_QUEUE_MAX];
void nas_assign(struct nas_entry *src, struct nas_entry *dst)
{
	if (!(src != NULL && dst != NULL))
		return;
	dst->avg_amp = src->avg_amp;
	dst->max_amp = src->max_amp;
	dst->min_amp = src->min_amp;
	dst->milli_index = src->milli_index;
	dst->milli_mul = src->milli_mul;
	sprintf(dst->time_str, "%s", src->time_str);
}
unsigned char nas_queue_push(struct nas_entry *entry)
{
	if (nas_queue_index + 1 > NAS_QUEUE_MAX)
	{

		for (int i = 0; i < NAS_QUEUE_MAX - 1; i++)
			nas_assign(&nas_queue_entity[i + 1], &nas_queue_entity[i]);
		nas_queue_index = NAS_QUEUE_MAX - 1;
		nas_assign(entry, &nas_queue_entity[nas_queue_index]);
		return 0;
	}
	else
	{
		nas_queue_index++;
		nas_assign(entry, &nas_queue_entity[nas_queue_index]);
		return 0;
	}
}
unsigned char nas_queue_empty()
{
	nas_queue_index = -1;
	return 0;
}
unsigned char nas_queue_is_empty()
{
	if (nas_queue_index <= -1)
		return 1;
	else
		return 0;
}
struct nas_entry *nas_queue_pop()
{
	if (nas_queue_index <= -1)
	{
		return NULL;
	}
	else
	{
		return &nas_queue_entity[nas_queue_index--];
	}
}
struct nas_entry *nas_queue_top()
{

	if (nas_queue_index <= -1)
		return NULL;
	else
		return &nas_queue_entity[nas_queue_index];
}