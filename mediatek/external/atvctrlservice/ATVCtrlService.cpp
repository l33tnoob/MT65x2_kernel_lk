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
 *   ATVCtrlService.cpp
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
 * 07 13 2012 changqing.yan
 * NULL
 * .
 *
 * 04 16 2012 changqing.yan
 * [ALPS00266565] [ICS][Free Test][Mobile TV]Launch Camera when recording TV,back to playing screen after prompt error
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
 * 11 22 2011 changqing.yan
 * [ALPS00095391] [Need Patch] [Volunteer Patch]
 * .
 *
 * 06 08 2011 changqing.yan
 * [ALPS00050556] [mATV]ANR happen after launch camera while scanning
 * .
 *
 * 06 08 2011 changqing.yan
 * [ALPS00050556] [mATV]ANR happen after launch camera while scanning
 * .
 *
 * 01 25 2011 changqing.yan
 * [ALPS00030050] [Need Patch] [Volunteer Patch]
 * .
 *
 *
 *******************************************************************************/

/*****************************************************************************
*                     C O M P I L E R   F L A G S
******************************************************************************
*/
//#define POWER_SAVING

/*****************************************************************************
*                E X T E R N A L   R E F E R E N C E S
******************************************************************************
*/
#define LOG_TAG "ATVCtrlService"
#include <math.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <binder/IServiceManager.h>
#include <utils/Log.h>
#include <binder/Parcel.h>
#include <binder/IPCThreadState.h>
#include <utils/String16.h>
#include <utils/threads.h>
#include <cutils/properties.h>
#include <cutils/xlog.h>
#include "ATVCtrlService.h"

/*****************************************************************************
*                          C O N S T A N T S
******************************************************************************
*/
#define TP_SINGLE     0     //Touch panel scan in single line mode
#define TP_MULTIPLE 1            //Touch panel scan in multiple line mode
#define TP_SLEEP       2            //Touch panel go to sleep
#define TP_RESUME    3            //Resume from sleep


/*****************************************************************************
*                         D A T A   T Y P E S
******************************************************************************
*/


/*****************************************************************************
*                        C L A S S   D E F I N I T I O N
******************************************************************************
*/
long long getTimeMs()
{
    struct timeval t1;
    long long ms;

    gettimeofday(&t1, NULL);
    ms = t1.tv_sec * 1000LL + t1.tv_usec / 1000;

    return ms;
}


// ----------------------------------------------------------------------------
// the sim build doesn't have gettid

#ifndef HAVE_GETTID
# define gettid getpid
#endif

// ----------------------------------------------------------------------------
extern int matv_use_analog_input;//from libaudiosetting.so

