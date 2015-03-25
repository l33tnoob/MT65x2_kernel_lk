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
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Rotation;
import com.mediatek.ngin3d.Scale;

/**
 * Node presentation interface.
 */
public interface IActorNodePresentation {
    /**
     * Enumeration of types of collision shape.
     */
    public static class CollisionType {
        /** No collision shape */
        public static final int NONE = 0;
        /** Unbounded plane collision shape */
        public static final int PLANE = 1;
        /** Bounded 1x1 plane collision shape */
        public static final int SQUARE = 2;
        /** Spherical collision shape of diameter 1 */
        public static final int SPHERE = 3;
    }

    /**
     * Initializes the presentation.
     */
    void initialize(Object owner);

    /**
     * Uninitialize the presentation.
     * Do not call any methods after uninitialization.
     */
    void uninitialize();

    /**
     * Returns the object which owns this presentation.
     * @return Owner
     */
    Object getOwner();

    /**
     * Sets the local position of the node.
     * @param position Position of node
     */
    void setPosition(Point position);

    /**
     * Sets the rotation of the node.
     * @param rotation Rotation of node
     */
    void setRotation(Rotation rotation);

    /**
     * Sets the scale of the node.
     * @param scale Scale of node
     */
    void setScale(Scale scale);

    /**
     * Sets whether the node is visible.
     * @param visible Visibility flag
     */
    void setVisible(boolean visible);

    /**
     * Sets the color of the node.
     * @param color Color
     */
    void setColor(Color color);

    /**
     * Sets the opacity of the node.
     * @param opacity Opacity
     */
    void setOpacity(int opacity);

    /**
     * Sets the type of collision shape used by this node for hit tests.
     * @param type Type of collision shape
     */
    void setCollisionShape(int type);

    /**
     * Sets the position of the collision sphere relative to the node.
     * @param position Position of shape
     */
    void setCollisionPosition(Point position);

    /**
     * Sets the rotation of the collision sphere relative to the node.
     * @param rotation Rotation of shape
     */
    void setCollisionRotation(Rotation rotation);

    /**
     * Sets the scale of the collision sphere relative to the node.
     * @param scale Scale of shape
     */
    void setCollisionScale(Scale scale);

    /**
     * Sets whether to render a representation of the collision shape.
     * @param visible Visibility flag
     */
    void setCollisionVisible(boolean visible);

    /**
     * Request renderer render a frame.
     */
    void requestRender();
}
