#ifndef _VOICE_UNLOCK_H
#define _VOICE_UNLOCK_H

#define ___open_new_interface_with_cust__
#define ___use_dual_mic_endPointDetection__

//#include "int64_t.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int getVoiceUnlockVersion();

#ifndef ___callback_function_def_
#define ___callback_function_def_
typedef void (*Func_t)(void*, const short*, int64_t);
#endif

typedef struct __VoiceUnlockCustomParameters
{
    unsigned char Cust_Para[8][32];

    short  *voiceFIRCoefMic1;
    short  *voiceFIRCoefMic2;
    int  voiceGainMic1;
    int  voiceGainMic2;
    int  micNumFlag;

} VOICE_UNLOCK_CUSTOM_INFO;

/* struct definition */
typedef struct __UnlockTrainingParameters
{
    char *str1;
    char *str2;

	int id;
	int fd1;
	int fd2;

	Func_t Callback;
	void *me;

} UNLOCK_TRAIN_INFO;

typedef struct __UnlockTestParameters
{
    char *str1;
    char *str2;

} UNLOCK_TEST_INFO;


/* Initialization and Release function */
//str1 : pattern directory; str2: UBM directory
#ifdef ___open_new_interface_with_cust__
int TrainingInit(UNLOCK_TRAIN_INFO *unlockTrainInfo,VOICE_UNLOCK_CUSTOM_INFO *voiceCustomInfo ) ;
#else
int TrainingInit(int id, int fd1, int fd2,const char *str1, const char *str2,Func_t Callback,void *me) ;
#endif

//str1: pattern directory str2:UBM directory
#ifdef ___open_new_interface_with_cust__
int TestingInit(UNLOCK_TEST_INFO *unlockTestInfo, VOICE_UNLOCK_CUSTOM_INFO *voiceCustomInfo);
#else
int TestingInit (const char *str1, const char *str2);
#endif
int DLInit(int downlink_latency);

int TrainingRelease();
int TestingRelease();

/* voice unlock utility function */
// return 1: stop  2: no speech in 5 seconds
#ifdef ___use_dual_mic_endPointDetection__
int endPointDetection (short *pMicBuf1, short *pMicBuf2, short *pDLBuf);
#else
int endPointDetection (short *pMicBuf, short *pDLBuf);
#endif


int PCMDiagonosis(int *confidence);
int onTraining();
int onTesting(int *command_id);

#ifdef __cplusplus
}
#endif
#endif
