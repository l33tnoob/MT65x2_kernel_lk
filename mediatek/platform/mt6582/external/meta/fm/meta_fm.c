/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   meta_fm.h
 *
 * Project:
 * --------
 *   YUSU
 *
 * Description:
 * ------------
 *   FM meta implement.
 *
 * Author:
 * -------
 *  LiChunhui (MTK80143)
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 * 12 11 2012 run.liu
 * [ALPS00405328] [FM]MR1 BIONIC_FORTIFY_INLINE build error
 * .
 *
 * 10 31 2012 run.liu
 * [ALPS00381117] [FM][Meta mode][6589]enable FM meta mode on 6589+6628 project
 * .
 *
 * 10 29 2012 run.liu
 * [ALPS00381117] [FM][Meta mode][6589]enable FM meta mode on 6589+6628 project
 * .
 * mask audio API
 *
 * 01 26 2011 hongcheng.xia
 * [ALPS00030208] [Need Patch] [Volunteer Patch][MT6620 FM]enable FM Meta mode
 * .
 *
 * 11 18 2010 hongcheng.xia
 * [ALPS00135614] [Need Patch] [Volunteer Patch]MT6620 FM Radio code check in
 * .
 *
 * 11 16 2010 hongcheng.xia
 * [ALPS00135614] [Need Patch] [Volunteer Patch]MT6620 FM Radio code check in
 * .
 *
 * 11 15 2010 hongcheng.xia
 * [ALPS00135614] [Need Patch] [Volunteer Patch]MT6620 FM Radio code check in
 * .
 *
 * 11 15 2010 hongcheng.xia
 * [ALPS00135614] [Need Patch] [Volunteer Patch]MT6620 FM Radio code check in
 * .
 *
 * 08 28 2010 chunhui.li
 * [ALPS00123709] [Bluetooth] meta mode check in
 * for FM meta enable

 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "FT_Public.h"
//#include "type.h"
#include "meta_common.h"
#include "WM2Linux.h"
#include "meta_fm_para.h"
#include "../Audio/meta_audio_para.h" 
#define FM_META_AUDIOBIST
#define  FM_META_DEBUG     1
#define  FM_RDS_ENABLE     1 // 1: enable RDS, 0:disable RDS
#define FM_AUDIO_SUPPORT 1//audio maybe not support meta mode
#define  FM_AUDIO_TX 
#undef   LOG_TAG
#define  LOG_TAG  "FM_META"

#define  ERR(f, ...)  LOGE("%s: " f, __func__, ##__VA_ARGS__)
#define  WAN(f, ...)  LOGW("%s: " f, __func__, ##__VA_ARGS__)
#if FM_META_DEBUG
#define DBG(f, ...)   LOGD("%s: " f, __func__, ##__VA_ARGS__)
#define TRC(f)        LOGW("%s #%d", __func__, __LINE__)
#define CONSOLE_PRINT(f, ...) printf(LOG_TAG " %s: " f, __func__, ##__VA_ARGS__)
#else
#define DBG(...)      ((void)0)
#define TRC(f)        ((void)0)
#define CONSOLE_PRINT(...)   ((void)0)
#endif

#define FM_META_BAND FM_BAND_JAPANW

#ifdef MTK_FM_50KHZ_SUPPORT

#if (FM_META_BAND == FM_BAND_UE)      /* US/Europe band  87.5MHz ~ 108MHz*/
#define FM_META_FREQ_MIN  8750
#define FM_META_FREQ_MAX  10800
#elif (FM_META_BAND == FM_BAND_JAPAN) /*Japan band      76MHz   ~ 90MH*/
#define FM_META_FREQ_MIN  7600
#define FM_META_FREQ_MAX  9000
#elif (FM_META_BAND == FM_BAND_JAPANW)/* Japan wideband  76MHZ   ~ 108MHz*/
#define FM_META_FREQ_MIN  7600
#define FM_META_FREQ_MAX  10800
#else                                 /*default Band*/
#define FM_META_FREQ_MIN  7600
#define FM_META_FREQ_MAX  10800
#endif  // FM_META_BAND

#else 

#if (FM_META_BAND == FM_BAND_UE)      /* US/Europe band  87.5MHz ~ 108MHz*/
#define FM_META_FREQ_MIN  875
#define FM_META_FREQ_MAX  1080
#elif (FM_META_BAND == FM_BAND_JAPAN) /*Japan band      76MHz   ~ 90MH*/
#define FM_META_FREQ_MIN  760
#define FM_META_FREQ_MAX  900
#elif (FM_META_BAND == FM_BAND_JAPANW)/* Japan wideband  76MHZ   ~ 108MHz*/
#define FM_META_FREQ_MIN  760
#define FM_META_FREQ_MAX  1080
#else                                 /*default Band*/
#define FM_META_FREQ_MIN  760
#define FM_META_FREQ_MAX  1080
#endif  // FM_META_BAND

#endif //MTK_FM_50KHZ_SUPPORT

static int g_fm_fd = -1;
static FM_CNF fm_cnf;
static FM_CNF fm_rds_cnf;
static fm_status gStatus;

extern BOOL WriteDataToPC(void *Local_buf,unsigned short Local_len,void *Peer_buf,unsigned short Peer_len);

#if FM_RDS_ENABLE
static int g_freq = 0;
static int g_kill_rds_thd = 0;
pthread_t fm_rds_thd;
static void *fm_rds_thread(void *priv);


