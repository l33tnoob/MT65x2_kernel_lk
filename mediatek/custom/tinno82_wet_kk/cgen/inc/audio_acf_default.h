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
 * audio_acf_default.h
 *
 * Project:
 * --------
 *   ALPS
 *
 * Description:
 * ------------
 * This file is the header of audio customization related parameters or definition.
 *
 * Author:
 * -------
 * Tina Tsai
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 *
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#ifndef AUDIO_ACF_DEFAULT_H
#define AUDIO_ACF_DEFAULT_H

   /* Compensation Filter HSF coeffs: default all pass filter       */
    /* BesLoudness also uses this coeffs    */ 
    #define BES_LOUDNESS_HSF_COEFF \
0x7b58f94,   0xf094e0d7,   0x7b58f94,   0x7b43c491,   0x0,     \
0x7af1bfe,   0xf0a1c804,   0x7af1bfe,   0x7ad8c4f4,   0x0,     \
0x7915c50,   0xf0dd4760,   0x7915c50,   0x78e5c6ba,   0x0,     \
0x76dd24d,   0xf1245b65,   0x76dd24d,   0x7689c8cf,   0x0,     \
0x76167cf,   0xf13d3061,   0x76167cf,   0x75b4c987,   0x0,     \
0x728aba0,   0xf1aea8bf,   0x728aba0,   0x71d5ccbf,   0x0,     \
0x6e5fe81,   0xf23402fe,   0x6e5fe81,   0x6d27d067,   0x0,     \
0x6cefaac,   0xf2620aa7,   0x6cefaac,   0x6b82d1a2,   0x0,     \
0x6679cfd,   0xf330c606,   0x6679cfd,   0x63e7d6f4,   0x0,     \
\
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0

    #define BES_LOUDNESS_BPF_COEFF \
0x41919011,   0x32506fee,   0xcc1e0000,     \
0x41b191ba,   0x31366e45,   0xcd170000,     \
0x424299e0,   0x2c46661f,   0xd1760000,     \
0x42e8a49a,   0x26a15b65,   0xd6760000,     \
0x4320a892,   0x24b6576d,   0xd8290000,     \
0x441abbe5,   0x1c35441a,   0xdfb00000,     \
\
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
\
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
\    
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
\    
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
\    
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
\    
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
\
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0
    
    #define BES_LOUDNESS_LPF_COEFF \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0
#define BES_LOUDNESS_WS_GAIN_MAX  0x0
#define BES_LOUDNESS_WS_GAIN_MIN  0x0
#define BES_LOUDNESS_FILTER_FIRST  0x0
#define BES_LOUDNESS_ATT_TIME  0xa4
#define BES_LOUDNESS_REL_TIME  0x4010
#define BES_LOUDNESS_GAIN_MAP_IN \
0xd3, 0xd7, 0xd8, 0xee, 0x0
#define BES_LOUDNESS_GAIN_MAP_OUT \            
0xc, 0xc, 0xc, 0xc, 0x0
         

#endif
