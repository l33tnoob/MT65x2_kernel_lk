#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <cutils/properties.h>
#include "tiny_util.h"

#define CURRENT_PROGNAME	"tiny_swapon"

static void __attribute__ ((__noreturn__)) usage(FILE *out)
{
	fprintf(out,"\nUsage:\n"
		    " %s [options] device(in canonical form)\n",CURRENT_PROGNAME);

	fprintf(out,"\nOptions:\n"
		    " -h, display this help and exit\n\n");

	exit(out == stderr ? EXIT_FAILURE : EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
	int c;
	char *device_name = NULL;
	int status = 0;
        char doit[2] = {0};

#if 0
	/* Get property to check whether we should execute this command */
        property_get(TINY_ENABLE_SWAP, doit, "1");
	if (!strcmp(doit, "0")) {
		printf("\n\n\n\n\n[%s][%d]\n\n\n\n\n",__FUNCTION__,__LINE__);
		return 0;
	}
#endif

	/* Get options */
	while((c = getopt(argc, argv, "h")) != -1) {
		switch(c) {
		case 'h':
		default :
			usage(stdout);
		}
	}

	/* Get device name */
	if (optind < argc) {
		device_name = argv[optind++];
	}
	
	/* Last check of arguments */
	if (optind != argc) {
		usage(stderr);
	}
	
	/* Swap on it */
	status = swapon(device_name, 0);
	if (status < 0) {
		printf("%s: swapon failed\n",device_name);
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}