/********************************************************************************
//FUNCTION:
//		fm_rds_thread
//DESCRIPTION:
//		FM RDS data get and parse thread.
//
//PARAMETERS:
//		void
//RETURN VALUE:
//		void
//
********************************************************************************/
static void *fm_rds_thread(void *priv)
{
    FM_CNF *pFM_CNF = (FM_CNF *)priv;
	FM_RDS_CNF_T RDS_Cnf;
    RDSData_Struct RDS_Struct;
    int cnt = 0;
    uint16_t flag_mask;
    int indx = 0;
    uint16_t event_mask;
    int i = 0;
	uint16_t set_freq, sw_freq, org_freq, PAMD_Value, AF_PAMD_LBound, AF_PAMD_HBound, TA_PAMD_Threshold;
	uint16_t PAMD_Level[25];
	uint16_t PAMD_DB_TBL[5] = {/*5dB, 10dB, 15dB, 20dB, 25dB*/ 13, 17, 21, 25, 29};
    struct fm_tune_parm parm;
	uint16_t rds_on = 1;
	uint16_t _backup_frequency = 0;

	DBG("fm_rds_thread create\n");

    memset(pFM_CNF, 0, sizeof(FM_CNF));	
    pFM_CNF->header.id = FT_FM_CNF_ID;
    pFM_CNF->header.token = 0;
    pFM_CNF->op = FM_OP_SET_RDS;
    
    while(!g_kill_rds_thd){
        if((gStatus.state != FM_ON_RX) || (gStatus.rds_t != FM_RDS_RX_ON)){
                //LOGI("FM RX is OFF, waitting for RX on\n");
                sleep(3);
                continue;
        }
        DBG("fm_rds_thread wait RDS data\n");
        if(read(g_fm_fd, &RDS_Struct, sizeof(RDSData_Struct)) == sizeof(RDSData_Struct)){
            DBG("RDS_Struct.event_status:%x\n", RDS_Struct.event_status);
            
            if(g_kill_rds_thd)
                break;

            //RDS Event max (actually 15)
            for(indx = 0; indx < 16; indx++){
                event_mask = 1<<indx;				
                if(RDS_Struct.event_status&event_mask){ 
                    memset(&RDS_Cnf, 0x0, sizeof(FM_RDS_CNF_T));
                    switch (event_mask){
                        case RDS_EVENT_FLAGS:{
							RDS_Cnf.eventtype = RDS_EVENT_FLAGS;	
							memcpy(RDS_Cnf.m_rRDS.m_rRDSStatus.m_buffer, &RDS_Struct.RDSFlag, sizeof(RDS_Struct.RDSFlag));
                            //RDS Flag max 8
                            for(cnt = 0; cnt < 8; cnt++){
                                flag_mask = 1<<cnt;
                                if((RDS_Struct.RDSFlag.flag_status)&flag_mask){
                                    switch(flag_mask){
				                        case RDS_FLAG_IS_TP:  //RDS_FLAG_IS_TP  = 0x0001
											RDS_Cnf.m_rRDS.m_rRDSStatus.m_eFlag |= RDS_FLAG_IS_TP;                                         
											DBG("RDS Event:RDS_FLAG_IS_TP:0x%02x\n", RDS_Struct.RDSFlag.TP);
					                        break;
				                        case RDS_FLAG_IS_TA:
											RDS_Cnf.m_rRDS.m_rRDSStatus.m_eFlag |= RDS_FLAG_IS_TA; 											
				                        	DBG("RDS Event:RDS_FLAG_IS_TA:0x%02x\n", RDS_Struct.RDSFlag.TA);											
				                        	break;
				                        case RDS_FLAG_IS_MUSIC:
											RDS_Cnf.m_rRDS.m_rRDSStatus.m_eFlag |= RDS_FLAG_IS_MUSIC;
				                        	DBG("RDS Event:RDS_FLAG_IS_MUSIC:0x%02x\n", RDS_Struct.RDSFlag.Music);
				                        	break;
				                        case RDS_FLAG_IS_STEREO:
											RDS_Cnf.m_rRDS.m_rRDSStatus.m_eFlag |= RDS_FLAG_IS_STEREO;
				                        	DBG("RDS Event:RDS_FLAG_IS_STEREO:0x%02x\n", RDS_Struct.RDSFlag.Stereo);					
				                        	break;
				                        case RDS_FLAG_IS_ARTIFICIAL_HEAD:
											RDS_Cnf.m_rRDS.m_rRDSStatus.m_eFlag |= RDS_FLAG_IS_ARTIFICIAL_HEAD;
				                        	DBG("RDS Event:RDS_FLAG_IS_ARTIFICIAL_HEAD:0x%02x\n", RDS_Struct.RDSFlag.Artificial_Head);
				                        	break;
				                        case RDS_FLAG_IS_COMPRESSED:
											RDS_Cnf.m_rRDS.m_rRDSStatus.m_eFlag |= RDS_FLAG_IS_COMPRESSED;
				                        	DBG("RDS Event:RDS_FLAG_IS_COMPRESSED:0x%02x\n", RDS_Struct.RDSFlag.Compressed);					
				                        	break;
				                        case RDS_FLAG_IS_DYNAMIC_PTY:
											RDS_Cnf.m_rRDS.m_rRDSStatus.m_eFlag |= RDS_FLAG_IS_DYNAMIC_PTY;
				                        	DBG("RDS Event:RDS_FLAG_IS_DYNAMIC_PTY:0x%02x\n", RDS_Struct.RDSFlag.Dynamic_PTY);		
				                        	break;
				                        case RDS_FLAG_TEXT_AB:
											RDS_Cnf.m_rRDS.m_rRDSStatus.m_eFlag |= RDS_FLAG_TEXT_AB;
				                        	DBG("RDS Event:RDS_FLAG_IS_TEXT_AB:0x%02x\n", RDS_Struct.RDSFlag.Text_AB);				
				                        	break;
				                        default:
				                            DBG("RDS Event:not valide\n");	
					                        break;					
			                        }
                                } 
                            }
							WriteDataToPC(pFM_CNF, sizeof(FM_CNF), &RDS_Cnf, sizeof(FM_RDS_CNF_T));
						}
                        break;
                        case RDS_EVENT_PI_CODE:
							RDS_Cnf.eventtype = RDS_EVENT_PI_CODE;
							memcpy(RDS_Cnf.m_rRDS.m_rRDSInfo.m_buffer, &RDS_Struct.PI, sizeof(RDS_Struct.PI));
							WriteDataToPC(pFM_CNF, sizeof(FM_CNF), &RDS_Cnf, sizeof(FM_RDS_CNF_T));
							DBG("RDS_EVENT_PI_CODE:0x%04x\n", RDS_Struct.PI);	
                            break;   
                        case RDS_EVENT_PTY_CODE:
							RDS_Cnf.eventtype = RDS_EVENT_PTY_CODE;
							memcpy(RDS_Cnf.m_rRDS.m_rRDSInfo.m_buffer, &RDS_Struct.PTY, sizeof(RDS_Struct.PTY));
							WriteDataToPC(pFM_CNF, sizeof(FM_CNF), &RDS_Cnf, sizeof(FM_RDS_CNF_T));
                            DBG("RDS_EVENT_PTY_CODE:0x%04x\n", RDS_Struct.PTY);	
                            break; 
                        case RDS_EVENT_PROGRAMNAME: 
							RDS_Cnf.eventtype = RDS_EVENT_PROGRAMNAME;
#ifdef MT6620_FM
							memcpy(RDS_Cnf.m_rRDS.m_rRDSInfo.m_buffer, &RDS_Struct.PS_Data.PS[2][0], sizeof(RDS_Struct.PS_Data.PS[2]));
							WriteDataToPC(pFM_CNF, sizeof(FM_CNF), &RDS_Cnf, sizeof(FM_RDS_CNF_T));
                            DBG("RDS_EVENT_PROGRAMNAME:%s\n", &(RDS_Struct.PS_Data.PS[2][0]));
#else
                        memcpy(RDS_Cnf.m_rRDS.m_rRDSInfo.m_buffer, &RDS_Struct.PS_Data.PS[3][0], sizeof(RDS_Struct.PS_Data.PS[3]));
                        WriteDataToPC(pFM_CNF, sizeof(FM_CNF), &RDS_Cnf, sizeof(FM_RDS_CNF_T));
                        DBG("RDS_EVENT_PROGRAMNAME:%s\n", &(RDS_Struct.PS_Data.PS[3][0]));
#endif
                            break;  
                        case RDS_EVENT_UTCDATETIME:
							RDS_Cnf.eventtype = RDS_EVENT_UTCDATETIME;
							memcpy(RDS_Cnf.m_rRDS.m_rRDSInfo.m_buffer, &RDS_Struct.CT, sizeof(RDS_Struct.CT));
							WriteDataToPC(pFM_CNF, sizeof(FM_CNF), &RDS_Cnf, sizeof(FM_RDS_CNF_T));
							DBG("RDS_EVENT_UTCDATETIME y:%d, mo:%d, d:%d, H:%d, mi:%d\n",RDS_Struct.CT.Year, RDS_Struct.CT.Month, RDS_Struct.CT.Day, RDS_Struct.CT.Hour, RDS_Struct.CT.Minute);
                            DBG("RDS_EVENT_UTCDATETIME Local_Time_offset_signbit:%d, Local_Time_offset_half_hour:%d\n",RDS_Struct.CT.Local_Time_offset_signbit, RDS_Struct.CT.Local_Time_offset_half_hour);
			                break;
                        case RDS_EVENT_LAST_RADIOTEXT:
					   	    RDS_Cnf.eventtype = RDS_EVENT_LAST_RADIOTEXT;
					   	    memcpy(RDS_Cnf.m_rRDS.m_rRDSInfo.m_buffer, &RDS_Struct.RT_Data.TextData[3][0], sizeof(RDS_Struct.RT_Data.TextData[3]));
                            WriteDataToPC(pFM_CNF, sizeof(FM_CNF), &RDS_Cnf, sizeof(FM_RDS_CNF_T));
                            RDS_Struct.RT_Data.TextData[3][63] = '\0';
                            DBG("RDS_EVENT_LAST_RADIOTEXT:%s, len=%d\n", &RDS_Struct.RT_Data.TextData[3][0], (int)RDS_Struct.RT_Data.TextLength);
			                break;
                        case RDS_EVENT_AF:{ 
					   	    RDS_Cnf.eventtype = RDS_EVENT_AF;
			                DBG("RDS_EVENT_AF\n"); //need udpate frequency			               
                            AF_PAMD_LBound = PAMD_DB_TBL[0]; //5dB
			                AF_PAMD_HBound = PAMD_DB_TBL[2]; //15dB
			                ioctl(g_fm_fd, FM_IOCTL_GETCURPAMD, &PAMD_Value);
							DBG("AF current, %d:%d\n", g_freq, PAMD_Value); 
			                sw_freq = g_freq; //current freq
			                org_freq = g_freq;
							parm.band = FM_META_BAND;
                            parm.freq = sw_freq;
                            parm.hilo = FM_AUTO_HILO_OFF;
                        parm.space = FM_SPACE_DEFAULT;
							
			                if(PAMD_Value < AF_PAMD_LBound){
			                    rds_on = 0;
							    ioctl(g_fm_fd, FM_IOCTL_RDS_ONOFF, &rds_on);
                                RDS_Struct.AF_Data.AF_Num = (RDS_Struct.AF_Data.AF_Num > 25)? 25 : RDS_Struct.AF_Data.AF_Num;
			                    for(i=0; i<RDS_Struct.AF_Data.AF_Num; i++){
			                        set_freq = RDS_Struct.AF_Data.AF[1][i];  //method A or B
			                        if(set_freq != org_freq){
       				                    parm.freq = sw_freq;
										ioctl(g_fm_fd, FM_IOCTL_TUNE, &parm);
										ioctl(g_fm_fd, FM_IOCTL_GETCURPAMD, &PAMD_Level[i]);
					                    if(PAMD_Level[i] > PAMD_Value){
						                     PAMD_Value = PAMD_Level[indx];
							                 sw_freq = set_freq;
					                    }
			                        }
			                    }
                                if((PAMD_Value > AF_PAMD_HBound)&&(sw_freq != 0)){
									parm.freq = sw_freq;
									ioctl(g_fm_fd, FM_IOCTL_TUNE, &parm);
									g_freq = parm.freq;
									DBG("AF set sw, %d:%d\n", g_freq, PAMD_Value);
								}else{				
                 					parm.freq = org_freq;
									ioctl(g_fm_fd, FM_IOCTL_TUNE, &parm);
									g_freq = parm.freq;
									DBG("AF set org, %d:%d\n", g_freq, PAMD_Value);
				                }
								rds_on = 1;							  
							    ioctl(g_fm_fd, FM_IOCTL_RDS_ONOFF, &rds_on);
							}else{ 
                                DBG("RDS_EVENT_AF old freq:%d\n", org_freq);
							}
							memcpy(RDS_Cnf.m_rRDS.m_rRDSInfo.m_buffer, &RDS_Struct.AF_Data.AF[0], sizeof(RDS_Struct.AF_Data.AF[0]));
                            WriteDataToPC(pFM_CNF, sizeof(FM_CNF), &RDS_Cnf, sizeof(FM_RDS_CNF_T));
                        }
                            break;
                        case RDS_EVENT_AF_LIST:
					        RDS_Cnf.eventtype = RDS_EVENT_AF_LIST;
                            RDS_Struct.AF_Data.AF_Num = (RDS_Struct.AF_Data.AF_Num > 25)? 25 : RDS_Struct.AF_Data.AF_Num;
							RDS_Cnf.m_rRDS.m_rRDSInfo.m_buffer[0] = (uint8_t)RDS_Struct.AF_Data.AF_Num;
							memcpy(&RDS_Cnf.m_rRDS.m_rRDSInfo.m_buffer[1], &RDS_Struct.AF_Data.AF[1][0], sizeof(uint16_t)*RDS_Struct.AF_Data.AF_Num);
                            WriteDataToPC(pFM_CNF, sizeof(FM_CNF), &RDS_Cnf, sizeof(FM_RDS_CNF_T));
                            DBG("RDS_EVENT_AF_LIST, NUM=%d, List:", RDS_Cnf.m_rRDS.m_rRDSInfo.m_buffer[0]);
			                for(i = 0; i < RDS_Struct.AF_Data.AF_Num; i++){
			                    LOGD("%d \n", RDS_Struct.AF_Data.AF[1][i]);   
			                }            
			                break;
                        case RDS_EVENT_AFON_LIST: //Not need show actually
			                RDS_Cnf.eventtype = RDS_EVENT_AFON_LIST;
                            RDS_Struct.AF_Data.AF_Num = (RDS_Struct.AF_Data.AF_Num > 25)? 25 : RDS_Struct.AF_Data.AF_Num;
							RDS_Cnf.m_rRDS.m_rRDSInfo.m_buffer[0] = RDS_Struct.AFON_Data.AF_Num;
							memcpy(&RDS_Cnf.m_rRDS.m_rRDSInfo.m_buffer[1], &RDS_Struct.AFON_Data.AF[1][0], sizeof(uint16_t)*RDS_Struct.AFON_Data.AF_Num);
                            WriteDataToPC(pFM_CNF, sizeof(FM_CNF), &RDS_Cnf, sizeof(FM_RDS_CNF_T)); 
							DBG("RDS_EVENT_AFON_LIST:");
			                for(i = 0; i < RDS_Struct.AFON_Data.AF_Num; i++){
			                    DBG("%d \n", RDS_Struct.AFON_Data.AF[1][i]);   
			                }			                
			                break; 
                        //need udpate frequency
                        case RDS_EVENT_TAON:{
					   	    RDS_Cnf.eventtype = RDS_EVENT_TAON;
			                DBG("RDS_EVENT_TAON\n");							
							rds_on = 0;
							ioctl(g_fm_fd, FM_IOCTL_RDS_ONOFF, &rds_on);
							TA_PAMD_Threshold = PAMD_DB_TBL[2]; //15dB
							sw_freq = g_freq;
							org_freq = g_freq;
							_backup_frequency = org_freq;
							parm.band = FM_META_BAND;
                            parm.freq = sw_freq;
                            parm.hilo = FM_AUTO_HILO_OFF;
                        parm.space = FM_SPACE_DEFAULT;
							
							for(i=0; i< RDS_Struct.AFON_Data.AF_Num; i++){
							    set_freq = RDS_Struct.AF_Data.AF[1][i]; 
                                if(set_freq != org_freq){
                                    parm.freq = sw_freq;
								    ioctl(g_fm_fd, FM_IOCTL_TUNE, &parm);
									ioctl(g_fm_fd, FM_IOCTL_GETCURPAMD, &PAMD_Level[i]);
				                    if(PAMD_Level[indx] > PAMD_Value){
					                    PAMD_Value = PAMD_Level[indx];
						                sw_freq = set_freq;
				                    }
                                }
							}

						    if((PAMD_Value > TA_PAMD_Threshold)&&(sw_freq != 0)){
				                RDS_Struct.Switch_TP= 1;
								parm.freq = sw_freq;
								ioctl(g_fm_fd, FM_IOCTL_TUNE, &parm);
								g_freq = parm.freq;
			                }else{
								parm.freq = org_freq;
								ioctl(g_fm_fd, FM_IOCTL_TUNE, &parm);
								g_freq = parm.freq;
			                }
							rds_on = 1;
							ioctl(g_fm_fd, FM_IOCTL_RDS_ONOFF, &rds_on);
							memcpy(RDS_Cnf.m_rRDS.m_rRDSInfo.m_buffer, &g_fm_fd, sizeof(g_fm_fd));
							WriteDataToPC(pFM_CNF, sizeof(FM_CNF), &RDS_Cnf, sizeof(FM_RDS_CNF_T));
                        }
			                break;
                        case RDS_EVENT_TAON_OFF: //need backup frequency
			               RDS_Cnf.eventtype = RDS_EVENT_TAON;
			               DBG("+RDS_EVENT_TAON_OFF\n");	
						   rds_on = 0;
						   ioctl(g_fm_fd, FM_IOCTL_RDS_ONOFF, &rds_on);
						   RDS_Struct.Switch_TP= 0;							
						   parm.freq = _backup_frequency;
						   ioctl(g_fm_fd, FM_IOCTL_TUNE, &parm);
						   g_freq = parm.freq;
						   rds_on = 1;
						   ioctl(g_fm_fd, FM_IOCTL_RDS_ONOFF, &rds_on);						   
						   memcpy(RDS_Cnf.m_rRDS.m_rRDSInfo.m_buffer, &g_fm_fd, sizeof(g_fm_fd));
						   WriteDataToPC(pFM_CNF, sizeof(FM_CNF), &RDS_Cnf, sizeof(FM_RDS_CNF_T));
						   DBG("-RDS_EVENT_TAON_OFF\n");
						   break;  
			                
                        default: 
			                DBG("RDS_EVENT not valid\n");		                
			                break;
                    } //end switch event
                }
            }
				
        }
        else
        {
            DBG("fm_rds_thread read failed\n");
			break;
        }
    }
    DBG("fm_rds_thread return\n");
    pthread_exit(NULL);
    return NULL;       
}
#endif


