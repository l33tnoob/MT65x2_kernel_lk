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

#ifndef _AE_TUNING_CUSTOM_H
#define _AE_TUNING_CUSTOM_H

#define AE_CYCLE_NUM (6)
#define AE_HIGH_FPS_THRES (300)

#define BIT(pos) (1 << (pos))

// AE cycle
typedef enum
{    
    AE_CYCLE_DUMMY                  = BIT(0),    
    AE_CYCLE_ALGO                     = BIT(1),     // not configure sensor, just run algorithm
    AE_CYCLE_ALGO_CONFIGURE = BIT(2),    
    AE_CYCLE_FLARE                    = BIT(3),    
} AE_CYCLE_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//         P U B L I C    F U N C T I O N    D E C L A R A T I O N              //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
MBOOL isAEEnabled();
MBOOL isAEinPreAFStatue();
AE_PARAM_T const& getAEParam();

const MINT32* getAEActiveCycle(MBOOL bVideoMode);
MINT32 getAECycleNum();
MINT32 getAEHighFPSThreshold();

#endif

