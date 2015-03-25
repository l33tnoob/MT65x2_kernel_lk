
#include "loader.h"

#define WCN_COMBO_LOADER_CHIP_ID_PROP    	"persist.mtk.wcn.combo.chipid"
#define WCN_COMBO_LOADER_DEV				"/dev/wmtdetect"
#define WCN_COMBO_DEF_CHIPID				"0x6582"
#define WMT_MODULES_PRE						"/system/lib/modules/"
#define WMT_MODULES_SUFF					".ko"
#define WMT_IOC_MAGIC        				'w'
#define COMBO_IOCTL_GET_CHIP_ID  			_IOR(WMT_IOC_MAGIC, 0, int)
#define COMBO_IOCTL_SET_CHIP_ID  			_IOW(WMT_IOC_MAGIC, 1, int)
#define COMBO_IOCTL_EXT_CHIP_DETECT 		_IOR(WMT_IOC_MAGIC, 2, int)
#define COMBO_IOCTL_GET_SOC_CHIP_ID  		_IOR(WMT_IOC_MAGIC, 3, int)
#define COMBO_IOCTL_DO_MODULE_INIT  	    _IOR(WMT_IOC_MAGIC, 4, int)
#define COMBO_IOCTL_MODULE_CLEANUP  	    _IOR(WMT_IOC_MAGIC, 5, int)




#define STP_WMT_MODULE_PRE_FIX "mtk_stp_wmt"
#define STP_BT_MODULE_PRE_FIX "mtk_stp_bt"
#define STP_GPS_MODULE_PRE_FIX "mtk_stp_gps"
#define HIF_SDIO_MODULE_PRE_FIX "mtk_hif_sdio"
#define STP_SDIO_MODULE_PRE_FIX "mtk_stp_sdio"
#define STP_UART_MODULE_PRE_FIX "mtk_stp_uart"



static int gLoaderFd = -1;

static char DRIVER_MODULE_PATH[64]  = {0};
static char DRIVER_MODULE_ARG[8] = "";


static char chip_version[PROPERTY_VALUE_MAX] = {0};

static int g_remove_ko_flag = 1;


extern int init_module(void *, unsigned long, const char *);
extern int delete_module(const char *, unsigned int);
extern int load_fm_module(int chip_id);
extern int load_wifi_module(int chip_id);
#if 0
extern int load_ant_module(int chip_id);
#endif
//insmod
static int insmod(const char *filename, const char *args)
{
    void *module;
    unsigned int size;
    int ret = -1;
	int retry = 10;

	printf("filename(%s)\n",filename);
	
    module = load_file(filename, &size);
    if (!module)
    {
    	printf("load file fail\n");
        return -1;
    }
	
	while(retry-- > 0){
	    ret = init_module(module, size, args);

		if(ret < 0)
		{
			printf("insmod module fail(%d)\n",ret);
			usleep(30000);
		}
		else
			break;
	
	}
	
    free(module);

    return ret;
}

static int rmmod(const char *modname)
{
    int ret = -1;
    int maxtry = 10;

    while (maxtry-- > 0) {
        ret = delete_module(modname, O_EXCL);//O_NONBLOCK | O_EXCL);
        if (ret < 0 && errno == EAGAIN)
            usleep(500000);
        else
            break;
    }

    if (ret != 0)
        printf("Unable to unload driver module \"%s\": %s,ret(%d)\n",
             modname, strerror(errno),ret);
    return ret;
}

static int insmod_by_path (char *nameBuf, char * modulePath, char *preFix, char *postFix )
{
	int iRet = -1;
	int len = 0;
	int path_len = 0;
	
	/*no need to check, upper layer API will makesure this condition fullfill*/
	strcat (nameBuf, modulePath);
	strcat (nameBuf, preFix);
	strcat (nameBuf, postFix);
	strcat (nameBuf, WMT_MODULES_SUFF);

	insmod_retry:
	iRet = insmod(nameBuf, DRIVER_MODULE_ARG);
	if(iRet)
	{
		printf("insert <%s> failed, len(%d), iret(%d), retrying\n", nameBuf, sizeof(nameBuf), iRet);
		/*break;*/
		usleep(800000);
		goto insmod_retry;
	}else
	{
		printf("insert <%s> succeed,len(%d)\n", nameBuf, len);
		iRet = 0;
	}
	return 0;
}


