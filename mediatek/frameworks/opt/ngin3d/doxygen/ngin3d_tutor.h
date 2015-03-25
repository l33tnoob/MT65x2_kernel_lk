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

/**************************************************************************
 *
 * Copyright (c) 2012 MediaTek Inc. All Rights Reserved.
 * --------------------
 * This software is protected by copyright and the information contained
 * herein is confidential. The software may not be copied and the information
 * contained herein may not be used or disclosed except with the written
 * permission of MediaTek Inc.
 **************************************************************************/
/** \file
 * Getting-started tutorials (Text only, no code).
 */

/** \defgroup nginTutor Getting started with Ngin3D

This is a short guide to getting started with 3D graphics views and Ngin3D.

The first section you should read is the description of the \ref
nginTutorProjection that explains how the graphics viewpoint is controlled
and the different options you have.

*/

/** \defgroup nginTutorProjection 3D Projections
    \ingroup nginTutor

An explanation of the 3D projections used in Ngin3D.

<h2>BEFORE you begin</h2>

It is critically important that you think about your application carefully
before starting to design an Ngin3D application because there are two
different ways to use the graphics system.

- To have the camera in a fixed place, pointing at a fixed region of
space.  This model is like a theatre where the camera is the audience's
viewpoint and the action is happening on the screen or stage.

- To have the camera under full control so you can move it \e anywhere, point
it \e anywhere, and \e zoom in and out.


The first is best for applications where you want to control your 3D objects as
if they are on a screen in front of the user.  This is the most common case for
user interfaces.  For this mode, use the projection \b UI_PERSPECTIVE.  You
will not be able to move the camera or change its settings (they are calculated
automatically so you won't get them wrong).

The second is necessary if you want to fly the camera around your model like in
a game.  This is the \b PERSPECTIVE model.

\warning Do \b not expect to be able to switch between these projections.  It is
allowed, but the results will be probably be extremely confusing.

There are other projections in Ngin3D:

- ORTHOGRAPHIC

<h2>UI_PERSPECTIVE Mode</h2>

UI_PERSPECTIVE is the default mode in Ngin3D.

In this mode the Z axis is directly away from the viewer, and the 'screen' is
on the XY plane.

\warning The Y axis points \b down in UI_PERSPECTIVE (but up in PERSPECTIVE).

The camera is positioned some distance from the screen in the -Z direction, and
points directly at the centre of this screen area. The origin of the screen
(0,0) is at the top left.

The zoom (Field of view) of the camera is automatically adjusted so that the
'screen' on the XY plane matches the physical screen pixels on the device.

<BR>
\image html proj_ui_01.png "UI_PERSPECTIVE - Camera fixed, pointing at XY screen."

An object placed at (0,0,0) in the world will \e always be visible at the top
left of the screen.  (The yellow ball in the diagram).

It is allowed to change the Z position of the camera, but this is the \b only
parameter that can be changed.  The default value is Z = -1111.

The value of -1111 is used as the default because it is the default set-up of
the Adobe After Effects tool.




<h2>Classical PERSPECTIVE mode</h2>

In this mode the resulting display will be what is in front of the camera, and
\e that will depend where you are pointing the 'lens'.

Everything uses a right-hand coordinate system with Y conventionally meaning
'up'.

The zoom (Field of view) of the camera is under the control of the application.
A smaller field-of-view will make objects appear larger (e.g. the red FOV in
the diagram fills the screen with a small area of the world).

<BR>
\image html proj_classic_01.png "PERSPECTIVE - Camera anywhere, pointing anywhere, variable zoom."

An object placed at (0,0,0) in the world will only be visible if the camera is
pointed in that direction.  (The yellow ball in the diagram is not visible on
the screen).

\warning The default camera position is at (0,0,0). If your scene model is based
here, remember to move the camera away from the scene. (Otherwise the camera is
\e inside the yellow ball!)


<h2>Projection Exercises</h2>

For a walk-through of the visual effects of these different projections, go to
\ref nginTutor01

*/
/* END OF FILE */


