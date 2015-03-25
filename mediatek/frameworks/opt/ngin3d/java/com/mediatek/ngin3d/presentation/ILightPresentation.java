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
 * MediaTek Inc. (C) 2013. All rights reserved.
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

package com.mediatek.ngin3d.presentation;

import com.mediatek.ngin3d.Color;

/**
 * Light presentation interface.
 */
public interface ILightPresentation extends Presentation {

    /**
     * Enumeration of light type.
     */
    /** Directional */
    int DIRECTIONAL = 0;
    /** Point light */
    int POINT = 1;
    /** Spot */
    int SPOT = 2;

    /**
     * Sets light type.
     * @param type Light type (DIRECTIONAL, POINT OR SPOT)
     */
    void setType(int type);

    /**
     * Sets light color.
     * @param color Color
     */
    void setColor(Color color);

    /**
     * Sets light ambient level.
     * @param level Level
     */
    void setAmbientLevel(float level);

    /**
     * Sets intensity.
     * @param intensity amount by which light color should be multiplied
     */
    void setIntensity(float intensity);

    /**
     * Sets near attenuation.
     * @param distance distance at which illumination starts to fall off
     */
    void setAttenuationNear(float distance);

    /**
     * Sets far attenuation.
     * @param distance distance at which illumination falls to zero
     */
    void setAttenuationFar(float distance);

    /**
     * Sets spot inner angle.
     * @param angle angle within which light is full strength
     */
    void setSpotInnerAngle(float angle);

    /**
     * Sets spot outer angle.
     * @param angle angle outside of which light falls to zero
     */
    void setSpotOuterAngle(float angle);

    /**
     * Sets light attenuation flag.
     * @param isAttenuated true if illumination should drop with distance
     */
    void setIsAttenuated(boolean isAttenuated);
}