static int insert_wmt_module_for_soc(int chipid, char *modulePath, char *nameBuf, int nameBufLen)
{
	int iRet = -1;
	int len = 0;
	int path_len = 0;
	int i = 0;
	char postFixStr[10] = {0};
	int totalLen = 0;
	char *soc_modulse[] = {
		STP_WMT_MODULE_PRE_FIX,
		STP_BT_MODULE_PRE_FIX,
		STP_GPS_MODULE_PRE_FIX,
	};
	
#if 0
	path_len = strlen(modulePath);
	strncpy(nameBuf, modulePath,path_len);
	printf("module subpath1(%s),sublen1(%d)\n",nameBuf,path_len);
	len = path_len;
#endif

	sprintf(postFixStr, "_%s", "soc");

#if 0	
	switch (chipid)
	{
		case 0x6572:
		case 0x6582:
			strcpy(postFixStr, "_6582");
			break;
		case 0x6571:
			strcpy(postFixStr, "_6592");
		case 0x6592:
			strcpy(postFixStr, "_6592");
			break;
		default:
			
			break;
	}
#endif

	if (NULL == modulePath || NULL == nameBuf || 0 >= nameBufLen)
	{
		printf("invalid parameter:modulePath(0x%x), nameBuf(0x%x), nameBufLen(%d)\n", modulePath, nameBuf, nameBufLen);
		return iRet;
	}
	
	for(i = 0;i < sizeof(soc_modulse)/sizeof(soc_modulse[0]);i++)
	{
		totalLen = sizeof (modulePath) + sizeof (soc_modulse[i]) + sizeof(postFixStr) + sizeof(WMT_MODULES_SUFF);
		if (nameBufLen > totalLen)
		{
			
			memset (nameBuf, 0, nameBufLen);
			insmod_by_path(nameBuf, modulePath, soc_modulse[i], postFixStr);
		}
		else
		{
			printf("nameBuf length(%d) too short, (%d) needed\n", nameBufLen, totalLen);
		}
#if 0	
		len = path_len;
		len += sprintf(nameBuf + len,"%s",soc_modulse[i]);
		printf("module subpath2(%s),sublen2(%d)\n",nameBuf,len);

		len += sprintf(nameBuf + len,"%s", postFixStr);
		printf("module subpath3(%s),sublen3(%d)\n",nameBuf,len);

		len += sprintf(nameBuf + len,"%s",WMT_MODULES_SUFF);
		printf("module subpath4(%s),sublen4(%d)\n",nameBuf,len);

		nameBuf[len] = '\0';
		printf("module subpath5(%s),sublen5(%d)\n",nameBuf,len);

		soc_retry:
		iRet = insmod(nameBuf, DRIVER_MODULE_ARG);
		if(iRet)
		{
			printf("(%d):current modules(%s) insert fail,len(%d),iret(%d), retrying\n", i, nameBuf, len, iRet);
			/*break;*/
			usleep(300000);
			goto soc_retry;
		}else
		{
			printf("(%d):current modules(%s) insert ok,len(%d)\n", i, nameBuf, len);
		}
#endif

	}

	
	
	return 0;
}

