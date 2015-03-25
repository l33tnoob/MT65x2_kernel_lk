#ifndef __ENV_H__
#define __ENV_H__

#define CFG_ENV_SIZE 0x4000 //(16KB)
#define CFG_ENV_OFFSET 0x20000 //(128KB)

#define CFG_ENV_DATA_SIZE (CFG_ENV_SIZE-sizeof(g_env.checksum)-sizeof(g_env.sig)-sizeof(g_env.sig_1))
#define CFG_ENV_DATA_OFFSET (sizeof(g_env.sig))
#define CFG_ENV_SIG_1_OFFSET (CFG_ENV_SIZE - sizeof(g_env.checksum)-sizeof(g_env.sig_1))
#define CFG_ENV_CHECKSUM_OFFSET (CFG_ENV_SIZE - sizeof(g_env.checksum))

#define ENV_PART PART_MISC

#define ENV_SIG "ENV_v1"

typedef struct env_struct
{
	char sig[8]; // "ENV_v1"
	char *env_data;
	char sig_1[8];  //"ENV_v1"
	int checksum; // checksum for env_data
}env_t;

extern void env_init(void);
extern char *get_env(char *name);
extern int set_env(char *name,char *value);
extern void print_env(void);
#endif
