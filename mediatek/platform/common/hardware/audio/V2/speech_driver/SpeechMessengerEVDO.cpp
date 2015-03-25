
#include <utils/Log.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <termios.h>
#include <dlfcn.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <termios.h>
#include <SpeechType.h>
#include <semaphore.h>
#include <cutils/properties.h>
#include <sys/prctl.h>

#include <AudioMTKStreamInManager.h>
#include <AudioUtility.h>
#include <SpeechMessengerEVDO.h>

  

#undef LOG_TAG
#define LOG_TAG "SpeechMessengerEVDO"


#define ENABLE_XLOG_SPEECH_EVDO_MSG
#ifdef ENABLE_XLOG_SPEECH_EVDO_MSG
#include <cutils/xlog.h>
#define SPEECH_MSG_VEB(fmt, arg...)  SXLOGV(fmt, ##arg)
#define SPEECH_MSG_DBG(fmt, arg...)  SXLOGD(fmt, ##arg)
#define SPEECH_MSG_INFO(fmt, arg...) SXLOGI(fmt, ##arg)
#define SPEECH_MSG_WARN(fmt, arg...) SXLOGW(fmt, ##arg)
#define SPEECH_MSG_ERR(fmt, arg...)  SXLOGE("Err: %5d:, "fmt, __LINE__, ##arg)
#else
#include <utils/Log.h>
#define SPEECH_MSG_VEB  ALOGV
#define SPEECH_MSG_DBG  ALOGD
#define SPEECH_MSG_INFO ALOGI
#define SPEECH_MSG_WARN ALOGW
#define SPEECH_MSG_ERR  ALOGE
#endif


#define DUMP_RECORD_PCM
#ifdef DUMP_RECORD_PCM
static   const char * gevdo_record_pcm = "/sdcard/evdo_record_pcm";
static	 const char * gevdo_record_propty = "evdo.record.pcm";

#endif

//#define RECORD_FAKE
#ifdef RECORD_FAKE
    #define RECORD_FUNCTION Record_main_fake
#else
    #define RECORD_FUNCTION RecordVoiceThread
#endif

//const variable
#define BUFFER_SIZE 512
#define MAX_RETRY 5
#define HALT 20000
#define HALT_OPEN 200000

#define READ_BUF_SIZE 4096
/*
#define DEVICE_NAME "/dev/ptty3audio"    //for atcmd send directly
#define AT_COMMAND_SPEECH_ON                       "AT+ESPEECH=1, %d\r\n"   //done by ATCommand
#define AT_COMMAND_SPEECH_OFF                      "AT+ESPEECH=0\n" //done by ATCommand
#define AT_COMMAND_SIDETONE_VOLUME                 "AT+ESSTV=%d\r\n"    //done by ATCommand
#define AT_COMMAND_DEFAULT_TONE_PLAY               "AT+EDTP="   //done by rpc
#define AT_COMMAND_DEFAULT_TONE_STOP               "AT+EDTS"   //done by rpc
#define AT_COMMAND_INPUT_SOURCE                    "AT+ESETDEV=0, "   //no support
#define AT_COMMAND_OUTPUT_DEVICE                   "AT+ESETDEV=1, "   //no support
#define AT_COMMAND_OUTPUT_VOLUME                   "AT+ESOV=%d, %d\r\n" //done by rpc
#define AT_COMMAND_MICROPHONE_VOLUME               "AT+ESMV="   //no support
#define AT_COMMAND_SPEECH_MODE_ADAPTATION          "AT+ESSMA="   //done by rpc
*/
//modem sr 8Khz,Ch 1 mono

#define atcmd_cpy(x,y) memcpy(x,y,sizeof(*x))   //copy msg command

//via deivce define
enum VIATEL_CHANNEL_ID
{    
    VIATEL_CHANNEL_AT,    
    VIATEL_CHANNEL_DATA,
    VIATEL_CHANNEL_ETS,
    VIATEL_CHANNEL_GPS,
    VIATEL_CHANNEL_PCV,
    VIATEL_CHANNEL_ASCI,
    VIATEL_CHANNEL_FLS,
    VIATEL_CHANNEL_MUX,
    VIATEL_CHANNEL_NUM
};


