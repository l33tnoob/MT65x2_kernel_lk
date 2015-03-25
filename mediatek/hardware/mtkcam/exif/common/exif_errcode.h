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

/********************************************************************************************
 *     LEGAL DISCLAIMER 
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES 
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED 
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS 
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, 
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR 
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY 
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, 
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK 
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION 
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *     
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH 
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, 
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE 
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE. 
 *     
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS 
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.  
 ************************************************************************************************/
#ifndef _EXIF_ERRCODE_H_
#define _EXIF_ERRCODE_H_


/*******************************************************************************
*
********************************************************************************/
    // Error codes
#define EXIF_NO_ERROR               0
#define EXIF_UNKNOWN_ERROR          (-1)
//
#define LIBEXIF_INIT_ERR0001        -(0x0101)
#define LIBEXIF_FILE_ERR0001        -(0x0201)       // unsupport file format 
#define LIBEXIF_APP1_ERR0001        -(0x0301)       // no thumbnail image
#define LIBEXIF_APP1_ERR0002        -(0x0302)       // tiffer header error found
#define LIBEXIF_APPN_ERR0003        -(0x0303)       // unrecognised Appn marker 
#define LIBEXIF_APPN_ERR0004        -(0x0304)       // Appn is not added 
#define LIBEXIF_APP1_ERR0005        -(0x0305)       // Unsupport thumb compression mode 
#define LIBEXIF_DQT_ERR0001         -(0x0401)       // too mamy QTs 
#define LIBEXIF_DQT_ERR0002         -(0x0402)       // no DQT Marker 
#define LIBEXIF_DQT_ERR0003         -(0x0403)       // unsupport DQT format
#define LIBEXIF_DHT_ERR0001         -(0x0501)       // no DHT marker 
#define LIBEXIF_DHT_ERR0002         -(0x0502)       // unsupport DHT00 format
#define LIBEXIF_DHT_ERR0003         -(0x0503)       // unsupport DHT10 format
#define LIBEXIF_DHT_ERR0004         -(0x0504)       // unsupport DHT01 format
#define LIBEXIF_DHT_ERR0005         -(0x0505)       // unsupport DHT11 format
#define LIBEXIF_DHT_ERR0006         -(0x0506)       // unsupport DHT format 
#define LIBEXIF_SOI_ERR0001         -(0x0601)       // no SOI marker 
#define LIBEXIF_EOI_ERR0001         -(0x0701)       // no EOI marker 
#define LIBEXIF_SOF_ERR0001         -(0x0801)       // no SOF marker 
#define LIBEXIF_SOF_ERR0002         -(0x0802)       // no SOF length error 
#define LIBEXIF_SOF_ERR0003         -(0x0803)       // unsupport data format
#define LIBEXIF_SOF_ERR0004         -(0x0804)       // unsupport compression mode 
#define LIBEXIF_SOS_ERR0001         -(0x0901)       // no SOS marker 
#define LIBEXIF_SOS_ERR0002         -(0x0902)       // no SOS length error 
#define LIBEXIF_MISC_ERR0001        -(0x0A01)       // unknow error 
#define LIBEXIF_MISC_ERR0002        -(0x0A02)       // file overflow 
#define LIBEXIF_IFD_ERR0001         -(0x0B01)       // unsupport IFD list
#define LIBEXIF_IFD_ERR0002         -(0x0B02)       // unsupport tag 
#define LIBEXIF_IFD_ERR0003         -(0x0B03)       // tag not found 
#define LIBEXIF_IFD_ERR0004         -(0x0B04)       // IFD list full 
#define LIBEXIF_IFD_ERR0005         -(0x0B05)       // IFD Duplicate 
                                            
                                            
#endif                                      