/********************************************************************************
//FUNCTION:
//		META_FM_init
//DESCRIPTION:
//		FM Init for META test.
//
//PARAMETERS:
//		void
//RETURN VALUE:
//		true : success
//      false: failed
//
********************************************************************************/
bool META_FM_init()
{
    g_fm_fd = open(FM_DEVICE_NAME, O_RDWR);
    if (g_fm_fd < 0){
        ERR("META Open FM %s failed", FM_DEVICE_NAME);
        return false;
    }    
    TRC();
	DBG("META Open FM %s Success", FM_DEVICE_NAME);
    
#if FM_AUDIO_SUPPORT
    /*for audio path*/
	META_Audio_init(); 
#endif
	
#if  FM_RDS_ENABLE
    g_kill_rds_thd = 0;
    gStatus.rds_t = FM_RDS_OFF;
	DBG("FM RDS enable");
    pthread_create(&fm_rds_thd, NULL, fm_rds_thread, &fm_rds_cnf);
#endif

    gStatus.state = FM_OFF;
    gStatus.tx_tone = FMTX_1K_TONE;
    gStatus.audio_path = FM_TX_AUDIO_I2S;

	return true;
}

/********************************************************************************
//FUNCTION:
//		META_FM_deinit
//DESCRIPTION:
//		FM deinit for META test.
//
//PARAMETERS:
//		void
//RETURN VALUE:
//		void
//     
********************************************************************************/
void META_FM_deinit()
{
#if FM_RDS_ENABLE
   g_kill_rds_thd = 1;
   if(lseek(g_fm_fd, 0, SEEK_SET) != -1){
	   ERR("force read thread return\n");	  
   }
#endif

#if FM_AUDIO_SUPPORT
    META_Audio_deinit();
#endif    
    if(g_fm_fd != -1){
        close(g_fm_fd);
        g_fm_fd = -1;
    }	
	TRC();
		
	return;   
}

