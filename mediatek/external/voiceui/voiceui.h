#ifndef _VOICEUI_H
#define _VOICEUI_H

#ifdef __cplusplus
extern "C" {
#endif

int getVoiceUiVersion();

typedef struct __VoiceUiCustomParameters
{
    unsigned char Cust_Para[8][32];

    short  *voiceFIRCoefMic1;
    short  *voiceFIRCoefMic2;
    int  voiceGainMic1;
    int  voiceGainMic2;
    int  micNumFlag;

} VOICE_UI_CUSTOM_INFO;

/* keyword config APIs ----------------------------------------------------------------------------------*/
/* usage: use before invoking initKWS*/
enum {kwConfigSuccess, kwConfigError}; /*config return flags*/

int setKeywordInfo(const char *pKeywordInfo);
    /* pKeywordInfo[0] = language_num;
       pKeywordInfo[1] = app num of language id 1
       pKeywordInfo[2] = app num of language id 2
       ...
       pKeywordInfo[language_num] = app num of language id (language_num)

       pKeywordInfo[language_num + 1         ] = keyword num of language id 1 for app id 1, named kn_1_1
       pKeywordInfo[language_num + 1 + 1     ] = one keyword id of language id 1 for app id 1
       pKeywordInfo[language_num + 1 + 2     ] = one keyword id of language id 1 for app id 1
       ...
       pKeywordInfo[language_num + 1 + kn_0_0] = one keyword id of language id 0 for app id 1

       pKeywordInfo[language_num + 1 + kn_0_0 + 1         ] = keyword num of language id 1 for app id 1, named kn_1_2
       pKeywordInfo[language_num + 1 + kn_0_0 + 1 + 1     ] = one keyword id of language id 1 for app id 2
       pKeywordInfo[language_num + 1 + kn_0_0 + 1 + 2     ] = one keyword id of language id 1 for app id 2
       ...
       pKeywordInfo[language_num + 1 + kn_0_0 + 1 + kn_0_1] = one keyword id of language id 1 for app id 2

       ...
       until all combination of languages and apps have corresponding keyword number and id settings
       note: it is accpetable that some language and app have no keyword; then give 0 to the number, and no further keyword ids follow after the number
       note: feasible id rules:  0 < id <= number
    */

int setCommandFileLocation(const int languageId, const char* fileLocationDir);
    /* fileLocationDir: resource directory where command files related to given language are stored
       please invoke once per language
       please invoke this function after setKeywordInfo
    */

/*-------------------------------------------------------------------------------------------------------*/

/* recognition config APIs ------------------------------------------------------------------------------*/
/* usage: use after invoking initKWS, */
enum {regConfigSuccess, regConfigError}; /*setting return flags*/

int setRecognitionConfig(const int languageConfig, const int appConfig);
    /*  for languageConfig
        if i-th bit is 1, then language id i is set for recognition; otherwise, disable the recognition for the language
        ex: 0x0005 means that language id 1 and 3 are set for recognition, and the recognition for other languages are disabled
        appConfig use the same rule as languageSetting
    */
/*-------------------------------------------------------------------------------------------------------*/

/* main APIs --------------------------------------------------------------------------------------------*/
void initKWS(const char* modelLocationDir, VOICE_UI_CUSTOM_INFO *voiceCustomInfo); /*Initialize Keyword Spotting Engine, (modelLocationDir: resource directory where model files are stored)*/
void releaseKWS(); /*Release Keyword Spotting Engine*/
void setUIDLLatency(int downlink_latency); /*Set Downlink Latency, (downlink_latency: the downlink latency)*/
int doKeywordSpotting();
    /*  Start Keyword Spotting, return value rules:
            if entire value is interprited, based on 32bit int, as -1, then no command is recognized
            else
                lowest 8 bits are recognized command id
                then 8 bits are recognized language id
    */
int uiEndPointDetection(short *pMicBuf1, short *pMicBuf2, short *pDLBuf); /*Judge If The Current Frame Is Endpoint, i.e End Of Speech Segment, (pMicBuf1: main uplink recorded voice, pMicBuf2: reference uplink recorded voice, pDLBuf, downlink recorded voice)*/
/*-------------------------------------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif

