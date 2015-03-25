#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <fcntl.h>
#include <errno.h>
#include <dlfcn.h>

#include "PerfServiceNative.h"

#define LIB_FULL_NAME "/system/lib/libperfservicenative.so"

void (*perfBoostEnable)(int) = NULL;
void (*perfBoostDisable)(int) = NULL;
void (*perfBoostEnableTimeout)(int, int) = NULL;
int  (*perfUserScnReg)(int, int) = NULL;
void (*perfUserScnUnreg)(int) = NULL;
void (*perfUserScnEnable)(int) = NULL;
void (*perfUserScnDisable)(int) = NULL;
void (*perfUserScnEnableTimeout)(int, int) = NULL;
void (*perfUserScnResetAll)(void) = NULL;
void (*perfUserScnDisableAll)(void) = NULL;

typedef void (*ena)(int);
typedef void (*disa)(int);
typedef void (*ena_timeout)(int, int);
typedef int  (*user_reg)(int, int);
typedef void (*user_unreg)(int);
typedef void (*user_enable)(int);
typedef void (*user_disable)(int);
typedef void (*user_enable_timeout)(int, int);
typedef void (*user_reset_all)(void);
typedef void (*user_disable_all)(void);

enum {
	CMD_BOOST_ENABLE = 1,
	CMD_BOOST_DISABLE,
	CMD_BOOST_ENABLE_TIMEOUT,
	CMD_USER_REG,
	CMD_USER_UNREG,
	CMD_USER_ENANLE,
	CMD_USER_DISABLE,
	CMD_USER_ENABLE_TIMEOUT,
	CMD_USER_RESET_ALL,
	CMD_USER_DISABLE_ALL,
};

static void usage(char *cmd);
static int load_api(void);


int main(int argc, char* argv[])
{
    int command, scenario, timeout, scn_core, scn_freq;
	int handle = -1;

    if(argc < 2) {
		usage(argv[0]);
        return 0;
    }

    command = atoi(argv[1]);
	switch(command) {
		case CMD_USER_RESET_ALL:
		case CMD_USER_DISABLE_ALL:
			if(argc!=2) {
				usage(argv[0]);
				return -1;
			}
			break;

		case CMD_BOOST_ENABLE:
		case CMD_BOOST_DISABLE:
		case CMD_USER_UNREG:
		case CMD_USER_ENANLE:
		case CMD_USER_DISABLE:
			if(argc!=3) {
				usage(argv[0]);
				return -1;
			}
			break;

		case CMD_BOOST_ENABLE_TIMEOUT:
		case CMD_USER_ENABLE_TIMEOUT:
		case CMD_USER_REG:
			if(argc!=4) {
				usage(argv[0]);
				return -1;
			}
			break;

		default:
			usage(argv[0]);
			return -1;
	}

	if(command == CMD_BOOST_ENABLE || command == CMD_BOOST_DISABLE || command == CMD_BOOST_ENABLE_TIMEOUT) {
		switch(atoi(argv[2])) {
			case 1:
				scenario = SCN_APP_SWITCH;
				break;

			case 2:
				scenario = SCN_APP_ROTATE;
				break;

			case 3:
				scenario = SCN_SW_CODEC;
				break;

	        case 4:
				scenario = SCN_SW_CODEC_BOOST;
				break;

			case 5:
				scenario = SCN_APP_TOUCH;
				break;

			default:
				scenario = SCN_NONE;
				break;
		}
	}
	else if(command == CMD_USER_REG) {
		scn_core = atoi(argv[2]);
		scn_freq = atoi(argv[3]);
	}
	else if(command == CMD_USER_UNREG || command == CMD_USER_ENANLE || command == CMD_USER_DISABLE) {
		handle = atoi(argv[2]);
	}
	else if(command == CMD_USER_ENABLE_TIMEOUT) {
		handle = atoi(argv[2]);
		timeout = atoi(argv[3]);
	}
	else if(command == CMD_BOOST_ENABLE_TIMEOUT) {
		timeout = atoi(argv[3]);
	}

	/* load api */
	if(load_api() != 0)
		return -1;

	/* command */
    if(command == CMD_BOOST_ENABLE)
        perfBoostEnable(scenario);
    else if(command == CMD_BOOST_DISABLE)
        perfBoostDisable(scenario);
	else if(command == CMD_BOOST_ENABLE_TIMEOUT)
		perfBoostEnableTimeout(scenario, timeout);
	else if(command == CMD_USER_REG) {
		handle = perfUserScnReg(scn_core, scn_freq);
		printf("handle:%d\n", handle);
	}
	else if(command == CMD_USER_UNREG) {
		perfUserScnUnreg(handle);
	}
	else if(command == CMD_USER_ENANLE) {
		perfUserScnEnable(handle);
	}
	else if(command == CMD_USER_DISABLE) {
		perfUserScnDisable(handle);
	}
	else if(command == CMD_USER_ENABLE_TIMEOUT) {
		perfUserScnEnableTimeout(handle, timeout);
	}
	else if(command == CMD_USER_RESET_ALL) {
		perfUserScnResetAll();
	}
	else if(command == CMD_USER_DISABLE_ALL) {
		perfUserScnDisableAll();
	}
}


