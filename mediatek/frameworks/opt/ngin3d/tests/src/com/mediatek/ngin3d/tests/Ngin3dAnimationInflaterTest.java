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

package com.mediatek.ngin3d.tests;

import com.mediatek.ngin3d.Color;
import com.mediatek.ngin3d.Image;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Rotation;
import com.mediatek.ngin3d.Scale;
import com.mediatek.ngin3d.android.Ngin3dAnimationInflater;
import com.mediatek.ngin3d.android.Ngin3dLayoutInflater;
import com.mediatek.ngin3d.animation.AnimationGroup;
import com.mediatek.ngin3d.animation.BasicAnimation;
import com.mediatek.ngin3d.animation.Mode;
import com.mediatek.ngin3d.animation.PropertyAnimation;

public class Ngin3dAnimationInflaterTest extends Ngin3dInstrumentationTestCase {
    protected PresentationStubActivity mActivity;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mActivity = getActivity();
    }

    @Override
    protected void tearDown() throws Exception {
        mStage = null;
        mActivity = null;
        super.tearDown();
    }

    public void testAnimationPositionPropertyAnimation() {

        Image image = (Image) Ngin3dLayoutInflater.inflateLayout(mActivity, R.xml.test_single_actor_image_with_full_info, mStage);
        assertNotNull(image);

        /* TC 1: test "position" PropertyAnimation with full info in xml */
        PropertyAnimation move = (PropertyAnimation) Ngin3dAnimationInflater.inflateAnimation(mActivity, R.xml.test_position_property_animatino_with_full_info, mStage);
        assertEquals(image , move.getTarget());

        /* TC 2: test "position" PropertyAnimation -- start property info in xml */
        Point startP = new Point(0, 0, 0);
        assertEquals(startP , move.getStartValue());

        /* TC 3: test "position" PropertyAnimation -- end property info in xml */
        Point endP = new Point(480, 800, 0);
        assertEquals(endP , move.getEndValue());

        /* TC 4: test "position" PropertyAnimation -- mode info in xml */
        assertEquals(Mode.EASE_IN_OUT_CUBIC, move.getMode());

        /* TC 5: test "position" PropertyAnimation -- loop info in xml */
        assertEquals(true, move.getLoop());

        /* TC 6: test "position" PropertyAnimation -- autoReverse info in xml */
        assertEquals(true, move.getAutoReverse());

        /* TC 7: test "position" PropertyAnimation -- duration info in xml */
        assertEquals(2000, move.getDuration());

        /* TC 8: test "position" PropertyAnimation with partial info in xml */
        BasicAnimation move2 = Ngin3dAnimationInflater.inflateAnimation(mActivity, R.xml.test_position_property_animatino_with_partial_info, mStage);
        assertEquals(image , move2.getTarget());

        /* TC 9: test "position" PropertyAnimation -- mode default value */
        assertEquals(Mode.LINEAR, move2.getMode());

        /* TC 10: test "position" PropertyAnimation -- loop default value */
        assertEquals(false, move2.getLoop());

        /* TC 11: test "position" PropertyAnimation -- autoReverse default value */
        assertEquals(false, move2.getAutoReverse());

        /* TC 12: test "position" PropertyAnimation -- duration default value */
        assertEquals(2000, move2.getDuration());

        /* TC 13: test "position" PropertyAnimation -- without target info */
        BasicAnimation move3 = Ngin3dAnimationInflater.inflateAnimation(mActivity, R.xml.test_position_property_animatino_without_target_info, mStage);
        assertNull(move3.getTarget());

        /* TC 14: test "position" PropertyAnimation -- without property info, invalid case*/
        try {
            BasicAnimation move4 = Ngin3dAnimationInflater.inflateAnimation(mActivity, R.xml.test_position_property_animatino_without_property_info, mStage);
            fail("Should throw exception.");
        } catch (IllegalArgumentException e) {
            //expected
        }

    }

    public void testAnimationRotationPropertyAnimation() {

        Image image = (Image)Ngin3dLayoutInflater.inflateLayout(mActivity, R.xml.test_single_actor_image_with_full_info, mStage);
        assertNotNull(image);

        /* TC 1: test "rotation" PropertyAnimation with full info in xml */
        PropertyAnimation rotation = (PropertyAnimation)Ngin3dAnimationInflater.inflateAnimation(mActivity, R.xml.test_rotation_property_animatino_with_full_info, mStage);
        assertEquals(image , rotation.getTarget());

        /* TC 2: test "rotation" PropertyAnimation -- start property info in xml */
        Rotation startR = new Rotation(0, 0, 0);
        assertEquals(startR , rotation.getStartValue());

        /* TC 3: test "rotation" PropertyAnimation -- end property info in xml */
        Rotation endR = new Rotation(0, 0, 360);
        assertEquals(endR, rotation.getEndValue());

        /* TC 4: test "rotation" PropertyAnimation -- mode info in xml */
        assertEquals(Mode.EASE_IN_SINE, rotation.getMode());

        /* TC 5: test "rotation" PropertyAnimation -- loop info in xml */
        assertEquals(true, rotation.getLoop());

        /* TC 6: test "rotation" PropertyAnimation -- autoReverse info in xml */
        assertEquals(true, rotation.getAutoReverse());

        /* TC 7: test "rotation" PropertyAnimation -- duration info in xml */
        assertEquals(2000, rotation.getDuration());

        /* TC 8: test "rotation" PropertyAnimation with partial info in xml */
        BasicAnimation rotation2 = Ngin3dAnimationInflater.inflateAnimation(mActivity, R.xml.test_rotation_property_animatino_with_partial_info, mStage);
        assertEquals(image , rotation2.getTarget());

        /* TC 9: test "rotation" PropertyAnimation -- mode default value */
        assertEquals(Mode.LINEAR, rotation2.getMode());

        /* TC 10: test "rotation" PropertyAnimation -- loop default value */
        assertEquals(false, rotation2.getLoop());

        /* TC 11: test "rotation" PropertyAnimation -- autoReverse default value */
        assertEquals(false, rotation2.getAutoReverse());

        /* TC 12: test "rotation" PropertyAnimation -- duration default value */
        assertEquals(2000, rotation2.getDuration());

        /* TC 13: test "rotation" PropertyAnimation -- without target info */
        BasicAnimation rotation3 = Ngin3dAnimationInflater.inflateAnimation(mActivity, R.xml.test_rotation_property_animatino_without_target_info, mStage);
        assertNull(rotation3.getTarget());

        /* TC 14: test "rotation" PropertyAnimation -- without property info, invalid case*/
        try {
            BasicAnimation rotation4 = Ngin3dAnimationInflater.inflateAnimation(mActivity, R.xml.test_rotation_property_animatino_without_property_info, mStage);
            fail("Should throw exception.");
        } catch (IllegalArgumentException e) {
            //expected
        }
    }

    public void testAnimationScalePropertyAnimation() {

        Image image = (Image)Ngin3dLayoutInflater.inflateLayout(mActivity, R.xml.test_single_actor_image_with_full_info, mStage);
        assertNotNull(image);

        /* TC 1: test "scale" PropertyAnimation with full info in xml */
        PropertyAnimation scale = (PropertyAnimation)Ngin3dAnimationInflater.inflateAnimation(mActivity, R.xml.test_scale_property_animatino_with_full_info, mStage);
        assertEquals(image , scale.getTarget());

        /* TC 2: test "scale" PropertyAnimation -- start property info in xml */
        Scale startS = new Scale(1, 1, 1);
        assertEquals(startS , scale.getStartValue());

        /* TC 3: test "scale" PropertyAnimation -- end property info in xml */
        Scale endS = new Scale(2, 2, 2);
        assertEquals(endS, scale.getEndValue());

        /* TC 4: test "scale" PropertyAnimation -- mode info in xml */
        assertEquals(Mode.EASE_IN_OUT_QUAD, scale.getMode());

        /* TC 5: test "scale" PropertyAnimation -- loop info in xml */
        assertEquals(true, scale.getLoop());

        /* TC 6: test "scale" PropertyAnimation -- autoReverse info in xml */
        assertEquals(true, scale.getAutoReverse());

        /* TC 7: test "scale" PropertyAnimation -- duration info in xml */
        assertEquals(2000, scale.getDuration());

        /* TC 8: test "scale" PropertyAnimation with partial info in xml */
        BasicAnimation scale2 = Ngin3dAnimationInflater.inflateAnimation(mActivity, R.xml.test_scale_property_animatino_with_partial_info, mStage);
        assertEquals(image , scale2.getTarget());

        /* TC 9: test "scale" PropertyAnimation -- mode default value */
        assertEquals(Mode.LINEAR, scale2.getMode());

        /* TC 10: test "scale" PropertyAnimation -- loop default value */
        assertEquals(false, scale2.getLoop());

        /* TC 11: test "scale" PropertyAnimation -- autoReverse default value */
        assertEquals(false, scale2.getAutoReverse());

        /* TC 12: test "scale" PropertyAnimation -- duration default value */
        assertEquals(2000, scale2.getDuration());

        /* TC 13: test "scale" PropertyAnimation -- without target info */
        BasicAnimation scale3 = Ngin3dAnimationInflater.inflateAnimation(mActivity, R.xml.test_scale_property_animatino_without_target_info, mStage);
        assertNull(scale3.getTarget());

        /* TC 14: test "scale" PropertyAnimation -- without property info, invalid case*/
        try {
            BasicAnimation scale4 = Ngin3dAnimationInflater.inflateAnimation(mActivity, R.xml.test_scale_property_animatino_without_property_info, mStage);
            fail("Should throw exception.");
        } catch (IllegalArgumentException e) {
            //expected
        }
    }

    public void testAnimationColorPropertyAnimation() {

        Image image = (Image)Ngin3dLayoutInflater.inflateLayout(mActivity, R.xml.test_single_actor_image_with_full_info, mStage);
        assertNotNull(image);

        /* TC 1: test "color" PropertyAnimation with full info in xml */
        PropertyAnimation color = (PropertyAnimation)Ngin3dAnimationInflater.inflateAnimation(mActivity, R.xml.test_color_property_animatino_with_full_info, mStage);
        assertEquals(image , color.getTarget());

        /* TC 2: test "color" PropertyAnimation -- start property info in xml */
        Color startC = new Color(Color.WHITE.getRgb());
        assertEquals(startC , color.getStartValue());

        /* TC 3: test "color" PropertyAnimation -- end property info in xml */
        Color endC = new Color(Color.BLACK.getRgb());
        assertEquals(endC, color.getEndValue());

        /* TC 4: test "color" PropertyAnimation -- mode info in xml */
        assertEquals(Mode.EASE_IN_OUT_QUAD, color.getMode());

        /* TC 5: test "color" PropertyAnimation -- loop info in xml */
        assertEquals(true, color.getLoop());

        /* TC 6: test "color" PropertyAnimation -- autoReverse info in xml */
        assertEquals(true, color.getAutoReverse());

        /* TC 7: test "color" PropertyAnimation -- duration info in xml */
        assertEquals(2000, color.getDuration());

        /* TC 8: test "color" PropertyAnimation with partial info in xml */
        BasicAnimation color2 = Ngin3dAnimationInflater.inflateAnimation(mActivity, R.xml.test_color_property_animatino_with_partial_info, mStage);
        assertEquals(image , color2.getTarget());

        /* TC 9: test "color" PropertyAnimation -- mode default value */
        assertEquals(Mode.LINEAR, color2.getMode());

        /* TC 10: test "color" PropertyAnimation -- loop default value */
        assertEquals(false, color2.getLoop());

        /* TC 11: test "color" PropertyAnimation -- autoReverse default value */
        assertEquals(false, color2.getAutoReverse());

        /* TC 12: test "color" PropertyAnimation -- duration default value */
        assertEquals(2000, color2.getDuration());

        /* TC 13: test "color" PropertyAnimation -- without target info */
        BasicAnimation color3 = Ngin3dAnimationInflater.inflateAnimation(mActivity, R.xml.test_color_property_animatino_without_target_info, mStage);
        assertNull(color3.getTarget());

        /* TC 14: test "color" PropertyAnimation -- without property info, invalid case*/
        try {
            BasicAnimation color4 = Ngin3dAnimationInflater.inflateAnimation(mActivity, R.xml.test_color_property_animatino_without_property_info, mStage);
            fail("Should throw exception.");
        } catch (IllegalArgumentException e) {
            //expected
        }
    }

    public void testAnimationOpacityPropertyAnimation() {

        Image image = (Image)Ngin3dLayoutInflater.inflateLayout(mActivity, R.xml.test_single_actor_image_with_full_info, mStage);
        assertNotNull(image);

        /* TC 1: test "opacity" PropertyAnimation with full info in xml */
        PropertyAnimation opacity = (PropertyAnimation)Ngin3dAnimationInflater.inflateAnimation(mActivity, R.xml.test_opacity_property_animatino_with_full_info, mStage);
        assertEquals(image , opacity.getTarget());

        /* TC 2: test "opacity" PropertyAnimation -- start property info in xml */
        Integer startO = 0;
        assertEquals(startO , opacity.getStartValue());

        /* TC 3: test "opacity" PropertyAnimation -- end property info in xml */
        Integer endO = 255;
        assertEquals(endO, opacity.getEndValue());

        /* TC 4: test "opacity" PropertyAnimation -- mode info in xml */
        assertEquals(Mode.EASE_IN_OUT_QUAD, opacity.getMode());

        /* TC 5: test "opacity" PropertyAnimation -- loop info in xml */
        assertEquals(true, opacity.getLoop());

        /* TC 6: test "opacity" PropertyAnimation -- autoReverse info in xml */
        assertEquals(true, opacity.getAutoReverse());

        /* TC 7: test "opacity" PropertyAnimation -- duration info in xml */
        assertEquals(2000, opacity.getDuration());

        /* TC 8: test "opacity" PropertyAnimation with partial info in xml */
        BasicAnimation opacity2 = Ngin3dAnimationInflater.inflateAnimation(mActivity, R.xml.test_opacity_property_animatino_with_partial_info, mStage);
        assertEquals(image , opacity2.getTarget());

        /* TC 9: test "opacity" PropertyAnimation -- mode default value */
        assertEquals(Mode.LINEAR, opacity2.getMode());

        /* TC 10: test "opacity" PropertyAnimation -- loop default value */
        assertEquals(false, opacity2.getLoop());

        /* TC 11: test "opacity" PropertyAnimation -- autoReverse default value */
        assertEquals(false, opacity2.getAutoReverse());

        /* TC 12: test "opacity" PropertyAnimation -- duration default value */
        assertEquals(2000, opacity2.getDuration());

        /* TC 13: test "opacity" PropertyAnimation -- without target info */
        BasicAnimation opacity3 = Ngin3dAnimationInflater.inflateAnimation(mActivity, R.xml.test_opacity_property_animatino_without_target_info, mStage);
        assertNull(opacity3.getTarget());

        /* TC 14: test "opacity" PropertyAnimation -- without property info, invalid case*/
        try {
            BasicAnimation opacity4 = Ngin3dAnimationInflater.inflateAnimation(mActivity, R.xml.test_opacity_property_animatino_without_property_info, mStage);
            fail("Should throw exception.");
        } catch (IllegalArgumentException e) {
            //expected
        }
    }

    public void testAnimationKeyframeAnimation() {

        Image image = (Image)Ngin3dLayoutInflater.inflateLayout(mActivity, R.xml.test_single_actor_image_with_full_info, mStage);
        assertNotNull(image);

        /* TC 1: test KeyframeAnimation with full info in xml */
        BasicAnimation keyframeAnimation = (BasicAnimation)Ngin3dAnimationInflater.inflateAnimation(mActivity, R.xml.test_keyframe_animation_with_full_info, mStage);
        assertEquals(image , keyframeAnimation.getTarget());

        /* TC 2: test KeyframeAnimation -- loop info in xml */
        assertEquals(true, keyframeAnimation.getLoop());

        /* TC 3: test KeyframeAnimation -- autoReverse info in xml */
        assertEquals(true, keyframeAnimation.getAutoReverse());

        /* TC 4: test KeyframeAnimation with partial info in xml */
        BasicAnimation keyframeAnimation2 = (BasicAnimation)Ngin3dAnimationInflater.inflateAnimation(mActivity, R.xml.test_keyframe_animation_with_partial_info, mStage);
        assertEquals(image , keyframeAnimation2.getTarget());

        /* TC 5: test KeyframeAnimation -- loop default valuel */
        assertEquals(true, keyframeAnimation2.getLoop());

        /* TC 6: test KeyframeAnimation -- autoReverse default value */
        assertEquals(true, keyframeAnimation2.getAutoReverse());

        /* TC 7: test KeyframeAnimation without target info in xml */
        BasicAnimation keyframeAnimation3 = (BasicAnimation)Ngin3dAnimationInflater.inflateAnimation(mActivity, R.xml.test_keyframe_animation_without_target_info, mStage);
        assertNull(keyframeAnimation3.getTarget());

        /* TC 8: test KeyframeAnimation without keyframe info in xml */
        try {
            BasicAnimation keyframeAnimation4 = Ngin3dAnimationInflater.inflateAnimation(mActivity, R.xml.test_keyframe_animation_without_keyframe_info, mStage);
            fail("Should throw exception.");
        } catch (IllegalArgumentException e) {
            //expected
        }
    }

    public void testAnimationAnimationGroup() {

        Image image = (Image) Ngin3dLayoutInflater.inflateLayout(mActivity, R.xml.test_single_actor_image_with_full_info, mStage);
        assertNotNull(image);

        /* TC 1: test AnimationGroup with full info in xml */
        AnimationGroup animationGroup = (AnimationGroup)Ngin3dAnimationInflater.inflateAnimation(mActivity, R.xml.test_single_animation_group_with_full_info, mStage);
        assertEquals(image , animationGroup.getTarget());

        /* TC 2: test AnimationGroup -- loop info in xml */
        assertEquals(true, animationGroup.getLoop());

        /* TC 3: test AnimationGroup -- autoReverse info in xml */
        assertEquals(true, animationGroup.getAutoReverse());

        /* TC 4: test AnimationGroup -- animation count in xml */
        assertEquals(4 , animationGroup.getAnimationCount());

        /* TC 5: test AnimationGroup with partial info in xml */
        AnimationGroup animationGroup2 = (AnimationGroup)Ngin3dAnimationInflater.inflateAnimation(mActivity, R.xml.test_single_animation_group_with_partial_info, mStage);
        assertEquals(image , animationGroup2.getTarget());

        /* TC 6: test AnimationGroup -- loop info in xml */
        assertEquals(false, animationGroup2.getLoop());

        /* TC 7: test AnimationGroup -- autoReverse info in xml */
        assertEquals(false, animationGroup2.getAutoReverse());

        /* TC 8: test AnimationGroup -- animation count in xml */
        assertEquals(2 , animationGroup2.getAnimationCount());

        /* TC 9: test AnimationGroup without target info in xml */
        AnimationGroup animationGroup3 = (AnimationGroup)Ngin3dAnimationInflater.inflateAnimation(mActivity, R.xml.test_single_animation_group_without_target_info, mStage);
        assertNull(animationGroup3.getTarget());

        /* TC 10: test nested AnimationGroup */
        AnimationGroup animationGroup4 = (AnimationGroup) Ngin3dAnimationInflater.inflateAnimation(mActivity, R.xml.test_nested_animation_group, mStage);
        assertEquals(image , animationGroup2.getTarget());

        /* TC 11: test AnimationGroup -- animation count in xml */
        /* failed case, need ngin3d's support 
        assertEquals(5 , animationGroup4.getAnimationCount());  */

        /* TC 12: test AnimationGroup -- check one of the child's info in xml */
        /* failed case, need ngin3d's support 
        BasicAnimation childAnimation = (BasicAnimation)animationGroup4.getAnimationByTag(R.id.keyframe_animation3);
        assertEquals(false , childAnimation.getLoop());
        assertEquals(false , childAnimation.getAutoReverse()); */
    }

}
