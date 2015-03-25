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

#include <stdio.h>
#include "meta_pmic.h"

// ---------------------------------------------------------------------------

int main(int argc, const char** args)
{
    FT_PMIC_REG_READ req_read;
	FT_PMIC_REG_READ_CNF config_read;
	FT_PMIC_REG_WRITE req_write;
	FT_PMIC_REG_WRITE_CNF config_write;
	
	printf("Meta Test pmic AP test : START\n");

	if (false == Meta_Pmic_Init())
    {
        printf("Meta_Pmic_Init() fail\n");
        return -1;
    }

	req_read.m_rWmCmdReadReq.registernumber=0x0;
	config_read=META_PMICR_OP(&req_read);
	if (!config_read.m_rWmCmdReadResult.status)
    {
        printf("META_PMICR_OP() fail\n");
        return -1;
    }

	req_write.m_rWmCmdWriteReq.registernumber=0x5D;
	req_write.m_rWmCmdWriteReq.registervalue=0x0;
	config_write=META_PMICW_OP(&req_write);
	if (!config_write.m_rWmCmdWriteResult.status)
    {
        printf("META_PMICW_OP() fail\n");
        return -1;
    }

	req_write.m_rWmCmdWriteReq.registernumber=0x5D;
	req_write.m_rWmCmdWriteReq.registervalue=0x8;
	config_write=META_PMICW_OP(&req_write);
	if (!config_write.m_rWmCmdWriteResult.status)
    {
        printf("META_PMICW_OP() fail\n");
        return -1;
    }

	if (false == Meta_Pmic_Deinit())
    {
        printf("Meta_Pmic_Deinit() fail\n");
        return -1;
    }

	printf("Meta Test pmic AP test : END\n");
	
    return 0;
}
