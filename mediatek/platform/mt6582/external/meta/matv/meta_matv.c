/*****************************************************************************
 *
 * Filename:
 * ---------
 *   meta_matv.c
 *
 * Project:
 * --------
 *   ALPS
 *
 * Description:
 * ------------
 *  mATV Function
 *
 * Author:
 * -------
 *  Siyang Miao (mtk80734)
 *
 ****************************************************************************/
 
/*************************************************************************
* Include Statements for KAL
 *************************************************************************/

#include "kal_release.h"
//#include "stack_common.h"
//#include "stack_msgs.h"
//#include "app_ltlcom.h"
//#include "drv_comm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*************************************************************************
* Include Statements for MAUI
 *************************************************************************/
//#include "ft_msg.h"
//#include "ft_private.h"

//#include "ft_fnc_matv.h"
#include "matvctrl.h"
#include "fmctrl.h"

//#include "meta.h"
#include "FT_Public.h"
#include "meta_matv_para.h"
#include "WM2Linux.h"

static kal_uint8 matv_channels = 0;
static kal_bool matv_power_on = KAL_FALSE;
static kal_uint8 matv_progress = 0;
void dummy_autoscan_cb(void* cb_param, kal_uint8 percent,kal_uint8 ch,kal_uint8 chnum)
{
    matv_progress = percent;
}
void fullscan_progress_cb(void* cb_param, kal_uint8 percent,kal_uint32 freq,kal_uint32 freq_start,kal_uint32 freq_end)
{
    matv_progress = percent;
}
void fullscan_finish_cb(void* cb_param, kal_uint8 chnum)
{
    matv_progress = 101;
    matv_channels = chnum;
}
void dummy_audioformat_cb(void* cb_param, kal_uint32 format)
{
}

