/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*******************************************************************************
 *
 * Filename:
 * ---------
 *   ATVCtrlService.h
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   ATV Control Service Interface
 *
 * Author:
 * -------
 *   Stan Huang (mtk01728)
 *
 *------------------------------------------------------------------------------
 * $Revision$
 * $Modtime:$
 * $Log:$
 *
 * 10 27 2012 ning.feng
 * [ALPS00367096] [Need Patch] [Volunteer Patch]mt6589 matv/libstagefright enhancement
 * .
 *
 * 07 19 2012 ning.feng
 * [ALPS00315896] [Need Patch] [Volunteer Patch]JB migration
 * .
 *
 * 07 17 2012 ning.feng
 * NULL
 * .
 *
 * 07 17 2012 ning.feng
 * NULL
 * .
 *
 * 07 13 2012 changqing.yan
 * NULL
 * .
 *
 * 12 12 2011 changqing.yan
 * [ALPS00098550] [Need Patch] [Volunteer Patch]migration
 * .
 *
 * 12 12 2011 changqing.yan
 * [ALPS00098550] [Need Patch] [Volunteer Patch]migration
 * .
 *
 * 11 30 2011 changqing.yan
 * [ALPS00098550] [Need Patch] [Volunteer Patch]migration
 * .
 *
 * 11 26 2011 changqing.yan
 * NULL
 * .
 *
 * 01 25 2011 changqing.yan
 * [ALPS00030050] [Need Patch] [Volunteer Patch]
 * .
 *
 *
 *******************************************************************************/

#ifndef ANDROID_IATVCTRLSERVICE_H
#define ANDROID_IATVCTRLSERVICE_H

/*****************************************************************************
*                     C O M P I L E R   F L A G S
******************************************************************************
*/

/*****************************************************************************
*                E X T E R N A L   R E F E R E N C E S
******************************************************************************
*/
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

#include <utils/RefBase.h>
#include <utils/Errors.h>
#include <binder/IInterface.h>
#include <media/IATVCtrlClient.h>

/*****************************************************************************
*                          C O N S T A N T S
******************************************************************************
*/

/*****************************************************************************
*                         D A T A   T Y P E S
******************************************************************************
*/

/*****************************************************************************
*                        C L A S S   D E F I N I T I O N
******************************************************************************
*/

namespace android
{

class IATVCtrlService : public IInterface
{
public:
    DECLARE_META_INTERFACE(ATVCtrlService);

    // IATVCtrlService interface
    // On/Off Control
    virtual int ATVCS_matv_init(void) = 0;
    virtual int ATVCS_matv_ps_init(int on) = 0;
    virtual int ATVCS_matv_set_parameterb(int on) = 0;
    virtual int ATVCS_matv_suspend(int on) = 0;
    virtual int ATVCS_matv_shutdown(void) = 0;

    // Channel Control
    virtual void ATVCS_matv_chscan(int mode) = 0;
    virtual void ATVCS_matv_chscan_stop(void) = 0;

    virtual int ATVCS_matv_get_chtable(int ch, void *entry, int len)  = 0;
    virtual int ATVCS_matv_set_chtable(int ch, void *entry, int len)  = 0;
    virtual int ATVCS_matv_clear_chtable(void) = 0;

    virtual void ATVCS_matv_change_channel(int ch) = 0;
    virtual void ATVCS_matv_set_country(int country) = 0;
    virtual void ATVCS_matv_set_tparam(int mode) = 0;

    // Audio Control
    virtual void ATVCS_matv_audio_play(void) = 0;
    virtual void ATVCS_matv_audio_stop(void) = 0;
    virtual int ATVCS_matv_audio_get_format(void) = 0;
    virtual void ATVCS_matv_audio_set_format(int val) = 0;
    virtual int ATVCS_matv_audio_get_sound_system(void) = 0;

    // Adjustment
    virtual int ATVCS_matv_adjust(int item, int val) = 0;

    // Meta
    virtual int ATVCS_matv_get_chipdep(int item) = 0;
    virtual int ATVCS_matv_set_chipdep(int item, int val) = 0;
    //FM
    virtual int ATVCS_fm_powerup(void *parm, int len) = 0;
    virtual int ATVCS_fm_powerdown(void) = 0;
    virtual int ATVCS_fm_getrssi(void) = 0;
    virtual int ATVCS_fm_tune(void *parm, int len) = 0;
    virtual int ATVCS_fm_seek(void *parm, int len) = 0;
    virtual int ATVCS_fm_scan(void *parm, int len) = 0;
    virtual int ATVCS_fm_mute(int val) = 0;
    virtual int ATVCS_fm_getchipid(void) = 0;
    virtual int ATVCS_fm_isFMPowerUp(void) = 0;

    // Callback
    virtual void ATVCS_matv_register_callback() = 0;

    // register a current process for audio output change notifications
    virtual void registerClient(const sp<IATVCtrlClient>& client) = 0;
    virtual void registerClient_FM(const sp<IATVCtrlClient>& client) = 0;

    // CLI
    virtual void CLI(char input) = 0;
};


class BnATVCtrlService : public BnInterface<IATVCtrlService>
{
public:
    virtual status_t    onTransact(uint32_t code,
                                   const Parcel &data,
                                   Parcel *reply,
                                   uint32_t flags = 0);
};

}; // namespace android

#endif // ANDROID_IATVCTRLSERVICE_H
