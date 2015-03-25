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
 *   ATV Control Service
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
 * 07 30 2012 changqing.yan
 * NULL
 * .
 *
 * 07 24 2012 changqing.yan
 * NULL
 * .
 *
 * 07 13 2012 changqing.yan
 * NULL
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

#ifndef ANDROID_ATVCTRLSERVICE_H
#define ANDROID_ATVCTRLSERVICE_H


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

#include <media/IATVCtrlService.h>
#include <media/IATVCtrlClient.h>
#include <media/AudioSystem.h>


#include <utils/Atomic.h>
#include <utils/Errors.h>
#include <utils/threads.h>
#include <binder/MemoryDealer.h>
#include <utils/KeyedVector.h>
#include <utils/SortedVector.h>
#include <utils/Vector.h>

//#define _MATV_UT_

#ifndef _MATV_UT_
extern "C" {
#include "matvctrl.h"  // provide by LH
#include "kal_release.h"  // provide by LH
#include "fmctrl.h"
}
#endif

/*****************************************************************************
*                          C O N S T A N T S
******************************************************************************
*/

namespace android
{


/*****************************************************************************
*                         D A T A   T Y P E S
******************************************************************************
*/
typedef int kal_bool;
typedef signed char  kal_int8;
typedef unsigned char  kal_uint8;
typedef signed short  kal_int16;
typedef unsigned short  kal_uint16;
typedef signed int    kal_int32;
typedef unsigned int  kal_uint32;

// callback function
typedef  void (*matv_autoscan_progress_cb)(void *cb_param, kal_uint8 precent, kal_uint8 ch, kal_uint8 chnum);
typedef  void (*matv_fullscan_progress_cb)(void *cb_param, kal_uint8 precent, kal_uint32 freq, kal_uint32 freq_start, kal_uint32 freq_end);
typedef  void (*matv_scanfinish_cb)(void *cb_param, kal_uint8 chnum);
typedef  void (*matv_audioformat_cb)(void *cb_param, kal_uint32 format);

#ifdef _MATV_UT_
typedef struct
{
    kal_uint32  freq; //khz
    kal_uint8   sndsys; // reference sv_const.h, TV_AUD_SYS_T ...
    kal_uint8   colsys; // reference sv_const.h, SV_CS_PAL_N, SV_CS_PAL,SV_CS_NTSC358...
    kal_uint8   flag;
} matv_ch_entry;

typedef struct
{
    kal_uint8       mode;
    kal_bool        is_scanning;
    kal_uint8       ch_latest_updated;
    matv_ch_entry       updated_entry;
} matv_chscan_state;

#endif

// state is to prevent the conflic of init and ps_init
// they have race condition toward i2c_fd
#define MATVCTRL_STATE_CLOSE    0
#define MATVCTRL_STATE_OPEN     1
#define MATVCTRL_STATE_PS          2
#define MATVCTRL_STATE_FM_OPEN     3

// ----------------------------------------------------------------------------

#define LIKELY( exp )       (__builtin_expect( (exp) != 0, true  ))
#define UNLIKELY( exp )     (__builtin_expect( (exp) != 0, false ))

// ----------------------------------------------------------------------------



/*****************************************************************************
*                        C L A S S   D E F I N I T I O N
******************************************************************************
*/

class ATVCtrlService : public BnATVCtrlService, public IBinder::DeathRecipient
{
public:
    static void instantiate();

    // ATVCtrlService interface ============================================================
    // On/Off Control
    virtual kal_bool ATVCS_matv_init(void);
    virtual kal_bool ATVCS_matv_ps_init(kal_bool on);
    virtual kal_int32 ATVCS_matv_set_parameterb(kal_int32 in);
    virtual kal_bool ATVCS_matv_suspend(kal_bool on);
    virtual kal_bool ATVCS_matv_shutdown(void);


    // Channel Control
    virtual void ATVCS_matv_chscan(int mode);
    virtual void ATVCS_matv_chscan_stop(void);

