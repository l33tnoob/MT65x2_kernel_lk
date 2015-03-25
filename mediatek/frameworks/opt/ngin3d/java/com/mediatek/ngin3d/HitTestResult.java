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

package com.mediatek.ngin3d;

/**
 * The results of a hit test.
 * <p>
 * Hit testing is the process of working back from a touch on the screen,
 * sending a 'ray' into the scene directly away from the touch point, and
 * working out which object was hit by the ray, and thus what the user
 * thinks they 'touched'.
 */
public class HitTestResult {
    private Actor mActor;
    private ActorNode mNode;
    private Point mRayStart;
    private Point mRayDirection;
    private Point mRayEnd;
    private Point mRayHit;
    private Point mRayHitNormal;

    /**
     * Fills the hit test result structure with data about the raycast.
     * @param rayStart Ray start point
     * @param rayEnd Ray end point
     * @param rayHit Ray intersection point
     * @param rayHitNormal Ray intersection normal vector
     */
    public void setRay(Point rayStart, Point rayDirection, Point rayEnd,
            Point rayHit, Point rayHitNormal) {
        mRayStart = rayStart;
        mRayDirection = rayDirection;
        mRayEnd = rayEnd;
        mRayHit = rayHit;
        mRayHitNormal = rayHitNormal;
    }

    /**
     * Sets the actor which was intersected by the raycast.
     * @param actor Actor intersected by ray
     * @hide Not useful to the external user
     */
    public void setActor(Actor actor) {
        mActor = actor;
    }

    /**
     * Returns the actor intersected by the hit test.
     * @return Actor intersected by ray
     */
    public Actor getActor() {
        return mActor;
    }

    /**
     * Sets the node which was intersected by the raycast.
     * If no node is set, it indicates that the actor was intersected, but
     * not node.
     * @param node Node intersected by ray
     * @hide Not useful to the external user
     */
    public void setNode(ActorNode node) {
        mNode = node;
    }

    /**
     * Returns the node intersected by the hit test.
     * @return Node intersected by ray (or null if none)
     */
    public ActorNode getNode() {
        return mNode;
    }

    /**
     * Returns the start point of the raycast.
     * @return Ray start point
     */
    public Point getRayStart() {
        return mRayStart;
    }

    /**
     * Returns the direction of the raycast.
     * @return Ray direction
     */
    public Point getRayDirection() {
        return mRayDirection;
    }

    /**
     * Returns the end point of the raycast.
     * @return Ray end point
     */
    public Point getRayEnd() {
        return mRayEnd;
    }

    /**
     * Returns the point at which the ray intersected the actor.
     * If no actor was intersected, this point is set to null.
     * @return Ray intersection point
     */
    public Point getRayHit() {
        return mRayHit;
    }

    /**
     * Returns the normal of the surface of the actor that was intersected.
     * If no actor was intersected, this point is set to null.
     * @return Ray intersection normal vector
     */
    public Point getRayHitNormal() {
        return mRayHitNormal;
    }
}