namespace android
{

RPC_Client::RPC_Client()
{
	SPEECH_MSG_DBG("%s()", __FUNCTION__);
	mHandle = NULL;
	mInited = false;
	//initial rpc file pointer 
    sendRpcRequest=NULL;
	sendRpcRequestComm=NULL; 
}
RPC_Client::~RPC_Client()
{
	SPEECH_MSG_DBG("%s()", __FUNCTION__);
	if(mInited)
	{
		RPC_release();
	}
}
void RPC_Client::RPC_release()
{
	SPEECH_MSG_DBG("%s()", __FUNCTION__);
    if(mHandle!=NULL)
	{
        dlclose(mHandle);
		mHandle=NULL;
    }

    sendRpcRequest=NULL;
    sendRpcRequestComm=NULL; 
	mInited = false;
}

status_t RPC_Client::RPC_init()
{
    SPEECH_MSG_DBG("+%s()", __FUNCTION__);
    pthread_mutex_init(&mSendMsgMutex, NULL);
    
	pthread_mutex_lock(&mSendMsgMutex);
	mHandle=(int*)dlopen(RPC_RIL_PATH, RTLD_NOW);
	if(mHandle==NULL)
	{
		SPEECH_MSG_ERR("dlopen fail: %s", dlerror());
		pthread_mutex_unlock(&mSendMsgMutex);
		return NO_INIT;
	}
	sendRpcRequest = (int (*)(int, int))dlsym(mHandle, "sendRpcRequest");
	sendRpcRequestComm = (int (*)(int, int, int, void *, int, int *, void ***))dlsym(mHandle, "sendRpcRequestComm");
	if(NULL==sendRpcRequest || NULL==sendRpcRequestComm)
	{
		SPEECH_MSG_ERR("can not find function sendRpcRequest or sendRpcRequestComm");
		RPC_release();
		pthread_mutex_unlock(&mSendMsgMutex);
		return NO_INIT;
	}
	mInited = true;
	pthread_mutex_unlock(&mSendMsgMutex);
    SPEECH_MSG_DBG("-%s()", __FUNCTION__);
	return NO_ERROR;
}

int RPC_Client::RPC_send(int v1, int v2)
{
    SPEECH_MSG_DBG("+%s(%d,%d)", __FUNCTION__,v1,v2);
    
	pthread_mutex_lock(&mSendMsgMutex);
    int ret=1;
    if(sendRpcRequest)
	{
        ret= sendRpcRequest(v1,v2);
    }
	pthread_mutex_unlock(&mSendMsgMutex);
    SPEECH_MSG_DBG("-%s return ret =%d", __FUNCTION__,ret);
    return ret;
}

int RPC_Client::RPC_send_comm(int v1, int v2, int v3, void *v4, int v5, int *v6, void *** v7)
{
    SPEECH_MSG_DBG("%s", __FUNCTION__);
    
	pthread_mutex_lock(&mSendMsgMutex);
    int ret=1;
    if(sendRpcRequestComm)
	{
        SPEECH_MSG_DBG("sendRpcRequestComm(%d,%d,%d,%x,%d,%x,%x);",v1,v2,v3,v4,v5,v6,v7);
        ret= sendRpcRequestComm(v1,v2,v3,v4,v5,v6,v7);
    }
	pthread_mutex_unlock(&mSendMsgMutex);
    return ret;
}

//constructor
SpeechMessengerEVDO::SpeechMessengerEVDO()
{	
    SPEECH_MSG_DBG("%s construct", __FUNCTION__); 
	mIsInitQ = false;
	mEVDOEnable = false;
	//initial record fp
    mRecHdl=-1;
	mRecordEnable = false;
	mClientPtr = NULL;
	mRecordSR = 8000;
}


SpeechMessengerEVDO::~SpeechMessengerEVDO()
{	
    SPEECH_MSG_DBG("%s Destructure", __FUNCTION__);
	if(mEVDOEnable||mClientPtr||mRecordEnable)
	{
		DeInitial();
	}	
}
status_t SpeechMessengerEVDO::Initial()
{
	SPEECH_MSG_DBG("%s", __FUNCTION__);    
	mClientPtr = new RPC_Client;
	if(mClientPtr->RPC_init()!=NO_ERROR)
	{
		return NO_INIT;
	}
    //initialize atcmd queue
	InitATCmdQueue();
	//create evdo msg thread
	int ret = pthread_create(&mATCmdT,NULL, ReadATCmdThread, (void*)this);
	if(ret !=0)
	{
		SPEECH_MSG_DBG("atcmd_task: thread create fail, err=%d:%s", errno, strerror(errno));
		return UNKNOWN_ERROR;
	}
	return NO_ERROR;
}

status_t SpeechMessengerEVDO::DeInitial()
{
	SPEECH_MSG_DBG("%s", __FUNCTION__);
	if(mEVDOEnable)
	{
		mEVDOEnable = false;
		pthread_join(mATCmdT,NULL);
    	SPEECH_MSG_DBG("DeInitial pthread_join");	
	}
	if(mClientPtr!=NULL)
	{
		delete mClientPtr;
		mClientPtr = NULL;
	}
	if(mRecordEnable)
	{
		mRecordEnable = false;
		pthread_join(mRecordT,NULL);
		SPEECH_MSG_DBG("CloseRecDevice pthread_join");
		CloseRecDevice();
	}
	mIsInitQ= false;
	sem_destroy(&mATCmdQ.sem);

    return NO_ERROR;	
}


// static
void* SpeechMessengerEVDO::ReadATCmdThread(void *me) 
{
	SPEECH_MSG_DBG("%s", __FUNCTION__);
	prctl(PR_SET_NAME, (unsigned long)__FUNCTION__, 0, 0, 0);
    return (void *) static_cast<SpeechMessengerEVDO *>(me)->ReadATCmdLooper();
}


status_t SpeechMessengerEVDO::InitATCmdQueue()
{
    SPEECH_MSG_DBG("+%s", __FUNCTION__);
    
	if(mIsInitQ) 
	{
		return NO_ERROR;
	}
	
	/* atcmdq init */
	mATCmdQ.tail = 0;
	mATCmdQ.head = 0;
	mATCmdQ.full = 0;
    
	pthread_mutex_init(&mATCmdQ.mutex, NULL);
	sem_init(&mATCmdQ.sem, 0, 0);
	
	mIsInitQ = true;
	SPEECH_MSG_DBG("-%s", __FUNCTION__);
	return NO_ERROR;
}

status_t SpeechMessengerEVDO::PostATCmdItem(int type,int arg1,int arg2)
{
    atcmd_req_t cmd;
    cmd.type=type;
    cmd.arg1=arg1;
    cmd.arg2=arg2;
    return PostATCmd(&cmd);
}

status_t SpeechMessengerEVDO::PostATCmd(atcmd_req_t *cmd)
{
    SPEECH_MSG_VEB("%s", __FUNCTION__);
    
	atcmdq_t *q=&mATCmdQ;
	unsigned int i;
    bool found=false;
    
	InitATCmdQueue();

	pthread_mutex_lock(&mATCmdQ.mutex);

	SPEECH_MSG_DBG("+atcmdq_add: index=%ld type:%d args1=%d arg2=%d", q->tail, cmd->type, cmd->arg1, cmd->arg2);
	if(q->full==1)
	{
		SPEECH_MSG_DBG("atcmdq_add: q is full");
		sem_post(&mATCmdQ.sem);
		pthread_mutex_unlock(&mATCmdQ.mutex);
		return -1;
	}

	/* take out the same type comment before: simple, assumption qlen is short 
	   this method will impact the command order (not good), marked REQ_CLEAR and then add, avoid the stress test
	   order, does not mater, replace so far. at least, 3 case
	*/
	for(i=q->head;i<q->tail;i++) 
	{
		if(q->cmd[i].type==cmd->type) 
		{
			found=true;
			atcmd_cpy(&(q->cmd[i]),cmd);
            break;
		}
	}
	if(!found) 
	{
        atcmd_cpy(&(q->cmd[q->tail]),cmd);
		q->tail++;
		sem_post(&mATCmdQ.sem);
	}

	if(q->tail==ATCMDQ_MAXSIZE)
	{
		q->tail=0;
	}
	if(q->tail==q->head)
	{
		q->full=1;
	}
	
	pthread_mutex_unlock(&mATCmdQ.mutex);

	SPEECH_MSG_DBG("-PostATCmd");
	
	return NO_ERROR;
}

status_t SpeechMessengerEVDO::GetCurrentATCmd(atcmd_req_t *cmd)
{
    SPEECH_MSG_DBG("%s", __FUNCTION__);

	atcmdq_t *q=&mATCmdQ;
	sem_wait(&mATCmdQ.sem);
	pthread_mutex_lock(&mATCmdQ.mutex);
	if(q->head==q->tail) 
	{
		SPEECH_MSG_DBG("GetCurrentATCmd: queue is empty");
		pthread_mutex_unlock(&mATCmdQ.mutex);
		return -1;
	}
    atcmd_cpy(cmd,&(q->cmd[q->head]));
    
	SPEECH_MSG_DBG("+GetCurrentATCmd: index=%ld type:%d args1=%d arg2=%d)", q->head, cmd->type, cmd->arg1, cmd->arg2);
	q->head++;
	if(q->head==ATCMDQ_MAXSIZE)
	{
		q->head=0;
	}
	q->full=0;
	pthread_mutex_unlock(&mATCmdQ.mutex);

	return NO_ERROR;
}


status_t SpeechMessengerEVDO::ReadATCmdLooper()
{
    SPEECH_MSG_DBG("%s", __FUNCTION__);
    	
	atcmd_req_t cmd;
	memset(&cmd,0x0,sizeof(atcmd_req_t));
	int result = NO_ERROR;
  mEVDOEnable = true;
	while(mEVDOEnable) 
	{
		result=GetCurrentATCmd(&cmd);
		SPEECH_MSG_DBG("GetCurrentATCmd: got it, result=%d cmd(%d, %d, %d)", result, cmd.type, cmd.arg1, cmd.arg2);
        
		if(result== NO_ERROR) 
		{
			switch(cmd.type) 
			{
				case REQ_VOICE_RECORD:
				{
                    result = mClientPtr->RPC_send(RIL_REQUEST_SET_VOICE_RECORD, cmd.arg1);
                    if(result<0) 
					{
                        SPEECH_MSG_ERR("setCPAudioRecord: fail, cmd=%d (1:start, 0:stop)", cmd.arg1);
                    }
                    break;
				}
				case REQ_AUDIO_MODE:
				{
					/* AT command to MD firmware */
					result = mClientPtr->RPC_send(RIL_REQUEST_SET_AUDIO_PATH,cmd.arg1);
					if(result<0) 
					{
						SPEECH_MSG_ERR("setCPAudioPath: fail, REQ_AUDIO_MODE mode=%d", cmd.arg1);
					}
					break;
				}
				case REQ_MUTE:
				{
					result=mClientPtr->RPC_send(RIL_REQUEST_SET_MUTE,cmd.arg1);
					if(result<0) 
					{
						SPEECH_MSG_ERR("setCPAudioMute: fail, REQ_MUTE enable=%d", cmd.arg1);
					}
					break;
				}
				case REQ_AUDIO_VOLUME:
				{
					result=mClientPtr->RPC_send(RIL_REQUEST_SET_VOICE_VOLUME,cmd.arg1);
					if(result<0) 
					{
						SPEECH_MSG_ERR("setCPAudioVolume: fail, REQ_AUDIO_VOLUME vol=%d result=%d", cmd.arg1, result);
					}
					break;
				}
				case REQ_PLAY_DTMF:
				{    
                    result=SetCPAudioPlayDtmf(cmd.arg1,cmd.arg2);
					if(result<0) 
					{
						SPEECH_MSG_ERR("SetCPAudioPlayDtmf: fail, REQ_PLAY_DTMF mode=%d toneId=%d result=%d", cmd.arg1, cmd.arg2, result);
					}
					break;
				}
				case REQ_PLAY_TONE_SEQ:
				{                    
					result=SetCPAudioPlayTone(cmd.arg1);
					if(result<0) 
					{
                        SPEECH_MSG_ERR("SetCPAudioPlayTone: fail, REQ_PLAY_TONE_SEQ toneId=%d result=%d", cmd.arg1, result);
					}
					break;
				}
				default:
				{
					SPEECH_MSG_ERR("ReadATCmdLooper: cmd:%d not support", cmd.type);
				}
			}   //end switch
		}   //end if 
	}   //end while
	pthread_exit(NULL);
    return NO_ERROR;
}

int SpeechMessengerEVDO::SetCPAudioVoiceRecord(int on)
{
	SPEECH_MSG_DBG("%s", __FUNCTION__);
    return mClientPtr->RPC_send(REQ_VOICE_RECORD, on);
}

int SpeechMessengerEVDO::SetCPAudioPlayDtmf(int mode, int dtmf)
{
    SPEECH_MSG_DBG("%s(mode=%d,dtmf=%d)", __FUNCTION__, mode, dtmf);
    
    int dtmfData[4]={0};
    int outLen = 0; 
    void **outValue = NULL;

    //DTMF burst tone        
    int duration=0;        
    if(dtmf>=TONE_MAX)
	{            
        dtmf-=TONE_MAX;            
        duration=1;        
    }
    
    dtmfData[0] = mode;	// mode:1 play, mode:0 stop
    dtmfData[1] = dtmf; 	// dtmf 
    dtmfData[2] = 5;	// volume ?
    dtmfData[3] = duration;	// duration, 0:continue 1:oneshot
    return mClientPtr->RPC_send_comm(REQ_PLAY_DTMF, 0, sizeof(dtmfData)/sizeof(int), dtmfData, 4, &outLen, &outValue);
}

int SpeechMessengerEVDO::SetCPAudioPlayTone(int toneId)
{
    SPEECH_MSG_DBG("%s(toneId=%d)", __FUNCTION__, toneId);
    
    int ret=-1,outLen = 0; 
    void **outValue = NULL;
    
    TONE_SPEC_T *tone=0;
    TONE_SPEC_T tone_spec[] = {
        {TONE_MMS_NOTIFICATION, 1, 0, 5, {10, 852, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {TONE_LOW_BATTERY_NOTIFICATION, 3, 0, 5, {15, 1100, 0}, {5, 0, 0}, {15, 900, 0}, {0, 0, 0}},
        {TONE_CALL_WAITING_INDICATION, 4, 3, 5, {10, 440, 0}, {5, 0, 0}, {10, 400, 0}, {175, 0, 0}},
        {TONE_CALL_REMINDER_INDICATION, 4, 1, 5, {10, 440, 0}, {5, 0, 0}, {10, 400, 0}, {175, 0, 0}},
        {TONE_CALL_SMS_INDICATION, 1, 1, 5, {25, 440, 880}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}
    };
    
    for(int i=0;i<(sizeof(tone_spec)/sizeof(TONE_SPEC_T));i++) 
	{
        if(tone_spec[i].id==toneId) 
		{
            tone=&tone_spec[i];
        }
    }
    if(tone!=0) 
	{
        ret=mClientPtr->RPC_send_comm(REQ_PLAY_TONE_SEQ, 0, (sizeof(TONE_SPEC_T)/sizeof(int))-1, &(tone->num), 4, &outLen, &outValue);
    }
    
    return ret;    
}

bool SpeechMessengerEVDO::Spc_SetAudioMode(int cdma_audio_mode )
{
    SPEECH_MSG_DBG("%s(cdma_audio_mode=%d)", __FUNCTION__, cdma_audio_mode);
    return PostATCmdItem(REQ_AUDIO_MODE, cdma_audio_mode);
}

bool SpeechMessengerEVDO::Spc_SetOutputVolume(uint32_t Gain)
{
    SPEECH_MSG_DBG("%s(%d)", __FUNCTION__, Gain);
    return PostATCmdItem(REQ_AUDIO_VOLUME,Gain);
}


int SpeechMessengerEVDO::Spc_Default_Tone_Play(uint8_t toneIdx)
{
    SPEECH_MSG_DBG("%s(toneIdx=%d)", __FUNCTION__,toneIdx); 
    if(toneIdx>TONE_START&&toneIdx<TONE_MAX)
	{
        return PostATCmdItem(REQ_PLAY_TONE_SEQ,toneIdx);   //12~16
    }
	else
	{
        return PostATCmdItem(REQ_PLAY_DTMF,1,toneIdx); //0~11
    } 
    //out of dtmf tone max
    return -1;
}

int SpeechMessengerEVDO::Spc_Default_Tone_Stop(void)
{
    SPEECH_MSG_DBG("%s", __FUNCTION__);
    return PostATCmdItem(REQ_PLAY_DTMF);
}

bool SpeechMessengerEVDO::Spc_MuteMicrophone(uint8_t enable)
{
    SPEECH_MSG_DBG("%s", __FUNCTION__);
    return PostATCmdItem(REQ_MUTE,enable);
}


status_t SpeechMessengerEVDO::Spc_OpenNormalRecPath(uint32_t sampleRate)
{
    SPEECH_MSG_DBG("%s sampleRate %d", __FUNCTION__,sampleRate);

    int ret=0;
    char tempstr[60] = {0}, dev_path[64]={0};
	mRecordSR = sampleRate;
#ifdef RECORD_FAKE
	ret =0;
	mRecHdl=999;   //fake file id
#else
    //get via voice recording device
    char * (*viatelAdjustDevice)(int);
    void *dlHandle;
    dlHandle = dlopen( "/system/lib/libviatelutils.so", RTLD_NOW);
    if (dlHandle == NULL) 
	{
        SPEECH_MSG_ERR("dlopen failed: %s", dlerror());
        return UNKNOWN_ERROR;
    }
    viatelAdjustDevice = (char * (*)(int))dlsym(dlHandle, "viatelAdjustDevicePathFromProperty");
    char *pcvdev = viatelAdjustDevice(VIATEL_CHANNEL_PCV);
    if(NULL == viatelAdjustDevice)
	{
        SPEECH_MSG_ERR("can not find function viatelAdjustDevice");
        return  UNKNOWN_ERROR;
    }
    dlclose(dlHandle);
    snprintf(dev_path, 64, "%s", pcvdev);
    free(pcvdev);
    //--------------------------------------
    SPEECH_MSG_DBG("VoiceRecord Device = %s",dev_path);
    ret = SetCPAudioVoiceRecord(1);
    if(ret <0)
    {
		SPEECH_MSG_ERR("SetCPAudioVoiceRecord fail!");
		return UNKNOWN_ERROR;
    }
    ret=OpenRecDevice(dev_path);
#endif

    if(mRecHdl>=0 )
	{
    	/* record thread */
    	if(pthread_create(&mRecordT, NULL, RECORD_FUNCTION, (void*)this)!=0)
		{
    		SPEECH_MSG_ERR("RecordVoiceThread: thread create fail, err=%d:%s", errno, strerror(errno));
			return UNKNOWN_ERROR;
		}
    }
    return ret;
}

status_t SpeechMessengerEVDO::Spc_CloseNormalRecPath()
{
    SPEECH_MSG_DBG("%s", __FUNCTION__);
    int ret =0;
#ifdef RECORD_FAKE	
    mRecHdl = -1;
#else
    ret =SetCPAudioVoiceRecord(0);
	if(ret <0)
	{
		SPEECH_MSG_ERR("SetCPAudioVoiceRecord close fail!");
		return UNKNOWN_ERROR;
	}
	mRecordEnable = false;
    pthread_join(mRecordT,NULL);
    SPEECH_MSG_DBG("CloseRecDevice pthread_join");
    CloseRecDevice();
#endif	
	return NO_ERROR;
}

int  SpeechMessengerEVDO::Spc_ReadRNormalRecData(uint16_t *pBuf, uint16_t u2Size)
{
    SPEECH_MSG_DBG("%s", __FUNCTION__);    
    
    int n;
    
    n=read(mRecHdl, pBuf, u2Size);
	return n;
}
status_t SpeechMessengerEVDO::OpenRecDevice(char *deviceName)
{
    SPEECH_MSG_DBG("%s(deviceName=%s)", __FUNCTION__,deviceName);
    
	struct termios term;

    if(mRecHdl<0)
	{
        mRecHdl=open(deviceName, O_RDONLY | O_NONBLOCK );
    }
	else
	{
		SPEECH_MSG_DBG("openRecDevice faild:%s (%d)", deviceName,mRecHdl );
		return NO_ERROR;
	}    
    SPEECH_MSG_DBG("open %s Success, mRecHdl=%d", deviceName, mRecHdl);
    // flush terminal
    if(tcgetattr(mRecHdl, &term)<0) 
	{
        SPEECH_MSG_ERR("Fail: tcgetattr errno=%d", errno);
    }
    term.c_cflag=0;
    term.c_oflag=0;
    term.c_iflag=0;
    term.c_lflag=0;
    tcflush(mRecHdl, TCIFLUSH);
    
    if(tcsetattr(mRecHdl, TCSANOW, &term)<0) 
	{
        SPEECH_MSG_ERR("Fail: tcsetattr errno=%d", errno);
		return UNKNOWN_ERROR;
    }
#ifdef DUMP_RECORD_PCM
	struct tm *timeinfo;
	time_t rawtime;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	sprintf(mRecFilePath, "%s", gevdo_record_pcm);
	strftime(mRecFilePath + strlen(gevdo_record_pcm), 60, "_%Y_%m_%d_%H_%M_%S_in.pcm", timeinfo);
	SPEECH_MSG_VEB("mRecFilePath =%s",mRecFilePath);
#endif	
    return NO_ERROR;
}

void SpeechMessengerEVDO::CloseRecDevice(void)
{
    SPEECH_MSG_DBG("%s", __FUNCTION__);	
    if(mRecHdl>=0)
	{
    	close(mRecHdl);
    }
    mRecHdl = -1;
}

void* SpeechMessengerEVDO::RecordVoiceThread(void *me)
{
	SPEECH_MSG_DBG("%s", __FUNCTION__);
	prctl(PR_SET_NAME, (unsigned long)__FUNCTION__, 0, 0, 0);
    return (void *) static_cast<SpeechMessengerEVDO *>(me)->RecordVoiceLooper();
}


status_t SpeechMessengerEVDO::RecordVoiceLooper()
{
	SPEECH_MSG_DBG("%s", __FUNCTION__);
	
	struct timeval tv;
	fd_set readfd;
	int result, n=0;
	const int sample_len = READ_BUF_SIZE;
	unsigned short* pBuf = NULL; // 
	mRecordEnable = true;
	pBuf = (unsigned short*)malloc(sizeof(unsigned short)*sample_len);
	memset((void*)pBuf,0,sizeof(unsigned short)*sample_len);	
	while(mRecordEnable)
	{ //while file pointer is not empty
		FD_ZERO(&readfd);
		FD_SET(mRecHdl, &readfd);
		tv.tv_sec=1;
		tv.tv_usec=0;		
		result=select(mRecHdl+1, &readfd, NULL, NULL, &tv);// max wait time 1s
		if(result>0) 
		{
			if(FD_ISSET(mRecHdl, &readfd)) 
			{
				n=read(mRecHdl, pBuf, sample_len); 			   	
#ifdef DUMP_RECORD_PCM
				RecordDump((uint8_t*)pBuf,n);
#endif 	
                if(n>0)
				{
                    SPEECH_MSG_VEB("%s n %d", __FUNCTION__,n);
					// setup ring buffer
					RingBuf ul_ring_buf;
					ul_ring_buf.bufLen = sample_len*sizeof(unsigned short);//4096 *2
					ul_ring_buf.pBufBase = (char*)pBuf;				 
					ul_ring_buf.pRead = ul_ring_buf.pBufBase;
					ul_ring_buf.pWrite = ul_ring_buf.pBufBase;					
					RingBuf_copyFromLinear(&ul_ring_buf, (const char*)pBuf, n);					
					AudioMTKStreamInManager::getInstance()->CopyBufferToClientIncall(ul_ring_buf);
                }
			}
		} 
		else if(result<0) 
		{
			// error
			SPEECH_MSG_ERR("%s select: errno = %d", __FUNCTION__,errno);
		}
	}
		

    if(pBuf!=NULL)
    {
		free(pBuf);
    }
	
	return NO_ERROR;

}
	
void SpeechMessengerEVDO::EnableDualMic(bool enable)
{
	PostATCmdItem(REQ_AUDIO_MODE, enable ? 32:33 );
}

void* SpeechMessengerEVDO::Record_main_fake(void *me)
{
	SPEECH_MSG_DBG("%s", __FUNCTION__);
	prctl(PR_SET_NAME, (unsigned long)__FUNCTION__, 0, 0, 0);
    return (void *) static_cast<SpeechMessengerEVDO *>(me)->Record_fake();
}

status_t SpeechMessengerEVDO::Record_fake()
{
    //play a sine tone ifdefine RECORD_FAKE
    SPEECH_MSG_DBG("%s", __FUNCTION__);

    char tone[]={0,20,39,59,78,97,115,133,149,165,180,193,206,217,227,235,242,247,251,254,255,254,251,247,242,235,227,217,206,193,180,165,149,133,115,97,78,59,39,20,0,-20,-39,-59,-78,-97,-115,-133,-149,-165,-180,-193,-206,-217,-227,-235,-242,-247,-251,-254,-255,-254,-251,-247,-242,-235,-227,-217,-206,-193,-180,-165,-149,-133,-115,-97,-78,-59,-39,-20,
              0,20,39,59,78,97,115,133,149,165,180,193,206,217,227,235,242,247,251,254,255,254,251,247,242,235,227,217,206,193,180,165,149,133,115,97,78,59,39,20,0,-20,-39,-59,-78,-97,-115,-133,-149,-165,-180,-193,-206,-217,-227,-235,-242,-247,-251,-254,-255,-254,-251,-247,-242,-235,-227,-217,-206,-193,-180,-165,-149,-133,-115,-97,-78,-59,-39,-20
              };

    RingBuf RingBuf1;
    memset(&RingBuf1,0x0,sizeof(RingBuf));
    RingBuf1.bufLen = READ_BUF_SIZE+1;
    RingBuf1.pBufBase = (char*)malloc(sizeof(char)*RingBuf1.bufLen+1);
	memset((void*)RingBuf1.pBufBase,0,sizeof(char)*RingBuf1.bufLen+1);
    RingBuf1.pRead = RingBuf1.pBufBase;
	RingBuf1.pWrite = RingBuf1.pBufBase;

    int idx = 0;
    int sample_length = sizeof(tone)/sizeof(char)/2; // length/2
    int ch=1;
	int bytePerSample =2;
	int64_t sleeptime = RingBuf1.bufLen/bytePerSample/ch *1000*1000 /mRecordSR;    
    char *linear_data = (char*)malloc(sizeof(char)*RingBuf1.bufLen+1);
	memset((void*)linear_data,0,sizeof(char)*RingBuf1.bufLen+1);
	SPEECH_MSG_DBG("%s sample_length %d,sleeptime %ld", __FUNCTION__,sample_length,sleeptime);

    //loop until record off
    for(;mRecHdl!=-1;usleep(sleeptime))
	{        
        SPEECH_MSG_VEB("copy fake recording data: idx:%d sample_length:%d RingBuf1.bufLen:%d", idx, sample_length, RingBuf1.bufLen);
        
        RingBuf RingBuf2 = RingBuf1;
        char *linear_data_pointer = linear_data;
        for(int remaind = READ_BUF_SIZE; remaind>0; remaind -= sample_length)
		{
            int write_sample_length = remaind > sample_length ? sample_length : remaind;
            memcpy(linear_data_pointer, tone + idx, write_sample_length);
            linear_data_pointer += write_sample_length;
            idx=(idx + write_sample_length)%sample_length;
        }
		
        RingBuf_copyFromLinear(&RingBuf2, (const char*)linear_data, READ_BUF_SIZE);
        AudioMTKStreamInManager::getInstance()->CopyBufferToClientIncall(RingBuf2);
    }
    if(linear_data!=NULL)
    {
    	free(linear_data);
    }
	if(RingBuf1.pBufBase != NULL)
	{
    	free(RingBuf1.pBufBase);
	}
    RingBuf1.pRead=NULL;
	RingBuf1.pWrite=NULL;
	RingBuf1.pBufBase=NULL;
    
	return NO_ERROR;    
}

bool SpeechMessengerEVDO::Spc_SetMicrophoneVolume( uint8_t mic_volume )
{
    SPEECH_MSG_DBG("%s(mic_volume=%d)", __FUNCTION__, mic_volume);
    return true;
}
bool SpeechMessengerEVDO::Spc_SetOutputDevice( uint8_t device )
{
    SPEECH_MSG_DBG("%s(device=%d)", __FUNCTION__, device);
    return true;
}

bool SpeechMessengerEVDO::Spc_SetInputSource( uint8_t src )
{
    SPEECH_MSG_DBG("%s(src=%d)", __FUNCTION__, src);
	return true;
}

bool SpeechMessengerEVDO::Spc_SetSidetoneVolume( uint8_t sidetone_volume )
{
    SPEECH_MSG_DBG("%s(sidetone_volume=%d)", __FUNCTION__, sidetone_volume);
	return true;
}
bool SpeechMessengerEVDO::Spc_Speech_On(phone_call_mode_t mode)
{
    SPEECH_MSG_DBG("%s(phone_call_mode_t=%d)", __FUNCTION__,mode);
    return true;
}

bool SpeechMessengerEVDO::Spc_Speech_Off(void)
{
    SPEECH_MSG_DBG("%s", __FUNCTION__);
    return true;
}

void SpeechMessengerEVDO::RecordDump(uint8_t *pBuf,int len)
{
	//SPEECH_MSG_VEB("RecordDump");
	char value[PROPERTY_VALUE_MAX];
	int ret;
	property_get(gevdo_record_propty, value, "0");
	int bflag=atoi(value);
	if(bflag)
	{
		if (len> 0)
		{
			FILE *fp = fopen(mRecFilePath,"ab");

			if (fp)
			{
			    fwrite(pBuf, 1, len, fp);
			    fclose(fp);
			}
		}
	}
}

//open device for derectly send at cmd
int SpeechMessengerEVDO::OpenDevice(char *deviceName)
{
    SPEECH_MSG_DBG("%s(%s)", __FUNCTION__,deviceName);
    
    int i;
    char buf[BUFFER_SIZE];
    
    if(atcmd_fd)
	{   //atcommand file did opened
        return atcmd_fd;
    }
    
    atcmd_fd = open(deviceName, O_RDWR | O_NONBLOCK);

    if(atcmd_fd < 0)
	{
        SPEECH_MSG_ERR("Fail to open %s, atcmd_fd=%d", deviceName, atcmd_fd);
        return -1;
    }
    
    SPEECH_MSG_DBG("open %s Success!, atcmd_fd=%d", deviceName, atcmd_fd);
    
    // +EIND will always feedback +EIND when open device,
    memset(buf, 0, BUFFER_SIZE);
    for(i=0; i<5 && strcmp(buf, "+EIND")!=0; i++)
	{
        usleep(HALT_OPEN);
        read(atcmd_fd, buf, BUFFER_SIZE);
    }
	
    SPEECH_MSG_DBG("Open feedback:'%s', i=%d", buf, i);
        
    return atcmd_fd;
}

};// namespace android
