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
 *   IATVCtrlClient.cpp
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   ATV Control Client Interface
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

/*****************************************************************************
*                     C O M P I L E R   F L A G S
******************************************************************************
*/
/*****************************************************************************
*                E X T E R N A L   R E F E R E N C E S
******************************************************************************
*/
#define LOG_TAG "IATVCtrlClient"
#include <utils/Log.h>
#include <stdint.h>
#include <sys/types.h>
#include <binder/Parcel.h>
#include <media/IATVCtrlClient.h>
#include <cutils/xlog.h>
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
enum
{
    AUTO_SCAN_CB = IBinder::FIRST_CALL_TRANSACTION,
    FULL_SCAN_CB,
    SCAN_FINISH_CB,
    AUDIO_FORMAT_CB,
    SHUTDOWN_CB,
    NOTIFY
};


/*****************************************************************************
*                        C L A S S   D E F I N I T I O N
******************************************************************************
*/

class BpATVCtrlClient : public BpInterface<IATVCtrlClient>
{
public:
    BpATVCtrlClient(const sp<IBinder>& impl)
        : BpInterface<IATVCtrlClient>(impl)
    {
    }

    virtual void AutoScan_CB(int precent, int ch, int chnum, void *ptr, int len)
    {
        Parcel data, reply;
        SXLOGD("AutoScan_CB precent=%d, ch=%d, chnum=%d, ptr=%p, len=%d", precent, ch, chnum, ptr, len);
        data.writeInterfaceToken(IATVCtrlClient::getInterfaceDescriptor());
        data.writeInt32(precent);
        data.writeInt32(ch);
        data.writeInt32(chnum);

        data.writeInt32(len);
        data.write(ptr, len);
        remote()->transact(AUTO_SCAN_CB, data, &reply);
        reply.read(ptr, len);
    }
    virtual void FullScan_CB(int precent, int freq, int freq_start, int freq_end)
    {
        Parcel data, reply;
        SXLOGD("FullScan_CB precent=%d, freq=%d, freq_start=%d, freq_end=%d", precent, freq, freq_start, freq_end);
        data.writeInterfaceToken(IATVCtrlClient::getInterfaceDescriptor());
        data.writeInt32(precent);
        data.writeInt32(freq);
        data.writeInt32(freq_start);
        data.writeInt32(freq_end);
        remote()->transact(FULL_SCAN_CB, data, &reply);
    }
    virtual void ScanFinish_CB(int chnum)
    {
        Parcel data, reply;
        SXLOGD("ScanFinish_CB chnum=%d", chnum);
        data.writeInterfaceToken(IATVCtrlClient::getInterfaceDescriptor());
        data.writeInt32(chnum);
        remote()->transact(SCAN_FINISH_CB, data, &reply);
    }
    virtual void AudioFmt_CB(int format)
    {
        Parcel data, reply;
        SXLOGD("AudioFmt_CB format=%d", format);
        data.writeInterfaceToken(IATVCtrlClient::getInterfaceDescriptor());
        data.writeInt32(format);
        remote()->transact(AUDIO_FORMAT_CB, data, &reply);
    }
    virtual void Shutdown_CB(int source)
    {
        Parcel data, reply;
        SXLOGD("Shutdown_CB format=%d", source);
        data.writeInterfaceToken(IATVCtrlClient::getInterfaceDescriptor());
        data.writeInt32(source);
        remote()->transact(SHUTDOWN_CB, data, &reply);
    }
    virtual void notify(int msg, int ext1, int ext2)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IATVCtrlClient::getInterfaceDescriptor());
        data.writeInt32(msg);
        data.writeInt32(ext1);
        data.writeInt32(ext2);
        remote()->transact(NOTIFY, data, &reply);
    }

};

IMPLEMENT_META_INTERFACE(ATVCtrlClient, "android.media.IATVCtrlClient");

// ----------------------------------------------------------------------

/*#define CHECK_INTERFACE(interface, data, reply) \
        do { if (!data.enforceInterface(interface::getInterfaceDescriptor())) { \
            LOGW("Call incorrectly routed to " #interface); \
            return PERMISSION_DENIED; \
        } } while (0)*/

status_t BnATVCtrlClient::onTransact(
    uint32_t code, const Parcel &data, Parcel *reply, uint32_t flags)
{
    switch (code)
    {
        case AUTO_SCAN_CB:
        {
            CHECK_INTERFACE(IATVCtrlClient, data, reply);
            int precent = data.readInt32();
            int ch = data.readInt32();
            int chnum = data.readInt32();

            int len = data.readInt32();
            void *ptr = malloc(len);
            data.read(ptr, len);
            SXLOGD("onTransact:AUTO_SCAN_CB precent=%d, ch=%d, chnum=%d, ptr=%p, len=%d", precent, ch, chnum, ptr, len);
            AutoScan_CB(precent, ch, chnum, ptr, len);
            reply->write(ptr, len);
            free(ptr);

            return NO_ERROR;
        }
        break;

        case FULL_SCAN_CB:
        {
            CHECK_INTERFACE(IATVCtrlClient, data, reply);
            int precent = data.readInt32();
            int freq = data.readInt32();
            int freq_start = data.readInt32();
            int freq_end = data.readInt32();
            SXLOGD("onTransact:FULL_SCAN_CB precent=%d, freq=%d, freq_start=%d, freq_end=%d", precent, freq, freq_start, freq_end);
            FullScan_CB(precent, freq, freq_start, freq_end);
            return NO_ERROR;
        }
        break;

        case SCAN_FINISH_CB:
        {
            CHECK_INTERFACE(IATVCtrlClient, data, reply);
            int chnum = data.readInt32();
            SXLOGD("onTransact:SCAN_FINISH_CB chnum=%d", chnum);
            ScanFinish_CB(chnum);
            return NO_ERROR;
        }
        break;

        case AUDIO_FORMAT_CB:
        {
            CHECK_INTERFACE(IATVCtrlClient, data, reply);
            int format = data.readInt32();
            SXLOGD("onTransact:AUDIO_FORMAT_CB format=%d", format);
            AudioFmt_CB(format);
            return NO_ERROR;
        }
        break;

        case SHUTDOWN_CB:
        {
            CHECK_INTERFACE(IATVCtrlClient, data, reply);
            int source = data.readInt32();
            SXLOGD("onTransact:SHUTDOWN_CB source=%d", source);
            Shutdown_CB(source);
            return NO_ERROR;
        }
        break;

        case NOTIFY:
        {
            CHECK_INTERFACE(IATVCtrlClient, data, reply);
            int msg = data.readInt32();
            int ext1 = data.readInt32();
            int ext2 = data.readInt32();
            notify(msg, ext1, ext2);
            return NO_ERROR;
        }
        break;

        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

// ----------------------------------------------------------------------------

}; // namespace android


