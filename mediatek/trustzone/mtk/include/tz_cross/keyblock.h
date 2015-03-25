/*----------------------------------------------------------------------------*
 * Copyright Statement:                                                       *
 *                                                                            *
 *   This software/firmware and related documentation ("MediaTek Software")   *
 * are protected under international and related jurisdictions'copyright laws *
 * as unpublished works. The information contained herein is confidential and *
 * proprietary to MediaTek Inc. Without the prior written permission of       *
 * MediaTek Inc., any reproduction, modification, use or disclosure of        *
 * MediaTek Software, and information contained herein, in whole or in part,  *
 * shall be strictly prohibited.                                              *
 * MediaTek Inc. Copyright (C) 2010. All rights reserved.                     *
 *                                                                            *
 *   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND     *
 * AGREES TO THE FOLLOWING:                                                   *
 *                                                                            *
 *   1)Any and all intellectual property rights (including without            *
 * limitation, patent, copyright, and trade secrets) in and to this           *
 * Software/firmware and related documentation ("MediaTek Software") shall    *
 * remain the exclusive property of MediaTek Inc. Any and all intellectual    *
 * property rights (including without limitation, patent, copyright, and      *
 * trade secrets) in and to any modifications and derivatives to MediaTek     *
 * Software, whoever made, shall also remain the exclusive property of        *
 * MediaTek Inc.  Nothing herein shall be construed as any transfer of any    *
 * title to any intellectual property right in MediaTek Software to Receiver. *
 *                                                                            *
 *   2)This MediaTek Software Receiver received from MediaTek Inc. and/or its *
 * representatives is provided to Receiver on an "AS IS" basis only.          *
 * MediaTek Inc. expressly disclaims all warranties, expressed or implied,    *
 * including but not limited to any implied warranties of merchantability,    *
 * non-infringement and fitness for a particular purpose and any warranties   *
 * arising out of course of performance, course of dealing or usage of trade. *
 * MediaTek Inc. does not provide any warranty whatsoever with respect to the *
 * software of any third party which may be used by, incorporated in, or      *
 * supplied with the MediaTek Software, and Receiver agrees to look only to   *
 * such third parties for any warranty claim relating thereto.  Receiver      *
 * expressly acknowledges that it is Receiver's sole responsibility to obtain *
 * from any third party all proper licenses contained in or delivered with    *
 * MediaTek Software.  MediaTek is not responsible for any MediaTek Software  *
 * releases made to Receiver's specifications or to conform to a particular   *
 * standard or open forum.                                                    *
 *                                                                            *
 *   3)Receiver further acknowledge that Receiver may, either presently       *
 * and/or in the future, instruct MediaTek Inc. to assist it in the           *
 * development and the implementation, in accordance with Receiver's designs, *
 * of certain softwares relating to Receiver's product(s) (the "Services").   *
 * Except as may be otherwise agreed to in writing, no warranties of any      *
 * kind, whether express or implied, are given by MediaTek Inc. with respect  *
 * to the Services provided, and the Services are provided on an "AS IS"      *
 * basis. Receiver further acknowledges that the Services may contain errors  *
 * that testing is important and it is solely responsible for fully testing   *
 * the Services and/or derivatives thereof before they are used, sublicensed  *
 * or distributed. Should there be any third party action brought against     *
 * MediaTek Inc. arising out of or relating to the Services, Receiver agree   *
 * to fully indemnify and hold MediaTek Inc. harmless.  If the parties        *
 * mutually agree to enter into or continue a business relationship or other  *
 * arrangement, the terms and conditions set forth herein shall remain        *
 * effective and, unless explicitly stated otherwise, shall prevail in the    *
 * event of a conflict in the terms in any agreements entered into between    *
 * the parties.                                                               *
 *                                                                            *
 *   4)Receiver's sole and exclusive remedy and MediaTek Inc.'s entire and    *
 * cumulative liability with respect to MediaTek Software released hereunder  *
 * will be, at MediaTek Inc.'s sole discretion, to replace or revise the      *
 * MediaTek Software at issue.                                                *
 *                                                                            *
 *   5)The transaction contemplated hereunder shall be construed in           *
 * accordance with the laws of Singapore, excluding its conflict of laws      *
 * principles.  Any disputes, controversies or claims arising thereof and     *
 * related thereto shall be settled via arbitration in Singapore, under the   *
 * then current rules of the International Chamber of Commerce (ICC).  The    *
 * arbitration shall be conducted in English. The awards of the arbitration   *
 * shall be final and binding upon both parties and shall be entered and      *
 * enforceable in any court of competent jurisdiction.                        *
 *---------------------------------------------------------------------------*/
#ifndef __KEY_BLOCK_H__
#define __KEY_BLOCK_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    WIDEVINE_ID = 0,
    MARLIN_ID,
    HDCP_1X_TX_ID,
    HDCP_2X_V1_TX_ID,
    HDCP_2X_V1_RX_ID,
    HDCP_2X_V2_TX_ID,
    HDCP_2X_V2_RX_ID,
    PLAYREADY_BGROUPCERT_ID,
    PLAYREADY_ZGPRIV_ID,
    PLAYREADY_KEYFILE_ID,
    DRM_KEY_MAX,
    DRM_SP_EKKB = 0xFFFF
}
DRMKeyID;

#define SZ_DRMKEY_ID 4
#define SZ_DRMKEY_TYPE 4
#define SZ_DRMKEY_SIZE 4
#define SZ_DRMKEY_ENC_SIZE 4
#define SZ_DRMKEY_RESERVED 64
#define SZ_DRMKEY_HEADER_SIZE (SZ_DRMKEY_ID+SZ_DRMKEY_TYPE+SZ_DRMKEY_SIZE+SZ_DRMKEY_ENC_SIZE+SZ_DRMKEY_RESERVED)
#define SZ_DRMKEY_SIG 16

//begin of uree using

/*
[in] keyID			  Enum DRMKeyID
[out] oneDrmkeyBlock  encrypt DRMBlock
[out] blockLeng 	  encrypt DRMBlockLength

return	 0: OK,  others: FAIL
*/
int get_encrypt_drmkey ( unsigned int keyID, unsigned char **oneDrmkeyBlock, unsigned int *blockLeng );

int get_clearDrmkey_size( unsigned int keyID, unsigned int *leng);

int free_encrypt_drmkey ( unsigned char *oneEncDrmkeyBlock );

int write_kbo_drmkey(DRMKeyID id,unsigned char* enckey,unsigned int length);

int delete_kbo_drmkey(DRMKeyID id);

int install_KB_OTA_API(unsigned char* buff,unsigned int len);

int query_drmkey ( unsigned int *count, unsigned int *keytype );

//end of uree using

//begin for tee using
int encrypt_drmkey(DRMKeyID id,unsigned char* clearKey,unsigned int inLength,
				   unsigned char** encKey,unsigned int* outLength);

int decrypt_drmkey ( unsigned char *encDrmKeyBlock, unsigned int inLength,
					 unsigned char **DrmKey, unsigned int *outLength );

int free_drmkey ( unsigned char *drmkey );

int free_drmkey_safe ( unsigned char *drmkey,int size);

//end for tee using

#ifdef __cplusplus
}
#endif

#endif /* __KEY_BLOCK_H__ */
