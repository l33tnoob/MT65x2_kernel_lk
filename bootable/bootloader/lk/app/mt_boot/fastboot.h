#ifndef __FASTBOOT_H
#define __FASTBOOT_H

struct fastboot_cmd {
	struct fastboot_cmd *next;
	const char *prefix;
	unsigned prefix_len;
    unsigned sec_support;
	void (*handle)(const char *arg, void *data, unsigned sz);
};

struct fastboot_var {
	struct fastboot_var *next;
	const char *name;
	const char *value;
};

void fastboot_okay(const char *info);
void fastboot_fail(const char *reason);
void fastboot_register(const char *prefix, void (*handle)(const char *arg, void *data, unsigned sz), unsigned char security_enabled);
extern void fastboot_info(const char *reason);
#define STATE_OFFLINE	0
#define STATE_COMMAND	1
#define STATE_COMPLETE	2
#define STATE_ERROR	3

#endif