/********************************************************************************
//FUNCTION:
//		META_FM_OP
//DESCRIPTION:
//		META FM test main process function.
//
//PARAMETERS:
//		req: FM Req struct
//      peer_buff: peer buffer pointer
//      peer_len: peer buffer length
//RETURN VALUE:
//		void
//      
********************************************************************************/
void META_FM_OP(FM_REQ *req, char *peer_buff, unsigned short peer_len) 
{

	unsigned short pdu_length=0;
	unsigned char sendstatus=0;
	unsigned char *pdu_ptr=NULL;
	int     ret = 0;
	struct fm_em_parm parm_em;
    struct fm_tune_parm parm_tune;
    struct fm_ctl_parm parm_ctl;
    struct fm_seek_parm parm_seek;
    struct fm_scan_parm parm_scan;
    struct fm_rds_tx_parm param_rds_tx;
    int rssi = 0, vol;
    unsigned short chipid,  CH_Data[20], tmp_val, ch_offset, MASK_CH, monoorstereo;
    int LOFREQ = FM_META_FREQ_MIN;
    int step = 0, ch_cnt = 0;
    FM_AutoScan_CNF_T *pScanTBL = NULL;
    uint16_t PamdLevl = 0;
    int i;
    int type = -1;
    int space_mode;

	fm_u16 min_freq = 8750;
	fm_u16 max_freq = 10800;
	struct fm_softmute_tune_t cur_freq;
	fm_u8 seek_space = 10;
	int band_channel_no = (max_freq-min_freq)/seek_space + 1;

    if(g_fm_fd == -1){
        ERR("META_FM_OP invalid FM driver handle\n");
        return;
    }
    
	memset(&fm_cnf, 0, sizeof(FM_CNF));	
	fm_cnf.header.id = FT_FM_CNF_ID;
	fm_cnf.header.token = req->header.token;
	fm_cnf.op = req->op;
	
	DBG("req->op:%d\n", req->op); 

    switch(req->op){
        case FM_OP_READ_CHIP_ID:
	        ret = ioctl(g_fm_fd, FM_IOCTL_GETCHIPID, &chipid);
            if(ret){    
                ERR("FM_OP_READ_CHIP_ID failed\n");
				fm_cnf.drv_status = ret;
            }else{
				fm_cnf.drv_status = FM_SUCCESS;
            }
            
            switch(chipid){
                case 0x6616:
                fm_cnf.fm_result.m_rChipId.m_ucChipId = FM_CHIP_ID_MT6616;
                    break;
                case 0x6620:
                fm_cnf.fm_result.m_rChipId.m_ucChipId = FM_CHIP_ID_MT6620;
                    break;
                case 0x6626:
                    fm_cnf.fm_result.m_rChipId.m_ucChipId = FM_CHIP_ID_MT6626;
                    break;
                case 0x6628:
                    fm_cnf.fm_result.m_rChipId.m_ucChipId = FM_CHIP_ID_MT6628;
                    break;
				case 0x6627:
		    		fm_cnf.fm_result.m_rChipId.m_ucChipId = FM_CHIP_ID_MT6627;
		    		break;
                case 0x1000:
                fm_cnf.fm_result.m_rChipId.m_ucChipId = FM_CHIP_ID_AR1000;       
                    break;
                default:
                	fm_cnf.fm_result.m_rChipId.m_ucChipId = FM_CHIP_ID_AR1000;       
                    break;
            }
			DBG("FM chip:%x\n", chipid);            
            fm_cnf.status = META_SUCCESS;
            WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			break;
			
        case FM_OP_GET_STEP_MODE:
            space_mode = FM_SPACE_DEFAULT;

            if (space_mode == FM_SPACE_50K) {
                fm_cnf.fm_result.m_rStep.m_i4Step = 0;
            } else if (space_mode == FM_SPACE_100K) {
                fm_cnf.fm_result.m_rStep.m_i4Step = 1;
            } else if (space_mode == FM_SPACE_200K) {
                fm_cnf.fm_result.m_rStep.m_i4Step = 2;
            } else {
                fm_cnf.fm_result.m_rStep.m_i4Step = 1;
            }

            DBG("FM StepMode:%d\n", fm_cnf.fm_result.m_rStep.m_i4Step);
            fm_cnf.status = META_SUCCESS;
            WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
            break;
            
		case FM_OP_POWER_ON:
            if(gStatus.state != FM_OFF){
                LOGE("Error status.\n");
                fm_cnf.status = META_FAILED;
                WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
                break;
            }
		    bzero(&parm_tune, sizeof(struct fm_tune_parm));
            parm_tune.band = FM_META_BAND;
            parm_tune.freq = FM_META_FREQ_MAX; //default value for FM power up.
            parm_tune.hilo = FM_AUTO_HILO_OFF;
            parm_tune.space = FM_SPACE_DEFAULT;
            
           // DBG("FM_OP_POWER_ON 1.\n");
            ret = ioctl(g_fm_fd, FM_IOCTL_POWERUP, &parm_tune);
            if(ret){    
                ERR("FM_OP_POWER_ON failed\n");
				fm_cnf.drv_status = errno;
            }else{
				fm_cnf.drv_status = -parm_tune.err;
				if (fm_cnf.drv_status == FM_SUCCESS)
				{
					DBG("FM_OP_POWER_ON ok.\n");
                    gStatus.state = FM_ON_RX;
#if FM_AUDIO_SUPPORT/*alps00671815: audio will assert if open I2S before fm power on*/
#ifdef FM_ANALOG_INPUT
                    FMLoopbackTest(true);
                    gStatus.audio_path = FM_RX_AUDIO_ANALOG;
#else
                    Audio_I2S_Play(true); 
                    gStatus.audio_path = FM_RX_AUDIO_I2S;
#endif 		    
#endif			
				}
				else
				{
					ERR("FM_OP_POWER_ON failed.\n");
				}
            }            
         
			fm_cnf.status = META_SUCCESS;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			break;
		    
		case FM_OP_POWER_ON_TX:
		    if(gStatus.state != FM_OFF){
                LOGE("Error status.\n");
                fm_cnf.status = META_FAILED;
                WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
                break;
            }        
		    bzero(&parm_tune, sizeof(struct fm_tune_parm));
            parm_tune.band = FM_META_BAND;
            parm_tune.freq = FM_META_FREQ_MAX; //default value for FM power up.
            parm_tune.hilo = FM_AUTO_HILO_OFF;
            parm_tune.space = FM_SPACE_DEFAULT;
            
            ret = ioctl(g_fm_fd, FM_IOCTL_POWERUP_TX, &parm_tune);
            if(ret){    
                ERR("FM_OP_POWER_ON_TX failed\n");
                fm_cnf.status = META_FAILED;
                WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
                break;
            }  
            DBG("FM_OP_POWER_ON_TX ok\n");  
            gStatus.audio_path = FM_TX_AUDIO_I2S;
#if FM_AUDIO_SUPPORT
            #ifdef FM_AUDIO_TX
            gStatus.tx_tone = FMTX_1K_TONE;
		    Audio_FMTX_Play(true, gStatus.tx_tone);
            #endif   
#endif			
            gStatus.state = FM_ON_TX;
            fm_cnf.drv_status = FM_SUCCESS;
			fm_cnf.status = META_SUCCESS;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			break;
		    
		case FM_OP_POWER_OFF:
            if(gStatus.state == FM_ON_TX){
#if FM_AUDIO_SUPPORT
                #ifdef FM_AUDIO_TX
                Audio_FMTX_Play(false, gStatus.tx_tone);
                #endif
#endif				
                type = FM_TX;
                LOGI("FM PowerDown Tx\n");
            }else if(gStatus.state == FM_ON_RX){
#if FM_AUDIO_SUPPORT
                #ifdef FM_ANALOG_INPUT
			    FMLoopbackTest(false);   
                #else
			    Audio_I2S_Play(false); 
                #endif
#endif				
                type = FM_RX;
                LOGI("FM PowerDown Rx\n");
            }else{
                LOGE("FM not PowerOn yet, please check!\n");
                fm_cnf.drv_status = errno;
                fm_cnf.status = META_FAILED;
                WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
            }

            gStatus.state = FM_OFF;
		    ret = ioctl(g_fm_fd, FM_IOCTL_POWERDOWN, &type);
		    if (ret){
		        ERR("FM_OP_POWER_OFF failed\n");
		    	fm_cnf.drv_status = errno;
                fm_cnf.status = META_FAILED;
            }else{
				fm_cnf.drv_status = FM_SUCCESS;
                fm_cnf.status = META_SUCCESS;
		        DBG("FM_OP_POWER_OFF ok\n"); 
			}
            //FMLoopbackTest(false);
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			break;

        case FM_OP_SET_AUDIO_PATH_TX:
		    if(gStatus.state != FM_ON_TX){
                LOGE("Error status: FM TX is OFF\n");
                fm_cnf.status = META_FAILED;
                WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
                break;
            }
            switch(req->cmd.m_rAudioPathCtrl.m_audioPath){
                case FM_TX_AUDIO_ANALOG:
                    //set audio TX to analog
                    LOGI("Set audio path to analog\n");
                    if(gStatus.audio_path == FM_TX_AUDIO_I2S){
#if FM_AUDIO_SUPPORT
                        //close I2S path
                        #ifdef FM_AUDIO_TX
                        Audio_FMTX_Play(false, gStatus.tx_tone);
                        #endif
                        //open analog path
#endif                        
                        //TBD
                        gStatus.audio_path = FM_TX_AUDIO_ANALOG;
                    }else if(gStatus.audio_path == FM_TX_AUDIO_ANALOG){
                        LOGI("already on analog, no need to switch\n");
                    }
                    fm_cnf.status = META_SUCCESS;
                    fm_cnf.drv_status = FM_SUCCESS;
                    break;
                case FM_TX_AUDIO_I2S:
                    LOGI("Set audio path to I2S\n");
                    if(gStatus.audio_path == FM_TX_AUDIO_ANALOG){
                        //close analog path
                        //TBD
#if FM_AUDIO_SUPPORT
                        //open I2S path
                        #ifdef FM_AUDIO_TX
                        Audio_FMTX_Play(false, gStatus.tx_tone);
                        Audio_FMTX_Play(true, gStatus.tx_tone);
                        #endif
#endif						
                        gStatus.audio_path = FM_TX_AUDIO_I2S;
                    }else if(gStatus.audio_path == FM_TX_AUDIO_I2S){
                        LOGI("already on I2S, no need to switch\n");
                    }
                    fm_cnf.status = META_SUCCESS;
                    fm_cnf.drv_status = FM_SUCCESS;
                    break;
                default:
                    LOGE("para err, [audioPath=%d]\n", req->cmd.m_rAudioPathCtrl.m_audioPath);
                    fm_cnf.drv_status = FM_BADSTATUS;
                    fm_cnf.status = META_FAILED;
                    break;
            }
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			break;
            
        case FM_OP_SET_AUDIO_FREQ_TX:
		    if(gStatus.state != FM_ON_TX){
                LOGE("Error status: FM TX is OFF\n");
                fm_cnf.status = META_FAILED;
                WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
                break;
            }
            if(req->cmd.m_rAudioFreqCtrl.m_audioFreq < FMTX_MAX_TONE){
                gStatus.tx_tone = req->cmd.m_rAudioFreqCtrl.m_audioFreq;
                if(gStatus.audio_path == FM_TX_AUDIO_I2S){
#if FM_AUDIO_SUPPORT
                    #ifdef FM_AUDIO_TX
                    Audio_FMTX_Play(false, gStatus.tx_tone);
                    Audio_FMTX_Play(true, gStatus.tx_tone);
                    #endif
#endif					
                }else{
                    //analog path
                }
                fm_cnf.drv_status = FM_SUCCESS;
                fm_cnf.status = META_SUCCESS;
            }else{
                LOGE("para err, [Freq=%d]\n", req->cmd.m_rAudioFreqCtrl.m_audioFreq);
                fm_cnf.status = META_FAILED;
            }
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			break;
        case FM_OP_SET_MONO_STEREO_BLEND:
			parm_em.group_idx = req->cmd.m_rMonoStereoSettings.m_u2MonoOrStereo; 
			parm_em.item_idx = req->cmd.m_rMonoStereoSettings.m_u2SblendOnOrOff; 
			parm_em.item_value = req->cmd.m_rMonoStereoSettings.m_u4ItemValue;
			
			ret = ioctl(g_fm_fd, FM_IOCTL_EM_TEST, &parm_em);
			if (ret){
				ERR("FT_FM_OP_SET_MONO_STEREO_BLEND\n");
				fm_cnf.drv_status = errno;
			}else{
				fm_cnf.drv_status = FM_SUCCESS;
				DBG("FM_OP_POWER_OFF ok\n");
			}
            
			fm_cnf.status = META_SUCCESS;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			break;
			
        case FM_OP_SET_FREQ:		   
		    bzero(&parm_tune, sizeof(struct fm_tune_parm));
		    parm_tune.band = FM_META_BAND;
            parm_tune.freq = req->cmd.m_rCurFreq.m_i2CurFreq;
            parm_tune.hilo = FM_AUTO_HILO_OFF;
            parm_tune.space = FM_SPACE_DEFAULT;
            
            ret = ioctl(g_fm_fd, FM_IOCTL_TUNE, &parm_tune);
            if (ret){
                ERR("FM_IOCTL_TUNE failed\n");
            	fm_cnf.drv_status = errno;
            }else{
				fm_cnf.drv_status = -parm_tune.err;
				if (fm_cnf.drv_status == FM_SUCCESS){
					DBG("FM_OP_SET_FREQ:%d ok.\n", req->cmd.m_rCurFreq.m_i2CurFreq);
				}else{
					ERR("FM_OP_SET_FREQ:%d failed.\n", req->cmd.m_rCurFreq.m_i2CurFreq);
				}
            }
		    		    
			fm_cnf.status = META_SUCCESS;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			break;
			
        case FM_OP_SET_FREQ_TX:		   
		    bzero(&parm_tune, sizeof(struct fm_tune_parm));
		    parm_tune.band = FM_META_BAND;
            parm_tune.freq = req->cmd.m_rCurFreq.m_i2CurFreq;
            parm_tune.hilo = FM_AUTO_HILO_OFF;
            parm_tune.space = FM_SPACE_DEFAULT;
            
            ret = ioctl(g_fm_fd, FM_IOCTL_TUNE_TX, &parm_tune);
            if (ret){
                ERR("FM_IOCTL_TUNE_TX failed\n");
            }
		    
		    DBG("FM_OP_SET_FREQ_TX:%d\n", req->cmd.m_rCurFreq.m_i2CurFreq);
		    
			fm_cnf.status = META_SUCCESS;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			break;
			
        case FM_OP_SET_RDS_TX:		   
		    bzero(&param_rds_tx, sizeof(struct fm_rds_tx_parm));
			for(i = 0; i < 4; i++){
				uint16_t blk_B = 0x0;
				uint16_t blk_C = 0x0;

				if(req->cmd.m_rRdsTx.tp == 1){
					blk_B |= 0x0400;	// blk_B[10] = TP
				}
				blk_B |= (req->cmd.m_rRdsTx.pty << 5);	// blk_B[9:5] = PTY
				if(req->cmd.m_rRdsTx.ta == 1){
					blk_B |= 0x0010;	// blk_B[4] = TA
				}
				if(req->cmd.m_rRdsTx.speech == 1){
					blk_B |= 0x0008;	// blk_B[3] = TA
				}
				switch(i){
					case 0:
						if(req->cmd.m_rRdsTx.dyn_pty == 1){
							blk_B |= 0x0004;	// blk_B[2] = d3 = dynamic pty
						}
						blk_B |= 0x0000;	// blk_B[1:0] = 00
						break;

					case 1:
						if(req->cmd.m_rRdsTx.compress == 1){
							blk_B |= 0x0004;	// blk_B[2] = d2 = compressed
						}
						blk_B |= 0x0001;	// blk_B[1:0] = 01
						break;

					case 2:
						if(req->cmd.m_rRdsTx.ah == 1){
							blk_B |= 0x0004;	// blk_B[2] = d1 = artificial head
						}
						blk_B |= 0x0002;	// blk_B[1:0] = 10
						break;

					case 3:
						if(req->cmd.m_rRdsTx.stereo == 1){
							blk_B |= 0x0004;	// blk_B[2] = d0 = mono/stereo
						}
						blk_B |= 0x0003;	// blk_B[1:0] = 11
						break;
				}

				if(req->cmd.m_rRdsTx.af == 0){
					blk_C = 0xE000;
				}else{
					blk_C = 0xE100 | req->cmd.m_rRdsTx.af;
				}
				
				param_rds_tx.ps[i*3+0] = blk_B;	// block B
				param_rds_tx.ps[i*3+1] = blk_C;	// block C
				param_rds_tx.ps[i*3+2] = (req->cmd.m_rRdsTx.ps_buf[2*i] << 8) + req->cmd.m_rRdsTx.ps_buf[2*i+1];	// block D
			}

		    param_rds_tx.pi = req->cmd.m_rRdsTx.pi_code;
            param_rds_tx.other_rds_cnt = 0;
            
            ret = ioctl(g_fm_fd, FM_IOCTL_RDS_TX, &param_rds_tx);
            if (ret){
                ERR("FM_IOCTL_RDS_TX failed\n");
                CONSOLE_PRINT("FM_IOCTL_RDS_TX failed:%d:%d\n", ret, parm_tune.err);
            }
		    
		    DBG("FM_OP_SET_RDS_TX:ok\n");
		    
			fm_cnf.status = META_SUCCESS;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			break;
			
        case FM_OP_GET_SIGNAL_VAL:
		    ret = ioctl(g_fm_fd, FM_IOCTL_GETRSSI, &rssi);
            if (ret){
                ERR("FM_OP_GET_SIGNAL_LEVEL failed \n");
				fm_cnf.drv_status = errno;
            }else{
				fm_cnf.drv_status = FM_SUCCESS;
                DBG("FM_IOCTL_GETRSSI:%d\n", rssi);
			}
            
            fm_cnf.fm_result.m_rSignalValue.m_ucSignalLevel = rssi;
     		fm_cnf.status = META_SUCCESS;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			break;
			
	    /*
	    //not implement for MT6616
	    case FT_FM_OP_GET_IF_CNT:
            //cnf.result.m_rIfCnt.m_u2IfCnt = FMR_GetIFCount(p_req->cmd.m_rCurFreq.m_i2CurFreq);
            break;
            
        case FM_OP_SELECT_SOFT_MUTE_STAGE:
            //FMR_SoftMuteStage(p_req->cmd.m_rStage.m_ucStage);
            break;
            
        case FM_OP_SELECT_STEREO_BLEND:
            //FMR_StereoBlendStage(p_req->cmd.m_rStage.m_ucStage);
            break;
            
        case FM_OP_GET_H_L_SIDE:
	        //FMR_Get_H_L_side(p_req->cmd.m_rCurFreq.m_i2CurFreq);
			break;
			
	    */
        case FM_OP_GET_STEREO_MONO:
	        ret = ioctl(g_fm_fd, FM_IOCTL_GETMONOSTERO, &monoorstereo);
            if (ret){
                ERR("FM_OP_GET_SIGNAL_LEVEL failed \n");
				fm_cnf.drv_status = errno;
            }else{
				fm_cnf.drv_status = FM_SUCCESS;
				DBG("FM_OP_GET_STEREO_MONO:%d\n", monoorstereo);
            }        
            
            fm_cnf.fm_result.m_rStereoMono.m_ucStereoOrMono = (unsigned char)monoorstereo;
     		fm_cnf.status = META_SUCCESS;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			break;
	       
	        //FMR_Get_stereo_mono();
	        break;
            
        case FM_OP_READ_ANY_BYTE:
		    bzero(&parm_ctl, sizeof(struct fm_ctl_parm));        
            parm_ctl.addr = req->cmd.m_rReadAddr.m_ucAddr;
            parm_ctl.val = 0;
            parm_ctl.rw_flag = 0x01; //0:write, 1:read
            
            ret = ioctl(g_fm_fd, FM_IOCTL_RW_REG, &parm_ctl);
            DBG("[ret=%d][err=%d][errno=%s]\n", ret, parm_ctl.err, strerror(errno));
            if (ret){
                ERR("FM_OP_READ_BYTE read failed \n");
            	fm_cnf.drv_status = errno;
            }else{
				fm_cnf.drv_status = -parm_ctl.err;
				if (fm_cnf.drv_status == FM_SUCCESS){
					DBG("FM_OP_READ_ANY_BYTE ok.\n");
				}else{
					ERR("FM_OP_READ_ANY_BYTE failed.\n");
				}
            }  
            
            if(parm_ctl.err == FM_SUCCESS){
			    fm_cnf.fm_result.m_rReadByte.m_u2ReadByte = parm_ctl.val;
			}else{
			    fm_cnf.fm_result.m_rReadByte.m_u2ReadByte = 0xffff;		  
			}
			
			DBG("FM_OP_READ_BYTE:%02x:%04x\n", req->cmd.m_rReadAddr.m_ucAddr, fm_cnf.fm_result.m_rReadByte.m_u2ReadByte); 
			    
			fm_cnf.status = META_SUCCESS;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			break;

		case FM_OP_WRITE_ANY_BYTE:		   
		    bzero(&parm_ctl, sizeof(struct fm_ctl_parm));
            
            parm_ctl.addr = req->cmd.m_rWriteByte.m_ucAddr;
            parm_ctl.val = req->cmd.m_rWriteByte.m_u2WriteByte;
            parm_ctl.rw_flag = 0x0; //0:write, 1:read
            
            ret = ioctl(g_fm_fd, FM_IOCTL_RW_REG, &parm_ctl);
            if (ret){
                ERR("FM_OP_WRITE_ANY_BYTE write failed \n");
            	fm_cnf.drv_status = errno;
            }else{
				fm_cnf.drv_status = -parm_ctl.err;
				if (fm_cnf.drv_status == FM_SUCCESS){
					DBG("FM_OP_WRITE_ANY_BYTE ok.\n");
				}else{
					ERR("FM_OP_WRITE_ANY_BYTE failed.\n");
				}
            } 
            
            DBG("FM_OP_WRITE_BYTE:%02x:%04x\n", req->cmd.m_rWriteByte.m_ucAddr, req->cmd.m_rWriteByte.m_u2WriteByte);  
             
			fm_cnf.status = META_SUCCESS;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			break;
			
        case FM_OP_SOFT_MUTE_ONOFF:
		    ret = ioctl(g_fm_fd, FM_IOCTL_MUTE, &req->cmd.m_rSoftMuteOnOff.m_bOnOff);
		    if(ret){
		        ERR("FM_OP_MUTE failed \n");
		    	fm_cnf.drv_status = errno;
            }else{
				fm_cnf.drv_status = FM_SUCCESS;
				DBG("FM_OP_GET_STEREO_MONO:%d\n", monoorstereo);
		    }
		    
		    DBG("FM_OP_MUTE:%d\n", req->cmd.m_rSoftMuteOnOff.m_bOnOff);
		    
			fm_cnf.status = META_SUCCESS;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			break;
				
		case FM_OP_SEARCH_NEXT_STAT:
		    bzero(&parm_seek, sizeof(struct fm_seek_parm));
		    
            parm_seek.band = FM_META_BAND;
            parm_seek.freq = req->cmd.m_rFreqRange.m_i2StartFreq;
            parm_seek.hilo = FM_AUTO_HILO_OFF; 
            parm_seek.space = FM_SPACE_DEFAULT; //default 100K.
            parm_seek.seekdir = FM_SEEK_UP;
            parm_seek.seekth = FM_SEEKTH_LEVEL_DEFAULT;
            
            ret = ioctl(g_fm_fd, FM_IOCTL_SEEK, &parm_seek);
            if(ret){                
                ERR("FM_OP_SEARCH_NEXT_STAT failed:%d:%d\n", ret, parm_seek.err);
            	fm_cnf.drv_status = errno;
            }else{
				fm_cnf.drv_status = -parm_seek.err;
				if (fm_cnf.drv_status == FM_SUCCESS){
					DBG("FM_OP_SEARCH_NEXT_STAT ok.\n");
				}else{
					ERR("FM_OP_SEARCH_NEXT_STAT failed.\n");
				}
            }
                      
            if(parm_seek.err == FM_SUCCESS){
			    if(parm_seek.freq <= req->cmd.m_rFreqRange.m_i2StopFreq){
			        fm_cnf.fm_result.m_rValidFreq.m_i2ValidFreq = parm_seek.freq;
			        fm_cnf.fm_result.m_rValidFreq.m_ucExit = 1;
			    }else{
			        fm_cnf.fm_result.m_rValidFreq.m_i2ValidFreq = 0;
			        fm_cnf.fm_result.m_rValidFreq.m_ucExit = 0;	           
			    }
			}else{
                fm_cnf.fm_result.m_rValidFreq.m_i2ValidFreq = 0;
			    fm_cnf.fm_result.m_rValidFreq.m_ucExit = -1;	 
			    ERR("FM_OP_SEARCH_NEXT_STAT failed:%d:%d\n", ret, parm_seek.err);
			}	
			
			fm_cnf.status = META_SUCCESS;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			break;
			
        case FM_OP_SEARCH_PREV_STAT:
		    bzero(&parm_seek, sizeof(struct fm_seek_parm));
		    
            parm_seek.band = FM_META_BAND;
            parm_seek.freq = req->cmd.m_rFreqRange.m_i2StartFreq;
            parm_seek.hilo = FM_AUTO_HILO_OFF;
            parm_seek.space = FM_SPACE_DEFAULT; //default 100K.
            parm_seek.seekdir = FM_SEEK_DOWN;
            parm_seek.seekth = FM_SEEKTH_LEVEL_DEFAULT;
            
            ret = ioctl(g_fm_fd, FM_IOCTL_SEEK, &parm_seek);
            if(ret){                
                ERR("FM_OP_SEARCH_PREV_STAT failed:%d:%d\n", ret, parm_seek.err);
            	fm_cnf.drv_status = errno;
            }else{
				fm_cnf.drv_status = -parm_seek.err;
				if (fm_cnf.drv_status == FM_SUCCESS){
					DBG("FM_OP_SEARCH_PREV_STAT ok.\n");
				}else{
					ERR("FM_OP_SEARCH_PREV_STAT failed.\n");
				}
            }
            
            DBG("FM_OP_SEARCH_PREV_STAT start freq:%d\n", req->cmd.m_rFreqRange.m_i2StartFreq);

            if(parm_seek.err == FM_SUCCESS){
			    if(parm_seek.freq >= req->cmd.m_rFreqRange.m_i2StopFreq){
			        fm_cnf.fm_result.m_rValidFreq.m_i2ValidFreq = parm_seek.freq;
			        fm_cnf.fm_result.m_rValidFreq.m_ucExit = 1;
			    }else{
			        fm_cnf.fm_result.m_rValidFreq.m_i2ValidFreq = 0;
			        fm_cnf.fm_result.m_rValidFreq.m_ucExit = 0;	           
			    }
			}else{
                fm_cnf.fm_result.m_rValidFreq.m_i2ValidFreq = 0;
			    fm_cnf.fm_result.m_rValidFreq.m_ucExit = -1;	 
			    ERR("FM_OP_SEARCH_PREV_STAT failed:%d:%d\n", ret, parm_seek.err);
			}	
			
			fm_cnf.status = META_SUCCESS;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			break;
			
		case FM_OP_SET_RSSI_THRESHOLD:
		    parm_em.group_idx = 2; 
			parm_em.item_idx = 0; 
			parm_em.item_value = req->cmd.m_rRssiThreshold.m_u4RssiThreshold;
			
			ret = ioctl(g_fm_fd, FM_IOCTL_EM_TEST, &parm_em);
            if (ret){
                ERR("FM_OP_SET_RSSI_THRESHOLD failed\n");
            	fm_cnf.drv_status = errno;
            }else{
				fm_cnf.drv_status = FM_SUCCESS;
            }
       
			fm_cnf.status = META_SUCCESS;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			break;
			
		case FM_OP_SET_IF_CNT_DELTA:  //question will call HCC_Enable
			/*m_u2ItemIdx=0:disable,1:enable*/
			parm_em.group_idx = 3; 
			parm_em.item_idx = 0; 
			parm_em.item_value = req->cmd.m_rIfCntDelta.m_u4IfCntDelta;
			
			ret = ioctl(g_fm_fd, FM_IOCTL_EM_TEST, &parm_em);
            if (ret){
                ERR("FM_OP_HCC_ENABLE failed\n");
            	fm_cnf.drv_status = errno;
            }else{
				fm_cnf.drv_status = FM_SUCCESS;
            }
            
			fm_cnf.status = META_SUCCESS;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			break;
		
		/* for MT6616 new API*/	
	    case FM_OP_SET_VOLUME:  //question
		    //req->cmd.m_rVolumeSetting.m_ucVolume
		    //req->cmd.m_rVolumeSetting.m_cDigitalGainIndex - not need this on Android.
		    vol = req->cmd.m_rVolumeSetting.m_ucVolume;
		    LOGD("%s, set vol to %d", __func__, vol);
		    vol = vol/17; //to map the META tool range to FM driver range.
		    
		    if(vol > 15)
		        vol = 15;
		        		     
		    ret = ioctl(g_fm_fd, FM_IOCTL_SETVOL, &vol);
            if (ret){
                ERR("FM_OP_SET_OUTPUT_VOLUME failed:%d\n", ret);
            	fm_cnf.drv_status = errno;
            }else{
				fm_cnf.drv_status = FM_SUCCESS;
            }
            
			fm_cnf.status = META_SUCCESS;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			break;
			
		case FM_OP_FM_AUTOSCAN:                        
            parm_scan.band = FM_BAND_UE; //sync with Meta tool side
            parm_scan.space = FM_SPACE_DEFAULT;
            parm_scan.hilo = FM_AUTO_HILO_OFF;
            parm_scan.freq = FM_META_FREQ_MAX;
            parm_scan.ScanTBLSize = sizeof(parm_scan.ScanTBL)/sizeof(unsigned short);
            
            /*
            ret = ioctl(g_fm_fd, FM_IOCTL_SCAN, &parm_scan);
            if (ret){
                ERR("FM_OP_HW_AUTO_SCAN failed\n");
				fm_cnf.drv_status = errno;
            }else{
				fm_cnf.drv_status = -parm_scan.err;
	        }

	        fm_cnf.status = META_SUCCESS;
            WriteDataToPC(&fm_cnf, sizeof(FM_CNF), parm_scan.ScanTBL, sizeof(parm_scan.ScanTBL));
            DBG("scan peer_buf 0x%08x, size %d\n", parm_scan.ScanTBL, sizeof(parm_scan.ScanTBL));
			break;*/

			for(i=0; i<band_channel_no; i++)
			{
				cur_freq.freq = min_freq+ seek_space*i;
				
				DBG("FM_OP_SOFT_MUTE_SEEK i=%d, freq=%d-----1\n", i, cur_freq.freq);
				ret = ioctl(g_fm_fd, FM_IOCTL_SOFT_MUTE_TUNE, &cur_freq);
				if (ret) 
				{
					LOGE("FM soft mute tune faild:%d\n",ret);
					parm_scan.err = FM_FAILED;
					goto scan_out;
				}
				
				if(cur_freq.valid == fm_true)
				{
					parm_scan.ScanTBL[i/16] |= 1<<(i%16);
				}
			}
			
scan_out:			
	        fm_cnf.status = META_SUCCESS;
            WriteDataToPC(&fm_cnf, sizeof(FM_CNF), parm_scan.ScanTBL, sizeof(parm_scan.ScanTBL));
            DBG("scan peer_buf 0x%08x, size %d\n", parm_scan.ScanTBL, sizeof(parm_scan.ScanTBL));
			break;
			/*
	    case FM_OP_HWSEEK:  
            bzero(&parm_seek, sizeof(struct fm_seek_parm));		    
            parm_seek.band = FM_META_BAND;
            parm_seek.freq = req->cmd.m_rHWSeek.m_i2StartFreq;
            parm_seek.hilo = FM_AUTO_HILO_OFF;
            parm_seek.space = FM_SPACE_DEFAULT; //default 100K.

            if (req->cmd.m_rHWSeek.m_ucDirection == 1){
                parm_seek.seekdir = FM_SEEK_UP; 
            }else{    
                parm_seek.seekdir = FM_SEEK_DOWN;
            }
            parm_seek.seekth = FM_SEEKTH_LEVEL_DEFAULT;
            
            ret = ioctl(g_fm_fd, FM_IOCTL_SEEK, &parm_seek);
            if(ret){                
                ERR("FT_FM_OP_HWSEEK failed:%d:%d\n", ret, parm_seek.err);
            	fm_cnf.drv_status = errno;
            }else{
				fm_cnf.drv_status = -parm_seek.err;
            }
            
            DBG("FT_FM_OP_HWSEEK start freq:%d\n", req->cmd.m_rHWSeek.m_i2StartFreq);

            if(parm_seek.err == FM_SUCCESS){
			    if(parm_seek.freq >= req->cmd.m_rFreqRange.m_i2StopFreq){
			        fm_cnf.fm_result.m_rHWSeek.m_i2EndFreq = parm_seek.freq;
			    }else{			        
			        fm_cnf.fm_result.m_rHWSeek.m_i2EndFreq = 0;	           
			    }
			}else{
			    fm_cnf.fm_result.m_rHWSeek.m_i2EndFreq = -1;	 
			    ERR("FT_FM_OP_HWSEEK failed:%d:%d\n", ret, parm_seek.err);
			}	
			
			fm_cnf.status = META_SUCCESS;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			break;*/
		case FM_OP_SOFT_MUTE_SEEK:
            bzero(&parm_seek, sizeof(struct fm_seek_parm));		    
            parm_seek.band = FM_META_BAND;
            parm_seek.freq = req->cmd.m_rHWSeek.m_i2StartFreq*seek_space; //soft mute tune frequency range : 8750~10800
            parm_seek.hilo = FM_AUTO_HILO_OFF;
            parm_seek.space = FM_SPACE_DEFAULT; //default 100K.
            parm_seek.err == FM_SUCCESS;

            //DBG("FM_OP_SOFT_MUTE_SEEK req->cmd.m_rFreqRange.m_i2StartFreq:%d\n", req->cmd.m_rFreqRange.m_i2StartFreq);
            //DBG("FM_OP_SOFT_MUTE_SEEK req->cmd.m_rFreqRange.m_i2StopFreq :%d\n", req->cmd.m_rFreqRange.m_i2StopFreq);

            if (req->cmd.m_rHWSeek.m_ucDirection == 1){	//seek up
                parm_seek.seekdir = FM_SEEK_UP; 
				
				for (i=((parm_seek.freq-min_freq)/seek_space+1); i<band_channel_no; i++)
				{
					cur_freq.freq = min_freq + seek_space*i;
					DBG("FM_OP_SOFT_MUTE_SEEK i=%d, freq=%d-----1\n", i, cur_freq.freq);
						
					ret = ioctl(g_fm_fd, FM_IOCTL_SOFT_MUTE_TUNE, &cur_freq);
					if (ret) 
					{
						LOGE("FM soft mute tune faild:%d\n",ret);
						parm_seek.err = FM_FAILED;
						goto out;
					}
					
					if(cur_freq.valid == fm_false)
					{
						continue;
					}
					else
					{
						parm_seek.freq = cur_freq.freq;
						goto out;
					}
				}
				
				for (i=0; i<((parm_seek.freq-min_freq)/seek_space); i++)
				{
					cur_freq.freq = min_freq + seek_space*i;
					DBG("FM_OP_SOFT_MUTE_SEEK i=%d, freq=%d-----2\n", i, cur_freq.freq);
					
					ret = ioctl(g_fm_fd, FM_IOCTL_SOFT_MUTE_TUNE, &cur_freq);
					if (ret) 
					{
						LOGE("FM soft mute tune faild:%d\n",ret);
						parm_seek.err = FM_FAILED;
						goto out;
					}
					
					if(cur_freq.valid == fm_false)
					{
						continue;
					}
					else
					{
						parm_seek.freq = cur_freq.freq;
						goto out;
					}
				}
            }
			else{    									//seek down
                parm_seek.seekdir = FM_SEEK_DOWN;

				for (i=((parm_seek.freq-min_freq)/seek_space-1); i>=0; i--)
				{
					cur_freq.freq = min_freq + seek_space*i;
					DBG("FM_OP_SOFT_MUTE_SEEK i=%d, freq=%d-----3\n", i, cur_freq.freq);

					ret = ioctl(g_fm_fd, FM_IOCTL_SOFT_MUTE_TUNE, &cur_freq);
					if (ret) 
					{
						LOGE("FM soft mute tune faild:%d\n",ret);
						parm_seek.err = FM_FAILED;
						goto out;
					}
					
					if(cur_freq.valid == fm_false)
					{
						continue;
					}
					else
					{
						parm_seek.freq = cur_freq.freq;
						goto out;
					}
				}
				
				for (i=(band_channel_no-1); i>((parm_seek.freq-min_freq)/seek_space); i--)
				{
					cur_freq.freq = min_freq + seek_space*i;
					DBG("FM_OP_SOFT_MUTE_SEEK i=%d, freq=%d-----4\n", i, cur_freq.freq);

					ret = ioctl(g_fm_fd, FM_IOCTL_SOFT_MUTE_TUNE, &cur_freq);
					if (ret) 
					{
						LOGE("FM soft mute tune faild:%d\n",ret);
						parm_seek.err = FM_FAILED;
						goto out;
					}
					
					if(cur_freq.valid == fm_false)
					{
						continue;
					}
					else
					{
						parm_seek.freq = cur_freq.freq;
						goto out;
					}
				}
            }

out:
			if(parm_seek.err == FM_SUCCESS){
			    if(parm_seek.freq >= req->cmd.m_rFreqRange.m_i2StopFreq){
			        fm_cnf.fm_result.m_rHWSeek.m_i2EndFreq = parm_seek.freq/seek_space;
			    }else{			        
			        fm_cnf.fm_result.m_rHWSeek.m_i2EndFreq = 0;	           
			    }
			}else{
			    fm_cnf.fm_result.m_rHWSeek.m_i2EndFreq = -1;	 
			    ERR("FM_OP_SOFT_MUTE_SEEK failed:%d:%d\n", ret, parm_seek.err);
			}	
			
			fm_cnf.status = META_SUCCESS;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			
			break;
		
		case FM_OP_GET_RXFILTER_BW:
		    fm_cnf.fm_result.m_rRXFilterBW.m_ucRXFilterBW = 0x0;
			fm_cnf.drv_status = FM_FAILED;
			fm_cnf.status = META_SUCCESS;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);		    
		    break;
		    
		case FM_OP_GET_PAMD_LEVEL:
		    ret = ioctl(g_fm_fd, FM_IOCTL_GETCURPAMD, &PamdLevl);
            if (ret){
                ERR("FM_OP_GET_PAMD_LEVEL failed \n");
            	fm_cnf.drv_status = errno;
            }else{
				fm_cnf.drv_status = FM_SUCCESS;
            }
            
            DBG("FM_OP_GET_PAMD_LEVEL: %d\n", PamdLevl);
            
            fm_cnf.fm_result.m_rPAMDLevel.m_ucPAMDLevel = (unsigned char)PamdLevl;
     		fm_cnf.status = META_SUCCESS;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			break;
		    
		case FM_OP_GET_MR:
		    fm_cnf.fm_result.m_rMR.m_ucMR = 0x01;
		    fm_cnf.drv_status = FM_FAILED;
		    fm_cnf.status = META_SUCCESS;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);	
		    break;
       
        case FM_OP_SET_DECODE_MODE:
		    parm_em.group_idx = 0;
			parm_em.item_idx = 0; 
			parm_em.item_value = req->cmd.m_rDecodeMode.m_u4DecodeMode;
			
			ret = ioctl(g_fm_fd, FM_IOCTL_EM_TEST, &parm_em);
            if (ret){
                ERR("FM_OP_SET_DECODE_MODE failed\n");
            	fm_cnf.drv_status = errno;
            }else{
				fm_cnf.drv_status = FM_SUCCESS;
            }
            
			DBG("FM_OP_SET_DECODE_MODE:%d\n", req->cmd.m_rDecodeMode.m_u4DecodeMode);

			fm_cnf.status = META_SUCCESS;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			break;   	
	    
	     case FM_OP_SET_HCC:  //is this right?
            parm_em.group_idx = 3; 
			parm_em.item_idx = req->cmd.m_rHCC.m_u4HCC; 
			parm_em.item_value = 0;
			
			ret = ioctl(g_fm_fd, FM_IOCTL_EM_TEST, &parm_em);
            if (ret){
                ERR("FM_OP_SET_HCC failed\n");
            	fm_cnf.drv_status = errno;
            }else{
				fm_cnf.drv_status = FM_SUCCESS;
            }
            
			DBG("FM_OP_SET_HCC:%d\n", req->cmd.m_rHCC.m_u4HCC);

			fm_cnf.status = META_SUCCESS;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
            break;
            
        case FM_OP_SET_PAMD_THRESHOLD:
            parm_em.group_idx = 4; 
			parm_em.item_idx = 0; 
			parm_em.item_value = req->cmd.m_rPAMDThreshold.m_u4PAMDThreshold;
			
			ret = ioctl(g_fm_fd, FM_IOCTL_EM_TEST, &parm_em);
            if (ret){
                ERR("FM_OP_SET_PAMD_THRESHOLD failed\n");
            	fm_cnf.drv_status = errno;
            }else{
				fm_cnf.drv_status = FM_SUCCESS;
            }
            
			DBG("FM_OP_SET_PAMD_THRESHOLD:%d\n", req->cmd.m_rPAMDThreshold.m_u4PAMDThreshold);

			fm_cnf.status = META_SUCCESS;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
            break;
            
       case FM_OP_SET_SOFTMUTE:
			/*m_u2ItemIdx=0:disable,1:enable*/
		    parm_em.group_idx = 5; 
			parm_em.item_idx = req->cmd.m_rSoftmuteEnable.m_u4SoftmuteEnable; 
			parm_em.item_value = 0;
			
			ret = ioctl(g_fm_fd, FM_IOCTL_EM_TEST, &parm_em);
            if (ret){
                ERR("FM_OP_SOFTMUTE_ONOFF failed\n");
            	fm_cnf.drv_status = errno;
            }else{
				fm_cnf.drv_status = FM_SUCCESS;
            }
            
            DBG("FM_OP_SET_SOFTMUTE:[%d],0-disable,1-enable\n", req->cmd.m_rSoftmuteEnable.m_u4SoftmuteEnable);
            
			fm_cnf.status = META_SUCCESS;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			break;
            
        case FM_OP_SET_DEEMPHASIS_LEVEL:
			parm_em.group_idx = 6; 
			parm_em.item_idx = req->cmd.m_rDeemphasisLevel.m_u4DeemphasisLevel; 
			parm_em.item_value = 0;
			
			ret = ioctl(g_fm_fd, FM_IOCTL_EM_TEST, &parm_em);
            if (ret){
                ERR("FM_OP_DE_EMPHASIS failed\n");
            	fm_cnf.drv_status = errno;
            }else{
				fm_cnf.drv_status = FM_SUCCESS;
            }
            
			DBG("FM_OP_SET_DEEMPHASIS_LEVEL:[%d]\n", req->cmd.m_rDeemphasisLevel.m_u4DeemphasisLevel);

			fm_cnf.status = META_SUCCESS;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			break;  
			
		case FM_OP_SET_H_L_SIDE:			
			parm_em.group_idx = 7; 
			parm_em.item_idx = req->cmd.m_rHLSide.m_u4HLSide; 
			parm_em.item_value = 0;
			
			ret = ioctl(g_fm_fd, FM_IOCTL_EM_TEST, &parm_em);
            if (ret){
                ERR("FM_OP_SET_H_L_SIDE failed\n");
            	fm_cnf.drv_status = errno;
            }else{
				fm_cnf.drv_status = FM_SUCCESS;
            }
            
			DBG("FM_OP_SET_H_L_SIDE:[%d]\n", req->cmd.m_rHLSide.m_u4HLSide);

			fm_cnf.status = META_SUCCESS;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			break; 
		
        case FM_OP_SET_DEMOD_BW:			
			parm_em.group_idx = 8; 
			parm_em.item_idx = req->cmd.m_rDemodBandwidth.m_u4DemodBandwidth; 
			parm_em.item_value = 0;
			
			ret = ioctl(g_fm_fd, FM_IOCTL_EM_TEST, &parm_em);
            if (ret){
                ERR("FM_OP_SET_DEMOD_BW failed\n");
            	fm_cnf.drv_status = errno;
            }else{
				fm_cnf.drv_status = FM_SUCCESS;
            }
            
			DBG("FM_OP_SET_DEMOD_BW:[%d]\n", req->cmd.m_rDemodBandwidth.m_u4DemodBandwidth);

			fm_cnf.status = META_SUCCESS;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			break; 
			
	   case FM_OP_SET_DYNAMIC_LIMITER:
			parm_em.group_idx = 9; 
			parm_em.item_idx = req->cmd.m_rDynamicLimiter.m_u4DynamicLimiter; 
			parm_em.item_value = 0;
			
			ret = ioctl(g_fm_fd, FM_IOCTL_EM_TEST, &parm_em);
            if (ret){
                ERR("FM_OP_DYNAMIC_LIMITER failed\n");
            	fm_cnf.drv_status = errno;
            }else{
				fm_cnf.drv_status = FM_SUCCESS;
            }
            
			DBG("FM_OP_SET_DYNAMIC_LIMITER:[%d]\n", req->cmd.m_rDynamicLimiter.m_u4DynamicLimiter);
            
			fm_cnf.status = META_SUCCESS;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			break;
			
	    case FM_OP_SET_SOFTMUTE_RATE:
			parm_em.group_idx =10 ; 
			parm_em.item_idx = 0; 
			parm_em.item_value = req->cmd.m_rSoftmuteRate.m_u4SoftmuteRate;
			
			ret = ioctl(g_fm_fd, FM_IOCTL_EM_TEST, &parm_em);
            if (ret){
                ERR("FM_OP_SET_SOFTMUTE_RATE\n");
            	fm_cnf.drv_status = errno;
            }else{
				fm_cnf.drv_status = FM_SUCCESS;
            }
            
			DBG("FM_OP_SET_SOFTMUTE_RATE:[%d]\n", req->cmd.m_rSoftmuteRate.m_u4SoftmuteRate);
			
			fm_cnf.status = META_SUCCESS;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			break;
		
		case FM_OP_SET_STEREO_BLEND:
			parm_em.group_idx = 1; 
			parm_em.item_idx = req->cmd.m_rStereoBlendControl.m_u2StereoBlendControl; 
			parm_em.item_value = 1; /*1:care blend, 0: not care blend*/
			
			ret = ioctl(g_fm_fd, FM_IOCTL_EM_TEST, &parm_em);
            if (ret){
                ERR("FM_OP_SET_STEREO_BLEND failed\n");
            	fm_cnf.drv_status = errno;
            }else{
				fm_cnf.drv_status = FM_SUCCESS;
            }
			DBG("FM_OP_SET_STEREO_BLEND:[%d]\n", req->cmd.m_rStereoBlendControl.m_u2StereoBlendControl);
			
			fm_cnf.status = META_SUCCESS;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			break;		
			
	    case FM_OP_HWSEARCH_STOP:
		    if(lseek(g_fm_fd, 0, SEEK_END) != -1){
                DBG("FM_OP_HW_SEARCH_FORCE_STOP OK\n");	
				fm_cnf.drv_status = FM_SUCCESS;
            }else{         
				fm_cnf.drv_status = errno;
                ERR("FM_OP_HW_SEARCH_FORCE_STOP Failed\n");
            }
			fm_cnf.status = META_SUCCESS;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			break;
    #if  FM_RDS_ENABLE
	    case FM_OP_SET_RDS:			
            DBG("FM_OP_SET_RDS:[%d]\n", req->cmd.m_rSetRDS.m_ucRDSOn);
			ret = ioctl(g_fm_fd, FM_IOCTL_RDS_ONOFF, &req->cmd.m_rSetRDS.m_ucRDSOn);
			if (ret){
			    ERR("FT_FM_OP_SET_RDS failed\n");
                fm_cnf.status = META_FAILED;
		    }else{
		        switch(req->cmd.m_rSetRDS.m_ucRDSOn){
                    case 0:                 
                        gStatus.rds_t = FM_RDS_OFF;
                        DBG("Set rds RX off\n");
                        break;
                    case 1:
                        gStatus.rds_t = FM_RDS_RX_ON;
                        DBG("Set rds RX on\n");
                        break;
                    default:
                        break;
                }
                DBG("FM_OP_SET_RDS OK\n");
			fm_cnf.status = META_SUCCESS;
            }
            fm_rds_cnf.header.token = fm_cnf.header.token;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
            break;
        case FM_OP_GET_GOOD_BLOCK_COUNTER:{
			uint16_t GOOD_BLK_CNT;
			ret = ioctl(g_fm_fd, FM_IOCTL_GETGOODBCNT, &GOOD_BLK_CNT);
			if (ret){
                ERR("FM_OP_GET_GOOD_BLOCK_COUNTER Failed\n");
                fm_cnf.status = META_FAILED;
            }else{
                DBG("FM_OP_GET_GOOD_BLOCK_COUNTER OK, [%d]\n", GOOD_BLK_CNT);
            fm_cnf.fm_result.m_rRDSGoodBlockCounter.m_u2GoodBlock = GOOD_BLK_CNT;
            fm_cnf.status = META_SUCCESS;
            }
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
            break;		
	    }
        case FM_OP_GET_BAD_BLOCK_COUNTER:{
			uint16_t BAD_BLK_CNT;
			ret = ioctl(g_fm_fd, FM_IOCTL_GETBADBNT, &BAD_BLK_CNT);
			if (ret){
                ERR("FM_OP_GET_BAD_BLOCK_COUNTER Failed\n");
                fm_cnf.status = META_FAILED;
            }else{
                DBG("FM_OP_GET_BAD_BLOCK_COUNTER OK, [%d]\n", BAD_BLK_CNT);
            fm_cnf.fm_result.m_rRDSBadBlockCounter.m_u2BadBlock = BAD_BLK_CNT;
            fm_cnf.status = META_SUCCESS;
            }
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
            break;		
	    }
        case FM_OP_RESET_BLOCK_COUNTER:{
            int tmp;
            ret = ioctl(g_fm_fd, FM_IOCTL_RDS_BC_RST, &tmp);
            if (ret) {
                LOGE("FM_OP_RESET_BLOCK_COUNTER err\n");
                fm_cnf.status = META_FAILED;
            }else{
                LOGD("FM_OP_RESET_BLOCK_COUNTER OK\n");
            fm_cnf.status = META_SUCCESS;
            }
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
            break;
        }
		case FM_OP_GET_GROUP_COUNTER:{
            struct rds_group_cnt_req gc_req;
            gc_req.op = RDS_GROUP_CNT_READ;
            ret = ioctl(g_fm_fd, FM_IOCTL_RDS_GROUPCNT, &gc_req);
            if(ret){
                ERR("FM_OP_GET_GROUP_COUNTER Failed\n");
                fm_cnf.status = META_FAILED;
            }else{
                DBG("FM_OP_GET_GROUP_COUNTER OK\n");
			fm_cnf.status = META_SUCCESS;
            }
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), &gc_req.gc, sizeof(struct rds_group_cnt));
            break;
		}		
		case FM_OP_RESET_GROUP_COUNTER:{
            struct rds_group_cnt_req gc_req;
            gc_req.op = RDS_GROUP_CNT_RESET;
            ret = ioctl(g_fm_fd, FM_IOCTL_RDS_GROUPCNT, &gc_req);
            if(ret){
                ERR("FM_OP_RESET_GROUP_COUNTER Failed\n");
                fm_cnf.status = META_FAILED;
            }else{
                DBG("FM_OP_RESET_GROUP_COUNTER OK\n");
			fm_cnf.status = META_SUCCESS;
            }
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
            break;
		}			
	    case FM_OP_GET_RDS_LOG:{
            struct rds_raw_data rds_log;
            ret = ioctl(g_fm_fd, FM_IOCTL_RDS_GET_LOG, &rds_log);
            if(ret){
                ERR("FM_OP_GET_RDS_LOG Failed\n");
                fm_cnf.status = META_FAILED;
            }else{
                DBG("FM_OP_GET_RDS_LOG OK\n");
            fm_cnf.status = META_SUCCESS;
            }
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), &rds_log.data[0], rds_log.len);
            break;
        }
		case FM_OP_GET_RDS_BLER:{
			uint16_t BLER_Ratio;
			ret = ioctl(g_fm_fd, FM_IOCTL_GETBLERRATIO, &BLER_Ratio);
			if (ret){
			    ERR("FT_FM_OP_GET_RDS_BLER failed\n");
                fm_cnf.status = META_FAILED;
		    }else{
            DBG("FM_OP_GET_RDS_BLER [%d]\n", BLER_Ratio);
            fm_cnf.status = META_SUCCESS;
            }
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), &BLER_Ratio, sizeof(uint16_t));
            break;		
	    }			
    #endif
        case FM_OP_SET_ANTENNA:
            LOGD("Setting antenna to %s\n", (req->cmd.m_rAntenna.ana == FM_ANA_LONG) ? "long" : "short");
            ret = ioctl(g_fm_fd, FM_IOCTL_ANA_SWITCH, &req->cmd.m_rAntenna.ana);
            if (ret) {
                ERR("FM_OP_SET_ANTENNA failed\n");
                fm_cnf.status = META_FAILED;
            } else {
                DBG("FM_OP_SET_ANTENNA ok\n");
                fm_cnf.status = META_SUCCESS;
            }
            WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
            break;		
	    
        case FM_OP_GET_CAPARRY:
            ret = ioctl(g_fm_fd, FM_IOCTL_GETCAPARRAY, &fm_cnf.fm_result.m_rCapArray.m_uCapArray);
            if (ret) {
                ERR("FM_OP_GET_CAPARRY failed\n");
                fm_cnf.status = META_FAILED;
            } else {
                DBG("FM_OP_GET_CAPARRY ok\n");
                fm_cnf.status = META_SUCCESS;
            }
            LOGD("caparray: %d\n", fm_cnf.fm_result.m_rCapArray.m_uCapArray);
            WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
            break;		
	    case FM_OP_AUDIO_BIST_TEST:
	    {
			int ret_true=0,ret_false=0;
#ifdef FM_META_AUDIOBIST
			short buffer[512];	   
			int ret=0,i=100;
	    
		    bzero(&parm_ctl, sizeof(struct fm_ctl_parm));   
#ifdef MT6628_FM		    
			parm_ctl.rw_flag=0;
			parm_ctl.addr = 0x60;
			parm_ctl.val = 0x0087;
            ret = ioctl(g_fm_fd, FM_IOCTL_RW_REG, &parm_ctl);
			if(ret)
            {
                ERR("audio bist wr0x60 failed\n");
            }
			parm_ctl.addr = 0x9D;
			parm_ctl.val = 0x8702;
            ret = ioctl(g_fm_fd, FM_IOCTL_RW_REG, &parm_ctl);
			if(ret)
            {
                ERR("audio bist wr0x9D failed\n");
            }
			parm_ctl.addr = 0x60;
			parm_ctl.val = 0x008F;
            ret = ioctl(g_fm_fd, FM_IOCTL_RW_REG, &parm_ctl);
			if(ret)
            {
                ERR("audio bist wr0x60 failed\n");
            }
#elif defined (MT6627_FM)	    	
            parm_ctl.addr = 0x9D;
            parm_ctl.val = 0x8702;
            ret = ioctl(g_fm_fd, FM_IOCTL_RW_REG, &parm_ctl);
            if(ret)
            {
                ERR("audio bist wr0x9D failed\n");
            }
#endif            
            while(i>0)
            {
		    	ret = readRecordData(buffer,1024);
			LOGD("readRecordData ret=%d\n",ret);
		    	ret=freqCheck(buffer,1024);
				if(ret == false)
				{
					ret_false++;//fail
				}
				else
				{
					ret_true++;//sucess
				}
		    	i--;
	    	}
	    	if(ret_false > ret_true)
	    	{
				fm_cnf.status = 1;//fail
	    	}
	    	else
#endif	    	
	    	{
				fm_cnf.status = 0;//sucess
	    	}
			fm_cnf.header.id = FT_FM_CNF_ID; //remember to add FT_BT_CNF_ID to ft_msg
			fm_cnf.op = req->op;
			LOGD("freqCheck ret_false=%d ret_true=%d fm_cnf.status=%d\n",ret_false,ret_true,fm_cnf.status);
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			break;
	    }
	    default:
			DBG("Not support OPCODE:req->op [%d]\n", req->op);
			fm_cnf.header.id = FT_FM_CNF_ID; //remember to add FT_BT_CNF_ID to ft_msg
			fm_cnf.op = req->op;
			fm_cnf.status = META_FAILED;
			WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);
			break;
	}	
    //WriteDataToPC(&fm_cnf, sizeof(FM_CNF), NULL, 0);	
}

