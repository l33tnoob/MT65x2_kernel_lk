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

/**
 * \file
 * Example to demonstrate the two main projection modes.
 */

package com.mediatek.ngin3d.demo;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import com.mediatek.ngin3d.Ngin3d;
import com.mediatek.ngin3d.Glo3D;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Container;
import com.mediatek.ngin3d.Rotation;
import com.mediatek.ngin3d.Scale;
import com.mediatek.ngin3d.Stage;
import com.mediatek.ngin3d.android.StageActivity;
import com.mediatek.ngin3d.android.StageView;
import com.mediatek.ngin3d.demo.R;

import javax.microedition.khronos.opengles.GL10;

/** \ingroup ngin3dDemos

Illustration of the Ngin3D projection modes.

This demo shows a single Glo3D model viewed by a camera demonstrating various
camera parameters. A single model is used as the focus of this demo, but the
same procedures work with a scene of many models.

<h2>Code Walkthrough</h2>

Note that the class extends StageActivity rather than Activity.
\code public class ProjectionsDemo extends StageActivity { \endcode

This conveniently creates a Stage (mStage) for us to place the Actors (the scene
objects) on.

The first operation is to obtain a suitable object and place it on the Stage.
The object model (a GLO format file created in an external tool such as 3DS Max)
is loaded from the assets folder. A Container object is created to hold the set
of objects (the scenario) and the object is added to the Container, and then the
scene is positioned where required.

\code
Glo3D.createFromAsset( <filename> );
scenario.add(landscape);
scenario.setPosition( x,y,z );
\endcode

The object is initially positioned at the origin (0,0,0) and the camera
is initially positioned a distance away along the Z axis and at the same time
it is pointed at the origin. This is achieved by:
\code
mStage.setCamera(
  new Point(CAM_X, CAM_Y, CAM_Z), // Camera position in world
  new Point(OBJ_X, OBJ_Y, OBJ_Z)  // Aim the camera at the object
  );
\endcode

<h2>Exercises</h2>
See \ref nginTutor01

 */
public class ProjectionsDemo extends StageActivity {
    /** Tag to identify log messages */
    private static final String TAG = "ProjectionsDemo";

    /* Camera position in world */
    private static final float CAM_X = 0f;
    private static final float CAM_Y = 0f;
    /* Both PERSPECTIVE and UI_PERSPECTIVE mode are right-handed coordinate
     * systems. We default here to PERSPECTIVE where Y is UP, and thus Z
     * is towards the viewer if X is to be left-to-right. Thus the initial
     * camera position is positive.
     */
    private static final float CAM_Z = 1000f;

    /* Field-of-view of the camera in degrees */
    private static final float CAM_FOV = 20.0f;
    private float camFov = CAM_FOV;

    /* Half-width & half-height of portrait 800x480 screen (in pixels) */
    private static final float HWIDTH = 240f;
    private static final float HHEIGHT = 400f;

    /* Clipping distances for camera */
    private static final float Z_NEAR = 2.f;
    private static final float Z_FAR = 2000.f;

    /* Object's position in world */
    private static final float OBJ_X = 0.f;
    private static final float OBJ_Y = 0.f;
    private static final float OBJ_Z = 0.f;

    /* Scaling factor to apply to model */
    private static final float OBJ_SIZE = 10.0f;
    private float objSize = OBJ_SIZE;

    /* Have we flipped? */
    private boolean isFlipped;

    private Point camPosn = new Point(CAM_X, CAM_Y, CAM_Z);
    private Point objPosn = new Point(OBJ_X, OBJ_Y, OBJ_Z);

    private Container scenario = new Container();

    /**
     * This method creates the scene on start-up.
     * @param savedInstanceState Standard parameter for android activities
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        final Glo3D landscape = Glo3D.createFromAsset("landscape.glo");
        // add a directional light to the scene to illuminate the landscape
        final Glo3D direct_light = Glo3D.createFromAsset("direct_light.glo");
        direct_light.setRotation(new Rotation (30, 30, 0));

        scenario.add(landscape, direct_light);

        resetView();

        /* Add the assembled 'scene' to the stage */
        mStage.add(scenario);
    }


    /**
     * Reset all the scene and camera parameters.
     */
    private void resetView() {
        camFov = CAM_FOV;
        camPosn = new Point(CAM_X, CAM_Y, CAM_Z);
        objPosn = new Point(OBJ_X, OBJ_Y, OBJ_Z);
        objSize = OBJ_SIZE;
        isFlipped = false;

        scenario.setPosition(objPosn);
        scenario.setScale(new Scale(objSize));
        scenario.setRotation(new Rotation(0, 0, 0));

        mStage.setProjection( Stage.PERSPECTIVE, Z_NEAR, Z_FAR, CAM_Z );
        mStage.setCamera( camPosn, objPosn );
        mStage.setCameraFov(camFov);
    }

    /* Excluding the menu handling classes from the Doxygen description. */
    /*! \cond */

    /** Class to support use of the Android menu function */
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.proj_demo_menu, menu);
        return true;
    }

    /** Class to handle the return from the Android menu function */
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int itemId = item.getItemId();

        // Menu is coded as 'reset' then a few parameter changers.
        switch (itemId){
        case R.id.proj_00: // Reset everything
            resetView();
            break;

        case R.id.proj_01: // Halve the field of view = zoom in
            camFov /= 2.0f;
            mStage.setCameraFov(camFov);
            break;

        case R.id.proj_02: // Switch to UI_PERSPECTIVE
            /*
             * Switching between projections NOT RECOMMENDED.  However, this
             * code is here to illustrate the different projections.
             *
             * Set the camera to the default UI_PERSPECTIVE position before
             * we lose control of the camera.  In UI_PERSPECTIVE various
             * camera parameters cannot be changed.
             */
            mStage.setCamera( new Point(HWIDTH, HHEIGHT, -1111.0f),
                              new Point(HWIDTH, HHEIGHT, 0.0f));
            mStage.setCameraFov(45f);

            mStage.setProjection( Stage.UI_PERSPECTIVE, Z_NEAR, Z_FAR, -1111.0f );
            break;

        case R.id.proj_03: // Move object to UI centre
            objPosn = new Point(HWIDTH, HHEIGHT, 0.0f);
            scenario.setPosition(objPosn);
            break;

        case R.id.proj_04: // double size of object
            objSize *= 2.0f;
            scenario.setScale(new Scale(objSize));
            break;

        case R.id.proj_05: // invert, by rotation around X axis
            if( isFlipped == false ) {
              scenario.setRotation(new Rotation(180, 0, 0));
              isFlipped = true;
            }
            else {
              scenario.setRotation(new Rotation(0, 0, 0));
              isFlipped = false;
            }
            break;

        default:
            return false;
        }
        return true;
    }

    /*! \endcond */

}


