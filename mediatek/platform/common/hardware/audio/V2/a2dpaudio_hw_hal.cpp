/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "a2dp_audio_hw_hal"
//#define LOG_NDEBUG 0

#include <stdint.h>

#include <hardware/hardware.h>
#include <system/audio.h>
#include <hardware/audio.h>

#include <hardware_legacy/AudioHardwareInterface.h>
#include <hardware_legacy/AudioSystemLegacy.h>
#include <A2dpAudioInterface.h>

namespace android_audio_legacy
{

extern "C" {

    struct a2dp_audio_module {
        struct audio_module module;
    };

    struct a2dp_audio_device {
        struct audio_hw_device device;

        struct AudioHardwareInterface *hwif;
    };

    struct a2dp_stream_out {
        struct audio_stream_out stream;

        AudioStreamOut *a2dp_out;
    };

    struct a2dp_stream_in {
        struct audio_stream_in stream;

        AudioStreamIn *a2dp_in;
    };

    /** audio_stream_out implementation **/
    static uint32_t out_get_sample_rate(const struct audio_stream *stream)
    {
        const struct a2dp_stream_out *out =
            reinterpret_cast<const struct a2dp_stream_out *>(stream);
        return out->a2dp_out->sampleRate();
    }

    static int out_set_sample_rate(struct audio_stream *stream, uint32_t rate)
    {
        struct a2dp_stream_out *out =
            reinterpret_cast<struct a2dp_stream_out *>(stream);

        ALOGE("(%s:%d) %s: Implement me!", __FILE__, __LINE__, __func__);
        /* TODO: implement this */
        return 0;
    }

    static size_t out_get_buffer_size(const struct audio_stream *stream)
    {
        const struct a2dp_stream_out *out =
            reinterpret_cast<const struct a2dp_stream_out *>(stream);
        return out->a2dp_out->bufferSize();
    }

    static uint32_t out_get_channels(const struct audio_stream *stream)
    {
        const struct a2dp_stream_out *out =
            reinterpret_cast<const struct a2dp_stream_out *>(stream);
        return out->a2dp_out->channels();
    }

    static audio_format_t out_get_format(const struct audio_stream *stream)
    {
        const struct a2dp_stream_out *out =
            reinterpret_cast<const struct a2dp_stream_out *>(stream);
        // legacy API, don't change return type
        return (audio_format_t) out->a2dp_out->format();
    }

    static int out_set_format(struct audio_stream *stream, audio_format_t format)
    {
        struct a2dp_stream_out *out =
            reinterpret_cast<struct a2dp_stream_out *>(stream);
        ALOGE("(%s:%d) %s: Implement me!", __FILE__, __LINE__, __func__);
        /* TODO: implement me */
        return 0;
    }

    static int out_standby(struct audio_stream *stream)
    {
        struct a2dp_stream_out *out =
            reinterpret_cast<struct a2dp_stream_out *>(stream);
        return out->a2dp_out->standby();
    }

    static int out_dump(const struct audio_stream *stream, int fd)
    {
        const struct a2dp_stream_out *out =
            reinterpret_cast<const struct a2dp_stream_out *>(stream);
        Vector<String16> args;
        return out->a2dp_out->dump(fd, args);
    }

    static int out_set_parameters(struct audio_stream *stream, const char *kvpairs)
    {
        struct a2dp_stream_out *out =
            reinterpret_cast<struct a2dp_stream_out *>(stream);
        return out->a2dp_out->setParameters(String8(kvpairs));
    }

    static char *out_get_parameters(const struct audio_stream *stream, const
                                    char *keys)
    {
        const struct a2dp_stream_out *out =
            reinterpret_cast<const struct a2dp_stream_out *>(stream);
        String8 s8;
        s8 = out->a2dp_out->getParameters(String8(keys));
        return strdup(s8.string());
    }

    static uint32_t out_get_latency(const struct audio_stream_out *stream)
    {
        const struct a2dp_stream_out *out =
            reinterpret_cast<const struct a2dp_stream_out *>(stream);
        return out->a2dp_out->latency();
    }

    static int out_set_volume(struct audio_stream_out *stream, float left,
                              float right)
    {
        struct a2dp_stream_out *out =
            reinterpret_cast<struct a2dp_stream_out *>(stream);
        return out->a2dp_out->setVolume(left, right);
    }

    static ssize_t out_write(struct audio_stream_out *stream, const void *buffer,
                             size_t bytes)
    {
        struct a2dp_stream_out *out =
            reinterpret_cast<struct a2dp_stream_out *>(stream);
        return out->a2dp_out->write(buffer, bytes);
    }

    static int out_get_render_position(const struct audio_stream_out *stream,
                                       uint32_t *dsp_frames)
    {
        const struct a2dp_stream_out *out =
            reinterpret_cast<const struct a2dp_stream_out *>(stream);
        return out->a2dp_out->getRenderPosition(dsp_frames);
    }

    static int out_get_next_write_timestamp(const struct audio_stream_out *stream,
                                            int64_t *timestamp)
    {
        const struct a2dp_stream_out *out =
            reinterpret_cast<const struct a2dp_stream_out *>(stream);
        return out->a2dp_out->getNextWriteTimestamp(timestamp);
    }


    static int out_add_audio_effect(const struct audio_stream *stream,
                                    effect_handle_t effect)
    {
        return 0;
    }

    static int out_remove_audio_effect(const struct audio_stream *stream,
                                       effect_handle_t effect)
    {
        return 0;
    }

    /** audio_hw_device implementation **/
    static inline struct a2dp_audio_device *to_ladev(struct audio_hw_device *
                                                     dev) {
        return reinterpret_cast<struct a2dp_audio_device *>(dev);
    }

    static inline const struct a2dp_audio_device *to_cladev(const struct
                                                            audio_hw_device *dev) {
        return reinterpret_cast<const struct a2dp_audio_device *>(dev);
    }

    static uint32_t adev_get_supported_devices(const struct audio_hw_device *dev)
    {
        /* XXX: The old AudioHardwareInterface interface is not smart enough to
         * tell us this, so we'll lie and basically tell AF that we support the
         * below input/output devices and cross our fingers. To do things properly,
         * audio hardware interfaces that need advanced features (like this) should
         * convert to the new HAL interface and not use this wrapper. */
        return AUDIO_DEVICE_OUT_ALL_A2DP;
    }

    static int adev_init_check(const struct audio_hw_device *dev)
    {
        const struct a2dp_audio_device *ladev = to_cladev(dev);
        ALOGD("a2dp adev_init_check");
        return ladev->hwif->initCheck();
    }

    static int adev_set_voice_volume(struct audio_hw_device *dev, float volume)
    {
        struct a2dp_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->setVoiceVolume(volume);
    }

    static int adev_set_master_volume(struct audio_hw_device *dev, float volume)
    {
        struct a2dp_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->setMasterVolume(volume);
    }

    static int adev_set_mode(struct audio_hw_device *dev, audio_mode_t mode)
    {
        struct a2dp_audio_device *ladev = to_ladev(dev);
        // as this is the legacy API, don't change it to use audio_mode_t instead of int
        return ladev->hwif->setMode((int) mode);
    }
    static int adev_set_mic_mute(struct audio_hw_device *dev, bool state)
    {
        struct a2dp_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->setMicMute(state);
    }

    static int adev_get_mic_mute(const struct audio_hw_device *dev, bool *state)
    {
        const struct a2dp_audio_device *ladev = to_cladev(dev);
        return ladev->hwif->getMicMute(state);
    }

    static int adev_set_parameters(struct audio_hw_device *dev, const char *
                                   kvpairs)
    {
        struct a2dp_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->setParameters(String8(kvpairs));
    }

    static char *adev_get_parameters(const struct audio_hw_device *dev,
                                     const char *keys)
    {
        const struct a2dp_audio_device *ladev = to_cladev(dev);
        String8 s8;

        s8 = ladev->hwif->getParameters(String8(keys));
        return strdup(s8.string());
    }


    static int adev_open_output_stream(struct audio_hw_device *dev,
                                       audio_io_handle_t handle,
                                       audio_devices_t devices,
                                       audio_output_flags_t flags,
                                       struct audio_config *config,
                                       struct audio_stream_out **stream_out)
    {
        struct a2dp_audio_device *ladev = to_ladev(dev);
        status_t status;
        struct a2dp_stream_out *out;
        int ret;
        ALOGD("adev_open_output_stream");

        out = (struct a2dp_stream_out *)calloc(1, sizeof(*out));
        if (!out)
            return -ENOMEM;

        out->a2dp_out = ladev->hwif->openOutputStream(devices, (int *) &config->format,
                                                      &config->channel_mask,
                                                      &config->sample_rate, &status);
        if (!out->a2dp_out) {
            ret = status;
            goto err_open;
        }

        out->stream.common.get_sample_rate = out_get_sample_rate;
        out->stream.common.set_sample_rate = out_set_sample_rate;
        out->stream.common.get_buffer_size = out_get_buffer_size;
        out->stream.common.get_channels = out_get_channels;
        out->stream.common.get_format = out_get_format;
        out->stream.common.set_format = out_set_format;
        out->stream.common.standby = out_standby;
        out->stream.common.dump = out_dump;
        out->stream.common.set_parameters = out_set_parameters;
        out->stream.common.get_parameters = out_get_parameters;
        out->stream.common.add_audio_effect = out_add_audio_effect;
        out->stream.common.remove_audio_effect = out_remove_audio_effect;
        out->stream.get_latency = out_get_latency;
        out->stream.set_volume = out_set_volume;
        out->stream.write = out_write;
        out->stream.get_render_position = out_get_render_position;
        out->stream.get_next_write_timestamp = out_get_next_write_timestamp;

        *stream_out = &out->stream;
        return 0;

err_open:
        free(out);
        *stream_out = NULL;
        return ret;
    }

    static void adev_close_output_stream(struct audio_hw_device *dev,
                                         struct audio_stream_out *stream)
    {
        struct a2dp_audio_device *ladev = to_ladev(dev);
        struct a2dp_stream_out *out = reinterpret_cast<struct a2dp_stream_out *>(stream);

        ladev->hwif->closeOutputStream(out->a2dp_out);
        free(out);
    }

    /** This method creates and opens the audio hardware input stream */
    static int adev_open_input_stream(struct audio_hw_device *dev,
                                      audio_io_handle_t handle,
                                      audio_devices_t devices,
                                      struct audio_config *config,
                                      struct audio_stream_in **stream_in)

    {
        return -ENOSYS;
    }

    //-----------------------------------------------------------------
    static int adev_set_emparameter(struct audio_hw_device *dev, void *ptr , int
                                    len)
    {
        struct a2dp_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->SetEMParameter(ptr, len);
    }

    static int adev_get_emparameter(struct audio_hw_device *dev, void *ptr , int
                                    len)
    {
        struct a2dp_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->GetEMParameter(ptr, len);
    }

    static int adev_set_audiocommand(struct audio_hw_device *dev, int par1 , int
                                     par2)
    {
        struct a2dp_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->SetAudioCommand(par1, par2);
    }

    static int adev_get_audiocommand(struct audio_hw_device *dev, int par1)
    {
        struct a2dp_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->GetAudioCommand(par1);
    }

    static int adev_set_audiodata(struct audio_hw_device *dev, int par1, size_t len,
                                  void *ptr)
    {
        struct a2dp_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->SetAudioData(par1, len, ptr);
    }

    static int adev_get_audiodata(struct audio_hw_device *dev, int par1, size_t len,
                                  void *ptr)
    {
        struct a2dp_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->GetAudioData(par1, len, ptr);
    }

    static int adev_set_acf_previewparameter(struct audio_hw_device *dev, void *
                                             ptr , int len)
    {
        struct a2dp_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->SetACFPreviewParameter(ptr, len);
    }

    static int adev_set_hcf_previewparameter(struct audio_hw_device *dev, void *
                                             ptr , int len)
    {
        struct a2dp_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->SetHCFPreviewParameter(ptr, len);
    }

    static int adev_xway_play_start(struct audio_hw_device *dev, int sample_rate)
    {
        struct a2dp_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->xWayPlay_Start(sample_rate);
    }

    static int adev_xway_play_stop(struct audio_hw_device *dev)
    {
        struct a2dp_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->xWayPlay_Stop();
    }

    static int adev_xway_play_write(struct audio_hw_device *dev, void *buffer , int
                                    size_bytes)
    {
        struct a2dp_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->xWayPlay_Write(buffer, size_bytes);
    }

    static int adev_xway_getfreebuffercount(struct audio_hw_device *dev)
    {
        struct a2dp_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->xWayPlay_GetFreeBufferCount();
    }

    static int adev_xway_rec_start(struct audio_hw_device *dev, int smple_rate)
    {
        struct a2dp_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->xWayRec_Start(smple_rate);
    }

    static int adev_xway_rec_stop(struct audio_hw_device *dev)
    {
        struct a2dp_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->xWayRec_Stop();
    }

    static int adev_xway_rec_read(struct audio_hw_device *dev, void *buffer , int
                                  size_bytes)
    {
        struct a2dp_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->xWayRec_Read(buffer, size_bytes);
    }

    //add by wendy
    static int adev_ReadRefFromRing(struct audio_hw_device *dev,void*buf, uint32_t datasz,void* DLtime)
    {
        struct a2dp_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->ReadRefFromRing(buf, datasz,DLtime);
    }

    static int adev_GetVoiceUnlockULTime(struct audio_hw_device *dev,void* ULtime)
    {
        struct a2dp_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->GetVoiceUnlockULTime(ULtime);
    }
    static int adev_SetVoiceUnlockSRC(struct audio_hw_device *dev,uint outSR, uint outChannel)
    {
        struct a2dp_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->SetVoiceUnlockSRC(outSR,outChannel);
    }
    static bool adev_startVoiceUnlockDL(struct audio_hw_device *dev)
    {
        struct a2dp_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->startVoiceUnlockDL();
    }
    static bool adev_stopVoiceUnlockDL(struct audio_hw_device *dev)
    {
        struct a2dp_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->stopVoiceUnlockDL();
    }

    static void adev_freeVoiceUnlockDLInstance(struct audio_hw_device *dev)
    {
        struct a2dp_audio_device *ladev = to_ladev(dev);
        return ladev->hwif->freeVoiceUnlockDLInstance();
    }
    static int adev_GetVoiceUnlockDLLatency(struct audio_hw_device *dev)
    {
     struct a2dp_audio_device *ladev = to_ladev(dev);
     return ladev->hwif->GetVoiceUnlockDLLatency();

    }
    static bool adev_getVoiceUnlockDLInstance(struct audio_hw_device *dev)
    {
    struct a2dp_audio_device *ladev = to_ladev(dev);
    return ladev->hwif->getVoiceUnlockDLInstance();

    }
    //-------------------------------------------------------------------------

    static void adev_close_input_stream(struct audio_hw_device *dev,
                                        struct audio_stream_in *stream)
    {
        return;
    }

    static int adev_dump(const struct audio_hw_device *dev, int fd)
    {
        const struct a2dp_audio_device *ladev = to_cladev(dev);
        Vector<String16> args;

        return ladev->hwif->dumpState(fd, args);
    }

    static int a2dp_adev_close(hw_device_t *device)
    {
        struct audio_hw_device *hwdev =
            reinterpret_cast<struct audio_hw_device *>(device);
        struct a2dp_audio_device *ladev = to_ladev(hwdev);

        if (!ladev)
            return 0;

        if (ladev->hwif)
            delete ladev->hwif;

        free(ladev);
        return 0;
    }

    static int a2dp_adev_open(const hw_module_t *module, const char *name,
                              hw_device_t **device)
    {
        struct a2dp_audio_device *ladev;
        int ret;

        if (strcmp(name, AUDIO_HARDWARE_INTERFACE) != 0)
            return -EINVAL;

        ladev = (struct a2dp_audio_device *)calloc(1, sizeof(*ladev));
        if (!ladev)
            return -ENOMEM;

        ladev->device.common.tag = HARDWARE_DEVICE_TAG;
        ladev->device.common.version = AUDIO_DEVICE_API_VERSION_2_0;
        ladev->device.common.module = const_cast<hw_module_t *>(module);
        ladev->device.common.close = a2dp_adev_close;

        ladev->device.get_supported_devices = adev_get_supported_devices;
        ladev->device.init_check = adev_init_check;
        ladev->device.set_voice_volume = adev_set_voice_volume;
        ladev->device.set_master_volume = adev_set_master_volume;
        ladev->device.set_mode = adev_set_mode;
        ladev->device.set_mic_mute = adev_set_mic_mute;
        ladev->device.get_mic_mute = adev_get_mic_mute;
        ladev->device.set_parameters = adev_set_parameters;
        ladev->device.get_parameters = adev_get_parameters;
        ladev->device.open_output_stream = adev_open_output_stream;
        ladev->device.close_output_stream = adev_close_output_stream;
        ladev->device.open_input_stream = adev_open_input_stream;
        ladev->device.close_input_stream = adev_close_input_stream;
        ladev->device.dump = adev_dump;

        ladev->device.SetEMParameter = adev_set_emparameter;
        ladev->device.GetEMParameter = adev_get_emparameter;
        ladev->device.SetAudioCommand = adev_set_audiocommand;
        ladev->device.GetAudioCommand = adev_get_audiocommand;
        ladev->device.SetAudioData = adev_set_audiodata;
        ladev->device.GetAudioData = adev_get_audiodata;
        ladev->device.SetACFPreviewParameter = adev_set_acf_previewparameter;
        ladev->device.SetHCFPreviewParameter = adev_set_hcf_previewparameter;
        ladev->device.xWayPlay_Start = adev_xway_play_start;
        ladev->device.xWayPlay_Stop = adev_xway_play_stop;
        ladev->device.xWayPlay_Write = adev_xway_play_write;
        ladev->device.xWayPlay_GetFreeBufferCount = adev_xway_getfreebuffercount;
        ladev->device.xWayRec_Start = adev_xway_rec_start;
        ladev->device.xWayRec_Stop = adev_xway_rec_stop;
        ladev->device.xWayRec_Read = adev_xway_rec_read;
        //added by wendy
        ladev->device.ReadRefFromRing = adev_ReadRefFromRing;
        ladev->device.GetVoiceUnlockULTime = adev_GetVoiceUnlockULTime;
        ladev->device.SetVoiceUnlockSRC = adev_SetVoiceUnlockSRC;
        ladev->device.startVoiceUnlockDL = adev_startVoiceUnlockDL;
        ladev->device.stopVoiceUnlockDL = adev_stopVoiceUnlockDL;
        ladev->device.freeVoiceUnlockDLInstance = adev_freeVoiceUnlockDLInstance;
        ladev->device.GetVoiceUnlockDLLatency = adev_GetVoiceUnlockDLLatency;
        ladev->device.getVoiceUnlockDLInstance = adev_getVoiceUnlockDLInstance;


        ALOGD("createA2DPAudioHardware");

        ladev->hwif = createA2DPAudioHardware();
        if (!ladev->hwif) {
            ret = -EIO;
            goto err_create_audio_hw;
        }

        ALOGD(" ladev->hwif =%p", ladev->hwif);

        *device = &ladev->device.common;

        return 0;

err_create_audio_hw:
        free(ladev);
        return ret;
    }

    static struct hw_module_methods_t hal_module_methods = {
open:
        a2dp_adev_open
    };

    struct a2dp_audio_module HAL_MODULE_INFO_SYM = {
module:
        {
common:
            {
tag:
                HARDWARE_MODULE_TAG,
module_api_version:
                AUDIO_MODULE_API_VERSION_0_1,
hal_api_version:
                HARDWARE_HAL_API_VERSION,
id:
                AUDIO_HARDWARE_MODULE_ID,
name: "A2DP Audio HW HAL"
                ,
author: "The Android Open Source Project"
                ,
methods:
                &hal_module_methods,
dso :
                NULL,
reserved :
                {0},
            },
        },
    };

}; // extern "C"

}; // namespace android_audio_legacy