void META_MATV_OP(FT_MATV_REQ *p_req)
{
    FT_MATV_CNF matv_cnf;

	
    //peer_buff_struct *peer_buff_ret;
    kal_uint16 pdu_length;
	
    //ilm_ptr = FT_ALLOC_MSG(sizeof(FT_MATV_CNF));
    //matv_cnf = (FT_MATV_CNF *)ilm_ptr->local_para_ptr;
	
	
	int peer_buff_size = 0;
    char* peer_buf = NULL;
	
	memset(&matv_cnf, 0, sizeof(FT_MATV_CNF));
    matv_cnf.header.id = FT_MATV_CMD_CNF_ID;
	matv_cnf.header.token = p_req->header.token;
    matv_cnf.status = META_FAILED;
    matv_cnf.type = p_req->type;
	
    // if the feature option is off, confirm fail
    // TODO:
    //if(Custom_META_ATV_Support_Option())
    //{
    //    matv_cnf.status = META_SUCCESS;
    //}
    
    META_LOG("[FTT_Drv:] FT_MATV, OP=%d ", p_req->type);
    switch(p_req->type) 
    {
        case FT_MATV_OP_POWER_ON:
            if(matv_power_on == KAL_FALSE)
            {
            	META_LOG("[FTT_Drv:] matv need init ");
                matv_power_on = matv_init();
                if(KAL_TRUE == matv_power_on)
                {
                	META_LOG("[FTT_Drv:] matv_init succeeded!");
                    matv_cnf.status = META_SUCCESS;
                }
                else
                {
					META_LOG("[FTT_Drv:] matv_init failed!");
                    matv_cnf.status = META_FAILED;
                }
            }
            else
            {
                matv_cnf.status = META_SUCCESS;
            }
            break;
        case FT_MATV_OP_POWER_OFF:
            if(matv_power_on == KAL_TRUE)
            {
                matv_power_on = !matv_shutdown();
                matv_cnf.status = META_SUCCESS;
            }
            else
            {
                matv_cnf.status = META_FAILED;
            }
            break;
        case FT_MATV_OP_SET_REGION:
            matv_set_country(p_req->cmd.m_ucRegion);
            matv_cnf.status = META_SUCCESS;
            break;
        case FT_MATV_OP_SCAN:
            matv_progress = 0;
            matv_register_callback(NULL, dummy_autoscan_cb, fullscan_progress_cb, fullscan_finish_cb, dummy_audioformat_cb);
            matv_chscan(p_req->cmd.m_ucScanMode);
            matv_cnf.status = META_SUCCESS;
            break;
        case FT_MATV_OP_STOP_SCAN:
            matv_chscan_stop();
            matv_cnf.status = META_SUCCESS;
            break;
        case FT_MATV_OP_GET_CHANNEL_LIST:
        {
            matv_cnf.result.m_ucProgress = matv_progress;
            if(matv_progress == 101)
            {
                kal_bool flag;
                kal_uint8 ch;
                // copy result to peer buffe
                FT_MATV_GET_CHANNEL_LIST_CNF_T *pdu_ptr = NULL;
				peer_buff_size = sizeof(FT_MATV_GET_CHANNEL_LIST_CNF_T);
				pdu_ptr = (FT_MATV_GET_CHANNEL_LIST_CNF_T*)malloc(sizeof(FT_MATV_GET_CHANNEL_LIST_CNF_T));
				memset(pdu_ptr, 0, sizeof(FT_MATV_GET_CHANNEL_LIST_CNF_T));
				
                if(NULL != pdu_ptr)
                {
                	// TODO:
                    //pdu_ptr = get_pdu_ptr(peer_buf, &pdu_length);
					
                    //peer_buff_ret->pdu_len = sizeof(FT_MATV_GET_CHANNEL_LIST_CNF_T);
                    
                    // assign peer buffer
                    //if(NULL != peer_buf)
                    //{
                        //ilm_ptr->peer_buff_ptr = peer_buff_ret;
                    //}

		    		peer_buf = (char *)pdu_ptr;
                }
                else
                {
                    //ASSERT(0);
                 	goto  MATV_Exit;
                }
                pdu_ptr->m_ucChannels = 0;
                for(ch=0;ch<70;ch++)
                {
                    memset(&(pdu_ptr->m_rmatv_ch_entry[ch]), 0x0, sizeof(matv_ch_entry));
                    flag = matv_get_chtable(ch, &(pdu_ptr->m_rmatv_ch_entry[ch]));
                    if(flag == KAL_FALSE)
                    {
                        // end of ch table
                        break;
                    }
                    if(pdu_ptr->m_rmatv_ch_entry[ch].flag == 1)
                    {
                        pdu_ptr->m_ucChannels++;
                    }
                }
            }
            matv_cnf.status = META_SUCCESS;
            break;
        }
        case FT_MATV_OP_CHANGE_CHANNEL:
            matv_change_channel(p_req->cmd.m_ucChannel);
            matv_cnf.status = META_SUCCESS;
            break;
        case FT_MATV_OP_SET_CHANNEL_PROPERTY:
        {
            kal_bool flag;
            flag = matv_set_chtable(p_req->cmd.m_rSetChannelProperty.m_ucChannel,
                    &(p_req->cmd.m_rSetChannelProperty.m_rmatv_ch_entry));
            if(flag == KAL_TRUE)
            {
                matv_cnf.status = META_SUCCESS;
            }
            else
            {
                matv_cnf.status = META_FAILED;
            }
            break;
        }
        case FT_MATV_OP_GET_CHANNEL_QUALITY:
        {
            matv_cnf.result.m_i4QualityIndex = matv_get_chipdep(p_req->cmd.m_ucItem);
            matv_cnf.status = META_SUCCESS;
            break;
        }
        case FT_MATV_OP_GET_CHANNEL_QUALITY_ALL:
        {
            // copy result to peer buffer
            kal_uint8 i;
            FT_MATV_GET_CHANNEL_QUALITY_ALL_CNF_T *pdu_ptr = NULL;
			peer_buff_size = sizeof(FT_MATV_GET_CHANNEL_QUALITY_ALL_CNF_T);
			pdu_ptr = (FT_MATV_GET_CHANNEL_QUALITY_ALL_CNF_T*)malloc(sizeof(FT_MATV_GET_CHANNEL_QUALITY_ALL_CNF_T));
			memset(pdu_ptr, 0, sizeof(FT_MATV_GET_CHANNEL_QUALITY_ALL_CNF_T));
			
            if(NULL != pdu_ptr)
            {
            	// TODO:
                //pdu_ptr = get_pdu_ptr(peer_buf, &pdu_length);
				
                //peer_buff_ret->pdu_len = sizeof(FT_MATV_GET_CHANNEL_QUALITY_ALL_CNF_T);

				
                // assign peer buffer
                //if(NULL != peer_buf)
                //{
                     //ilm_ptr->peer_buff_ptr = peer_buff_ret;
                //}
				
				peer_buf = (char *)pdu_ptr;
            }
            else
            {
                 //ASSERT(0);
                 goto  MATV_Exit;
            }
            for(i=100;i<200;i++)
            {
                pdu_ptr->m_i4QualityIndex[i-100] = matv_get_chipdep(i);
            }
            matv_cnf.status = META_SUCCESS;
            break;
        }
		case FT_MATV_OP_GET_CHIPNAME:
		{
			///char chipname[20];			
			int chipid = fm_getchipid();	
			if(chipid == 0x91)
				sprintf(matv_cnf.result.chipname,"%s", "mt5192");
			else if(chipid == 0x92)
				sprintf(matv_cnf.result.chipname,"%s", "mt5193");
			else
				sprintf(matv_cnf.result.chipname,"%s", "unkown");
			
			matv_cnf.status = META_SUCCESS;
			META_LOG("[FTT_Drv:] FT_MATV_OP_GET_CHIPNAME id %c%c%c%c%c%c succeeded! ", matv_cnf.result.chipname[0], matv_cnf.result.chipname[1],
			  matv_cnf.result.chipname[2], matv_cnf.result.chipname[3], matv_cnf.result.chipname[4], matv_cnf.result.chipname[5],matv_cnf.result.chipname[6]);
			break;
		}
        default:
            break;
    }
    //FT_SEND_MSG(MOD_FT, MOD_TST, FT_TST_SAP, MSG_ID_FT_TO_TST, ilm_ptr);

MATV_Exit:
	if (NULL != peer_buf)
	{
		WriteDataToPC(&matv_cnf, sizeof(matv_cnf), peer_buf, peer_buff_size);
		free(peer_buf);
		peer_buf = NULL;
	}
	else
	{
		WriteDataToPC(&matv_cnf, sizeof(matv_cnf), NULL, 0);
	}
}