/*!
 * \defgroup nginTutor01 Exercise #1 Projections with ProjectionsDemo app
 * \ingroup nginTutor

A walk-through demonstration of the effects of camera settings.

The Ngin3D Demo "ProjectionsDemo" shows a single Glo3D model viewed by a camera
demonstrating various camera parameters.  Read the \ref nginTutorProjection
"Explanation of 3D Projections" first!

<h2>Exercises</h2>

Open the ProjectionsDemo app on a handset and follow these instructions.
Please do \b NOT use the menu options until instructed to do so.

If you get lost, use the menu option \b Reset to get back to this starting point.

When you first start up ProjectionsDemo there is a small model in the distance
in the centre of the screen. The \e centre of this model is positioned at the
origin (0,0,0).  The camera is on the Z axis looking directly at the origin.

This initial scene is using classic \b PERSPECTIVE mode. At this moment \b Y is
up, \b X is to your right and \b Z is towards you out of the phone.

<h2>Scale the model</h2>

Using the menu, select \b x2.  This doubles the scaling of the model each time
you press it. Press it a few times. The camera zoom is \e not changing, the
model is getting bigger.

Note that the detail is preserved when the scaling is high.

<h2>Narrow the Field of View</h2>
Reset the scene.

Using the menu select \b FOV/2. Notice that this \e seems to have the same effect
as scaling the model by two.

If you keep pressing this option you will see that the edges of the model
get slightly corrupted.  This is because of rounding errors.  This only happens
at very small FOV angles - less than 2 degrees!!  You would normally view objects
with a FOV of around 45 degrees which is an approximation to the human FOV when
looking at a screen.

<div class="learn">Do not use very small FOV angles.</div>


<h2>Switch to UI_PERSPECTIVE</h2>
Reset the scene.

Using the menu, select <b>UI Mode</b>.

The model is now in the top left corner, slightly smaller and up-side-down!
This \e is expected!

In \b UI_PERSPECTIVE the UI screen 'origin' is in the top left corner of the
screen.  The model is positioned at 0,0,0 so it is now also at the top left of
the screen.

The model is slightly smaller because the default camera Z position in
UI_PERSPECTIVE is \b 1111 units.  In the Classic PERSPECTIVE mode it was set to
\b 1000 units, so it is now a little further away.

The model is up-side-down because the \b Y axis in UI_PERSPECTIVE mode points \b
downwards.  The model's Y axis points upwards, so when you render this model in
this mode it appears up-side-down.

\note The model is not \e reflected in Y, it is 'turned' up-side-down.  Both
perspective modes are right-handed. To bring Y around to point downwards, rotate
the coordinate system around the X axis.  Note Z is now pointing \e away from the
viewer.

<div class="learn">In <i>UI_</i>PERSPECTIVE, 0,0 is top left and Y is down </div>


<h2>Position the model in XY</h2>

In \b UI_PERSPECTIVE the XY coordinate UI screen at Z=0, and screen pixels, are
the same thing.

Using the menu, select <b>240,480</b>. Assuming you are using a 480x800 phone the
model is moved to the centre of the screen - i.e. 240 units ( = pixels ) to the
right in X and 400 \e down in Y.  Remember the Y axis points downwards in
UI_PERSPECTIVE.

\note In Ngin3D you can specify a screen position with normalized coordinates,
e.g. (0.5, 0.5), to mean the centre of the screen.

<div class="learn">In <i>UI_</i>PERSPECTIVE, XY position is in pixels (if Z=0) </div>

<h2>Try Changing the FOV</h2>

Using the menu select \b FOV/2.

<b>Nothing happens</b>.  This is because the FOV is fixed in UI_PERSPECTIVE.  It
is fixed at whatever FOV is required to show all of the stage area (i.e. 800x480
of the XY plane) on the screen.

If you move the camera in Z the FOV changes automatically.  There is no need for
the app to calculate FOV in \b UI_PERSPECTIVE.


 *
 */
