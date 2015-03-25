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

typedef void (*ena)(int);
typedef void (*disa)(int);

static void usage(char *cmd);


int main(int argc, char* argv[])
{

    void *handle, *func;
    int command, scenario;

    if(argc!=3) {
		usage(argv[0]);
        return 0;
    }

    command = atoi(argv[1]);
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

		default:
			scenario = SCN_NONE;
			break;
	}

	handle = dlopen(LIB_FULL_NAME, RTLD_NOW);

	func = dlsym(handle, "PerfServiceNative_boostEnable");
	perfBoostEnable = (ena)(func);

	if (perfBoostEnable == NULL) {
		dlclose(handle);
		return 0;
	}

	func = dlsym(handle, "PerfServiceNative_boostDisable");
	perfBoostDisable = (disa)(func);

	if (perfBoostDisable == NULL) {
		dlclose(handle);
		return 0;
	}

    if(command == 1)
        perfBoostEnable(scenario);
    else if(command == 2)
        perfBoostDisable(scenario);

    return 0;
}


static void usage(char *cmd) {
    fprintf(stderr, "\nUsage: %s command scenario\n"
                    "    command\n"
                    "        1: boost enbale\n"
                    "        2: boost disable\n"
                    "    scenario\n"
                    "        1: app switch\n"
                    "        2: app rotate\n"
                    "        3: sw codec\n", cmd);
}

