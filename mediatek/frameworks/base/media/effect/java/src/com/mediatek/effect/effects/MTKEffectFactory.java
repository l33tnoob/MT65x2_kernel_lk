/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2013. All rights reserved.
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

package com.mediatek.effect.effects;

public class MTKEffectFactory {
    /**
     * Converts 2D image to a side-by-side image used for 3D display
     */
    public final static String EFFECT_2DTO3D = "com.mediatek.effect.effects.Stereo3D2Dto3DEffect";

    /**
     * Executes convergence algorithm for a 3D side-by-side image and produces
     * offsets information for better alignment of the left and right images
     */
    public final static String EFFECT_ANTIFATIGUE = "com.mediatek.effect.effects.Stereo3DAntiFatigueEffect";

    public final static String EFFECT_ANTIFATIGUE_RIGHT_BITMAP = "inbitmapR";
    public final static String EFFECT_ANTIFATIGUE_LEFT_BITMAP = "inbitmapL";
    public final static String EFFECT_ANTIFATIGUE_OPERATION = "operation";
    public final static String EFFECT_ANTIFATIGUE_MTK3DTAG = "mtk3dtag";
    public final static int EFFECT_ANTIFATIGUE_MTK3DTAG_N = 0;
    public final static int EFFECT_ANTIFATIGUE_MTK3DTAG_Y = 1;
    public final static int EFFECT_ANTIFATIGUE_OPERATION_PLAYBACK = 0;
    public final static int EFFECT_ANTIFATIGUE_OPERATION_ZOOM = 1;
    public final static String EFFECT_ANTIFATIGUE_LAYOUT = "screenLayout";
    public final static int EFFECT_ANTIFATIGUE_LAYOUT_HORIZONTAL = 0;
    public final static int EFFECT_ANTIFATIGUE_LAYOUT_VERTICAL = 1;

    public MTKEffectFactory() {
    }
}
