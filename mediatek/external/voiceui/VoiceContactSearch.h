#ifndef _VOICECONTACTSEARCH_H
#define _VOICECONTACTSEARCH_H


#ifdef __cplusplus
extern "C" {
#endif

typedef struct __VoiceContactSearchCustomParameters
{
    unsigned char Cust_Para[8][32];

    short  *voiceFIRCoefMic1;
    short  *voiceFIRCoefMic2;
    int  voiceGainMic1;
    int  voiceGainMic2;
    int  micNumFlag;

} VOICE_CONTACT_SEARCH_CUSTOM_INFO;

/*APIs ---------------------------------------------------------------------------------------------*/

    /*return values (i.e., flags) ----------------------------------------------------------------------*/
    enum {
           vcsExecuteError = -1, /*function ends if something goes wrong*/
           vcsExecuteCorrect,    /*function ends if nothing goes wrong*/
           vcsReturnAvailable    /*function ends if nothing goes wrong and returning values are available*/
         };
        /*
          NOTE: for the functions that have returning parameters, whose name starting with "rtn",
          their values are available if and only if the funtions return vcsReturnAvailable
        */

int vcsSetDirs(const char* infoHandlingDir, const char* pronunciationModelDir);
    /*
       - set all required working dirs;
         invoke before first time invoke vcsSetContactNames, vcsDetectEndpoint, and vcsRecognize
       infoHandlingDir: the dir that SWIP can put dbs for current contacts;
                        the dir is required to have authority of read and write
       pronunciationModelDir: the dir includes the input pronunciation models
    */

int vcsInit(VOICE_CONTACT_SEARCH_CUSTOM_INFO* voiceCustomInfo);
    /*
       - initialize the voice contact search recognition;
         symmetry function of vcsRelease()
       voiceCustomInfo: costomization parameters of this feature
    */

int vcsSetContactNames(const char** names, const int size);
    /*
       - set the contact name domain of searching recognition; invoke when contact names change
       names: an array of char arrays; each char array is a contact name, which ends by '\0';
              the order of names should be in alphabetical order (?)
       size: the number of char array in names
    */

int vcsDetectEndpoint(const short *pMicBuf1, const short *pMicBuf2, const short *pDlBuf,
                      char* rtnDetected);
    /*
       - detect whether a speech ends; invoke for each speech frame
       pMicBuf1: uplink mic1 audio data
       pMicBuf2: uplink mic2 audio data
       pDlBuf: downlink audio data
       rtnDetected: a char pointer for receving endpoint results;
                    *rtnDetected is non-zero if endpoint detected
    */

int vcsRecognize(const int demandContactNum,
                 char** rtnNameArrayPtr, int* rtnArraySizePtr);
    /*
       - compute which names are similar to current speech;
         invoke only after vcsDetectEndpoint gives non-zero *rtnDetected
       demandContactNum: the number of contacts demanded by APP, i.e.,
                         a desired contact number presenting to users
       rtnNameArrayPtr : contact name array pointer for receiving results;
                         names are in the order of recognition score;
                         each name ends with '\0'
       rtnArraySizePtr : contact name array size pointer for receiving results;
                         value is the size of rtnNameArrayPtr;
                         " *rtnArraySizePtr <= demandContactNum " is always true
    */


int vcsNotifyContactSelected(const char* name);
    /*
       - signal SWIP some contact is selected by user to learn user's choice
       name: the select contact name
    */

int vcsNotifyCancelRecognition();
    /*
       - signal SWIP no contact is selected by user
    */

int vcsSetScreenOrientation(int orientation);
    /*
       - set screen orientation
       orientation: 0=undefined, 1=portrait, 2=landscape
    */

int vcsRelease();
    /*
       - release the voice contact search functionality; all memory data are cleaned;
         symmetry function of vcsInit()
    */

/*misc. --------------------------------------------------------------------------------------------*/
float vcsGetVersion();
    /*
       - return current version number of this feature
    */


#ifdef __cplusplus
}
#endif

#endif // _VOICECONTACTSEARCH_H
