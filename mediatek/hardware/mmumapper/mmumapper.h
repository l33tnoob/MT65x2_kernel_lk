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

#ifndef ANDROID_INCLUDE_HARDWARE_MMUMAPPER_H
#define ANDROID_INCLUDE_HARDWARE_MMUMAPPER_H

#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include <hardware/hardware.h>

__BEGIN_DECLS

/*****************************************************************************/

#define MMAPPER_API_VERSION 1

/**
 * The id of this module
 */

#define MMAPPER_HARDWARE_MODULE_ID "mmumapper"

/**
 * Name of the device to open
 */

#define MMAPPER_HARDWARE_NAME "mmumapper"

typedef struct mmu_module {
    struct hw_module_t common;
} mmu_module_t;

typedef struct mmu_mapper_device_t {
    struct hw_device_t common;

    int (*register_buffer)(unsigned int addr, unsigned int width, unsigned int height, unsigned int size, unsigned int format, unsigned int *mva);
    int (*unregister_buffer)(unsigned int addr, unsigned int size, unsigned int format, unsigned int mva);
    int (*query_buffer)(unsigned int addr, unsigned int size, unsigned int format, unsigned int *mva);

    void* reserved_proc[6];

} mmu_mapper_device_t;

/** convenience API for opening and closing a supported device */

static inline int mmumap_open(const struct hw_module_t* module,
        struct mmu_mapper_device_t** device) {
    return module->methods->open(module,
            MMAPPER_HARDWARE_NAME, (struct hw_device_t**)device);
}

static inline int mmumap_close(struct mmu_mapper_device_t* device) {
    return device->common.close(&device->common);
}

/*****************************************************************************/

__END_DECLS

#endif /* ANDROID_INCLUDE_HARDWARE_MMUMAPPER_H */
