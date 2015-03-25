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


#ifndef META_PMIC_PARA_H
#define META_PMIC_PARA_H
#include "FT_Public.h"

/************************PMIC***********************/

#if 0 
/* The TestCase Enum define of XXX_module */
typedef enum{
	WM_CMD_WritePMICRegister,
	WM_CMD_ReadPMICRegister
}WM_PMIC_CMD_TYPE;
#endif

/* The command  parameter define of every TestCase */
typedef struct{
	BYTE registernumber;   //the valid value: 20~~ 96
	BYTE registervalue;
}WM_CMD_WritePMICRegister_REQ_T;


typedef struct{
	BYTE	registernumber;	/// the valid value: 0--96
}WM_CMD_ReadPMICRegister_REQ_T;


typedef struct{
	int value;   //not used
	BOOL		status;
}WM_CMD_WriteRegister_CNF_T;

typedef struct{
	BYTE value;
	BOOL		status;
}WM_CMD_ReadRegister_CNF_T;

#if 0
typedef union{
	WM_CMD_WritePMICRegister_REQ_T						m_rWmCmdWriteReq;
	WM_CMD_ReadPMICRegister_REQ_T						m_rWmCmdReadReq;
}WM_PMIC_REQ_CMD_U;

typedef union{
	WM_CMD_WriteRegister_CNF_T						m_rWmCmdWriteResult;
	WM_CMD_ReadRegister_CNF_T						m_rWmCmdReadResult;
}WM_PMIC_CNF_CMD_U;

typedef struct{
	FT_H				header;
	WM_PMIC_CMD_TYPE	type;
	WM_PMIC_REQ_CMD_U	cmd;
}FT_PMIC_COMMAND_REQ;

typedef struct{
	FT_H				header;
	WM_PMIC_CMD_TYPE	type;
	WM_PMIC_CNF_CMD_U	cmd;
	unsigned char		status;
}FT_PMIC_COMMAND_CNF;

#endif

typedef struct 
{
	FT_H							header;
	WM_CMD_ReadPMICRegister_REQ_T	m_rWmCmdReadReq;
}FT_PMIC_REG_READ;

typedef struct 
{
	FT_H						header;
	WM_CMD_ReadRegister_CNF_T	m_rWmCmdReadResult;
	unsigned char				status;
}FT_PMIC_REG_READ_CNF;

typedef struct 
{
	FT_H							header;
	WM_CMD_WritePMICRegister_REQ_T	m_rWmCmdWriteReq;
}FT_PMIC_REG_WRITE;

typedef struct 
{
	FT_H						header;
	WM_CMD_WriteRegister_CNF_T	m_rWmCmdWriteResult;	
	unsigned char				status;
}FT_PMIC_REG_WRITE_CNF;


//Low Power
 typedef enum{
    WM_CMD_POWER_DOWN =0, 
    WM_CMD_POWER_UP,
}  WM_LOW_POWER_TYPE;

 typedef enum{
    LOW_POWER_FAILED=0, 
    LOW_POWER_SUCCESS,
}  WM_LOW_POWER_STATUS;

typedef struct 
{
	FT_H							header;
	WM_LOW_POWER_TYPE	            type;
}FT_LOW_POWER_REQ;

typedef struct 
{
	FT_H							header;
	WM_LOW_POWER_TYPE               type;
	WM_LOW_POWER_STATUS             power_status;
	unsigned char				    status;		
}FT_LOW_POWER_CNF;


FT_LOW_POWER_CNF  		META_LOW_POWER_OP( FT_LOW_POWER_REQ  *req );
FT_PMIC_REG_READ_CNF   	META_PMICR_OP( FT_PMIC_REG_READ  *req );
FT_PMIC_REG_WRITE_CNF	META_PMICW_OP( FT_PMIC_REG_WRITE  *req );

BOOL Meta_Pmic_Init();
BOOL Meta_Pmic_Deinit();
BOOL FM_Low_Power(BOOL bOn);

#endif