    virtual kal_bool ATVCS_matv_get_chtable(int ch, void *entry, int len);
    virtual kal_bool ATVCS_matv_set_chtable(int ch, void *entry, int len);
    virtual kal_bool ATVCS_matv_clear_chtable(void);

    virtual void ATVCS_matv_change_channel(int ch);
    virtual void ATVCS_matv_set_country(int country);

    // Tune
    virtual void ATVCS_matv_set_tparam(int item);


    // Audio Control
    virtual void ATVCS_matv_audio_play(void);
    virtual void ATVCS_matv_audio_stop(void);
    virtual int ATVCS_matv_audio_get_format(void);
    virtual void ATVCS_matv_audio_set_format(int val);
    virtual int ATVCS_matv_audio_get_sound_system(void);

    // Adjustment
    virtual kal_bool ATVCS_matv_adjust(int item, int val);

    // Meta
    virtual kal_int32 ATVCS_matv_get_chipdep(int item);
    virtual kal_bool ATVCS_matv_set_chipdep(int item, int val);

    //FM
    virtual kal_bool ATVCS_fm_powerup(void *parm, int len);
    virtual kal_bool ATVCS_fm_powerdown(void);
    virtual kal_int32 ATVCS_fm_getrssi(void);
    virtual kal_bool ATVCS_fm_tune(void *parm, int len);
    virtual kal_bool ATVCS_fm_seek(void *parm, int len);
    virtual kal_bool ATVCS_fm_scan(void *parm, int len);
    virtual kal_bool ATVCS_fm_mute(int val);
    virtual kal_int32 ATVCS_fm_getchipid(void);
    virtual kal_int32 ATVCS_fm_isFMPowerUp(void);
    // Callback
    virtual void ATVCS_matv_register_callback();

    // CLI
    virtual void CLI(char input);

    static void atv_autoscan_progress_cb(void *cb_param, kal_uint8 precent, kal_uint8 ch, kal_uint8 chnum);
    static void atv_fullscan_progress_cb(void *cb_param, kal_uint8 precent, kal_uint32 freq, kal_uint32 freq_start, kal_uint32 freq_end);
    static void atv_scanfinish_cb(void *cb_param, kal_uint8 chnum);
    static void atv_audioformat_cb(void *cb_param, kal_uint32 format);
    static void atv_shutdown_cb(void *cb_param, kal_uint32 source);


    /*
       void ATVCS_matv_register_callback(void* cb_param,
           matv_autoscan_progress_cb auto_cb,
           matv_fullscan_progress_cb full_cb,
           matv_scanfinish_cb finish_cb,
           matv_audioformat_cb audfmt_cb);
    */


    // register a current process for notifications
    virtual void registerClient(const sp<IATVCtrlClient>& client);
    virtual void registerClient_FM(const sp<IATVCtrlClient>& client);

    // IBinder::DeathRecipient
    virtual     void        binderDied(const wp<IBinder>& who);

    virtual     status_t    onTransact(
        uint32_t code,
        const Parcel &data,
        Parcel *reply,
        uint32_t flags);

    sp<IATVCtrlClient>      mClient;
    sp<IATVCtrlClient>      mClient_FM;
    matv_chscan_state       scan_state;

protected:
    /*
       matv_autoscan_progress_cb m_auto_cb;
        matv_fullscan_progress_cb m_full_cb;
        matv_scanfinish_cb        m_finish_cb;
        matv_audioformat_cb       m_audfmt_cb;
    */
private:
    ATVCtrlService();
    virtual                 ~ATVCtrlService();
    SortedVector< wp<IBinder> >         mNotificationClients;
    mutable     Mutex         mLock;
    int                               mState;
    //   sp<IATVCtrlClient>      mClient;


};

// ----------------------------------------------------------------------------

}; // namespace android

#endif // ANDROID_ATVCTRLSERVICE_H