static int insert_wmt_module_for_combo(int chipid, char *modulePath, char *nameBuf, int nameBufLen)
{
	int iRet = -1;
	int len = 0;
	int path_len = 0;
	int i = 0;
	char postFixStr[10] = {0};
	int totalLen = 0;
	
	char *combo_modulse[] = {
		HIF_SDIO_MODULE_PRE_FIX,
		STP_WMT_MODULE_PRE_FIX,
		STP_UART_MODULE_PRE_FIX,
		STP_SDIO_MODULE_PRE_FIX,
		STP_BT_MODULE_PRE_FIX,
		STP_GPS_MODULE_PRE_FIX
	};

	if (NULL == modulePath || NULL == nameBuf || 0 >= nameBufLen)
	{
		printf("invalid parameter:modulePath(0x%x), nameBuf(0x%x), nameBufLen(%d)\n", modulePath, nameBuf, nameBufLen);
		return iRet;
	}
	
#if 0
	path_len = strlen(modulePath);
	strncpy(nameBuf, modulePath,path_len);
	printf("module subpath1(%s),sublen1(%d)\n",nameBuf,path_len);

	len = path_len;
#endif

	switch (chipid)
	{
		case 0x6620:
		case 0x6628:
			/*strcpy(postFixStr, "_6620_28");*/
			strcpy(postFixStr, "");
			break;
		case 0x6630:
			//strcpy(postFixStr, "_6630");
			strcpy(postFixStr, "");
			break;
		default:
	
			break;
	}
	
	for(i = 0;i < sizeof(combo_modulse)/sizeof(combo_modulse[0]);i++)
	{
		totalLen = sizeof (modulePath) + sizeof (combo_modulse[i]) + sizeof(postFixStr) + sizeof(WMT_MODULES_SUFF);
		if (nameBufLen > totalLen)
		{
			memset (nameBuf, 0, nameBufLen);
			insmod_by_path(nameBuf, modulePath, combo_modulse[i], postFixStr);
		}
		else
		{
			printf("nameBuf length(%d) too short, (%d) needed\n", nameBufLen, totalLen);
		}
#if 0
		len = path_len;
		len += sprintf(nameBuf + len,"%s",combo_modulse[i]);
		printf("module subpath2(%s),sublen2(%d)\n",nameBuf,len);

		len += sprintf(nameBuf + len,"%s",postFixStr);
		printf("module subpath3(%s),sublen3(%d)\n",nameBuf,len);

		len += sprintf(nameBuf + len,"%s",WMT_MODULES_SUFF);
		printf("module subpath4(%s),sublen4(%d)\n",nameBuf,len);

		nameBuf[len] = '\0';
		printf("module subpath5(%s),sublen5(%d)\n",nameBuf,len);
		
		combo_retry:
		iRet = insmod(nameBuf, DRIVER_MODULE_ARG);
		if(iRet)
		{
			printf("(%d):current modules(%s) insert fail,len(%d),iret(%d), retrying\n", i, nameBuf, len, iRet);
			/*break;*/
			usleep(300000);
			goto combo_retry;
		}else
		{
			printf("(%d):current modules(%s) insert ok,len(%d)\n",i, nameBuf, len);
		}
#endif		
	}

	
	return 0;
}



/******************************************************
arg1: 
= 0:there is already a valid chipid in peroperty or there is no external combo chip
	chipid is just 	MT6582
> 0:there is no valid chipid in peroperty, boot system firstly

arg2: // handle combo chip (there is an external combo chip)
= 0:insert mtk_hif_sdio.ko for detech combo chipid
> 0:insert combo modules except mtk_hif_sdio.ko
******************************************************/

static int insert_wmt_modules(int chipid,int arg1,int arg2)
{
	int iRet = -1;

	switch (chipid)
	{
		case 0x6582:
		case 0x6572:
		case 0x6571:
		case 0x6592:
			iRet = insert_wmt_module_for_soc(chipid, WMT_MODULES_PRE, DRIVER_MODULE_PATH, sizeof (DRIVER_MODULE_PATH));
			break;
		case 0x6620:
		case 0x6628:
		case 0x6630:
			iRet = insert_wmt_module_for_combo(chipid, WMT_MODULES_PRE, DRIVER_MODULE_PATH, sizeof (DRIVER_MODULE_PATH));
			break;
		default:
			break;
	}
	
	return iRet;
}

int do_kernel_module_init(int gLoaderFd, int chipId)
{
	int iRet = 0;
	if (gLoaderFd < 0)
	{
		printf("invalid gLoaderFd: %d\n", gLoaderFd);
		return -1;
	}

	iRet = ioctl (gLoaderFd, COMBO_IOCTL_MODULE_CLEANUP, chipId);
	if (iRet)
	{
		printf("do WMT-DETECT module cleanup failed: %d\n", iRet);
		return -2;
	}
	
	iRet = ioctl (gLoaderFd, COMBO_IOCTL_DO_MODULE_INIT, chipId);
	if (iRet)
	{
		printf("do kernel module init failed: %d\n", iRet);
		return -3;
	}
	printf("do kernel module init succeed: %d\n", iRet);
	return 0;
}


