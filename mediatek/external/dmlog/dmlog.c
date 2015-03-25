#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#define BUFFER_SIZRE    256

static void usage(char *myname) {
    fprintf(stderr, "Usage: %s [ -c | -f | -h ]\n"
                    "    -c  Clear buffer.\n"
                    "    -f  Show format.\n"
                    "    -h  Display this help screen.\n",
    myname);
}


int main(int argc, char *argv[]) {
    char buffer[BUFFER_SIZRE + 1];
    int ret;
    int arg;
    int f_mlog;
    int format_only = 0;
    int clear_only = 0;

    /* show usage */
    for (arg = 1; arg < argc; arg++) {
        if (!strcmp(argv[arg], "-f")) { format_only = 1; continue; }
        if (!strcmp(argv[arg], "-c")) { clear_only = 1; continue; }
        if (!strcmp(argv[arg], "-h")) { usage(argv[0]); exit(0); }
        fprintf(stderr, "Invalid argument \"%s\".\n", argv[arg]);
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    if (!clear_only)
    {
        /* print header */
        f_mlog = open("/proc/mlog_fmt", O_RDONLY);
        while(ret = read(f_mlog, buffer, BUFFER_SIZRE)) {
            if (ret <= 0)
                break;
            buffer[ret] = 0;
            fprintf(stdout, "%s", buffer);
        }
        close(f_mlog);
    }

    if (format_only)
        return 0;

    /* dump mlog from ring buffer */
    f_mlog = open("/proc/mlog", O_RDONLY | O_NONBLOCK | O_EXCL);

    while(ret = read(f_mlog, buffer, BUFFER_SIZRE)) {
        if (ret <= 0)
            break;
        if (!clear_only) {
            buffer[ret] = 0;
            fprintf(stdout, "%s", buffer);
        }
    }

    if (!clear_only)
        fprintf(stdout, "\n");

    close(f_mlog);
    return 0;


}

