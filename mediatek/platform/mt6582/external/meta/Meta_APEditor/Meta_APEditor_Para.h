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
 *   Meta_APEditor.h
 *
 * Project:
 * --------
 *   DUMA
 *
 * Description:
 * ------------
 *    header file of main function
 *
 * Author:
 * -------
 *   Nick Huang (mtk02183) 12/09/2009
 *
 *******************************************************************************/


#ifndef __AP_EDITOR_PARA_H__
#define __AP_EDITOR_PARA_H__

#ifdef __cplusplus
extern "C"
{
#endif
    /********************************************
    * Generic Primitives for AP_Editor READ/WRITE
    ********************************************/

    typedef struct
    {
        FT_H			header;         //the header of ft module
        unsigned short  file_idx;       // file lid
        unsigned short  para;           //record id
    } FT_AP_Editor_read_req;

    typedef struct
    {
        FT_H			header;         //the header of ft module
        unsigned short  file_idx;       //file lid
        unsigned short  para;           //record id
        unsigned char   read_status;    //read nvram file status: 0 is fail
        unsigned char	status;         //the status of ft module:  0 is success
    } FT_AP_Editor_read_cnf;


    typedef struct
    {
        FT_H			header;         //the header of ft module
        unsigned short  file_idx;       //file lid
        unsigned short  para;           //record id
    } FT_AP_Editor_write_req;

    typedef struct
    {
        FT_H			header;         //the header of ft module
        unsigned short  file_idx;       //file lid
        unsigned short  para;           //record id
        unsigned char   write_status;   //the write status
        unsigned char	status;         //the status of ft module:  0 is success
    } FT_AP_Editor_write_cnf;


    typedef struct
    {
        FT_H			header;         //the header of ft module
        unsigned char	reset_category;	//0xff indicate reset all files
        unsigned short	file_idx;   	//0xffff indicate reset all files
    } FT_AP_Editor_reset_req;

    typedef struct
    {
        FT_H			header;         //the header of ft module
        unsigned char	reset_status;   //the status of reset file to default value
        unsigned char	status;         //the status of ft module, 0 is success
    } FT_AP_Editor_reset_cnf;


    /* implement these functions in AP_Editor.LIB  */
    /********************************************************************************
    //FUNCTION:
    //		META_Editor_Init
    //DESCRIPTION:
    //		this function is called to initial the meta_editor module.
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
    bool 					META_Editor_Init(void);


    /********************************************************************************
    //FUNCTION:
    //		META_Editor_Deinit
    //DESCRIPTION:
    //		this function is called to de-initial the meta_editor module.
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
    bool 					META_Editor_Deinit(void);

    /********************************************************************************
    //FUNCTION:
    //		META_Editor_ReadFile_OP
    //DESCRIPTION:
    //		this function is called to Read a record of NvRam file from Target side to PC.
    //
    //PARAMETERS:
    //		req:
    //
    //RETURN VALUE:
    //		TRUE: is scuccess, otherwise is fail. the data will be send to PC in the function body
    //
    //DEPENDENCY:
    //		META_Editor_Init must have been called
    //
    //GLOBALS AFFECTED
    ********************************************************************************/
    bool					META_Editor_ReadFile_OP(FT_AP_Editor_read_req *req);

    /********************************************************************************
    //FUNCTION:
    //		META_Editor_WriteFile_OP
    //DESCRIPTION:
    //		this function is called to write a record of NvRam file from PC side to Target.
    //
    //PARAMETERS:
    //		None
    //
    //RETURN VALUE:
    //		refers to the definition of "FT_AP_Editor_write_cnf"
    //
    //DEPENDENCY:
    //		META_Editor_Init must have been called
    //
    //GLOBALS AFFECTED
    //		None
    ********************************************************************************/
    FT_AP_Editor_write_cnf	META_Editor_WriteFile_OP(FT_AP_Editor_write_req *req,
            char *peer_buf,
            unsigned short peer_len);

    /********************************************************************************
    //FUNCTION:
    //		META_Editor_ResetFile_OP
    //DESCRIPTION:
    //		this function is called to reset a NvRam to default value.
    //
    //PARAMETERS:
    //		None
    //
    //RETURN VALUE:
    //		refers to the definition of "FT_AP_Editor_reset_cnf"
    //
    //DEPENDENCY:
    //		META_Editor_Init must have been called
    //
    //GLOBALS AFFECTED
    //		None
    ********************************************************************************/
    FT_AP_Editor_reset_cnf	META_Editor_ResetFile_OP(FT_AP_Editor_reset_req *req);

    /********************************************************************************
    //FUNCTION:
    //		META_Editor_ResetAllFile_OP
    //DESCRIPTION:
    //		this function is called to Reset all of NvRam files to default value.
    //
    //PARAMETERS:
    //		None
    //
    //RETURN VALUE:
    //		refers to the definition of "FT_AP_Editor_reset_cnf"
    //
    //DEPENDENCY:
    //		META_Editor_Init must have been called
    //
    //GLOBALS AFFECTED
    //		None
    ********************************************************************************/
    FT_AP_Editor_reset_cnf	META_Editor_ResetAllFile_OP(FT_AP_Editor_reset_req *req);


#ifdef __cplusplus
}
#endif

#endif