static int load_api(void)
{
    void *handle, *func;

	handle = dlopen(LIB_FULL_NAME, RTLD_NOW);

	func = dlsym(handle, "PerfServiceNative_boostEnable");
	perfBoostEnable = reinterpret_cast<ena>(func);

	if (perfBoostEnable == NULL) {
		dlclose(handle);
		return -1;
	}

	func = dlsym(handle, "PerfServiceNative_boostDisable");
	perfBoostDisable = reinterpret_cast<disa>(func);

	if (perfBoostDisable == NULL) {
		dlclose(handle);
		return -1;
	}

	func = dlsym(handle, "PerfServiceNative_boostEnableTimeout");
	perfBoostEnableTimeout = reinterpret_cast<ena_timeout>(func);

	if (perfBoostEnableTimeout == NULL) {
		dlclose(handle);
		return -1;
	}

	func = dlsym(handle, "PerfServiceNative_userReg");
	perfUserScnReg = reinterpret_cast<user_reg>(func);

	if (perfUserScnReg == NULL) {
		dlclose(handle);
		return -1;
	}

	func = dlsym(handle, "PerfServiceNative_userUnreg");
	perfUserScnUnreg = reinterpret_cast<user_unreg>(func);

	if (perfUserScnUnreg == NULL) {
		dlclose(handle);
		return -1;
	}

	func = dlsym(handle, "PerfServiceNative_userEnable");
	perfUserScnEnable = reinterpret_cast<user_enable>(func);

	if (perfUserScnEnable == NULL) {
		dlclose(handle);
		return -1;
	}

	func = dlsym(handle, "PerfServiceNative_userDisable");
	perfUserScnDisable = reinterpret_cast<user_disable>(func);

	if (perfUserScnDisable == NULL) {
		dlclose(handle);
		return -1;
	}

	func = dlsym(handle, "PerfServiceNative_userEnableTimeout");
	perfUserScnEnableTimeout = reinterpret_cast<user_enable_timeout>(func);

	if (perfUserScnEnableTimeout == NULL) {
		dlclose(handle);
		return -1;
	}

	func = dlsym(handle, "PerfServiceNative_userResetAll");
	perfUserScnResetAll = reinterpret_cast<user_reset_all>(func);

	if (perfUserScnResetAll == NULL) {
		dlclose(handle);
		return -1;
	}

	func = dlsym(handle, "PerfServiceNative_userDisableAll");
	perfUserScnDisableAll = reinterpret_cast<user_disable_all>(func);

	if (perfUserScnDisableAll == NULL) {
		dlclose(handle);
		return -1;
	}

	return 0;
}


static void usage(char *cmd) {
    fprintf(stderr, "\nUsage: %s command scenario\n"
                    "    command\n"
                    "        1: boost enbale\n"
                    "        2: boost disable\n"
                    "        3: boost enbale timeout\n"
                    "        4: user reg\n"
                    "        5: user unreg\n"
                    "        6: user enable\n"
                    "        7: user disable\n"
                    "        8: user enable timeout\n"
                    "        9: user reset all\n"
                    "       10: user disable all\n"
                    "    scenario\n"
                    "        1: app switch\n"
                    "        2: app rotate\n"
                    "        3: sw codec\n"
                    "        4: sw codec boost\n"
                    "        5: app boost\n", cmd);
}