int main(int argc, char *argv[])
{
	int iRet = -1;
	int noextChip = -1;
	int chipId = -1;
	int count = 0;
	char chipidStr[PROPERTY_VALUE_MAX] = {0};
	int loadFmResult = -1;
	int loadAntResult = -1;
	int loadWlanResult = -1;
	
	do{
		gLoaderFd = open(WCN_COMBO_LOADER_DEV, O_RDWR | O_NOCTTY);
		if(gLoaderFd < 0)
		{
			count ++;
			printf("Can't open device node(%s) count(%d)\n", WCN_COMBO_LOADER_DEV,count);
			usleep(300000);
		}
		else
			break;
	}while(1);
	
	//read from system property
	iRet = property_get(WCN_COMBO_LOADER_CHIP_ID_PROP, chipidStr, NULL);
	chipId = strtoul(chipidStr, NULL, 16);

	if ((0 != iRet) && (-1 != chipId))
	{
		/*valid chipid detected*/
		printf("key:(%s)-value:(%s),chipId:0x%04x,iRet(%d)\n", WCN_COMBO_LOADER_CHIP_ID_PROP, chipidStr, chipId,iRet);
	}
	else
	{
		/*trigger external combo chip detect and chip identification process*/
		/*detect is there is an external combo chip*/
		noextChip = ioctl(gLoaderFd,COMBO_IOCTL_EXT_CHIP_DETECT,NULL);

		if(noextChip)// use soc itself
		{
			printf("no external combo chip detected, get current soc chipid\n");
			chipId = ioctl(gLoaderFd, COMBO_IOCTL_GET_SOC_CHIP_ID, NULL);
			printf("soc chipid (0x%x) detected\n", chipId);  
		}
		else
		{
			printf("external combo chip detected\n");
		
			chipId = ioctl(gLoaderFd, COMBO_IOCTL_GET_CHIP_ID, NULL);
			printf("chipid (0x%x) detected\n", chipId);  
		}
	}
	
	sprintf (chipidStr, "0x%04x", chipId);
	
	/*set chipid to kernel*/
	ioctl(gLoaderFd,COMBO_IOCTL_SET_CHIP_ID,chipId);

	if (g_remove_ko_flag)
	{
		do_kernel_module_init(gLoaderFd, chipId);
		if(gLoaderFd >= 0)
		{
			close(gLoaderFd);
			gLoaderFd = -1;
		}

	}
	else
	{
		if(gLoaderFd >= 0)
		{
			close(gLoaderFd);
			gLoaderFd = -1;
		}
		printf("rmmod mtk_wmt_detect\n");
		rmmod("mtk_wmt_detect");

		/*INSERT TARGET MODULE TO KERNEL*/
		
		iRet = insert_wmt_modules(chipId, 0, -1);
		/*this process should never fail*/
		if(iRet)
		{
			printf("insert wmt modules fail(%d):(%d)\n",iRet,__LINE__);
			/*goto done;*/
		}

		
		loadFmResult = load_fm_module(chipId);
		if(loadFmResult)
		{
			printf("load FM modules fail(%d):(%d)\n",iRet,__LINE__);
			/*continue, we cannot let this process interrupted by subsystem module load fail*/
			/*goto done;*/
		}
#if 0		
		loadAntResult = load_ant_module(chipId);
		if(loadAntResult)
		{
			printf("load ANT modules fail(%d):(%d)\n",iRet,__LINE__);
			/*continue, we cannot let this process interrupted by subsystem module load fail*/
			/*goto done;*/
		}
#endif
		loadWlanResult = load_wifi_module(chipId);
		if(loadWlanResult)
		{
			printf("load WIFI modules fail(%d):(%d)\n",iRet,__LINE__);
			/*continue, we cannot let this process interrupted by subsystem module load fail*/
			/*goto done;*/
		}
	}	
	
	iRet = property_set(WCN_COMBO_LOADER_CHIP_ID_PROP,chipidStr);
	if (0 != iRet)
	{
		printf("set property(%s) to %s failed,iRet:%d, errno:%d\n", WCN_COMBO_LOADER_CHIP_ID_PROP, chipidStr, iRet, errno);
	}
	else
	{
		printf("set property(%s) to %s succeed.\n", WCN_COMBO_LOADER_CHIP_ID_PROP, chipidStr);
	}

	
	if((chown("/proc/driver/wmt_dbg",2000,1000) == -1) || (chown("/proc/driver/wmt_aee",2000,1000) == -1))
	{
		printf("chown wmt_dbg or wmt_aee fail:%s\n",strerror(errno));
	}

	if(chown("/proc/wmt_tm/wmt_tm",0,1000) == -1)
	{
		printf("chown wmt_tm fail:%s\n",strerror(errno));
	}
#if 0
	while (loadWlanResult || loadFmResult)	
	{
		if(loadFmResult)
		{
			static int retryCounter = 0;
			retryCounter++;
			printf("retry loading fm module, retryCounter:%d\n", retryCounter);
			loadFmResult = load_fm_module(chipId);
		}
		
		if(loadWlanResult)
		{
			static int retryCounter = 0;
			retryCounter++;
			printf("retry loading wlan module, retryCounter:%d\n", retryCounter);
			loadWlanResult = load_wifi_module(chipId);
		}
		usleep(1000000);
	}
#endif
	
	return iRet;
}



