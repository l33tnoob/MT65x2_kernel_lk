#ifndef _FLICKERDETECTION_H_
#define _FLICKERDETECTION_H_

//typedef unsigned char   MUINT8;
//typedef unsigned short  MUINT16;
//typedef unsigned int    MUINT32;
//typedef unsigned long long  MUINT64;
//typedef signed char     MINT8;
//typedef signed short    MINT16;
//typedef signed int      MINT32;
//typedef signed long long  MINT64;
//typedef void            MVOID;
//typedef int             MBOOL;
//#ifndef MTRUE
//    #define MTRUE       1
//#endif
//#ifndef MFALSE
//    #define MFALSE      0
//#endif

//typedef float				MFLOAT;
typedef struct
{
MINT32 flickerFreq[9];
MINT32 flickerGradThreshold;
MINT32 flickerSearchRange;
MINT32 minPastFrames;
MINT32 maxPastFrames;
FLICKER_STATISTICS EV50_L50;
FLICKER_STATISTICS EV50_L60;
FLICKER_STATISTICS EV60_L50;
FLICKER_STATISTICS EV60_L60;
MINT32 EV50_thresholds[2];
MINT32 EV60_thresholds[2];
MINT32 freq_feature_index[2];


}FLICKER_EXT_PARA;



typedef enum LMV_STATUS_t {LARGE_MOTION, SMALL_MOTION} LMV_STATUS;// status from local motion vectors: large inter-frame motion & small inter-frame motion
MVOID setThreshold(const MINT32 threc[2], const MINT32 threa[2], const MINT32 thref[3]);
MVOID setLMVcnt(MUINT32 u4LMVNum);
FLICKER_STATUS detectFlicker_SW(const MINT32 *C_list0, const MINT32 *C_list1, const MINT32 n_win_h,  const MINT32 n_win_w, const MINT32 win_wd, const MINT32 win_ht,
								const MINT32 cur_freq, const flkSensorInfo sensor_info, flkEISVector eis_vec, const flkAEInfo ae_info,  MINT32 *AF_stat);

MVOID setFlkPause(MINT32 bOnOff);
MINT32 getFlkPause();

void flicker_setTable(MINT32* tab1, MINT32* tab2, int tabDim1);
void flicker_init(int line, int column_length, int win_width, double read_freq);
void flicker_uninit();
void flicker_setExtPara(FLICKER_EXT_PARA* para);

#endif