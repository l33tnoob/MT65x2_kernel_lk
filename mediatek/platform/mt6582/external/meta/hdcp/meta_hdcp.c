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

#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include "android/log.h"

#include "FT_Public.h"
#include "meta_common.h"

#include "meta_hdcp_para.h"


#include "Dx_Hdcp_Provisioning.h"

//#include <stdlib.h>
#include <unistd.h> 
#include <errno.h>
#include <private/android_filesystem_config.h>

#undef   LOG_TAG

#define  LOG_TAG  "HDCP_META"
#define  HDCPLOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)


static HDCP_CNF hdcp_cnf;


bool META_HDCP_init()
{
    HDCPLOGI("META_HDCP_init()");
    /* Do nothing */ 
    return true;
}

void META_HDCP_deinit()
{
    HDCPLOGI("META_HDCP_deinit()");
	/* Do nothing */ 
}


void changeKeyFileOwner(void)
{
    const char* dataFile = "/persist/data/dxhdcp2/dxhdcp2/dxhdcp2.sfs";
    int ret = chown(dataFile, AID_MEDIA, AID_MEDIA); 

    if (ret == 0){
        HDCPLOGI("chown() succeed");
    } else {         
        HDCPLOGI("chown() fail. error:[%d] %s", errno, strerror(errno));    
    }     
}

bool startKeyProvisioning(unsigned char *data, int dataLen, unsigned char *cekData, int cekLen)
{
    if (dataLen != 572 || cekLen != 16) {
        HDCPLOGI("keyProvisioning() len is wrong! dataLen:%d, cekLen:%d", dataLen, cekLen);
        return false;
    }    

    DxStatus result = DX_SUCCESS;
    result = DxHdcp_Provisioning_Init();
    if (result != 0)
    {
        HDCPLOGI("DxHdcp_Provisioning_Init failed");
        goto end;
    }
    HDCPLOGI("DxHdcp_Provisioning_Init succeed");

    result = DxHDCP_ProvisionWithCEK(data, dataLen, cekData);   
    if (result != 0)
    {
        HDCPLOGI("DxHdcp_ProvisionWithCEK failed");
        goto end;
    }
    HDCPLOGI("DxHdcp_ProvisionWithCEK succeed");     

    /*
    result = DxHDCP_StoreCEK(cekData);  
    if (result != 0)
    {
        ALOGI("DxHDCP_StoreCEK failed\n");
        goto end;
    }
    ALOGI("DxHDCP_StoreCEK succeed");
    */
    
    result = DxHDCP_ProvisionValidate(data, dataLen, cekData);
    if (result != 0)
    {
        HDCPLOGI("DxHdcp_ProvisionValidate failed");
        goto end;
    }
    HDCPLOGI("DxHdcp_ProvisionValidate succeed");


end:
    DxHdcp_Provisioining_Terminate();

    HDCPLOGI("provisioning test finished with return code: 0x%08x\n",(unsigned int)result);

    /* The file permission is 660 by default.
     * So change file owner and group as media for mediaserver process to R/W
     */
    changeKeyFileOwner();      
     
    return (result == 0) ? true : false;
}


void META_HDCP_OP(HDCP_REQ *req, char *peer_buff, unsigned short peer_len) 
{	
    bool ret;
    
	memset(&hdcp_cnf, 0, sizeof(HDCP_CNF));	
	hdcp_cnf.header.id = FT_HDCP_CNF_ID;
	hdcp_cnf.header.token = req->header.token;
	hdcp_cnf.op = req->op;

    HDCPLOGI("META_HDCP_OP(), op:%d", req->op);
        
    switch(req->op){
        case HDCP_OP_INSTALL:

            ret = startKeyProvisioning(
                req->cmd.hdcp_install_req.data,
                req->cmd.hdcp_install_req.data_len,
                req->cmd.hdcp_install_req.cek_data,
                req->cmd.hdcp_install_req.cek_len);

            // Operation request is accepted successfully
            hdcp_cnf.status = META_SUCCESS;

            // Operation result
            if (ret == true) {
                hdcp_cnf.result.hdcp_install_cnf.install_result = HDCP_CNF_OK;
            } else {
                hdcp_cnf.result.hdcp_install_cnf.install_result = HDCP_CNF_FAIL;
            }             
            
            WriteDataToPC(&hdcp_cnf, sizeof(HDCP_CNF), NULL, 0);
			break;       
            
		
	    default:
			HDCPLOGI("Unsupported OP");
			hdcp_cnf.status = META_FAILED;
			WriteDataToPC(&hdcp_cnf, sizeof(HDCP_CNF), NULL, 0);
			break;
	}  
}

