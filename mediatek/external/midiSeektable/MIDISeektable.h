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

#ifndef __MIDI_SEEKTABLE
#define __MIDI_SEEKTABLE

typedef enum{false,true}bool;

//next parsing event
typedef struct _ParsingEvent
{
   int  tickCount;
   int  trackNum;
   //int  deltaTime;
   unsigned long deltaTime;
   long offset;
   unsigned char runState;
   bool isFilled;
}ParsingEvent;
//offset of next parsing event
typedef struct _EventOffset
{
   int  tickCount;
   int  trackNum;
   long deltaTime;
   unsigned char runState;
   long offset;
}EventOffset;
//special messages
typedef struct _SpecialEvent
{
   int tickCount;
   unsigned char status;//0xAn-KeyPressMsg,0xBn-ControllerMsg,0xCn-ProgramChangeMsg,0xDn-ChannelPressureMsg,0xEn-PitchBendMsg,0xFn-SystemMsg
   int  dataNum;
   char * pData;
}SpecialEvent;
typedef struct _SpecialMessage
{
   SpecialEvent * pBuffer;
   struct _SpecialMessage * pNext;
}SpecialMessage;

struct MSTDATA
{
   int TRACKNUM;
   int MAXENTRY;
   ParsingEvent *pParsingEventTable;   //next parsing event table 
   SpecialMessage **pSpecailMSGTableListBegin; //the beginning of special message table list
   SpecialMessage **pSpecailMSGTableListEnd; //the ending of special message table list
   int curEntry;
   int interVal;//1s 
   char NextParsingEventFileName[100];     
   char SpecialMessageFileName[100];
   int preInterval;
   int maxTime;
};
struct MST
{
   //public interfaces for sonivox
   struct MSTDATA  mstdata;

   bool (*MST_Init)(struct MST * mst,int trackNum);
   bool (*MST_Reset)(struct MST * mst);
   bool (*MST_Release)(struct MST * mst);
   bool (*MST_UpdateParsingEventTable)(struct MST * mst,int tickCount, int deltaTime,int trackNum,int offset,unsigned char runState);
   bool (*MST_UpdateSpecialMsgTable)(struct MST * mst,void * pSpecialEvent,int length, int trackNum);
   bool (*MST_GetParsingEvent)(struct MST * mst,int seektoPostition,EventOffset * pEO);
   int  (*MST_GetSpecialMsgCount)(struct MST * mst,int seektoPostition, int trackNum);
   void (*MST_DumpSeektable)(struct MST * mst);
   bool (*MST_Extraction)(struct MST * mst);
   bool (*MST_FillBlank)(struct MST * mst,int tickCount,int deltaTime,int trackNum,int offset, unsigned char runState);
};

void MST_Register(struct MST * mst);
void MST_Unregister(struct MST * mst);

#endif
