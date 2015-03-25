#ifndef ANDROID_SPEECH_MESSENGER_EVDO_H
#define ANDROID_SPEECH_MESSENGER_EVDO_H

#include <stdio.h>
#include <semaphore.h>
#include <utils/threads.h>

//mtk
#include "CFG_AUDIO_File.h"
#include "AudioCustParam.h"
#include "SpeechType.h"
#include "AudioType.h"



namespace android
{
/**
* msg queue &item
*/
#define ATCMDQ_MAXSIZE 32
typedef enum e_atcmd_req_type {
	REQ_CLEAR=0,
	REQ_AUDIO_MODE=168,     /* RIL_REQUEST_SET_AUDIO_PATH=168 */
	REQ_MUTE=188,           /* #define RIL_REQUEST_SET_MUTE_FOR_RPC 188 */
	REQ_AUDIO_VOLUME=184,   /* RIL_REQUEST_SET_VOICE_VOLUME=140->184, new add */
	REQ_PLAY_DTMF=185,      /* #define RIL_REQUEST_PLAY_DTMF_TONE  185 */
	REQ_PLAY_TONE_SEQ=186,  /* #define RIL_REQUEST_PLAY_TONE_SEQ 186 */
	REQ_VOICE_RECORD=187	/* #define RIL_REQUEST_SET_VOICE_RECORD 187 */
} atcmd_req_type_e;

typedef struct s_atcmd_req {
	int type;//atcmd_req_type_e
	int arg1;
	int arg2;
} atcmd_req_t;

typedef struct s_atcmdq {
	atcmd_req_t cmd[ATCMDQ_MAXSIZE];// max 32
	unsigned long head;
	unsigned long tail;
	int full;
	pthread_mutex_t mutex;
	sem_t sem;
} atcmdq_t;

//vendor rpc (proceed atcmd) library path
#define RPC_RIL_PATH "/system/lib/librpcril.so"
//vendor Cmd Id
#define RIL_REQUEST_SET_AUDIO_PATH  168
#define RIL_REQUEST_SET_MUTE  188
#define RIL_REQUEST_SET_VOICE_VOLUME 184
#define RIL_REQUEST_SET_VOICE_RECORD 187
/**
* vendor warning tone
*/
enum e_tone_spec {
	TONE_START=11,
	TONE_MMS_NOTIFICATION=12, //MMS: Multi-Media Messaging Service 
	TONE_LOW_BATTERY_NOTIFICATION=13,
	TONE_CALL_WAITING_INDICATION=14,
	TONE_CALL_REMINDER_INDICATION=15,
	TONE_CALL_SMS_INDICATION=16,//SMS:Short Messaging Service
	TONE_MAX=17,
};

typedef struct s_tone_freq {
	int duration; /* # of 20 msec frames  */
	int freq1; /* Dual Tone Frequencies in Hz; for single tone, set 2nd freq to zero */
	int freq2;
} tone_freq_t;

typedef struct s_tone_spec {
	int id;
	int num;
	int iteration;//?
	int volume;
	tone_freq_t tone1;
	tone_freq_t tone2;
	tone_freq_t tone3;
	tone_freq_t tone4;
} TONE_SPEC_T;

class RPC_Client
{

public:
	RPC_Client();
	~RPC_Client();
	status_t RPC_init();
    void RPC_release();
	int RPC_send(int, int);
	int RPC_send_comm(int, int, int, void *, int, int *, void ***); 
    int (*sendRpcRequest)(int, int);
    int (*sendRpcRequestComm)(int, int, int, void *, int, int *, void ***);
protected:
	
private:
	int *mHandle;  //for rpc    
    pthread_mutex_t mSendMsgMutex;
	bool mInited;
};


class SpeechMessengerEVDO
{
public:
	SpeechMessengerEVDO();
	~SpeechMessengerEVDO();
	status_t    Initial();
	status_t    DeInitial();
	

    int  Spc_Default_Tone_Play(uint8_t toneIdx);
    int  Spc_Default_Tone_Stop();
    bool Spc_SetOutputVolume( uint32_t Gain )  ;
    bool Spc_SetAudioMode( int cdma_audio_mode );
    bool Spc_MuteMicrophone( uint8_t enable);    
    int  Spc_OpenNormalRecPath(uint32_t sampleRate);
    int  Spc_CloseNormalRecPath();

    int  Spc_ReadRNormalRecData(uint16_t *pBuf, uint16_t u2Size);
    void EnableDualMic(bool enable);
    int  SetCPAudioPlayDtmf(int mode, int dtmf);
    int  SetCPAudioPlayTone(int toneId);
    int  SetCPAudioVoiceRecord(int on);

    //no implement
	bool Spc_SetSidetoneVolume( uint8_t sidetone_volume );
	bool Spc_SetMicrophoneVolume( uint8_t mic_volume );//
	bool Spc_SetInputSource( uint8_t src );//
    bool Spc_SetOutputDevice( uint8_t device );//
    bool Spc_Speech_On(phone_call_mode_t mode);
    bool Spc_Speech_Off();
protected:


private:
// AT Cmd Queue 
    status_t     InitATCmdQueue();
	status_t     PostATCmd(atcmd_req_t *cmd);
	status_t     PostATCmdItem(int type,int arg1=0,int arg2=0);
	status_t     GetCurrentATCmd(atcmd_req_t * cmd);
	static void *ReadATCmdThread(void *arg);
	status_t     ReadATCmdLooper();	
// Record Voice Thread	
	static void *RecordVoiceThread(void *me);
	status_t     RecordVoiceLooper();
	status_t     OpenRecDevice(char *deviceName);
	void         CloseRecDevice();
	void         RecordDump(uint8_t *pBuf,int len);
	static void *Record_main_fake(void *me);
	status_t     Record_fake();	
//open evdo modem and send at command directly
	int          OpenDevice(char *deviceName);

private:
	pthread_t   mATCmdT;
	atcmdq_t    mATCmdQ;
	bool        mIsInitQ;
	bool        mEVDOEnable;
	RPC_Client *mClientPtr;
	pthread_t   mRecordT;
    int         mRecHdl; 
	bool        mRecordEnable;
    char        mRecFilePath[256];
	int         atcmd_fd;//for send direct evdo modem
	int         mRecordSR;
};



};// namespace android

#endif //SPEECH_MESSENGER_EVDO_H

