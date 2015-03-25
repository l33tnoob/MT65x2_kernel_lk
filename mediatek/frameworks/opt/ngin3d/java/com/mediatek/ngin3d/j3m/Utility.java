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
 * Utility functions for J3M presentation layer.
 */
package com.mediatek.ngin3d.j3m;

import com.mediatek.j3m.Appearance;
import com.mediatek.j3m.SceneNode;
import com.mediatek.j3m.Solid;

/**
 * Contains utility functions shared by several J3M presentation classes.
 */
final class Utility {
    private static final String TAG = "Utility";

    /**
     * Private constructor to prevent instantiation.
     */
    private Utility() {
    }

    /**
     * Enabled transparency in an Appearance.
     *
     * @param appearance Appearance
     */
    public static void enableTransparency(Appearance appearance) {
        appearance.setBlendFactors(
                Appearance.BlendFactor.SRC_ALPHA,
                Appearance.BlendFactor.SRC_ALPHA,
                Appearance.BlendFactor.ONE_MINUS_SRC_ALPHA,
                Appearance.BlendFactor.ONE_MINUS_SRC_ALPHA);
        appearance.setDepthWriteEnabled(false);
    }

    /**
     * Recursively sets the color of all sub-nodes within a node.
     *
     * @param node Node for which to set color
     * @param color Color to set
     */
    public static void setColorRecursive(
            J3mPresentationEngine engine, SceneNode node,
            float r, float g, float b, float a) {

        // Check for debug nodes in case this ActorNode contains nodes which
        // are referenced by other ActorNodes.
        if (node == null || node.getFlags(engine.getRenderFlags().DEBUG)) {
            return;
        }

        if (Solid.class.isInstance(node)) {
            Solid solid = (Solid) node;
            Appearance appearance = solid.getAppearance();
            appearance.setVector4f("M_DIFFUSE_COLOUR", r, g, b, a);

            // Switch transparency on if the alpha component is less than one.
            // Do not switch transparency off if alpha is one, because there
            // may be a texture alpha channel in use.
            if (a < 1.0f) {
                enableTransparency(appearance);
            }
        }

        // Iteratively set color for all child J3M scene nodes.
        for (int i = 0; i < node.getChildCount(); ++i) {
            setColorRecursive(engine, node.getChild(i), r, g, b, a);
        }
    }

    /**
     * Recursively sets the opacity of all sub-nodes within a node.
     *
     * @param node Node for which to set opacity
     * @param opacity Opacity to set
     */
    public static void setOpacityRecursive(
            J3mPresentationEngine engine, SceneNode node, float opacity) {

        // Check for debug nodes in case this ActorNode contains nodes which
        // are referenced by other ActorNodes.
        if (node == null || node.getFlags(engine.getRenderFlags().DEBUG)) {
            return;
        }

        if (Solid.class.isInstance(node)) {
            Solid solid = (Solid) node;
            Appearance appearance = solid.getAppearance();

            float appearanceOpacity;

            if (appearance.propertyExists("_APPEARANCE_OPACITY")) {
                appearanceOpacity = appearance.getFloat("_APPEARANCE_OPACITY");
            } else {
                // Cache the appearance opacity since we are about to overwrite it
                appearanceOpacity = appearance.getFloat("M_OPACITY");
                appearance.setFloat("_APPEARANCE_OPACITY", appearanceOpacity);
            }

            appearance.setFloat("M_OPACITY", appearanceOpacity * opacity);

            // Switch transparency on if the opacity is less than one.  Do not
            // switch transparency off if opacity is one, because there may be
            // a texture alpha channel in use.
            if (opacity < 1.0f) {
                enableTransparency(appearance);
            }
        }

        // Iteratively set opacity for all child J3M scene nodes.
        for (int i = 0; i < node.getChildCount(); ++i) {
            setOpacityRecursive(engine, node.getChild(i), opacity);
        }
    }
}
