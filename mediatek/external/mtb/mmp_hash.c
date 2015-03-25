#include <stdio.h>
#include <stdlib.h>
#include <mmp_hash.h>

#define MMP_EVENT_NAME_LENGTH   32   

struct mmp_event_info mmpHashTable[MMP_EVENT_HASHTABLE_LENGTH]; 

unsigned int hashKey(const char *str) {
	unsigned int key = 0;
	
	for (key = 0; *str != '\0'; str++) {
		key = *str + 31 * key;
	}

	return key % MMP_EVENT_HASHTABLE_LENGTH;
}

struct mmp_event_info *mmp_hash_search(const char *str, struct mmp_event_info *ht) {
	struct mmp_event_info *event;
	
	event = &ht[hashKey(str)];
	if ((event->active == 1) && (strcmp(str, event->name) == 0)) {
		return event;
	}

	return NULL;
}

struct mmp_event_info *mmp_hash_insert(const char *str, unsigned int id, struct mmp_event_info *ht) {   
    struct mmp_event_info *event;
	unsigned int key;

    key = hashKey(str);

    event = &ht[key];
	event->active = 1;
	event->id = id;
    event->name = strdup(str);

	return event;
}   
