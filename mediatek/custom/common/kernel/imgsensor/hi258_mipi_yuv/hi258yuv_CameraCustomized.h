/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
 * applicable license agreements with MediaTek Inc.     */
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   sensor.h
 *
 * Project:
 * --------
 *   DUMA
 *
 * Description:
 * ------------
 *   Header file of camera customized parameters.
 *
 *
 * Author:
 * -------
 *   Anyuan Huang (MTK70663)
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/

#ifndef __CAMERA_CUSTOMIZED_H
#define __CAMERA_CUSTOMIZED_H

// the angle between handset and sensor placement in clockwise, should be one of 0, 90, 270
#define MAIN_SENSOR_ORIENTATION_ANGLE	90
#define SUB_SENSOR_ORIENTATION_ANGLE	0		// do not care if the sub sensor does not exist


// First, we think you hold the cell phone vertical.
// Second, we suppose the direction of upward is 0
// Third, it is 90, 180, 270 in clockwise
// here we define the main sensor and sub sensor angles to deal with the jpeg orientation
#define MAIN_SENSOR_TO_PHONE_ANGLE	90
#define SUB_SENSOR_TO_PHONE_ANGLE	0


#define CAM_SIZE_QVGA_WIDTH 	320
#define CAM_SIZE_QVGA_HEIGHT 	240
#define CAM_SIZE_VGA_WIDTH 		640
#define CAM_SIZE_VGA_HEIGHT 	480
#define CAM_SIZE_05M_WIDTH 		800
#define CAM_SIZE_05M_HEIGHT 	600
#define CAM_SIZE_1M_WIDTH 		1280
#define CAM_SIZE_1M_HEIGHT 		960
#define CAM_SIZE_2M_WIDTH 		1600
#define CAM_SIZE_2M_HEIGHT 		1200
#define CAM_SIZE_3M_WIDTH 		2048
#define CAM_SIZE_3M_HEIGHT 		1536
#define CAM_SIZE_5M_WIDTH 		2592
#define CAM_SIZE_5M_HEIGHT 		1944

// for main sensor
#define MAIN_NUM_OF_PREVIEW_RESOLUTION 3
#define MAIN_NUM_OF_VIDEO_RESOLUTION 4
#define MAIN_NUM_OF_STILL_RESOLUTION 7
#define MAIN_VIDEO_RESOLUTION_PROFILE		{{176,144},{320,240},{640,480},{720,480}}
#define MAIN_PREVIEW_RESOLUTION_PROFILE	{{232,174},{320,240},{240,320}}
#define MAIN_STILL_RESOLUTION_PROFILE		{{CAM_SIZE_QVGA_WIDTH,CAM_SIZE_QVGA_HEIGHT}, \
									 {CAM_SIZE_VGA_WIDTH,CAM_SIZE_VGA_HEIGHT}, \
									 {CAM_SIZE_05M_WIDTH,CAM_SIZE_05M_HEIGHT}, \
									 {CAM_SIZE_1M_WIDTH,CAM_SIZE_1M_HEIGHT}, \
									 {CAM_SIZE_2M_WIDTH,CAM_SIZE_2M_HEIGHT}, \
									 {CAM_SIZE_3M_WIDTH,CAM_SIZE_3M_HEIGHT}, \
									 {CAM_SIZE_5M_WIDTH,CAM_SIZE_5M_HEIGHT}}

// if sub sensor does not exist, set all the parameters as 0
#define SUB_NUM_OF_PREVIEW_RESOLUTION 0
#define SUB_NUM_OF_VIDEO_RESOLUTION 0
#define SUB_NUM_OF_STILL_RESOLUTION 0
#define SUB_VIDEO_RESOLUTION_PROFILE		{{0,0}}
#define SUB_PREVIEW_RESOLUTION_PROFILE		{{0,0}}
#define SUB_STILL_RESOLUTION_PROFILE		{{0,0}}

//#define NUM_OF_PREVIEW_RESOLUTION 	max(MAIN_NUM_OF_PREVIEW_RESOLUTION,SUB_NUM_OF_PREVIEW_RESOLUTION)
//#define NUM_OF_VIDEO_RESOLUTION 	max(MAIN_NUM_OF_VIDEO_RESOLUTION,SUB_NUM_OF_VIDEO_RESOLUTION)
//#define NUM_OF_STILL_RESOLUTION 	max(MAIN_NUM_OF_STILL_RESOLUTION,SUB_NUM_OF_STILL_RESOLUTION)

#define NUM_OF_VIDEO_STREAM_BUFF	8	// Maximun is 8
#endif
