#ifndef __DDP_COLOR_H__
#define __DDP_COLOR_H__

#include "ddp_drv.h"


#define CFG_MAIN          (DISPSYS_COLOR_BASE+0x400)
#define G_PIC_ADJ_MAIN_2  (DISPSYS_COLOR_BASE+0x438)
#define DISP_COLOR_CK_ON  (DISPSYS_COLOR_BASE+0xF28)

#define LOCAL_HUE_CD_0 (DISPSYS_COLOR_BASE+0x620)

#define PARTIAL_SAT_GAIN1_0   (DISPSYS_COLOR_BASE+0x7FC)	
#define PARTIAL_SAT_GAIN1_1   (DISPSYS_COLOR_BASE+0x800)	
#define PARTIAL_SAT_GAIN1_2   (DISPSYS_COLOR_BASE+0x804)	
#define PARTIAL_SAT_GAIN1_3   (DISPSYS_COLOR_BASE+0x808)	
#define PARTIAL_SAT_GAIN1_4   (DISPSYS_COLOR_BASE+0x80C)	
#define PARTIAL_SAT_GAIN2_0   (DISPSYS_COLOR_BASE+0x810)	
#define PARTIAL_SAT_GAIN2_1   (DISPSYS_COLOR_BASE+0x814)	
#define PARTIAL_SAT_GAIN2_2   (DISPSYS_COLOR_BASE+0x818)	
#define PARTIAL_SAT_GAIN2_3	  (DISPSYS_COLOR_BASE+0x81C)	
#define PARTIAL_SAT_GAIN2_4   (DISPSYS_COLOR_BASE+0x820)	
#define PARTIAL_SAT_GAIN3_0   (DISPSYS_COLOR_BASE+0x824)	
#define PARTIAL_SAT_GAIN3_1   (DISPSYS_COLOR_BASE+0x828)	
#define PARTIAL_SAT_GAIN3_2   (DISPSYS_COLOR_BASE+0x82C)	
#define PARTIAL_SAT_GAIN3_3   (DISPSYS_COLOR_BASE+0x830)	
#define PARTIAL_SAT_GAIN3_4   (DISPSYS_COLOR_BASE+0x834)	
#define PARTIAL_SAT_POINT1_0  (DISPSYS_COLOR_BASE+0x838)	
#define PARTIAL_SAT_POINT1_1  (DISPSYS_COLOR_BASE+0x83C)	
#define PARTIAL_SAT_POINT1_2  (DISPSYS_COLOR_BASE+0x840)	
#define PARTIAL_SAT_POINT1_3  (DISPSYS_COLOR_BASE+0x844)	
#define PARTIAL_SAT_POINT1_4  (DISPSYS_COLOR_BASE+0x848)	
#define PARTIAL_SAT_POINT2_0  (DISPSYS_COLOR_BASE+0x84C)	
#define PARTIAL_SAT_POINT2_1  (DISPSYS_COLOR_BASE+0x850)	
#define PARTIAL_SAT_POINT2_2  (DISPSYS_COLOR_BASE+0x854)	
#define PARTIAL_SAT_POINT2_3  (DISPSYS_COLOR_BASE+0x858)	
#define PARTIAL_SAT_POINT2_4  (DISPSYS_COLOR_BASE+0x85C)	





//---------------------------------------------------------------------------
#define GAMMA_SIZE 1024

#define PURP_TONE    0
#define SKIN_TONE    1
#define GRASS_TONE   2
#define SKY_TONE     3

#define PURP_TONE_START    0 
#define PURP_TONE_END      2
#define SKIN_TONE_START    3
#define SKIN_TONE_END     10
#define GRASS_TONE_START  11
#define GRASS_TONE_END    16
#define SKY_TONE_START    17
#define SKY_TONE_END      19

#define SG1 0
#define SG2 1
#define SG3 2
#define SP1 3
#define SP2 4



//---------------------------------------------------------------------------
void DpEngine_COLORonConfig(unsigned int srcWidth,unsigned int srcHeight);
void DpEngine_COLORonInit(void);
void DpEngine_COLORonReset(void);
void DpEngine_COLORonEnableIrq(unsigned int value);


//IOCTL , for AAL service to wait vsync and get latest histogram
void disp_set_hist_readlock(unsigned long bLock);

DISP_AAL_STATISTICS * disp_get_hist_ptr(void);


int disp_get_hist(unsigned int * pHist);

//Called by interrupt to refresh histogram
void disp_update_hist(void);

DISP_PQ_PARAM * get_Color_config(void);
DISP_PQ_PARAM * get_Color_Cam_config(void);
DISP_PQ_PARAM * get_Color_Gal_config(void);
DISPLAY_PQ_T * get_Color_index(void);

//Called by tasklet to config registers
void disp_onConfig_luma(unsigned long *luma);

void disp_color_set_window(unsigned int sat_upper, unsigned int sat_lower, 
			   unsigned int hue_upper, unsigned int hue_lower);

#endif

