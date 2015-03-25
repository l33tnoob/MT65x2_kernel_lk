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

/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/


#include "camera_custom_types.h"
#include "camera_custom_nvram.h"
#include "awb_feature.h"
#include "awb_param.h"
#include "awb_tuning_custom.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
//adb for log
#include <cutils/xlog.h>
char * mod_cam = "/sys/devices/platform/image_sensor/camera_info";
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
isAWBEnabled()
{
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
isAWBCalibrationBypassed()
{
    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AWB_PARAM_T const&
getAWBParam()
{
    char module_name[32]={0};
    int fd = open(mod_cam, O_RDONLY);
    int res = read(fd,module_name,sizeof(module_name));
    static AWB_PARAM_T rAWBParam;

     if(strstr(module_name,"s5k3hymipiraw"))
     rAWBParam =
    {
    	// Chip dependent parameter
    	{
    	    512, // i4AWBGainOutputScaleUnit: 1.0x = 512 for MT6589
    	   8191, // i4AWBGainOutputUpperLimit: format 4.9 (11 bit) for MT6589
    	    256  // i4RotationMatrixUnit: 1.0x = 256 for MT6589
    	},

        // AWB Light source probability look-up table (Max: 100; Min: 0)
    	{
            AWB_LV_INDEX_NUM, // i4SizeX: horizontal dimension
    	    AWB_LIGHT_NUM, // i4SizeY: vertical dimension
    	    // LUT
    		{ // LV0   1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
		       {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  66,  33,   1,   1,   1,   1,   1,   1,   1}, // Strobe
    			{100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  66,  33,   1,   1,   1,   1,   1,   1,   1}, // Tungsten
    			{100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  66,  33,   1,   1,   1,   1,   1,   1,   1}, // Warm fluorescent
    			{100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  66,  66,   66,   66,   33,   1,   1,   1,   1}, // Fluorescent //LINE <> <DATE20140103> <S5500:darling ov8825 tuning 0103.> wupingzhou
    			{100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  66,  33,   1,   1,   1,   1,   1,   1,   1}, // CWF
    			{100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100}, // Daylight
    			{100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  33,  1,   1,   1,   1,   1,   1}, // Shade
    			{100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  66,  33,   1,   1,   1,   1,   1,   1,   1}  // Daylight fluorescent
    		},
    	},

    	// AWB convergence parameter
    	{
            10, // i4Speed: Convergence speed: (0 ~ 100)
            100,//225 // i4StableThr: Stable threshold ((currentRgain - targetRgain)^2 + (currentBgain - targetBgain)^2), WB gain format: 4.9
    	},

        // AWB daylight locus target offset ratio LUT for tungsten
    	{
    	    AWB_DAYLIGHT_LOCUS_NEW_OFFSET_INDEX_NUM, // i4Size: LUT dimension
    		{// LUT: use daylight locus new offset (0~10000) as index to get daylight locus target offset ratio (0~100)
             // 0  500 1000 1500 2000 2500 3000 3500 4000 4500 5000 5500 6000 6500 7000 7500 8000 8500 9000 9500 10000
    	       50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  55,  60,  65,  70,  75,  80,  85,  90,  95,  100
    		}
    	},

        // AWB daylight locus target offset ratio LUT for warm fluorescent
    	{
    	    AWB_DAYLIGHT_LOCUS_NEW_OFFSET_INDEX_NUM, // i4Size: LUT dimension
    		{// LUT: use daylight locus new offset (0~10000) as index to get daylight locus target offset ratio (0~100)
             // 0  500 1000 1500 2000 2500 3000 3500 4000 4500 5000 5500 6000 6500 7000 7500 8000 8500 9000 9500 10000
    	       50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  55,  60,  65,  70,  75,  80,  85,  90,  95,  100
    		}
    	},

    	// AWB green offset threshold for warm fluorescent
    	{
    	    AWB_DAYLIGHT_LOCUS_OFFSET_INDEX_NUM, // i4Size: LUT dimension
    		{// LUT: use daylight locus offset (0~10000) as index to get green offset threshold
             // 0  500 1000 1500 2000 2500 3000 3500 4000 4500 5000 5500 6000  6500  7000  7500  8000  8500  9000  9500 10000
    	      600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600,  600,  600,  600,  750,   900, 1050, 1200
    		}
    	},

        // AWB light source weight LUT for tungsten light
    	{
            AWB_TUNGSTEN_MAGENTA_OFFSET_INDEX_NUM, // i4Size: LUT dimension
    		{// LUT: use magenta offset (0~1000) as index to get tungsten weight (x/256)
    	     //  0  100  200  300  400  500  600  700  800  900 1000
    	       256, 256, 256, 256, 256, 256, 256, 128,  64,  32,  16
    		}
    	},

        // AWB light source weight LUT for warm fluorescent
    	{
            AWB_WARM_FLUORESCENT_GREEN_OFFSET_INDEX_NUM, // i4Size: LUT dimension
    		{// LUT: use green offset (0~2000) as index to get fluorescent0 weight (x/256)
    	     //  0  200  400  600  800 1000 1200 1400 1600 1800 2000
    	       256, 256, 256, 256, 128,  64,  32,  16,  16,  16,  16
    		}
    	},

        // AWB light source weight LUT for shade light
    	{
            AWB_SHADE_MAGENTA_OFFSET_INDEX_NUM, // i4MagentaLUTSize: Magenta LUT dimension
    		{// MagentaLUT: use magenta offset (0~1000) as index to get shade light weight (x/256)
    	     //  0  100  200  300  400  500  600  700 800 900 1000
    	       256, 256, 256, 256, 128,  64,  32,  16, 16, 16, 16
    		},
    	    AWB_SHADE_GREEN_OFFSET_INDEX_NUM, // i4GreenLUTSize: Green LUT dimension
    		{// GreenLUT: use green offset (0~1000) as index to get shade light weight (x/256)
    	     //  0  100  200  300  400  500  600  700 800 900 1000
    	       256, 256, 256, 256, 256, 128,  64,  32, 16, 16, 16
    		}
    	},

    	// One-shot AWB parameter
    	{
            MFALSE,
    	    10, // LV 1.0
    	    50  // LV 5.0
    	},

    	// Parent block weight parameter
    	{
            MTRUE, // bEnable
            6 // i4ScalingFactor: [6] 1~12, [7] 1~6, [8] 1~3, [9] 1~2, [>=10]: 1
    	},

    	// AWB gain prediction parameter
    	{
            // Strobe
		    {
			       0, // i4IntermediateSceneLvThr_L1: useless
                   0, // i4IntermediateSceneLvThr_H1: useless
    			  90, // i4IntermediateSceneLvThr_L2
                 120, // i4IntermediateSceneLvThr_H2
			       0, // i4DaylightLocusLvThr_L: useless
                   0  // i4DaylightLocusLvThr_H: useless
		    },
            // Tungsten
    		{
    			 100, // i4IntermediateSceneLvThr_L1
                 130, // i4IntermediateSceneLvThr_H1
    			 100, // i4IntermediateSceneLvThr_L2
                 130, // i4IntermediateSceneLvThr_H2
    			  50, // i4DaylightLocusLvThr_L
                 100  // i4DaylightLocusLvThr_H
    		},
            // Warm fluorescent
    		{
    			 100, // i4IntermediateSceneLvThr_L1
                 130, // i4IntermediateSceneLvThr_H1
    			 100, // i4IntermediateSceneLvThr_L2
                 130, // i4IntermediateSceneLvThr_H2
    			  50, // i4DaylightLocusLvThr_L
                 100  // i4DaylightLocusLvThr_H
    		},
            // Fluorescent
    		{
    			   0, // i4IntermediateSceneLvThr_L1: useless
                   0, // i4IntermediateSceneLvThr_H1: useless
    			 105, // i4IntermediateSceneLvThr_L2
                 135, // i4IntermediateSceneLvThr_H2
    			   0, // i4DaylightLocusLvThr_L: useless
                   0  // i4DaylightLocusLvThr_H: useless
    		},
            // CWF
    		{
    			   0, // i4IntermediateSceneLvThr_L1: useless
                   0, // i4IntermediateSceneLvThr_H1: useless
    			  90, // i4IntermediateSceneLvThr_L2
                 120, // i4IntermediateSceneLvThr_H2
    			   0, // i4DaylightLocusLvThr_L: useless
                   0  // i4DaylightLocusLvThr_H: useless
    		},
            // Daylight
    		{
    			   0, // i4IntermediateSceneLvThr_L1: useless
                   0, // i4IntermediateSceneLvThr_H1: useless
    			 140, // i4IntermediateSceneLvThr_L2
                 170, // i4IntermediateSceneLvThr_H2
    			   0, // i4DaylightLocusLvThr_L: useless
                   0  // i4DaylightLocusLvThr_H: useless
    		},
            // Daylight fluorescent
    		{
    			   0, // i4IntermediateSceneLvThr_L1: useless
                   0, // i4IntermediateSceneLvThr_H1: useless
    			  75, // i4IntermediateSceneLvThr_L2
                 120, // i4IntermediateSceneLvThr_H2
    			   0, // i4DaylightLocusLvThr_L: useless
                   0  // i4DaylightLocusLvThr_H: useless
    		},
            // Shade
    		{
    			 100, // i4IntermediateSceneLvThr_L1
                 130, // i4IntermediateSceneLvThr_H1
    			 100, // i4IntermediateSceneLvThr_L2
                 130, // i4IntermediateSceneLvThr_H2
    			  50, // i4DaylightLocusLvThr_L
                 100  // i4DaylightLocusLvThr_H
    		}
    	},

    	// Neutral parent block number threshold
    	{
          // LV0   1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
		    { 5,   5,   5,   5,   6,   7,   10,   10,   10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10},  // m_i4NeutralParentBlkNumThr (%)
		    { 5,   5,   5,   5,   6,   7,  10,   10,   10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10},  // m_i4NeutralParentBlkNumThr_CWF (%)
		    { 5,   5,   5,   5,   6,   7,   10,   10,   10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10}   // m_i4NeutralParentBlkNumThr_DF (%)
    	}
    };

    else
  
    rAWBParam =
    {
    	// Chip dependent parameter
    	{
    	    512, // i4AWBGainOutputScaleUnit: 1.0x = 512 for MT6589
    	   8191, // i4AWBGainOutputUpperLimit: format 4.9 (11 bit) for MT6589
    	    256  // i4RotationMatrixUnit: 1.0x = 256 for MT6589
    	},

        // AWB Light source probability look-up table (Max: 100; Min: 0)
    	{
            AWB_LV_INDEX_NUM, // i4SizeX: horizontal dimension
    	    AWB_LIGHT_NUM, // i4SizeY: vertical dimension
    	    // LUT
    		{ // LV0   1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
			    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  66,  33,   1,   1,   1,   1,   1,   1,   1}, // Strobe
    			{100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  66,  33,   1,   1,   1,   1,   1,   1,   1}, // Tungsten
    			{100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  66,  33,   1,   1,   1,   1,   1,   1,   1}, // Warm fluorescent
    			{100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  66,  66,   66,   66,   33,   1,   1,   1,   1}, // Fluorescent //LINE <> <DATE20140103> <S5500:darling ov8825 tuning 0103.> wupingzhou
    			{100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  66,  33,   1,   1,   1,   1,   1,   1,   1}, // CWF
    			{100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100}, // Daylight
    			{100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  33,  1,   1,   1,   1,   1,   1}, // Shade
    			{100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  66,  33,   1,   1,   1,   1,   1,   1,   1}  // Daylight fluorescent
    		},
    	},

    	// AWB convergence parameter
    	{
            10, // i4Speed: Convergence speed: (0 ~ 100)
            100,//225 // i4StableThr: Stable threshold ((currentRgain - targetRgain)^2 + (currentBgain - targetBgain)^2), WB gain format: 4.9
    	},

        // AWB daylight locus target offset ratio LUT for tungsten
    	{
    	    AWB_DAYLIGHT_LOCUS_NEW_OFFSET_INDEX_NUM, // i4Size: LUT dimension
    		{// LUT: use daylight locus new offset (0~10000) as index to get daylight locus target offset ratio (0~100)
             // 0  500 1000 1500 2000 2500 3000 3500 4000 4500 5000 5500 6000 6500 7000 7500 8000 8500 9000 9500 10000
    	       50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  55,  60,  65,  70,  75,  80,  85,  90,  95,  100
    		}
    	},

        // AWB daylight locus target offset ratio LUT for warm fluorescent
    	{
    	    AWB_DAYLIGHT_LOCUS_NEW_OFFSET_INDEX_NUM, // i4Size: LUT dimension
    		{// LUT: use daylight locus new offset (0~10000) as index to get daylight locus target offset ratio (0~100)
             // 0  500 1000 1500 2000 2500 3000 3500 4000 4500 5000 5500 6000 6500 7000 7500 8000 8500 9000 9500 10000
    	       50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  55,  60,  65,  70,  75,  80,  85,  90,  95,  100
    		}
    	},

    	// AWB green offset threshold for warm fluorescent
    	{
    	    AWB_DAYLIGHT_LOCUS_OFFSET_INDEX_NUM, // i4Size: LUT dimension
    		{// LUT: use daylight locus offset (0~10000) as index to get green offset threshold
             // 0  500 1000 1500 2000 2500 3000 3500 4000 4500 5000 5500 6000  6500  7000  7500  8000  8500  9000  9500 10000
    	      600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600,  600,  600,  600,  750,   900, 1050, 1200
    		}
    	},

        // AWB light source weight LUT for tungsten light
    	{
            AWB_TUNGSTEN_MAGENTA_OFFSET_INDEX_NUM, // i4Size: LUT dimension
    		{// LUT: use magenta offset (0~1000) as index to get tungsten weight (x/256)
    	     //  0  100  200  300  400  500  600  700  800  900 1000
    	       256, 256, 256, 256, 256, 256, 256, 128,  64,  32,  16
    		}
    	},

        // AWB light source weight LUT for warm fluorescent
    	{
            AWB_WARM_FLUORESCENT_GREEN_OFFSET_INDEX_NUM, // i4Size: LUT dimension
    		{// LUT: use green offset (0~2000) as index to get fluorescent0 weight (x/256)
    	     //  0  200  400  600  800 1000 1200 1400 1600 1800 2000
    	       256, 256, 256, 256, 128,  64,  32,  16,  16,  16,  16
    		}
    	},

        // AWB light source weight LUT for shade light
    	{
            AWB_SHADE_MAGENTA_OFFSET_INDEX_NUM, // i4MagentaLUTSize: Magenta LUT dimension
    		{// MagentaLUT: use magenta offset (0~1000) as index to get shade light weight (x/256)
    	     //  0  100  200  300  400  500  600  700 800 900 1000
    	       256, 256, 256, 256, 128,  64,  32,  16, 16, 16, 16
    		},
    	    AWB_SHADE_GREEN_OFFSET_INDEX_NUM, // i4GreenLUTSize: Green LUT dimension
    		{// GreenLUT: use green offset (0~1000) as index to get shade light weight (x/256)
    	     //  0  100  200  300  400  500  600  700 800 900 1000
    	       256, 256, 256, 256, 256, 128,  64,  32, 16, 16, 16
    		}
    	},

    	// One-shot AWB parameter
    	{
            MFALSE,
    	    10, // LV 1.0
    	    50  // LV 5.0
    	},

    	// Parent block weight parameter
    	{
            MTRUE, // bEnable
            6 // i4ScalingFactor: [6] 1~12, [7] 1~6, [8] 1~3, [9] 1~2, [>=10]: 1
    	},

    	// AWB gain prediction parameter
    	{
            // Strobe
		    {
			       0, // i4IntermediateSceneLvThr_L1: useless
                   0, // i4IntermediateSceneLvThr_H1: useless
    			  90, // i4IntermediateSceneLvThr_L2
                 120, // i4IntermediateSceneLvThr_H2
			       0, // i4DaylightLocusLvThr_L: useless
                   0  // i4DaylightLocusLvThr_H: useless
		    },
            // Tungsten
    		{
    			 100, // i4IntermediateSceneLvThr_L1
                 130, // i4IntermediateSceneLvThr_H1
    			 100, // i4IntermediateSceneLvThr_L2
                 130, // i4IntermediateSceneLvThr_H2
    			  50, // i4DaylightLocusLvThr_L
                 100  // i4DaylightLocusLvThr_H
    		},
            // Warm fluorescent
    		{
    			 100, // i4IntermediateSceneLvThr_L1
                 130, // i4IntermediateSceneLvThr_H1
    			 100, // i4IntermediateSceneLvThr_L2
                 130, // i4IntermediateSceneLvThr_H2
    			  50, // i4DaylightLocusLvThr_L
                 100  // i4DaylightLocusLvThr_H
    		},
            // Fluorescent
    		{
    			   0, // i4IntermediateSceneLvThr_L1: useless
                   0, // i4IntermediateSceneLvThr_H1: useless
    			 105, // i4IntermediateSceneLvThr_L2
                 135, // i4IntermediateSceneLvThr_H2
    			   0, // i4DaylightLocusLvThr_L: useless
                   0  // i4DaylightLocusLvThr_H: useless
    		},
            // CWF
    		{
    			   0, // i4IntermediateSceneLvThr_L1: useless
                   0, // i4IntermediateSceneLvThr_H1: useless
    			  90, // i4IntermediateSceneLvThr_L2
                 120, // i4IntermediateSceneLvThr_H2
    			   0, // i4DaylightLocusLvThr_L: useless
                   0  // i4DaylightLocusLvThr_H: useless
    		},
            // Daylight
    		{
    			   0, // i4IntermediateSceneLvThr_L1: useless
                   0, // i4IntermediateSceneLvThr_H1: useless
    			 140, // i4IntermediateSceneLvThr_L2
                 170, // i4IntermediateSceneLvThr_H2
    			   0, // i4DaylightLocusLvThr_L: useless
                   0  // i4DaylightLocusLvThr_H: useless
    		},
            // Daylight fluorescent
    		{
    			   0, // i4IntermediateSceneLvThr_L1: useless
                   0, // i4IntermediateSceneLvThr_H1: useless
    			  90, // i4IntermediateSceneLvThr_L2
                 120, // i4IntermediateSceneLvThr_H2
    			   0, // i4DaylightLocusLvThr_L: useless
                   0  // i4DaylightLocusLvThr_H: useless
    		},
            // Shade
    		{
    			 100, // i4IntermediateSceneLvThr_L1
                 130, // i4IntermediateSceneLvThr_H1
    			 100, // i4IntermediateSceneLvThr_L2
                 130, // i4IntermediateSceneLvThr_H2
    			  50, // i4DaylightLocusLvThr_L
                 100  // i4DaylightLocusLvThr_H
    		}
    	},

    	// Neutral parent block number threshold
    	{
          // LV0   1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
		    { 5,   5,   5,   5,   6,   7,   8,   9,   10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10},  // m_i4NeutralParentBlkNumThr (%)
		    { 5,   5,   5,   5,   6,   7,  8,   9,   10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10},  // m_i4NeutralParentBlkNumThr_CWF (%)
		    { 5,   5,   5,   5,   6,   7,   8,   9,   10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10}   // m_i4NeutralParentBlkNumThr_DF (%)
    	}
    };
    XLOGD("Test awb_tuning_custom.cpp, module_name == %s\n",module_name);
    close(fd);
    return (rAWBParam);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AWB_STAT_PARAM_T const&
getAWBStatParam()
{
    // AWB Statistics Parameter
    static AWB_STAT_PARAM_T rAWBStatParam =
    {
        // Thresholds
    	  1, // Low threshold of R
    	  1, // Low threshold of G
    	  1, // Low threshold of B
    	254, // High threshold of R
    	254, // High threshold of G
    	254, // High threshold of B

        // Pre-gain maximum limit clipping
       	0xFFF, // Maximum limit clipping for R color
       	0xFFF, // Maximum limit clipping for G color
       	0xFFF, // Maximum limit clipping for B color

        // AWB error threshold
       	0 // Programmable threshold for the allowed total over-exposed and under-exposed pixels in one main stat window
    };

    return (rAWBStatParam);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define AWB_CYCLE_NUM_PREVIEW   (4)
#define AWB_CYCLE_NUM_VIDEO (4)

const MINT32*
getAWBActiveCycle_Preview(MINT32 i4SceneLV)
{
    // Default AWB cycle
    static MINT32 i4AWBActiveCycle_Preview[AWB_CYCLE_NUM_PREVIEW] =
    {
        MTRUE,
        MTRUE,//MFALSE,
        MTRUE,
        MTRUE//MFALSE
    };

    return (&i4AWBActiveCycle_Preview[0]);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
const MINT32*
getAWBActiveCycle_Video(MINT32 i4SceneLV)
{
    // Default AWB cycle
    static MINT32 i4AWBActiveCycle_Video[AWB_CYCLE_NUM_VIDEO] =
    {
        MTRUE,
        MTRUE,//MFALSE,
        MTRUE,
        MTRUE//MFALSE
    };

    return (&i4AWBActiveCycle_Video[0]);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
getAWBCycleNum_Preview()
{
    return AWB_CYCLE_NUM_PREVIEW;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
getAWBCycleNum_Video()
{
    return AWB_CYCLE_NUM_VIDEO;
}


