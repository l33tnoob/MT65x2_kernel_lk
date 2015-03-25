
#ifndef _SEC_UNLOCK_H_
#define _SEC_UNLOCK_H_

void fastboot_oem_key(const char *arg, void *data, unsigned sz);
int fastboot_oem_unlock(const char *arg, void *data, unsigned sz);
int fastboot_oem_unlock_chk();
int fastboot_oem_lock(const char *arg, void *data, unsigned sz);
int fastboot_oem_lock_chk();
void fastboot_oem_query_lock_state(const char *arg, void *data, unsigned sz);

#endif

