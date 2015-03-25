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

#define CURRENT_PROGNAME	"tiny_swapoff"

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
	
	/* Swap off it */
	status = swapoff(device_name);
	if (status < 0) {
		printf("%s: swapoff failed\n",device_name);
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}
