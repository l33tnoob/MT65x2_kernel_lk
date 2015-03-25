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
 * applicable license agreements with MediaTek Inc.
 */

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////
// ShadingATNTable.h  $Revision$
////////////////////////////////////////////////////////////////////////////////

//! \file  ShadingATNTable.h
//! \brief

INT32 ShadingATNTable[21][6]={
    {     0,	         0,	         0,	         0,       0,       1000000,},    // 0  100%	
    { 35556,	   -195556,	    131111,	    -21111,       0,       1000000,},	 // 1  95%
    {-640000,	   1173333,	   -760000,	    126667,       0,       1000000,},	 // 2  90%
    {-35556,	    -17778,	   -131111,	     34444,       0,       1000000,},    // 3  85%
    {-142222,	    355556,	   -524444,	    111111,       0,       1000000,},	 // 4  80%
    {746667,	  -1546667,	    833333,	   -283333,       0,       1000000,},	 // 5  75%
    {640000,	  -1173333,	    440000,	   -206667,       0,       1000000,},	 // 6  70%
    {533333,	   -800000,	     46667,	   -130000,       0,       1000000,},	 // 7  65%
    {284444,	   -711111,	    408889,	   -382222,       0,       1000000,},	 // 8  60%
    {888889,	  -1902222,	   1037778,	   -474444,       0,       1000000,},	 // 9  55%
    {-497778,	   1031111,	   -875556,	   -157778,       0,       1000000,},	 // 10 50%
    {675556,	  -2008889,	   1531111,	   -747778,       0,       1000000,},	 // 11 45%
    {2133333,	  -4906667,	   3386667,	  -1213333,       0,       1000000,},	 // 12 40%
    {177778,	   -977778,	    975556,	   -825556,       0,       1000000,},	 // 13 35%
    { 71111,	   -604444,	    582222,	   -748889,       0,       1000000,},	 // 14 30%
    {5511111,	 -11751111,	   7522222,	  -2032222,       0,       1000000,},	 // 15 25%
    {3555556,	  -7822222,	   5111111,	  -1644444,       0,       1000000,},	 // 16 20%
    {320000,	  -1333333,	   1180000,	  -1016667,       0,       1000000,},	 // 17 15%
    {-1066667,	   1600000,	   -733333,	   -700000,       0,       1000000,},	 // 18 10%
    {391111,	  -1297778,	   1122222,	  -1165556,       0,       1000000,},	 // 19 5%
    {-924444,	   1884444,	  -1168889,	   -771111,       0,       1000000 },	 // 20 0%
};