namespace android
{

#ifdef _MATV_UT_
int matv_init(void)
{
    SXLOGD("_UT_ matv_init()");
    return 1;
}

int matv_suspend(int on)
{
    SXLOGD("_UT_ matv_suspend() on=%d", on);
    return 1;
}

int matv_shutdown(void)
{
    SXLOGD("_UT_ matv_shutdown()");
    return 1;
}

void matv_chscan(kal_uint8 mode)
{
    SXLOGD("_UT_ matv_chscan() mode=%d", mode);
}
void matv_chscan_stop(void)
{
    SXLOGD("_UT_ matv_chscan_stop()");
}
void matv_register_callback(void *cb_param,
                            matv_autoscan_progress_cb auto_cb,
                            matv_fullscan_progress_cb full_cb,
                            matv_scanfinish_cb finish_cb,
                            matv_audioformat_cb audfmt_cb)
{

    //   auto_cb(cb_param,1,2,3);
    //   full_cb(cb_param,1,2,3,5);
    //   finish_cb(cb_param,1);
    //   audfmt_cb(cb_param,1);

    SXLOGD("_UT_ matv_register_callback() cb_param=%x, auto_cb=%x, full_cb=%x, finish_cb=%x, audfmt_cb=%x", cb_param, auto_cb, full_cb, finish_cb, audfmt_cb);
}

kal_bool matv_get_chtable(kal_uint8 ch, matv_ch_entry *entry)
{
    SXLOGD("_UT_ matv_get_chtable() ch=%d, entry=%x", ch, entry);
    return 1;
}

kal_bool matv_set_chtable(kal_uint8 ch, matv_ch_entry *entry)
{
    SXLOGD("_UT_ matv_set_chtable() ch=%d, entry=%x", ch, entry);
    return 1;
}

int matv_clear_chtable(void)
{
    SXLOGD("_UT_ matv_clear_chtable()");
    return 1;
}

void matv_change_channel(kal_uint8 ch)
{
    SXLOGD("_UT_ matv_change_channel() ch=%d", ch);
}
void matv_set_country(kal_uint8 country)
{
    SXLOGD("_UT_ matv_set_country() country=%d", country);
}

void matv_chscan_query(matv_chscan_state *scan_state)
{

    scan_state->updated_entry.flag = true;
    SXLOGD("_UT_ matv_chscan_query() scan_state=%x", scan_state);
}
void matv_audio_play(void)
{
    SXLOGD("_UT_ matv_audio_play()");
}
void matv_audio_stop(void)
{
    SXLOGD("_UT_ matv_audio_stop()");
}
kal_uint32 matv_audio_get_format(void)
{
    SXLOGD("_UT_ matv_audio_get_format()");
    return 1;
}

void matv_audio_set_format(kal_uint32 val)
{
    SXLOGD("_UT_ matv_audio_set_format() val=%d", val);
}
kal_uint8 matv_audio_get_sound_system(void)
{
    SXLOGD("_UT_ matv_audio_get_sound_system()");
    return 1;
}

kal_uint8 matv_audio_get_fm_detect_status(kal_uint32 *qulity)
{
    SXLOGD("_UT_ matv_audio_get_fm_detect_status()");
    return 1;
}

int matv_get_chipdep(kal_uint8 item)
{
    SXLOGD("_UT_ matv_get_chipdep() item=%d", item);
    return 1;
}

int matv_set_chipdep(kal_uint8 item, kal_int32 val)
{
    SXLOGD("_UT_ matv_set_chipdep() item=%d, val=%d", item, val);
    return 1;
}

int matv_adjust(kal_uint8 item, kal_int32 val)
{
    SXLOGD("_UT_ matv_adjust() item=%d, val=%d", item, val);
    return 1;
}
#endif

#if 0
kal_bool fm_powerup(struct fm_tune_parm *parm)
{
    SXLOGD("_UT_ fm_powerup() parm=%x", parm);
    return 1;
}
kal_bool fm_powerdown(void)
{
    SXLOGD("_UT_ fm_powerdown()");
    return 1;
}
int fm_getrssi(void)
{
    SXLOGD("_UT_ fm_getrssi()");
    return 1;
}
kal_bool fm_tune(struct fm_tune_parm *parm)
{
    SXLOGD("_UT_ fm_tune() parm=%x", parm);
    return 1;
}
kal_bool fm_seek(struct fm_seek_parm *parm)
{
    SXLOGD("_UT_ fm_seek() parm=%x", parm);
    return 1;
}
kal_bool fm_scan(struct fm_scan_parm *parm)
{
    SXLOGD("_UT_ fm_scan() parm=%x", parm);
    return 1;
}
kal_bool fm_mute(kal_uint32 val)
{
    SXLOGD("_UT_ fm_mute() val=%d", val);
    return 1;
}
int fm_getchipid(void)
{
    SXLOGD("_UT_ fm_getchipid()");
    return 1;
}

#endif
// for bounding MTK Chip
static int parameterb;
static int isfmdrvpd = 0;
static int isatvdrvpd = 0;
static int fm_psFlag = 0;
static int atvaud_Flag = 0;

ATVCtrlService::ATVCtrlService()
    : BnATVCtrlService()
{
    SXLOGD("ATVCtrlService()");
    atvaud_Flag = 0;
    parameterb = 0;
#ifdef POWER_SAVING
    SXLOGD("[73+92PS] In constructor, force to ps init state");
    matv_ps_init(1);
    mState = MATVCTRL_STATE_PS;
#else
    mState = MATVCTRL_STATE_CLOSE;
#endif
}

ATVCtrlService::~ATVCtrlService()
{
    atvaud_Flag = 0;
    SXLOGD("~ATVCtrlService()");
}

void ATVCtrlService::instantiate()
{
    SXLOGD("ATVCtrlService::instantiate()");

    ATVCtrlService *mATVCtrlService = new ATVCtrlService();
    SXLOGD("Pass verification");
    defaultServiceManager()->addService(
        String16("media.ATVCtrlService"), mATVCtrlService);
}

void ATVCtrlService::registerClient(const sp<IATVCtrlClient>& client)
{
    SXLOGD("registerClient() %p, tid %d, calling tid %d", client.get(), gettid(), IPCThreadState::self()->getCallingPid());
    Mutex::Autolock _l(mLock);

    mClient = client;

    sp<IBinder> binder = client->asBinder();

    if (mNotificationClients.indexOf(binder) < 0)
    {
        SXLOGD("Adding notification client %p", binder.get());
        binder->linkToDeath(this);
        mNotificationClients.add(binder);
    }
}

void ATVCtrlService::registerClient_FM(const sp<IATVCtrlClient>& client)
{
    SXLOGD("registerClient_FM() %p, tid %d, calling tid %d", client.get(), gettid(), IPCThreadState::self()->getCallingPid());
    Mutex::Autolock _l(mLock);

    mClient_FM = client;

    sp<IBinder> binder = client->asBinder();

    if (mNotificationClients.indexOf(binder) < 0)
    {
        SXLOGD("Adding notification client %p", binder.get());
        binder->linkToDeath(this);
        mNotificationClients.add(binder);
    }
}
void ATVCtrlService::CLI(char input)
{
    SXLOGD("CLI() tid %d, calling tid %d", gettid(), IPCThreadState::self()->getCallingPid());
    Mutex::Autolock _l(mLock);
    //SXLOGD("CLI input is %x",input);
    matv_cli_input((kal_uint8)input);
}

void ATVCtrlService::binderDied(const wp<IBinder>& who)
{
    SXLOGD("binderDied() %p, tid %d, calling tid %d", who.unsafe_get(), gettid(), IPCThreadState::self()->getCallingPid());
    Mutex::Autolock _l(mLock);

    IBinder *binder = who.unsafe_get();

    if (binder != NULL)
    {
        int index = mNotificationClients.indexOf(binder);

        if (index >= 0)
        {
            SXLOGD("Removing notification client %p", binder);
            mNotificationClients.removeAt(index);
        }
    }

    sp<IBinder> binder_atv = mClient->asBinder();
    sp<IBinder> binder_fm = mClient_FM->asBinder();

    if (binder == binder_atv.get())
    {
        SXLOGD("binderDied: atv client");
        // once application died, we resume TP
        ATVCS_matv_set_tparam(TP_RESUME);     //resume
        ATVCS_matv_set_tparam(TP_MULTIPLE);       //set to normal scan mode

        // when bider died, and state is open, just do shut down
        // only ATV apk died, this will be called. Because ATVCtrl is the only registered client
        if (mState == MATVCTRL_STATE_OPEN)
        {
            kal_bool ret;
            SXLOGW("Binder died when state = %d, force to shutdown", mState);
            ret = matv_shutdown();

            if (ret == false)
            {
                SXLOGE("ATVCS_matv_shutdown fail");
            }

            mState = MATVCTRL_STATE_CLOSE;
        }
    }
    else if (binder == binder_fm.get())
    {
        SXLOGD("binderDied: fm client");

        if (mState == MATVCTRL_STATE_FM_OPEN)
        {
            kal_bool ret;
            SXLOGW("Binder died when state = %d, force to shutdown FM", mState);
            ret = fm_powerdown(0);

            if (ret == false)
            {
                SXLOGE("ATVCS_fm_powerdown fail");
            }

            mState = MATVCTRL_STATE_CLOSE;
        }
    }
}

status_t ATVCtrlService::onTransact(
    uint32_t code, const Parcel &data, Parcel *reply, uint32_t flags)
{
    //   SXLOGD("onTransact()");
    return BnATVCtrlService::onTransact(code, data, reply, flags);
}

kal_bool ATVCtrlService::ATVCS_matv_init(void)
{
    Mutex::Autolock _l(mLock);

    kal_bool ret;
    SXLOGD("ATVCS_matv_init");
    /*
       SXLOGD("time before matv_ps_init(1) = %lld", getTimeMs());
       ret = matv_ps_init(1);
       SXLOGD("time after matv_ps_init(1) = %lld", getTimeMs());

       usleep(100000);

       SXLOGD("time before matv_ps_init(0) = %lld", getTimeMs());
       ret = matv_ps_init(0);
       SXLOGD("time after matv_ps_init(0) = %lld", getTimeMs());
    */

    //if calling init in wrong state, do not change state, just return
    if (mState == MATVCTRL_STATE_OPEN)
    {
        SXLOGW("Calling ATVCS_matv_init when state = %d, skip", mState);
        return true;
    }
    else if (mState == MATVCTRL_STATE_PS)
    {
#ifdef POWER_SAVING
        SXLOGD("[73+92PS] Calling ATVCS_matv_init when state ps, leave ps_init and then do init");
        matv_ps_init(0);
#else
        SXLOGW("Calling ATVCS_matv_init when state = %d, skip with return false", mState);
        return false;
#endif
    }
    else if (mState == MATVCTRL_STATE_FM_OPEN)
    {
        SXLOGW("Calling ATVCS_matv_init when state = %d, powerdown fm before matv init", mState);
        ret = fm_powerdown(0);

        if (ret == true)
        {
            isfmdrvpd = 1;
            SXLOGD("ATVCS_fm_powerdown success");

            if (mClient_FM.get() != NULL)
            {
                mClient_FM->notify(0, 0, 0);
                SXLOGD("notify FM application");
            }
        }
        else
        {
            SXLOGD("ATVCS_fm_powerdown fail");
            return false;
        }
    }

    ret = matv_init();
    parameterb = 1;
    mState = MATVCTRL_STATE_OPEN;

    if (ret == false)
    {
        SXLOGD("ATVCS_matv_init fail");
        parameterb = 0;
        mState = MATVCTRL_STATE_CLOSE;
    }

    return ret;
}

kal_bool ATVCtrlService::ATVCS_matv_ps_init(kal_bool on)
{
    Mutex::Autolock _l(mLock);

    kal_bool ret;
    SXLOGD("ATVCS_matv_ps_init");

#ifdef POWER_SAVING
    SXLOGD("[73+92PS] Calling ATVCS_matv_ps_init [%d] is useless, skip");
    return true;
#endif

    //if calling ps_init in wrong state, do not change state, just return
    if (mState == MATVCTRL_STATE_OPEN)
    {
        SXLOGW("Calling ATVCS_matv_ps_init when state = %d, skip", mState);
        return false;
    }
    else if (mState == MATVCTRL_STATE_FM_OPEN)
    {
        SXLOGW("Calling ATVCS_matv_ps_init when state = %d, keep psFlag", mState);
        fm_psFlag = on;
        return false;
    }

    ret = matv_ps_init(on);

    if (ret == false)
    {
        SXLOGD("ATVCS_matv_ps_init fail");
        mState = MATVCTRL_STATE_CLOSE;
    }

    if (on == 1)
    {
        mState = MATVCTRL_STATE_PS;
    }
    else
    {
        mState = MATVCTRL_STATE_CLOSE;
    }

    return ret;
}

kal_int32 ATVCtrlService::ATVCS_matv_set_parameterb(kal_int32 in)
{
    kal_int32 ret = 0;

	Mutex::Autolock _l(mLock);

    SXLOGD("ATVCS_matv_set_parameterb");

    int a = 7393;
    int b = 739;
    int c = 73939;

    // for bounding MTK ATV Chip
    if (parameterb == 1)
    {
        SXLOGD("ATVCS_matv_set_parameterb success");
        ret = (in + a) * b + c;
    }
    else
    {
        SXLOGD("ATVCS_matv_set_parameterb fail");
        ret = (in + a) * b - c;
    }

    return ret;

}

kal_bool ATVCtrlService::ATVCS_matv_suspend(kal_bool on)
{
    kal_bool ret;
    SXLOGD("ATVCS_matv_suspend on=%d", on);

    ret = matv_suspend(on);

    if (ret == false)
    {
        SXLOGD("ATVCS_matv_suspend fail");
    }

    return ret;
}

kal_bool ATVCtrlService::ATVCS_matv_shutdown(void)
{
    Mutex::Autolock _l(mLock);

    kal_bool ret;
    SXLOGD("ATVCS_matv_shutdown");

#ifdef POWER_SAVING

    if (mState == MATVCTRL_STATE_CLOSE || mState == MATVCTRL_STATE_FM_OPEN)
    {
        SXLOGD("[73+92PS] Calling Shutdown when state = %d, skip ", mState);
        return true;
    }

    SXLOGD("[73+92PS] state = %d, Shutdown then, enter ps init ");
    ret = matv_shutdown();
    ret = matv_ps_init(1);

    if (ret == true)
    {
        SXLOGD("Force to callback for shutdown");
        atv_shutdown_cb(this, 0);
    }
    else
    {
        SXLOGD("ATVCS_matv_shutdown fail");
    }

    mState = MATVCTRL_STATE_PS;
    return ret;
#endif

    if (mState == MATVCTRL_STATE_FM_OPEN && isatvdrvpd)
    {
        SXLOGW("Calling ATVCS_matv_powerdown when state = %d && isatvdrvpd, skip", mState);
        isatvdrvpd = 0;
        return true;
    }
    //if calling shutdown in wrong state, do not change state, just return
    else if (mState == MATVCTRL_STATE_CLOSE || mState == MATVCTRL_STATE_FM_OPEN || mState == MATVCTRL_STATE_PS)
    {
        SXLOGW("Calling ATVCS_matv_shutdown when state = %d, skip", mState);
        return false;
    }

    ret = matv_shutdown();

    if (ret == true)
    {
        SXLOGD("Force to callback for shutdown");
        atv_shutdown_cb(this, 0);
    }
    else
    {
        SXLOGD("ATVCS_matv_shutdown fail");
    }

    mState = MATVCTRL_STATE_CLOSE;
    return ret;

}

void ATVCtrlService::ATVCS_matv_chscan(int mode)
{
    Mutex::Autolock _l(mLock);

    if (mState != MATVCTRL_STATE_OPEN)
    {
        atv_scanfinish_cb(this, 0);
        return;
    }

    SXLOGD("ATVCS_matv_chscan mode=%d", mode);
    matv_chscan((kal_uint8)mode);
}

void ATVCtrlService::ATVCS_matv_chscan_stop(void)
{
    SXLOGD("ATVCS_matv_chscan_stop");
    matv_chscan_stop();
}

kal_bool ATVCtrlService::ATVCS_matv_get_chtable(int ch, void *entry, int len)
{
    SXLOGD("ATVCS_matv_get_chtable ch=%d, entry=%p, len=%d", ch, entry, len);
    kal_bool ret;
    matv_ch_entry *entry_l;
    kal_uint8 ch_l = (kal_uint8)ch;

    entry_l = (matv_ch_entry *)entry;

    ret = matv_get_chtable(ch_l, entry_l);

    if (ret == false)
    {
        SXLOGD("ATVCS_matv_get_chtable fail");
    }

    return ret;
}

kal_bool ATVCtrlService::ATVCS_matv_set_chtable(int ch, void *entry, int len)
{
    SXLOGD("ATVCS_matv_set_chtable ch=%d, entry=%p, len=%d", ch, entry, len);
    kal_bool ret;
    matv_ch_entry *entry_l;
    kal_uint8 ch_l = (kal_uint8)ch;

    entry_l = (matv_ch_entry *)entry;


    ret = matv_set_chtable(ch_l, entry_l);

    if (ret == false)
    {
        SXLOGD("ATVCS_matv_set_chtable fail");
    }

    return ret;
}

kal_bool ATVCtrlService::ATVCS_matv_clear_chtable(void)
{
    SXLOGD("ATVCS_matv_clear_chtable");
    kal_bool ret;
    ret = matv_clear_chtable();

    if (ret == false)
    {
        SXLOGD("ATVCS_matv_clear_chtable fail");
    }

    return ret;
}

void ATVCtrlService::ATVCS_matv_change_channel(int ch)
{
    SXLOGD("ATVCS_matv_clear_chtable ch=%d", ch);

    if ((atvaud_Flag == 1) && (matv_use_analog_input == 1))
    {
        AudioSystem::setParameters(0, (String8)"AtvAudioLineInEnable=0");
    }

    matv_change_channel((kal_uint8)ch);

    if ((atvaud_Flag == 1) && (matv_use_analog_input == 1))
    {
        AudioSystem::setParameters(0, (String8)"AtvAudioLineInEnable=1");
    }
}

void ATVCtrlService::ATVCS_matv_set_country(int country)
{
    Mutex::Autolock _l(mLock);

    if (mState != MATVCTRL_STATE_OPEN)
    {
        return;
    }

    SXLOGD("ATVCS_matv_set_country country=%d", country);
    matv_set_country((kal_uint8)country);
}

void ATVCtrlService::ATVCS_matv_set_tparam(int mode)
{
    SXLOGD("ATVCS_matv_set_tparam mode=%d", mode);
    matv_SetTpMode(mode);

}

void ATVCtrlService::ATVCS_matv_audio_play(void)
{
    SXLOGD("ATVCS_matv_audio_play");
    matv_audio_play();
    atvaud_Flag = 1;
}

void ATVCtrlService::ATVCS_matv_audio_stop(void)
{
    SXLOGD("ATVCS_matv_audio_stop");
    matv_audio_stop();
    atvaud_Flag = 0;
}

int ATVCtrlService::ATVCS_matv_audio_get_format(void)
{
    SXLOGD("ATVCS_matv_audio_get_format");
    int ret = matv_audio_get_format();
    return ret;
}

void ATVCtrlService::ATVCS_matv_audio_set_format(int val)
{
    SXLOGD("ATVCS_matv_audio_set_format val=%d", val);
    matv_audio_set_format((kal_uint32)val);
}

kal_bool ATVCtrlService::ATVCS_matv_adjust(int item, int val)
{
    SXLOGD("ATVCS_matv_adjust item=%d, val=%d", item, val);
    kal_bool ret;
    ret = matv_adjust((kal_uint8)item, (kal_int32)val);

    if (ret == false)
    {
        SXLOGD("ATVCS_matv_adjust fail");
    }

    return ret;
}

int ATVCtrlService::ATVCS_matv_audio_get_sound_system(void)
{
    SXLOGD("ATVCS_matv_audio_get_sound_system");
    kal_uint8 ret = matv_audio_get_sound_system();
    return (int)ret;
}

kal_int32 ATVCtrlService::ATVCS_matv_get_chipdep(int item)
{
    SXLOGD("ATVCS_matv_get_chipdep item=%d", item);
    kal_int32 ret = matv_get_chipdep((kal_uint8)item);
    return ret;
}

kal_bool ATVCtrlService::ATVCS_matv_set_chipdep(int item, kal_int32 val)
{
    SXLOGD("ATVCS_matv_set_chipdep item=%d, val=%d", item, val);
    kal_bool ret;
    ret = matv_set_chipdep((kal_uint8)item, val);

    if (ret == false)
    {
        SXLOGD("ATVCS_matv_set_chipdep fail");
    }

    return ret;
}

kal_bool ATVCtrlService::ATVCS_fm_powerup(void *parm, int len)
{
    Mutex::Autolock _l(mLock);

    kal_bool ret;
    struct fm_tune_parm *parm_l;
    SXLOGD("ATVCS_fm_powerup parm=%p, len=%d", parm, len);

    //if calling init in wrong state, do not change state, just return
    if (mState == MATVCTRL_STATE_FM_OPEN)
    {
        SXLOGW("Calling ATVCS_fm_powerup when state = %d, skip", mState);
        return true;
    }
    else if (mState == MATVCTRL_STATE_PS)
    {
        SXLOGW("Calling ATVCS_fm_powerup when state = %d, skip with return false", mState);
        return false;
    }
    else if (mState == MATVCTRL_STATE_OPEN)
    {
        SXLOGW("Calling ATVCS_fm_powerup when state = %d, powerdown atv before fm init", mState);
        ret = matv_shutdown();

        if (ret == true)
        {
            isatvdrvpd = 1;
            SXLOGD("ATVCS_matv_shutdown success");
        }
        else
        {
            SXLOGD("ATVCS_matv_shutdown fail");
            return false;
        }
    }

    parm_l = (struct fm_tune_parm *)parm;
    ret = fm_powerup(parm_l);
    parameterb = 1;
    mState = MATVCTRL_STATE_FM_OPEN;

    if (ret == false)
    {
        SXLOGD("ATVCS_fm_powerup fail");
        parameterb = 0;
        mState = MATVCTRL_STATE_CLOSE;
    }

    return ret;
}

kal_bool ATVCtrlService::ATVCS_fm_powerdown(void)
{
    Mutex::Autolock _l(mLock);

    kal_bool ret;
    SXLOGD("ATVCS_fm_powerdown");

    if (mState == MATVCTRL_STATE_OPEN && isfmdrvpd)
    {
        SXLOGW("Calling ATVCS_fm_powerdown when state = %d && isfmdrvpd, skip", mState);
        isfmdrvpd = 0;
        return true;
    }
    //if calling shutdown in wrong state, do not change state, just return
    else if (mState == MATVCTRL_STATE_OPEN || mState == MATVCTRL_STATE_CLOSE || mState == MATVCTRL_STATE_PS)
    {
        SXLOGW("Calling ATVCS_fm_powerdown when state = %d, skip", mState);
        return false;
    }

#if 0
    ret = fm_powerdown();

    //TODO?
    if (ret == true)
    {
        SXLOGD("Force to callback for powerdown");
        //atv_shutdown_cb(this, 0);
    }
    else
    {
        SXLOGD("ATVCS_fm_powerdown fail");
    }

#endif

    if (fm_psFlag)
    {
        ret = fm_powerdown(1);
        //ret = matv_ps_init(1);

        if (ret == false)
        {
            SXLOGD("ATVCS_matv_ps_init fail");
            mState = MATVCTRL_STATE_CLOSE;
        }
        else
        {
            SXLOGD("ATVCS_matv_ps_init success");
            mState = MATVCTRL_STATE_PS;
        }

        fm_psFlag = 0;
    }
    else
    {
        ret = fm_powerdown(0);
        mState = MATVCTRL_STATE_CLOSE;
    }

    return ret;
}

kal_int32 ATVCtrlService::ATVCS_fm_getrssi(void)
{
    SXLOGD("ATVCS_fm_getrssi");

    int ret = fm_getrssi();

    return ret;
}

kal_bool ATVCtrlService::ATVCS_fm_tune(void *parm, int len)
{
    SXLOGD("ATVCS_fm_tune parm=%p, len=%d", parm, len);
    kal_bool ret;
    struct fm_tune_parm *parm_l;

    parm_l = (struct fm_tune_parm *)parm;


    ret = fm_tune(parm_l);

    if (ret == false)
    {
        SXLOGD("ATVCS_fm_tune fail");
    }

    return ret;
}

kal_bool ATVCtrlService::ATVCS_fm_seek(void *parm, int len)
{
    SXLOGD("ATVCS_fm_seek parm=%p, len=%d", parm, len);
    kal_bool ret;
    struct fm_seek_parm *parm_l;

    parm_l = (struct fm_seek_parm *)parm;


    ret = fm_seek(parm_l);

    if (ret == false)
    {
        SXLOGD("ATVCS_fm_seek fail");
    }

    return ret;
}

kal_bool ATVCtrlService::ATVCS_fm_scan(void *parm, int len)
{
    SXLOGD("ATVCS_fm_scan parm=%p, len=%d", parm, len);
    kal_bool ret;
    struct fm_scan_parm *parm_l;

    parm_l = (struct fm_scan_parm *)parm;


    ret = fm_scan(parm_l);

    if (ret == false)
    {
        SXLOGD("ATVCS_fm_scan fail");
    }

    return ret;
}

kal_bool ATVCtrlService::ATVCS_fm_mute(int val)
{
    SXLOGD("ATVCS_fm_mute val=%d", val);
    kal_bool ret;

    ret = fm_mute((kal_uint32)val);

    if (ret == false)
    {
        SXLOGD("ATVCS_fm_seek fail");
    }

    return ret;
}

kal_int32 ATVCtrlService::ATVCS_fm_getchipid(void)
{
    SXLOGD("ATVCS_fm_getchipid");
    int ret = fm_getchipid();
    return ret;
}

kal_int32 ATVCtrlService::ATVCS_fm_isFMPowerUp(void)
{
    SXLOGD("ATVCS_fm_isFMPowerUp");

    if (mState == MATVCTRL_STATE_FM_OPEN)
    {
        return true;
    }
    else
    {
        return false;
    }
}


void ATVCtrlService::atv_autoscan_progress_cb(void *cb_param, kal_uint8 precent, kal_uint8 ch, kal_uint8 chnum)
{
    SXLOGD("atv_autoscan_progress_cb cb_param=%p, precent=%d, ch=%d, chnum=%d", cb_param, precent, ch, chnum);

    //matv_chscan_state scan_state;

    ATVCtrlService *atvObject = (ATVCtrlService *)cb_param;
    int len = sizeof(matv_chscan_state);
    void *ptr = NULL;

    matv_chscan_query(&(atvObject->scan_state));
    ptr = (void *)(&(atvObject->scan_state));

    sp<IATVCtrlClient> c = atvObject->mClient;

    if (c != NULL)
    {
        SXLOGD("atv_autoscan_progress_cb AutoScan_CB ptr=%p, len=%d", ptr, len);
        c->AutoScan_CB(precent, ch, chnum, ptr, len);
    }

}

void ATVCtrlService::atv_fullscan_progress_cb(void *cb_param, kal_uint8 precent, kal_uint32 freq, kal_uint32 freq_start, kal_uint32 freq_end)
{
    SXLOGD("atv_fullscan_progress_cb cb_param=%p, precent=%d, freq=%d, freq_start=%d, freq_end=%d", cb_param, precent, freq, freq_start, freq_end);

    ATVCtrlService *atvObject = (ATVCtrlService *)cb_param;
    sp<IATVCtrlClient> c = atvObject->mClient;

    if (c != NULL)
    {
        SXLOGD("atv_fullscan_progress_cb FullScan_CB");
        c->FullScan_CB(precent, freq, freq_start, freq_end);
    }

    //   mClient->FullScan_CB(precent, freq, freq_start, freq_end);
}

void ATVCtrlService::atv_scanfinish_cb(void *cb_param, kal_uint8 chnum)
{
    SXLOGD("atv_scanfinish_cb cb_param=%p, chnum=%d", cb_param, chnum);

    ATVCtrlService *atvObject = (ATVCtrlService *)cb_param;
    sp<IATVCtrlClient> c = atvObject->mClient;

    if (c != NULL)
    {
        SXLOGD("atv_scanfinish_cb ScanFinish_CB");
        c->ScanFinish_CB(chnum);
    }

    //   mClient->ScanFinish_CB(chnum);
}
void ATVCtrlService::atv_audioformat_cb(void *cb_param, kal_uint32 format)
{
    SXLOGD("atv_audioformat_cb format=%d", format);

    ATVCtrlService *atvObject = (ATVCtrlService *)cb_param;
    sp<IATVCtrlClient> c = atvObject->mClient;

    if (c != NULL)
    {
        SXLOGD("atv_audioformat_cb AudioFmt_CB");
        c->AudioFmt_CB(format);
    }

    //   mClient->AudioFmt_CB(format);
}

// This function is not related to matvctrl lib directly
void ATVCtrlService::atv_shutdown_cb(void *cb_param, kal_uint32 source)
{
    SXLOGD("atv_shutdown_cb source=%d", source);

    ATVCtrlService *atvObject = (ATVCtrlService *)cb_param;
    sp<IATVCtrlClient> c = atvObject->mClient;

    if (c != NULL)
    {
        SXLOGD("atv_shutdown_cb Shutdown_CB");
        c->Shutdown_CB(source);
    }
}

void ATVCtrlService::ATVCS_matv_register_callback()
{
    SXLOGD("ATVCS_matv_register_callback");
    void *cb_param = (void *)0;
    matv_register_callback(this,
                           atv_autoscan_progress_cb,
                           atv_fullscan_progress_cb,
                           atv_scanfinish_cb,
                           atv_audioformat_cb);
}


}; // namespace android

