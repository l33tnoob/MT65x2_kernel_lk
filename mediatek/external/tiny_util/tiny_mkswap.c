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

#define CURRENT_PROGNAME	"tiny_mkswap"
#define UUID_LENGTH 		16
#define LABEL_LENGTH 		16

struct swap_header_v1_2 {
	char	      bootbits[1024];    /* Space for disklabel etc. */
	unsigned int  version;
	unsigned int  last_page;
	unsigned int  nr_badpages;
	unsigned char uuid[UUID_LENGTH];
	char	      volume_name[LABEL_LENGTH];
	unsigned int  padding[117];
	unsigned int  badpages[1];
};

static void __attribute__ ((__noreturn__)) usage(FILE *out)
{
	fprintf(out,"\nUsage:\n"
		    " %s [options] device\n",CURRENT_PROGNAME);

	fprintf(out,"\nOptions:\n"
		    " -h, display this help and exit\n\n");

	exit(out == stderr ? EXIT_FAILURE : EXIT_SUCCESS);
}

static unsigned long get_dev_size(const char *file)
{
	int fd;
	unsigned long size = 0;

	fd = open(file, O_RDONLY);
	if (fd < 0) {
		perror(file);
		exit(EXIT_FAILURE);
	}

	if (ioctl(fd, BLKGETSIZE, &size) >= 0) {
		/* In sector(s) of 512 bytes */
		size <<= 9;
	}

	close(fd);
	return size;
}

static inline int write_it_all(int fd, const void *buf, size_t count)
{
	const char *inbuf = buf;

	while (count) {
		ssize_t tmp;

		errno = 0;
		tmp = write(fd, inbuf, count);
		if (tmp > 0) {
			count -= tmp;
			if (count)
				inbuf += tmp;
		} else if (errno != EINTR && errno != EAGAIN)
			return -1;
		if (errno == EAGAIN)	/* Try later, *sigh* */
			usleep(10000);
	}
	return 0;
}

int main(int argc, char **argv)
{
	int c;
	char *device_name = NULL;
	struct stat statbuf;
	int dev_fd = -1;
	long sz = 0;
	int pgsize = getpagesize();
	struct swap_header_v1_2 *hdr;
	unsigned long *swapheader_page = NULL;
	char *sp;
	off_t offset;
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
	
	/* Acquire some information */
	if (stat(device_name, &statbuf) < 0) {
		perror(device_name);
		exit(EXIT_FAILURE);
	}
	if (S_ISBLK(statbuf.st_mode))
		dev_fd = open(device_name, O_RDWR | O_EXCL);
	else
		dev_fd = open(device_name, O_RDWR);

	if (dev_fd < 0) {
		perror(device_name);
		exit(EXIT_FAILURE);
	}

	/* Check device size */
	sz = get_dev_size(device_name);
	sz = sz/pgsize;
	if (!sz) {
		printf("Error : device size is zero!!\n");
		exit(EXIT_FAILURE);
	}

	/* No boundary checks! */

	/* Initialize swapheader_page */
	swapheader_page = (unsigned long *) calloc(1, pgsize);
	if (!swapheader_page) {
		printf("Error : cannot allocate memory for swapheader_page!!\n");
		exit(EXIT_FAILURE);
	}
	hdr = (struct swap_header_v1_2 *) swapheader_page;
	hdr->version = 1;
	hdr->last_page = sz - 1;
	hdr->nr_badpages = 0;					// Assume it has no badpages.
	sp = (char *) swapheader_page;
	strncpy(sp + pgsize - 10, "SWAPSPACE2", 10);
	
	/* Write swap header */
	offset = 1024;
	if (lseek(dev_fd, offset, SEEK_SET) != offset) {
		printf("Error : unable to rewind swap-device!!\n");
		exit(EXIT_FAILURE);
	}
	if (write_it_all(dev_fd, (char *) swapheader_page + offset, pgsize - offset) == -1) {
		printf("Error : %s: unable to write swapheader page!!\n", device_name);
		exit(EXIT_FAILURE); 
	}
	
	/* Sync it to device */
	if (fsync(dev_fd)) {
		printf("Error : fsync error!!\n");
		exit(EXIT_FAILURE);
	}
	
	return EXIT_SUCCESS;
}
