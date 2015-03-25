/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2011
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

#ifndef SEC_KEY_H
#define SEC_KEY_H

/**************************************************************************
 * [SEC-KEY ID]
 **************************************************************************/
#define ROM_INFO_SEC_KEY_ID              "AND_SECRO_v"  //AND_SECKEY_v
#define ROM_INFO_SEC_KEY_VER             0x1

/**************************************************************************
 * [SEC-KEY FORNAT]
 **************************************************************************/
#define AND_SEC_KEY_SIZE                (592)

typedef struct {
    unsigned char                       m_identifier[16];
    unsigned int                        m_sec_key_ver;    

    /* rsa key */
    /* image auth key length is 256/2 = 128 */
    unsigned char                       img_auth_rsa_n[256];
    unsigned char                       img_auth_rsa_e[5];

    /* sml aes key */    
    unsigned char                       sml_aes_key[32];

    /* crypto seed */
    unsigned char                       crypto_seed[16];

    /* rsa key */
    /* sml auth key length is 256/2 = 128 */
    unsigned char                       sml_auth_rsa_n[256];
    unsigned char                       sml_auth_rsa_e[5];

} AND_SECKEY_T;

/**************************************************************************
 * [GENERAL RSA PUBK FORMAT]
 **************************************************************************/
typedef struct RSA_PUBK
{
    unsigned short                      E_Key[256>>1];
    unsigned short                      N_Key[256>>1];
} RSA_PUBK;

#endif /* SEC_KEY_H */

