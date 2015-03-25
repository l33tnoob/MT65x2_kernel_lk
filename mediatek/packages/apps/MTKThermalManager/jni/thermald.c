#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <cutils/xlog.h>
#include <sys/types.h>

static int debug_on = 0;

#define TM_LOG_TAG "thermald"
#define TM_DBG_LOG(_fmt_, args...) \
    do { \
        if (1 == debug_on) { \
            sxlog_printf(ANDROID_LOG_INFO, TM_LOG_TAG, _fmt_, ##args); \
        } \
    } while(0)

#define TM_INFO_LOG(_fmt_, args...) \
    do { sxlog_printf(ANDROID_LOG_INFO, TM_LOG_TAG, _fmt_, ##args); } while(0)

#define PROCFS_MTK_CL_SD_PID "/proc/driver/mtk_cl_sd_pid"

static void signal_handler(int signo, siginfo_t *si, void *uc)
{
	switch(si->si_signo) {
	    // Add more signo or code to expand thermald
		case SIGIO:
			if(1 == si->si_code) {
				system("am start com.mediatek.thermalmanager/.ShutDownAlertDialogActivity");
				TM_INFO_LOG("thermal shutdown signal received, si_signo=%d, si_code=%d\n", si->si_signo, si->si_code);
			}
		break;
		default:
			TM_INFO_LOG("what!!!\n");
		break;
	}
}

int main(int argc, char *argv[])
{
	int fd = open(PROCFS_MTK_CL_SD_PID, O_RDWR);
	int pid = getpid();
	int ret = 0;
	char pid_string[32] = {0};

	struct sigaction act;

	TM_INFO_LOG("START+++++++++ %d", getpid());

	/* Create signal handler */
	memset(&act, 0, sizeof(act));
	act.sa_flags = SA_SIGINFO;
	//act.sa_handler = signal_handler;
	act.sa_sigaction = signal_handler;
	sigemptyset(&act.sa_mask);

	sigaction(SIGIO, &act, NULL);

	/* Write pid to procfs */
	sprintf(pid_string, "%d", pid);

   	ret = write(fd, pid_string, sizeof(char) * strlen(pid_string));
	if (ret <= 0)	{
		TM_INFO_LOG("Fail to write %d to %s %x\n", pid, PROCFS_MTK_CL_SD_PID, ret);
	} else {
		TM_INFO_LOG("Success to write %d to %s\n", pid, PROCFS_MTK_CL_SD_PID);
	}
	close(fd);

	TM_INFO_LOG("Enter infinite loop");

	while(1) {
		sleep(100);
	}

	TM_INFO_LOG("END-----------");

	return 0;
}
