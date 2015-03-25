/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2009
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
 *   bli_exp.h
 *
 * Project:
 * --------
 *
 *
 * Description:
 * ------------
 *   BLI SRC Interface Definition
 *
 * Author:
 * -------
 *   HP Cheng
 *
 *==============================================================================
 *             HISTORY
 * Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *==============================================================================
 *******************************************************************************/

#ifndef BLI_EXP_H
#define BLI_EXP_H

#ifdef __cplusplus
extern "C" {
#endif

    typedef void     BLI_HANDLE;

#ifndef NULL
#define NULL    0
#endif


    /*----------------------------------------------------------------------*/
    /* Get required buffer size for BLI Software SRC                        */
    /*----------------------------------------------------------------------*/
    void BLI_GetMemSize(unsigned int inSR,                  /* Input, input sampling rate of the conversion */
                        unsigned int inChannel,             /* Input, input channel number of the conversion */
                        unsigned int outSR,                 /* Input, output sampling rate of the conversion */
                        unsigned int outChannel,            /* Input, output channel number of the conversion */
                        unsigned int *workBufSize);         /* Output, the required working buffer size in byte */

    /*----------------------------------------------------------------------*/
    /* Get the BLI Software SRC handler.                                    */
    /* Return: the handle of current BLI Software SRC                       */
    /*----------------------------------------------------------------------*/
    BLI_HANDLE *BLI_Open(unsigned int inSR,                 /* Input, input sampling rate of the conversion */
                         unsigned int inChannel,            /* Input, input channel number of the conversion */
                         unsigned int outSR,                /* Input, output sampling rate of the conversion */
                         unsigned int outChannel,           /* Input, output channel number of the conversion */
                         char *buffer,                      /* Input, pointer to the working buffer */
                         void * (*custom_alloc)(unsigned int));

    /*----------------------------------------------------------------------*/
    /* Decompress the bitstream to PCM data                                 */
    /* Return: consumed input buffer size(byte)                             */
    /*----------------------------------------------------------------------*/
    unsigned int BLI_Convert(void *hdl,                  /* Input, handle of this conversion */
                             short *inBuf,               /* Input, pointer to input buffer */
                             unsigned int *inLength,     /* Input, length(byte) of input buffer */
                             /* Output, length(byte) left in the input buffer after conversion */
                             short *outBuf,              /* Input, pointer to output buffer */
                             unsigned int *outLength);   /* Input, length(byte) of output buffer */
    /* Output, output data length(byte) */

    /*----------------------------------------------------------------------*/
    /* Close the process                                                    */
    /*----------------------------------------------------------------------*/
    void BLI_Close(void *hdl,
                   void (*custom_free)(void *));


    /*----------------------------------------------------------------------*/
    /* Change the input sampling rate during the process                    */
    /* Return: error code, 0 represents "everything is OK"                  */
    /*----------------------------------------------------------------------*/
    int BLI_SetSamplingRate(void *hdl,
                            unsigned int inSR);     /* Input, input sampling rate of the conversion */

    /*----------------------------------------------------------------------*/
    /* Clear the internal status for the discontinuous input buffer         */
    /*----------------------------------------------------------------------*/
    int BLI_Reset(void *hdl);

#ifdef __cplusplus
}
#endif

#endif

