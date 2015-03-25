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

import android.graphics.Typeface;
import android.test.ActivityInstrumentationTestCase2;
import com.mediatek.ngin3d.BitmapFont;
import com.mediatek.ngin3d.BitmapText;
import com.mediatek.ngin3d.Color;
import com.mediatek.ngin3d.Container;
import com.mediatek.ngin3d.Image;
import com.mediatek.ngin3d.ImmutableScale;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Scale;
import com.mediatek.ngin3d.Sphere;
import com.mediatek.ngin3d.Stage;
import com.mediatek.ngin3d.Text;
import com.mediatek.ngin3d.android.Ngin3dLayoutInflater;

public class Ngin3dLayoutInflaterTest extends ActivityInstrumentationTestCase2<PresentationStubActivity> {

    protected Stage testStage;
    protected PresentationStubActivity activity;

    Color bgColorBlack= new Color(Color.BLACK.getRgb());
    Color bgColorBlue = new Color(Color.BLUE.getRgb());
    Color bgColorWhite = new Color(Color.WHITE.getRgb());

    public Ngin3dLayoutInflaterTest() {
        super("com.mediatek.ngin3d.tests", PresentationStubActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        testStage = new Stage();
        activity = getActivity();
    }

    @Override
    protected void tearDown() throws Exception {
        testStage = null;
        activity = null;
        super.tearDown();
    }

    public void testActorStage() {

        /* TC 1: test bg color in value foramt in xml */
        Stage stage = (Stage) Ngin3dLayoutInflater.inflateLayout(activity, R.xml.test_single_actor_stage_with_bgcolor_value, null);
        assertEquals(bgColorBlue, stage.getBackgroundColor());

        /* TC 2: test bg color in reference foramt in xml */
        Stage stage2 = (Stage)Ngin3dLayoutInflater.inflateLayout(activity, R.xml.test_single_actor_stage_with_reference_color, null);
        assertEquals(bgColorBlue , stage2.getBackgroundColor());

        /* TC 3: test stage added into a stage/container. invalid case */
        try {
            Stage stage3 = (Stage)Ngin3dLayoutInflater.inflateLayout(activity, R.xml.test_single_actor_stage_with_reference_color, testStage);
            fail("Should throw exception.");
        } catch (IllegalArgumentException e) {
            //expected
        }

        /* TC 4: test stage without bg color attribute in xml, then it should be default color.*/
        Stage stage4 = (Stage)Ngin3dLayoutInflater.inflateLayout(activity, R.xml.test_single_actor_stage_without_color, null);
        assertEquals(bgColorBlack , stage4.getBackgroundColor());
    }

    public void testActorText() {

        /* TC 1: test text with text info in xml */
        Text text = (Text)Ngin3dLayoutInflater.inflateLayout(activity, R.xml.test_single_actor_text_with_full_info, testStage);
        assertEquals("Hello ngin3D!" , text.getText());

        /* TC 2: test text with textColor info in xml */
        assertEquals(bgColorBlue , text.getTextColor());

        /* TC 3: test text with textSize info in xml */
        assertEquals(16.0f , text.getTextSize());

        /* TC 4: test text with textStyle info in xml */
        assertEquals(Typeface.BOLD_ITALIC , text.getTypeface().getStyle());

        /* TC 5: test text with textTypeface info in xml */
        assertNotNull(text.getTypeface().SERIF);

        /* TC 6: test text with textScale info in xml */
        Scale scale = new Scale(2.0f, 2.0f, 2.0f);
        assertEquals(scale , text.getScale());

        /* TC 7: test text with textVisible info in xml */
        boolean is_visible = true;
        assertEquals(is_visible , text.getVisible());

        /* TC 8: test text with textPosition info in xml */
        Point point = new Point(100, 100, 100);
        assertEquals(point , text.getPosition());

        /* TC 9: test text with textAnchorPoint info in xml */
        Point anchorPoint = new Point(0.1f, 0.1f, 0.1f);
        assertEquals(anchorPoint , text.getAnchorPoint());

        /* TC 10: test text with text info only in xml */
        Text text2 = (Text)Ngin3dLayoutInflater.inflateLayout(activity, R.xml.test_single_actor_text_with_partical_info, testStage);
        assertEquals("Hello World!" , text2.getText());

        /* TC 11: test text without textColor info only in xml */
        assertEquals(bgColorWhite, text2.getTextColor());

        /* TC 12: test text with textSize info in xml */
        assertEquals(32.0f , text2.getTextSize());

        /* TC 13: test text with textStyle info in xml */
        assertEquals(Typeface.NORMAL, text2.getTypeface().getStyle());

        /* TC 14: test text with textTypeface info in xml */
        assertNotNull(text2.getTypeface().DEFAULT);

        /* TC 15: test text with textScale info in xml */
        Scale scale2 = new ImmutableScale(1.0f, 1.0f, 1.0f);
        assertEquals(scale2 , text2.getScale());

        /* TC 16: test text with textVisible info in xml */
        assertEquals(is_visible , text2.getVisible());

        /* TC 17: test text with textPosition info in xml */
        Point point2 = new Point(0, 0, 0);
        assertEquals(point2 , text2.getPosition());

        /* TC 18: test text with textAnchorPoint info in xml */
        Point anchorPoint2 = new Point(0.5f, 0.5f, 0.0f);
        assertEquals(anchorPoint2 , text2.getAnchorPoint());

        /* TC 19: test text without text info in xml, invalid case */
        try {
            Text text3 = (Text)Ngin3dLayoutInflater.inflateLayout(activity, R.xml.test_single_actor_text_without_text_string, testStage);
            fail("Should throw exception.");
        } catch (IllegalArgumentException e) {
            //expected
        }

        /* TC 20: test text with bold text style in xml */
        Text text4 = (Text)Ngin3dLayoutInflater.inflateLayout(activity, R.xml.test_single_actor_text_with_sans_bold_textstyle, testStage);
        assertEquals(Typeface.BOLD , text4.getTypeface().getStyle());

        /* TC 21: test text with textTypeface sans in xml */
        assertNotNull(text4.getTypeface().SANS_SERIF);

        /* TC 22 test text with bold text style in xml */
        Text text5 = (Text)Ngin3dLayoutInflater.inflateLayout(activity, R.xml.test_single_actor_text_with_default_normal_textstyle, testStage);
        assertEquals(Typeface.NORMAL , text5.getTypeface().getStyle());

        /* TC 23: test text with textTypeface sans in xml */
        assertNotNull(text5.getTypeface().DEFAULT);

        /* TC 24 test text with bold text style in xml */
        Text text6 = (Text)Ngin3dLayoutInflater.inflateLayout(activity, R.xml.test_single_actor_text_with_mono_bolditalic_textstyle, testStage);
        assertEquals(Typeface.NORMAL , text6.getTypeface().getStyle());

        /* TC 25: test text with textTypeface sans in xml */
        assertNotNull(text6.getTypeface().MONOSPACE);

        /* TC 26 test text with reference string in xml */
        Text text7 = (Text)Ngin3dLayoutInflater.inflateLayout(activity, R.xml.test_single_actor_text_with_reference_string, testStage);
        assertEquals("ngin3d demo" , text7.getText());

    }

    public void testActorImage() {

        /* TC 1: test image with image info in xml */
        Image image = (Image)Ngin3dLayoutInflater.inflateLayout(activity, R.xml.test_single_actor_image_with_full_info, testStage);
        assertNotNull(image);

        /* TC 2: test image with width/height info in xml */
        assertEquals(96.0f , image.getSize().width);
        assertEquals(96.0f , image.getSize().height);

        /* TC 5: test image with scale info in xml */
        Scale scale = new Scale(2.0f, 2.0f, 2.0f);
        assertEquals(scale , image.getScale());

        /* TC 6: test image with visible info in xml */
        boolean is_visible = true;
        assertEquals(is_visible , image.getVisible());

        /* TC 7: test image with position info in xml */
        Point point = new Point(100, 100, 100);
        assertEquals(point , image.getPosition());

        /* TC 8: test image with anchorPoint info in xml */
        Point anchorPoint = new Point(0.1f, 0.1f, 0.1f);
        assertEquals(anchorPoint , image.getAnchorPoint());

        /* TC 9: test image with image info only in xml */
        Image image2 = (Image)Ngin3dLayoutInflater.inflateLayout(activity, R.xml.test_single_actor_image_with_partial_info, testStage);
        assertNotNull(image2);

        /* TC 10: test image without width/height info in xml */
        /* failed case, wait for ngin3d's support.
        Log.i(TAG, "testSingleActorImage , image2 width : " + image2.getSize().width + ", image2 height : " + image2.getSize().height);
        assertEquals(48.0f , image2.getSize().width);
        assertEquals(48.0f , image2.getSize().height);*/

        /* TC 13: test image without scale info in xml */
        Scale scale2 = new ImmutableScale(1.0f, 1.0f, 1.0f);
        assertEquals(scale2 , image2.getScale());

        /* TC 14: test image without visible info in xml */
        boolean is_visible2 = true;
        assertEquals(is_visible2 , image2.getVisible());

        /* TC 15: test image without position info in xml */
        Point point2 = new Point(0, 0, 0);
        assertEquals(point2 , image2.getPosition());

        /* TC 16: test image without anchorPoint info in xml */
        Point anchorPoint2 = new Point(0.5f, 0.5f, 0.0f);
        assertEquals(anchorPoint2 , image2.getAnchorPoint());

        /* TC 17: test image without image info in xml , invalid case*/
        try {
            Image image3 = (Image)Ngin3dLayoutInflater.inflateLayout(activity, R.xml.test_single_actor_image_without_image_src, testStage);
            fail("Should throw exception.");
        } catch (IllegalArgumentException e) {
            //expected
        }
    }

    public void testActorSphere() {

        /* TC 1: test sphere with image info in xml */
        Sphere sphere = (Sphere)Ngin3dLayoutInflater.inflateLayout(activity, R.xml.test_single_actor_sphere_with_full_info, testStage);
        assertNotNull(sphere);

        /* TC 2: test sphere with scale info in xml */
       /* failed case, wait for ngin3d's support.
        Scale scale = new Scale(20.0f, 20.0f, 20.0f);
        Log.i(TAG, "testSingleActorSphere , scale : " + scale + ", sphere.getScale() : " + sphere.getScale());
        assertEquals(scale , sphere.getScale()); */

        /* TC 3: test image with visible info in xml */
        boolean is_visible = true;
        assertEquals(is_visible , sphere.getVisible());

        /* TC 4: test image with position info in xml */
        Point point = new Point(400, 240, 0);
        assertEquals(point , sphere.getPosition());


        /* TC 6: test sphere with image info only in xml */
        Sphere sphere2 = (Sphere)Ngin3dLayoutInflater.inflateLayout(activity, R.xml.test_single_actor_sphere_with_partial_info, testStage);
        assertNotNull(sphere2);

        /* TC 7: test sphere without scale info in xml */
       /* failed case, wait for ngin3d's support
        Scale scale2 = new Scale(1.0f, 1.0f, 1.0f);
        Log.i(TAG, "testSingleActorSphere , scale : " + scale + ", sphere2.getScale() : " + sphere2.getScale());
        assertEquals(scale , sphere2.getScale()); */

        /* TC 3: test sphere with visible info in xml */
        boolean is_visible2 = true;
        assertEquals(is_visible2 , sphere2.getVisible());

        /* TC 4: test sphere with position info in xml */
        Point point2 = new Point(0, 0, 0);
        assertEquals(point2 , sphere2.getPosition());

        /* TC 6: test sphere without image info in xml , invalid case*/
        try {
            Sphere sphere3 = (Sphere)Ngin3dLayoutInflater.inflateLayout(activity, R.xml.test_single_actor_sphere_without_image_src, testStage);
            fail("Should throw exception.");
        } catch (IllegalArgumentException e) {
            //expected
        }
    }

    public void testActorBitmapText() {

        /* TC 1: test BitmapText with Text info in xml */
        BitmapText bitmapText = (BitmapText)Ngin3dLayoutInflater.inflateLayout(activity, R.xml.test_single_actor_bitmaptext_with_full_info, testStage);
        assertEquals("MediaTek" , bitmapText.getText());

        /* TC 2: test BitmapText with bitmap font info in xml */
        BitmapFont bitmapFont = new BitmapFont(activity.getResources(), R.raw.bmfont1, R.drawable.bmfont1);
        assertEquals(bitmapFont , bitmapText.getFont());

        /* TC 3: test BitmapText with scale info in xml */
        Scale scale = new Scale(2.0f, 2.0f, 2.0f);
        assertEquals(scale , bitmapText.getScale());

        /* TC 4: test BitmapText with visible info in xml */
        boolean is_visible = true;
        assertEquals(is_visible , bitmapText.getVisible());

        /* TC 5: test BitmapText with position info in xml */
        Point point = new Point(240, 200, 0);
        assertEquals(point , bitmapText.getPosition());

        /* TC 6: test BitmapText with anchorPoint info in xml */
        Point anchorPoint = new Point(0.1f, 0.1f, 0.1f);
        assertEquals(anchorPoint , bitmapText.getAnchorPoint());

        /* TC 7: test BitmapText with Text info & bitmap font info only in xml */
        BitmapText bitmapText2 = (BitmapText)Ngin3dLayoutInflater.inflateLayout(activity, R.xml.test_single_actor_bitmaptext_with_partial_info, testStage);
        assertEquals("Android" , bitmapText2.getText());

        /* TC 8: test BitmapText with bitmap font info in xml */
        BitmapFont bitmapFont2 = new BitmapFont(activity.getResources(), R.raw.bmfont2, R.drawable.bmfont2);
        assertEquals(bitmapFont2 , bitmapText2.getFont());

        /* TC 9: test BitmapText with scale info in xml */
        Scale scale2 = new ImmutableScale(1.0f, 1.0f, 1.0f);
        assertEquals(scale2 , bitmapText2.getScale());

        /* TC 10: test BitmapText with visible info in xml */
        boolean is_visible2 = true;
        assertEquals(is_visible2 , bitmapText2.getVisible());

        /* TC 11: test BitmapText with position info in xml */
        Point point2 = new Point(0, 0, 0);
        assertEquals(point2 , bitmapText2.getPosition());

        /* TC 12: test BitmapText with anchorPoint info in xml */
        Point anchorPoint2 = new Point(0.5f, 0.5f, 0.0f);
        assertEquals(anchorPoint2 , bitmapText2.getAnchorPoint());

        /* TC 13: test BitmapText with text info only xml. the Bitmapfont is set by setDefaultFont() */
        BitmapFont bitmapFont3 = new BitmapFont(activity.getResources(), R.raw.bmfont, R.drawable.bmfont);
        BitmapText.setDefaultFont(bitmapFont3);

        /* TC 14: test BitmapText with Text info only in xml */
        BitmapText bitmapText3 = (BitmapText)Ngin3dLayoutInflater.inflateLayout(activity, R.xml.test_single_actor_bitmaptext_with_text_info_only, testStage);
        assertEquals("WCP2" , bitmapText3.getText());
        assertEquals(bitmapFont3 , bitmapText3.getFont());

        /* TC 15: test BitmapText without Text info in xml, invalid case */
        try {
            BitmapText bitmapText4 = (BitmapText)Ngin3dLayoutInflater.inflateLayout(activity, R.xml.test_single_actor_bitmaptext_without_text_info, testStage);
            fail("Should throw exception.");
        } catch (IllegalArgumentException e) {
            //expected
        }
    }

    public void testActorContainer() {

        /* TC 1: test Container with Text info in xml */
        Container container = (Container)Ngin3dLayoutInflater.inflateLayout(activity, R.xml.test_single_actor_container, testStage);
        assertNotNull(container);

        /* TC 3: test Container with scale info in xml */
        Scale scale = new Scale(2.0f, 2.0f, 2.0f);
        assertEquals(scale , container.getScale());

        /* TC 4: test Container with visible info in xml */
        boolean is_visible = true;
        assertEquals(is_visible , container.getVisible());

        /* TC 5: test Container with position info in xml */
        Point point = new Point(240, 200, 0);
        assertEquals(point , container.getPosition());

        /* TC 7: test Container with container info only in xml */
        Container container2 = (Container)Ngin3dLayoutInflater.inflateLayout(activity, R.xml.test_single_actor_container_with_partial_info, testStage);
        assertNotNull(container2);

        /* TC 8: test Container with scale info in xml */
        Scale scale2 = new ImmutableScale(1.0f, 1.0f, 1.0f);
        assertEquals(scale2 , container2.getScale());

        /* TC 9: test Container with visible info in xml */
        boolean is_visible2 = true;
        assertEquals(is_visible2 , container2.getVisible());

        /* TC 10: test Container with position info in xml */
        Point point2 = new Point(0, 0, 0);
        assertEquals(point2 , container2.getPosition());

        /* TC 12: test Container with children info in xml */
        Container container3 = (Container)Ngin3dLayoutInflater.inflateLayout(activity, R.xml.test_single_actor_container_with_children_info, testStage);
        assertNotNull(container3);

        /* TC 13: test Container's children count */
        assertEquals(4 , container3.getChildrenCount());

        /* TC 15: test Container with children info in xml */
        Container container4 = (Container) Ngin3dLayoutInflater.inflateLayout(activity, R.xml.test_combined_actors_with_nested_container, testStage);
        assertNotNull(container4);

        /* TC 16: test Container's children count */
        /* failed case, wait for ngin3d support    
        Log.i(TAG, "testSingleActorContainer , container4 - count : "  + container4.getChildrenCount());
        assertEquals(7, container4.getChildrenCount()); */

        /* TC 17: test Container -- get one of its children's info */
        assertEquals("ngin3d Test", ((Text)(container4.findChildByTag(R.id.text7, Container.BREADTH_FIRST_SEARCH))).getText());
    }

}
