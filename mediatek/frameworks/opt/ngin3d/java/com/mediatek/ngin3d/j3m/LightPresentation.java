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
/** \file
 * Light Presentation for J3M
 */
package com.mediatek.ngin3d.j3m;

import com.mediatek.ngin3d.Color;
import com.mediatek.ngin3d.presentation.ILightPresentation;

import com.mediatek.j3m.AngularUnits;
import com.mediatek.j3m.Light;

/**
 * j3m implementation of ILightPresentation interface
 * @hide
 */

public class LightPresentation extends ActorPresentation
    implements ILightPresentation {

    private static final String TAG = "LightPresentation";
    private Light mLight;

    /**
     * Initializes this object with A3M presentation engine
     * @param engine    Presentation engine
     */
    public LightPresentation(J3mPresentationEngine engine) {
        super(engine);
    }

    /**
     * Initializes this object
     */
    @Override
    public void onInitialize() {
        super.onInitialize();

        // Replace the default scene node with a light object
        getSceneNode().setParent(null);
        mLight = getEngine().getJ3m().createLight();
        setSceneNode(mLight);
        getSceneNode().setParent(getAnchorSceneNode());
    }

    @Override
    public void setType(int type) {

        int j3mLightType = Light.Type.OMNI;

        switch (type) {
        case ILightPresentation.DIRECTIONAL:
            j3mLightType = Light.Type.DIRECTIONAL;
            break;
        case ILightPresentation.POINT:
            j3mLightType = Light.Type.OMNI;
            break;
        case ILightPresentation.SPOT:
            j3mLightType = Light.Type.SPOT;
            break;
        default:
            j3mLightType = Light.Type.OMNI;
            break;
        }
        mLight.setLightType(j3mLightType);
    }

    @Override
    public void setColor(Color color) {
        mLight.setColour(
            color.red / 255.0f, color.green / 255.0f,
            color.blue / 255.0f, color.alpha / 255.0f);
    }

    @Override
    public void setAmbientLevel(float level) {
        mLight.setAmbientLevel(level);
    }

    @Override
    public void setIntensity(float intensity) {
        mLight.setIntensity(intensity);
    }

    @Override
    public void setAttenuationNear(float distance) {
        mLight.setAttenuationNear(distance);
    }

    @Override
    public void setAttenuationFar(float distance) {
        mLight.setAttenuationFar(distance);
    }

    @Override
    public void setSpotInnerAngle(float angle) {
        mLight.setSpotInnerAngle(AngularUnits.DEGREES, angle);
    }

    @Override
    public void setSpotOuterAngle(float angle) {
        mLight.setSpotOuterAngle(AngularUnits.DEGREES, angle);
    }

    @Override
    public void setIsAttenuated(boolean isAttenuated) {
        mLight.setIsAttenuated(isAttenuated);
    }
}
