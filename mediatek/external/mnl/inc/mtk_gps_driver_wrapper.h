/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   mtk_gps_driver_wrapper.h
 *
 * Description:
 * ------------
 *   Prototype of  driver layer API 
 *
 ****************************************************************************/

#ifndef MTK_GPS_DRIVER_WRAPPER_H
#define MTK_GPS_DRIVER_WRAPPER_H

#include "mtk_gps_type.h"

#ifdef __cplusplus
  extern "C" {
#endif

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_threads_create
 * DESCRIPTION
 *  Create MNL thread in the porting layer
 * PARAMETERS
 * void
 * RETURNS
 *  MTK_GPS_ERROR / MTK_GPS_SUCCESS
 *****************************************************************************/
INT32 
mtk_gps_threads_create(MTK_GPS_THREAD_ID_ENUM thread_id);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_threads_release
 * DESCRIPTION
 *  Release MNL thread (array) in the porting layer
 * PARAMETERS
 * void
 * RETURNS
 *  MTK_GPS_ERROR / MTK_GPS_SUCCESS
 *****************************************************************************/
INT32 
mtk_gps_threads_release(void);

//----------------------------------------------------------------------------------

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_mnl_run
 * DESCRIPTION
 *  RUN MTK Nav Library
 * PARAMETERS
 *  default_cfg     [IN]  factory default configuration
 *  driver_dfg       [IN]  UART/COM PORT setting
 * RETURNS
 *  MTK_GPS_BOOT_STATUS
 *****************************************************************************/
MTK_GPS_BOOT_STATUS 
mtk_gps_mnl_run(const MTK_GPS_INIT_CFG* default_cfg, const MTK_GPS_DRIVER_CFG* driver_dfg);
/*****************************************************************************
 * FUNCTION
 *  mtk_gps_mnl_stop
 * DESCRIPTION
 *  STOP MTK Nav Library
 * PARAMETERS
 * void
 * RETURNS
 *  void
 *****************************************************************************/
void 
mtk_gps_mnl_stop(void);


#ifdef __cplusplus
   }
#endif

#endif /* MTK_GPS_DRIVER_WRAPPER_H */
