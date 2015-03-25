#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <cutils/xlog.h>
#include <netutils/ifc.h>
#if defined(MTK_THERMAL_PA_VIA_ATCMD)
#include <cutils/sockets.h>
#include <assert.h>
#endif

#if defined(MTK_THERMAL_PA_VIA_ATCMD)
#define uint8 unsigned char
#define uint32 unsigned int
#endif

static int debug_on = 0;

#define TM_LOG_TAG "thermal_repeater"
#define TM_DBG_LOG(_fmt_, args...) \
    do { \
        if (1 == debug_on) { \
            sxlog_printf(ANDROID_LOG_INFO, TM_LOG_TAG, _fmt_, ##args); \
        } \
    } while(0)

#define TM_INFO_LOG(_fmt_, args...) \
    do { sxlog_printf(ANDROID_LOG_INFO, TM_LOG_TAG, _fmt_, ##args); } while(0)

#define ONE_MBITS_PER_SEC 1000
#define PROCFS_TM_PID "/proc/wmt_tm/tm_pid"
#define COOLER_NUM 3

#define WLAN_IFC_PATH "/sys/class/net/wlan0/operstate"
#define AP_IFC_PATH "/sys/class/net/ap0/operstate"
#define P2P_IFC_PATH "/sys/class/net/p2p0/operstate"

enum {
	WLAN_IFC = 0,
	AP_IFC = 1,
	P2P_IFC = 2,
	IFC_NUM /*Last one*/
};

static char IFC_NAME[IFC_NUM][10] = {"wlan0","ap0","p2p0"};
static char IFC_PATH[IFC_NUM][50] = {WLAN_IFC_PATH, AP_IFC_PATH, P2P_IFC_PATH};

#ifdef NEVER
static char THROTTLE_SCRIPT_PATH[] = "/system/etc/throttle.sh";

static void exe_cmd(int wifi_ifc, int level)
{
	if (0 == access(THROTTLE_SCRIPT_PATH, R_OK | X_OK) && wifi_ifc >= 0) {
		char cmd[256] = {0};

		sprintf(cmd, "%s %s %d %d", THROTTLE_SCRIPT_PATH, IFC_NAME[wifi_ifc], level * ONE_MBITS_PER_SEC, level * ONE_MBITS_PER_SEC);

		TM_INFO_LOG("cmd=%s", cmd);

		/*Need to execute twice to effect the command*/
		int ret = system(cmd);
		if ((-1 == ret) || (0 != WEXITSTATUS(ret))) {
			TM_INFO_LOG("1. executing %s failed: %s", THROTTLE_SCRIPT_PATH, strerror(errno));
		}

		ret = system(cmd);
		if ((-1 == ret) || (0 != WEXITSTATUS(ret))) {
			TM_INFO_LOG("2. executing %s failed: %s", THROTTLE_SCRIPT_PATH, strerror(errno));
		}
	} else {
		TM_INFO_LOG("failed to access %s", THROTTLE_SCRIPT_PATH);
	}
}
#endif /* NEVER */

static void set_wifi_throttle(int level)
{
	int i = 0;
	for ( i=0; i<IFC_NUM; i++) {
		TM_DBG_LOG("checking %s", IFC_PATH[i]);
		if (0 == access(IFC_PATH[i], R_OK)) {
			char buf[80];
			int fd = open(IFC_PATH[i], O_RDONLY);
			if (fd < 0) {
				TM_INFO_LOG("Can't open %s: %s", IFC_PATH[i], strerror(errno));
				continue;
			}

			int len = read(fd, buf, sizeof(buf) - 1);
			if (len < 0) {
				TM_INFO_LOG("Can't read %s: %s", IFC_PATH[i], strerror(errno));
				continue;
			}
			close(fd);
			if(!strncmp (buf, "up", 2)) {
				ifc_set_throttle(IFC_NAME[i], level * ONE_MBITS_PER_SEC, level * ONE_MBITS_PER_SEC);

				#ifdef NEVER
			 	exe_cmd(i, level);
				#endif /* NEVER */
			} else
				TM_DBG_LOG("%s is down!", IFC_NAME[i]);
		}
	}
}

static void signal_handler(int signo, siginfo_t *si, void *uc)
{
	static int cur_thro = 0;
	int set_thro = si->si_code;

	switch(si->si_signo) {
		case SIGIO:
			TM_DBG_LOG("cur=%d, set=%d\n", cur_thro, set_thro);
			if(cur_thro != set_thro) {
				set_thro = set_thro?:1; /*If set_thro is 0, set 1Mb/s*/
				set_wifi_throttle(set_thro);
				cur_thro = set_thro;
			}
		break;
		default:
			TM_INFO_LOG("what!!!\n");
		break;
	}
}

#if defined(MTK_THERMAL_PA_VIA_ATCMD)
int queryMdThermalInfo(int sock, int slotId)
{
	int ret = -1, strLen = 0, count = 1;
	char *strParm = NULL, *pTmp = NULL;
		
	assert(sock > 0);
	

	strLen = strlen("THERMAL") + 3;
	strParm	= (char *)malloc(strLen); 
	strParm[strLen - 1] = '\0';
	strcpy(strParm, "THERMAL");
	strcat(strParm, ",");
	sprintf(strParm + strlen(strParm), "%d", slotId);
	TM_INFO_LOG("%d %s will sent to rild\n", strLen, strParm);

	ret = send(sock, (int)&count, sizeof(int), 0);
	if(sizeof(int) == ret)
		ret = send(sock, &strLen, sizeof(strLen), 0);
	else
	{	
		ret = -4;
		goto failed;
	}

	if (sizeof(strLen) == ret)
	{
		ret = send(sock, strParm, strLen, 0);
		if (strLen == ret)
		{
			ret = 0;
			TM_INFO_LOG("%s ok\n", __FUNCTION__);
		}
		else
		{
			ret = -5;
			goto failed;	
		}
	}
	else
	{
		ret = -3;
	}

failed:	
	free(strParm);
	TM_INFO_LOG("oh, %s (%d)%s\n", __FUNCTION__, ret, strerror(errno));

	return ret;
}

int recvMdThermalInfo(int sock, int slotId)
{
	int ret = -1, strLen = 31;
	uint8 strParm[32] = {0};

	assert(sock > 0);

	//ret = recv(sock, (uint8 *)&strLen, sizeof(strLen), 0);
    //TM_INFO_LOG("[%s] ret=%d, strLen=%d\n", __FUNCTION__, ret, strLen);
    TM_INFO_LOG("[%s]\n", __FUNCTION__);
	
	//if (sizeof(strLen) == ret)
	{
		//strParm = (uint8 *)malloc(strLen + 1);
		memset(strParm, '\0', strLen+1);
		strParm[31] = '\0';
		ret = recv(sock, strParm, strLen, 0);
		TM_INFO_LOG("[%s] ret=%d, strLen=%d, %s\n", __FUNCTION__, ret, strLen, strParm);
		
		if (0 < ret)
		{
		    if (strncmp("ERROR", strParm, 5) != 0)
		    {
		        int md, temp, tx;
		        int tok;
		        tok = sscanf(strParm, "%d,%d,%d", &md, &temp, &tx);

                TM_INFO_LOG("[%s] tok=%d md=%d temp=%d tx=%d\n", strParm, tok, md, temp, tx);

                // write to proc
#if 1
                {
                    int fd = open("/proc/mtk_mdm_txpwr/mdinfo", O_RDWR);
                    char mdinfo_string[64] = {0};
                    sprintf(mdinfo_string, "%d,%d,%d,%d", slotId, md, temp, ((tok>2)?tx:-127));
                    ret = write(fd, mdinfo_string, sizeof(char) * strlen(mdinfo_string));
            		if (ret <= 0)	{
            			TM_INFO_LOG("Fail to write %s to %s %x\n", mdinfo_string, "/proc/mtk_mdm_txpwr/mdinfo", ret);
            		} else {
            			TM_INFO_LOG("Success to write %s to %s\n", mdinfo_string, "/proc/mtk_mdm_txpwr/mdinfo");
            		}
            		close(fd);
                }
#endif
            }
            else
            {
                TM_INFO_LOG("[%s] ERROR\n", strParm);
                ret = -1;
            }

		}
		//free(strParm);		
	}

	return ret;
}

static int connectToRild(int *solt_id)
{
	int sock = -1;
    sock = socket_local_client("rild-oem", ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_STREAM);
    if(sock < 0)
        TM_INFO_LOG("connectToRild %s\n", strerror(errno));
#if 0
	char telephony_mode[] = "0", first_md[] = "0";

	property_get("ril.telephony.mode", telephony_mode, NULL);
	property_get("ril.first.md", first_md, NULL);
	wpa_printf(MSG_DEBUG, "RIL: slot=%d, ril.telephony.mode=%s, ril.first.md=%s",*solt_id , telephony_mode, first_md);
	if(telephony_mode[0]=='1' || telephony_mode[0]=='3')
	{
		sock = socket_local_client("rild-debug", 
			ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_STREAM);
		wpa_printf(MSG_DEBUG, "RIL: try to connect to rild-debug");
	}
	else if(telephony_mode[0]=='2' || telephony_mode[0]=='4')
	{
		sock = socket_local_client("rild-debug-md2", 
			ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_STREAM);
		if(sock < 0) 
		{
			sock = socket_local_client("rild-debug", //6572,6582 is single modem project, only has MD1
				ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_STREAM);
			wpa_printf(MSG_DEBUG, "RIL: try to connect to rild-debug");
		}
		else
			wpa_printf(MSG_DEBUG, "RIL: try to connect to rild-debug-md2");
	}
	else if(telephony_mode[0]>='5' && telephony_mode[0]<='8')
	{
		if(first_md[0]-'1' == *solt_id) //ril.first.md==1 indicate MD1 connect to SIM1, ==2 indicate MD1 connect to SIM2
		{
			sock = socket_local_client("rild-debug",
				ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_STREAM);
			wpa_printf(MSG_DEBUG, "RIL: try to connect to rild-debug");
		}
		else
		{
			sock = socket_local_client("rild-debug-md2",
				ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_STREAM);
			wpa_printf(MSG_DEBUG, "RIL: try to connect to rild-debug-md2");
		}
		*solt_id = 0;
		wpa_printf(MSG_DEBUG, "RIL: Reset slot to slot0");
	}
	else
	{
		wpa_printf(MSG_DEBUG, "RIL: unsupport ril.telephony.mode, try to connect to default socket rild-debug");
		sock = socket_local_client("rild-debug", 
			ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_STREAM);
	}
	if(sock < 0)
		wpa_printf(MSG_ERROR, "connectToRild %s", strerror(errno));
#endif
	return sock;
}

int disconnectRild(int sock)
{
	int ret;
	
	assert(sock > 0);
	ret = close(sock);
	
	return ret;
}
#endif

int main(int argc, char *argv[])
{
	if(argc == 3) {
		char ifc[16] = {0};
		char tmp[16] = {0};
		int thro = 0;
		int i = 0;

		strncpy(ifc, argv[1], sizeof(char)*16);
		strncpy(tmp, argv[2], sizeof(char)*16);
		thro = atoi(tmp);

		TM_INFO_LOG("CMD MODE %s %d", ifc, thro);

		for ( i=0; i<IFC_NUM; i++) {
			if(!strncmp (IFC_NAME[i], ifc, 2)) {
				ifc_set_throttle(IFC_NAME[i], thro * ONE_MBITS_PER_SEC, thro * ONE_MBITS_PER_SEC);
				#ifdef NEVER
			 	exe_cmd(i, thro);
				#endif
			} else
				TM_DBG_LOG("NOT %s!", IFC_NAME[i]);
		}
	} else {
		int fd = open(PROCFS_TM_PID, O_RDWR);
		int pid = getpid();
		int ret = 0;
		char pid_string[32] = {0};
		struct sigaction act;

#if defined(MTK_THERMAL_PA_VIA_ATCMD)
		int count = 0;
		int socket;
#endif

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
			TM_INFO_LOG("Fail to write %d to %s %x\n", pid, PROCFS_TM_PID, ret);
		} else {
			TM_INFO_LOG("Success to write %d to %s\n", pid, PROCFS_TM_PID);
		}
		close(fd);

#ifdef NEVER
		/* Check throttl.sh */
		if (0 == access(THROTTLE_SCRIPT_PATH, R_OK | X_OK)) {
			ret = chmod(THROTTLE_SCRIPT_PATH, S_ISUID | S_ISVTX | S_IRUSR | S_IXUSR);
			if (ret == 0)	{
				TM_INFO_LOG("Success to chomd\n");
			} else {
				TM_INFO_LOG("Fail to chmod %x\n", ret);
			}
		} else {
			TM_INFO_LOG("failed to access %s", THROTTLE_SCRIPT_PATH);
		}
#endif /* NEVER */

#if defined(MTK_THERMAL_PA_VIA_ATCMD)
        sleep(60);

		TM_INFO_LOG("Enter infinite loop\n");

		while(1) 
		{
            count++;

            TM_INFO_LOG("count %d\n", count);

            // connect to rild...
            do {
                sleep(1);
                socket = connectToRild(NULL);
                TM_INFO_LOG("socket %d\n", socket);
            } while (socket == -1);

			{
                queryMdThermalInfo(socket, 0);

                recvMdThermalInfo(socket, 0);
			}

			disconnectRild(socket);
		}
		

		TM_INFO_LOG("END-----------\n");
#else
 		TM_INFO_LOG("Enter infinite loop");

		while(1) {
			sleep(100);
		}

		TM_INFO_LOG("END-----------");
#endif
	}

	return 0;
}
