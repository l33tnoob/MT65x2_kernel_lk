#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cutils/properties.h>
#include "tiny_util.h"

#define CURRENT_PROGNAME	"tiny_switch"

#define IS_SWAP()				\
	{					\
		if (strcmp("s", optarg)) {	\
			usage(stdout);		\
		}				\
	}

static void __attribute__ ((__noreturn__)) usage(FILE *out)
{
	fprintf(out,"\nUsage:\n"
		    " %s [Options]\n",CURRENT_PROGNAME);

	fprintf(out,"\nOptions:\n"
		    " -h, display this help and exit\n\n"
		    " -es, enable  swap capability (It takes effect by rebooting system after triggering this command.)\n\n"
		    " -ds, disable swap capability (It takes effect by rebooting system after triggering this command.)\n\n");

	exit(out == stderr ? EXIT_FAILURE : EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
	int c;

	/* Get options */
	while((c = getopt(argc, argv, "he:d:")) != -1) {
		switch(c) {
		case 'e':
			IS_SWAP();
        		property_set(TINY_ENABLE_SWAP, "1");
			break;
		case 'd':
			IS_SWAP();
        		property_set(TINY_ENABLE_SWAP, "0");
			break;
		case 'h':
		default :
			usage(stdout);
		}
	}
	
	return EXIT_SUCCESS;
}
