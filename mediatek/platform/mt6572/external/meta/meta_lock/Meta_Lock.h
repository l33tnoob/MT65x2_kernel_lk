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


/*******************************************************************************
 *
 * Filename:
 * ---------
 *   Meta_Lock.h
 *
 * Project:
 * --------
 *   YUSU
 *
 * Description:
 * ------------
 *    header file of main function
 *
 * Author:
 * -------
 *   Ning Zhang (mtk80860) 08/17/2010
 *
 *******************************************************************************/


#ifndef __META_LOCK_H__
#define __META_LOCK_H__


#define META_LOCK_MAGIC_NUM		0x58D12AB4
#define END_BLOCK 23*128*1024
#define BLOCK_SIZE 128*1024
#define LOGO_PARTITION 10

#ifdef __cplusplus
extern "C"
{
#endif
    
    /********************************************************************************
    //FUNCTION:
    //		META_Lock_Init
    //DESCRIPTION:
    //		this function is called to initial the meta_Lock module.
    //
    //PARAMETERS:
    //		None
    //
    //RETURN VALUE:
    //		TRUE: is scuccess, otherwise is fail
    //
    //DEPENDENCY:
    //		None
    //
    //GLOBALS AFFECTED
    //		None
    ********************************************************************************/
//    bool 					META_Lock_Init(void);


    /********************************************************************************
    //FUNCTION:
    //		META_Lock_Deinit
    //DESCRIPTION:
    //		this function is called to de-initial the meta_Lock module.
    //
    //PARAMETERS:
    //		None
    //
    //RETURN VALUE:
    //		TRUE: is scuccess, otherwise is fail
    //
    //DEPENDENCY:
    //		META_Editor_Init must have been called
    //
    //GLOBALS AFFECTED
    //		None
    ********************************************************************************/
//    bool 					META_Lock_Deinit(void);

    /********************************************************************************
    //FUNCTION:
    //		META_Lock_OP
    //DESCRIPTION:
    //		this function is called to write a magic number into Nand Flash.
    //
    //PARAMETERS:
    //		req:
    //
    //RETURN VALUE:
    //		TRUE: is scuccess, otherwise is fail. 
    //
    //DEPENDENCY:
    //		META_Lock_Init must have been called
    //
    //GLOBALS AFFECTED
    ********************************************************************************/
    unsigned char					META_Lock_OP(void);
    
    
    static int META_Lock_WriteFlash(unsigned int iMagicNum);

#ifdef __cplusplus
}
#endif
#endif
