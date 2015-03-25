#ifndef _MMP_HASH_H_
#define _MMP_HASH_H_

#define MMP_EVENT_HASHTABLE_LENGTH 100

struct mmp_event_info {
	unsigned int id;
    char *name;   
	int active;
};


struct mmp_event_info *mmp_hash_search(const char *str, struct mmp_event_info *ht);

struct mmp_event_info *mmp_hash_insert(const char *str, unsigned int id, struct mmp_event_info *ht);

#endif  // _MMP_HASH_H_
