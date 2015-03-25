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

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <malloc.h>
#include <time.h>
/*#include <fstream>
#include <iostream>
using namespace std;*/
#include <utils/Log.h>
//#include <cutils/xlog.h>

#ifndef LOGV
#define SXLOGV ALOGV
#endif

#ifndef LOGD
#define SXLOGD ALOGD
#endif

#ifndef LOGI
#define SXLOGI ALOGI
#endif

#ifndef LOGW
#define SXLOGW ALOGW
#endif

#ifndef LOGE
#define SXLOGE ALOGE
#endif

#include "MIDISeektable.h"

#undef LOG_TAG
#define LOG_TAG "MIDIST"

#define ZERO_VALUE  0
   
bool _MST_Init(struct MST * mst,int trackNum);
bool _MST_Reset(struct MST * mst);
bool _MST_Release(struct MST * mst);
bool _MST_UpdateParsingEventTable(struct MST * mst,int tickCount, int deltaTime,int trackNum,int offset,unsigned char runState);
bool _MST_UpdateSpecialMsgTable(struct MST * mst,void * pSpecialEvent,int length, int trackNum);
bool _MST_GetParsingEvent(struct MST * mst,int seektoPostition,EventOffset * pEO);
int  _MST_GetSpecialMsgCount(struct MST * mst,int seektoPostition, int trackNum);
void _MST_DumpSeektable(struct MST * mst);
bool _MST_Extraction(struct MST * mst);
bool _MST_FillBlank(struct MST * mst,int tickCount,int deltaTime,int trackNum,int offset, unsigned char runState);

void _FreeSpecialMsgTable(struct MST * mst);

bool _MST_Init(struct MST * mst,int trackNum)//malloc a storage space to keep nextparsingevent
{
 	  SXLOGV("MIDISeekTableInit\n"); 

    if ((mst->mstdata.TRACKNUM = trackNum) <= 0)
      return false;
    _MST_Release(mst);
	  mst->mstdata.pParsingEventTable = (ParsingEvent *)malloc(sizeof(ParsingEvent) * mst->mstdata.MAXENTRY * mst->mstdata.TRACKNUM);
    if (mst->mstdata.pParsingEventTable == NULL)
      return false;
    memset(mst->mstdata.pParsingEventTable, ZERO_VALUE,sizeof(ParsingEvent) * mst->mstdata.MAXENTRY * mst->mstdata.TRACKNUM);

    mst->mstdata.pSpecailMSGTableListBegin = (SpecialMessage **)malloc(sizeof(SpecialMessage*) * mst->mstdata.TRACKNUM);
    if (mst->mstdata.pSpecailMSGTableListBegin == NULL)
        return false;
    memset(mst->mstdata.pSpecailMSGTableListBegin, ZERO_VALUE, sizeof(SpecialMessage*) * mst->mstdata.TRACKNUM);
    mst->mstdata.pSpecailMSGTableListEnd = (SpecialMessage **)malloc(sizeof(SpecialMessage*) * mst->mstdata.TRACKNUM);
    if (mst->mstdata.pSpecailMSGTableListEnd == NULL)
        return false;
    memset(mst->mstdata.pSpecailMSGTableListEnd, ZERO_VALUE, sizeof(SpecialMessage*) * mst->mstdata.TRACKNUM);
    return true;
}

bool _MST_Reset(struct MST * mst)//Reset pParsingEventTable and pSpecailMSGTableListBegin
{
    SXLOGV("MIDISeekTableReset\n"); 
    //reset the content of pParsingEventTable
    if (mst->mstdata.pParsingEventTable != NULL)
        memset(mst->mstdata.pParsingEventTable, ZERO_VALUE,sizeof(ParsingEvent) * mst->mstdata.MAXENTRY * mst->mstdata.TRACKNUM);
    mst->mstdata.curEntry = 0;
    //reset the content of pSpecailMSGTableListBegin
    _FreeSpecialMsgTable(mst);
    return true;
}

bool _MST_Release(struct MST * mst)//Release pParsingEventTable and pSpecailMSGTableListBegin 
{
    SXLOGV("MIDISeekTableRelease\n");
    // release pParsingEventTable
    if (mst->mstdata.pParsingEventTable != NULL)
    {
        free(mst->mstdata.pParsingEventTable);
        mst->mstdata.pParsingEventTable = NULL;
    }
    // release pSpecailMSGTableListBegin
    _FreeSpecialMsgTable(mst);
    if (mst->mstdata.pSpecailMSGTableListBegin) {
        free(mst->mstdata.pSpecailMSGTableListBegin);
        mst->mstdata.pSpecailMSGTableListBegin = NULL;
    }
    if (mst->mstdata.pSpecailMSGTableListEnd) {
        free(mst->mstdata.pSpecailMSGTableListEnd);
        mst->mstdata.pSpecailMSGTableListEnd = NULL;
    }
    return true;
}

bool _MST_UpdateParsingEventTable(struct MST * mst,int tickCount, int deltaTime,int trackNum,int offset,unsigned char runState)
{
    SXLOGV("UpdateParsingEventTable\n");
     int nEntry = tickCount / mst->mstdata.interVal;
     if (nEntry < mst->mstdata.MAXENTRY)
     {
      int i;
	    for (i = 0;i <= nEntry; i++)
	    {
	  	 if(!mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + trackNum].isFilled)
	      {
	         mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + trackNum].tickCount = tickCount;
		       mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + trackNum].trackNum = trackNum;
		       mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + trackNum].deltaTime = deltaTime;
		       mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + trackNum].offset = offset;
			     mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + trackNum].runState = runState;
		       mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + trackNum].isFilled = true; 		
	      }
	    }
	   if ( nEntry > mst->mstdata.curEntry)
	      mst->mstdata.curEntry = nEntry; 		
    }

 if(tickCount >= mst->mstdata.maxTime)
 {
 	mst->mstdata.preInterval = mst->mstdata.interVal;
	SXLOGD("mst->mstdata.preInterval is %d s", mst->mstdata.preInterval / 1000);
	while (tickCount >= mst->mstdata.maxTime)
	{
	     mst->mstdata.interVal *= 2;
		 mst->mstdata.maxTime = mst->mstdata.MAXENTRY * mst->mstdata.interVal;
	}
	
    SXLOGD("mst->mstdata.interVal is %d s", mst->mstdata.interVal / 1000);
	_MST_Extraction(mst);
	_MST_FillBlank(mst,tickCount,deltaTime,trackNum,offset,runState);
 }
 return true;
}

bool _MST_UpdateSpecialMsgTable(struct MST * mst,void * pSpecialEvent,int length, int trackNum)
{
	SpecialEvent * pSE = (SpecialEvent *)pSpecialEvent;
	SpecialEvent * psEvent = (SpecialEvent *)malloc(sizeof(SpecialEvent)) ;
	psEvent->tickCount = pSE->tickCount;
	psEvent->status = pSE->status;
	psEvent->dataNum = pSE->dataNum;
	psEvent->pData = (char *)malloc(length);
	memcpy(psEvent->pData,pSE->pData,length);

    SXLOGV("UpdateSpecialMsgTable\n");

    if (mst->mstdata.pSpecailMSGTableListBegin[trackNum] == NULL)
    {
        mst->mstdata.pSpecailMSGTableListBegin[trackNum] = (SpecialMessage *)malloc(sizeof(SpecialMessage));
        mst->mstdata.pSpecailMSGTableListBegin[trackNum]->pBuffer = psEvent;
        mst->mstdata.pSpecailMSGTableListBegin[trackNum]->pNext = NULL;
        mst->mstdata.pSpecailMSGTableListEnd[trackNum] = mst->mstdata.pSpecailMSGTableListBegin[trackNum];
    }
    else
    {
        SpecialMessage * ptempSM = (SpecialMessage *)malloc(sizeof(SpecialMessage));
        ptempSM->pBuffer = psEvent;
        ptempSM->pNext = NULL;
        mst->mstdata.pSpecailMSGTableListEnd[trackNum]->pNext = ptempSM;
        mst->mstdata.pSpecailMSGTableListEnd[trackNum] = mst->mstdata.pSpecailMSGTableListEnd[trackNum]->pNext;
    }
   return true;
}

bool _MST_GetParsingEvent(struct MST * mst,int seektoPostition,EventOffset * pEO)//retrieve the next parsing event offset
{
   if (seektoPostition < 0)
   	return false;
   int nEntry = seektoPostition / mst->mstdata.interVal;
   if (nEntry > mst->mstdata.curEntry)
   	nEntry = mst->mstdata.curEntry;
   int i;
   for(i = 0 ; i < mst->mstdata.TRACKNUM ;i++)
   {
    pEO[i].tickCount = mst->mstdata.pParsingEventTable[nEntry * mst->mstdata.TRACKNUM + i].tickCount;
   	pEO[i].trackNum = mst->mstdata.pParsingEventTable[nEntry * mst->mstdata.TRACKNUM + i].trackNum;
   	pEO[i].offset = mst->mstdata.pParsingEventTable[nEntry * mst->mstdata.TRACKNUM + i].offset;
	  pEO[i].runState = mst->mstdata.pParsingEventTable[nEntry * mst->mstdata.TRACKNUM + i].runState;
	  pEO[i].deltaTime = mst->mstdata.pParsingEventTable[nEntry * mst->mstdata.TRACKNUM + i].deltaTime;
   }
   return true;
}

int  _MST_GetSpecialMsgCount(struct MST * mst,int seektoPostition, int trackNum)//retrieve the special message
{
   SpecialMessage * pTempSMsg = mst->mstdata.pSpecailMSGTableListBegin[trackNum];
   int count = 0;
   for ( ; pTempSMsg != NULL ; pTempSMsg = pTempSMsg->pNext, count++)
   {
	   if(pTempSMsg->pBuffer->tickCount >= seektoPostition)
   	   break;
   }
   return count;       	
}
  
void _MST_DumpSeektable(struct MST * mst)
{
   FILE * pOutput1 = NULL;
	 FILE * pOutput2 = NULL;
	 if ((pOutput1 = fopen(mst->mstdata.NextParsingEventFileName,"a")) != NULL)
	 {
	   int i,j;
		 fprintf(pOutput1,"Interval:%d s\n",mst->mstdata.interVal/1000);
		 for (i = 0 ; i <= mst->mstdata.curEntry ; i ++)
		 {
			fprintf(pOutput1,"Calibration%d :\n",i);
			for (j = 0 ; j < mst->mstdata.TRACKNUM ; j ++)
			{
				fprintf(pOutput1,"track %d :%d--%d--%ld--%ld\n",j,mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + j].tickCount,mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + j].trackNum,mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + j].deltaTime,mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + j].offset);
			}
	   }
		 fclose(pOutput1);
	  }

	 if ((pOutput2 = fopen(mst->mstdata.SpecialMessageFileName,"a")) != NULL)
	 {
        int i, j;
        for (i = 0; i < mst->mstdata.TRACKNUM; i++)
        {
            SpecialMessage * pTempSMsg = mst->mstdata.pSpecailMSGTableListBegin[i];
            for (j = 0; pTempSMsg != NULL; pTempSMsg = pTempSMsg->pNext,j++)
            {
                fprintf(pOutput2,"track %d, SpecialMessage%d :%d--%d--%d--%s\r\n",
                        i, j, pTempSMsg->pBuffer->tickCount, pTempSMsg->pBuffer->status,
                        pTempSMsg->pBuffer->dataNum, pTempSMsg->pBuffer->pData);
            }
        }

		fclose(pOutput2);
	 }
}

bool _MST_Extraction(struct MST * mst)
{
	int i, trackNum;
	int intervalSecs = mst->mstdata.interVal / mst->mstdata.preInterval;

	SXLOGD("_MST_Extraction");
//	mst->mstdata.curEntry = mst->mstdata.MAXENTRY / 2;
	mst->mstdata.curEntry = mst->mstdata.MAXENTRY / intervalSecs;  
	for (i = 0 ;i < mst->mstdata.curEntry ; i++)
		for (trackNum = 0; trackNum < mst->mstdata.TRACKNUM ;trackNum++)
	   {
	       mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + trackNum].tickCount = mst->mstdata.pParsingEventTable[i * intervalSecs * mst->mstdata.TRACKNUM + trackNum].tickCount;
		   mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + trackNum].trackNum = mst->mstdata.pParsingEventTable[i * intervalSecs * mst->mstdata.TRACKNUM + trackNum].trackNum;
		   mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + trackNum].deltaTime = mst->mstdata.pParsingEventTable[i * intervalSecs * mst->mstdata.TRACKNUM + trackNum].deltaTime;
		   mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + trackNum].offset = mst->mstdata.pParsingEventTable[i * intervalSecs * mst->mstdata.TRACKNUM + trackNum].offset;
		   mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + trackNum].runState = mst->mstdata.pParsingEventTable[i * intervalSecs * mst->mstdata.TRACKNUM + trackNum].runState;
		   mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + trackNum].isFilled = mst->mstdata.pParsingEventTable[i * intervalSecs * mst->mstdata.TRACKNUM + trackNum].isFilled;
	   }
	  
	for (i = mst->mstdata.curEntry ;i < mst->mstdata.MAXENTRY ; i++)
		for (trackNum = 0; trackNum < mst->mstdata.TRACKNUM ;trackNum++)
	   {
		   if ((mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + trackNum].tickCount > 0) && (mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + trackNum].tickCount < mst->mstdata.interVal * i))
              memset(&mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + trackNum], ZERO_VALUE,sizeof(ParsingEvent));
	     /*
         	  if ( !mst->mstdata.pParsingEventTable[(i - 1) * mst->mstdata.TRACKNUM + trackNum].isFilled || 
          	  (mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + trackNum].tickCount < mst->mstdata.pParsingEventTable[(i - 1) * mst->mstdata.TRACKNUM + trackNum].tickCount))
			  memset(&mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + trackNum], ZERO_VALUE,sizeof(ParsingEvent));
		   else if (((i + 1) < mst->mstdata.MAXENTRY) && (mst->mstdata.pParsingEventTable[(i + 1) * mst->mstdata.TRACKNUM + trackNum].isFilled) && (mst->mstdata.pParsingEventTable[(i + 1) * mst->mstdata.TRACKNUM + trackNum].tickCount >= mst->mstdata.interVal * i))
		   {
		      mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + trackNum].tickCount = mst->mstdata.pParsingEventTable[(i + 1) * mst->mstdata.TRACKNUM + trackNum].tickCount;
     		  mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + trackNum].trackNum = mst->mstdata.pParsingEventTable[(i + 1) * mst->mstdata.TRACKNUM + trackNum].trackNum;
     		  mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + trackNum].deltaTime = mst->mstdata.pParsingEventTable[(i + 1) * mst->mstdata.TRACKNUM + trackNum].deltaTime;
   		  mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + trackNum].offset = mst->mstdata.pParsingEventTable[(i + 1) * mst->mstdata.TRACKNUM + trackNum].offset;
		  mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + trackNum].runState = mst->mstdata.pParsingEventTable[(i + 1) * mst->mstdata.TRACKNUM + trackNum].runState;
   		  mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + trackNum].isFilled = true; 	
		   }
	          */
	  }
	return true;
}

bool _MST_FillBlank(struct MST * mst,int tickCount,int deltaTime,int trackNum,int offset, unsigned char runState)
{
	int nTempEntry = tickCount / mst->mstdata.interVal;
	int i;
   for (i = 0; i <= nTempEntry; i++)
   {
   	if(!mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + trackNum].isFilled)
   	{
   	   mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + trackNum].tickCount = tickCount;
     	 mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + trackNum].trackNum = trackNum;
     	 mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + trackNum].deltaTime = deltaTime;
   		 mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + trackNum].offset = offset;
		   mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + trackNum].runState = runState;
   		 mst->mstdata.pParsingEventTable[i * mst->mstdata.TRACKNUM + trackNum].isFilled = true; 		
   	}
   }
   mst->mstdata.curEntry = nTempEntry; 
   return true;
}

void MST_Register(struct MST * mst)
{
	  mst->mstdata.TRACKNUM = 0;
    mst->mstdata.MAXENTRY = 128;
    mst->mstdata.pParsingEventTable = NULL;
    mst->mstdata.pSpecailMSGTableListBegin = NULL;
    mst->mstdata.pSpecailMSGTableListEnd = NULL;
    mst->mstdata.curEntry = 0;
    mst->mstdata.interVal = 1000;//1s 
	mst->mstdata.preInterval = mst->mstdata.interVal;
	mst->mstdata.maxTime = mst->mstdata.MAXENTRY * mst->mstdata.interVal;
	  
    struct timeval tm;
    gettimeofday(&tm,NULL);
    memset(mst->mstdata.NextParsingEventFileName,0,sizeof(mst->mstdata.NextParsingEventFileName));
    memset(mst->mstdata.SpecialMessageFileName,0,sizeof(mst->mstdata.SpecialMessageFileName));
    sprintf(mst->mstdata.NextParsingEventFileName,"/data/NextParsingEvent_%ld_%ld",tm.tv_sec,tm.tv_usec);
    sprintf(mst->mstdata.SpecialMessageFileName,"/data/SpecialMessage_%ld_%ld",tm.tv_sec,tm.tv_usec);
	
	  mst->MST_Init = _MST_Init;
	  mst->MST_Reset = _MST_Reset;
	  mst->MST_Release = _MST_Release;
	  mst->MST_UpdateParsingEventTable = _MST_UpdateParsingEventTable;
	  mst->MST_UpdateSpecialMsgTable = _MST_UpdateSpecialMsgTable;
	  mst->MST_GetParsingEvent = _MST_GetParsingEvent;
	  mst->MST_GetSpecialMsgCount = _MST_GetSpecialMsgCount;
	  mst->MST_DumpSeektable = _MST_DumpSeektable;
	  mst->MST_Extraction = _MST_Extraction;
	  mst->MST_FillBlank = _MST_FillBlank;

}

void MST_Unregister(struct MST * mst)
{
	  mst->mstdata.TRACKNUM = 0;
    mst->mstdata.MAXENTRY = 128;
    mst->mstdata.pParsingEventTable = NULL;
    mst->mstdata.pSpecailMSGTableListBegin = NULL;
    mst->mstdata.pSpecailMSGTableListEnd = NULL;
    mst->mstdata.curEntry = 0;
    mst->mstdata.interVal = 1000;//1s 
    mst->mstdata.preInterval = mst->mstdata.interVal;
	mst->mstdata.maxTime = mst->mstdata.MAXENTRY * mst->mstdata.interVal; 
	
    (void)unlink(mst->mstdata.NextParsingEventFileName);
    (void)unlink(mst->mstdata.SpecialMessageFileName);
	
	  mst->MST_Init = NULL;
	  mst->MST_Reset = NULL;
	  mst->MST_Release = NULL;
	  mst->MST_UpdateParsingEventTable = NULL;
	  mst->MST_UpdateSpecialMsgTable = NULL;
	  mst->MST_GetParsingEvent = NULL;
	  mst->MST_GetSpecialMsgCount = NULL;
	  mst->MST_DumpSeektable = NULL;
	  mst->MST_Extraction = NULL;
	  mst->MST_FillBlank = NULL;
}

void _FreeSpecialMsgTable(struct MST * mst)
{
    if (mst->mstdata.pSpecailMSGTableListBegin && mst->mstdata.pSpecailMSGTableListEnd) {
        int i;
        for (i = 0; i < mst->mstdata.TRACKNUM; i++)
        {
            if (mst->mstdata.pSpecailMSGTableListBegin[i] != NULL) {
                while (mst->mstdata.pSpecailMSGTableListBegin[i] != NULL) {
                    SpecialMessage * pTemp = mst->mstdata.pSpecailMSGTableListBegin[i];
                    mst->mstdata.pSpecailMSGTableListBegin[i] = mst->mstdata.pSpecailMSGTableListBegin[i]->pNext;
                    if (pTemp->pBuffer) {
                        if (pTemp->pBuffer->pData) {
                            free(pTemp->pBuffer->pData);
                            pTemp->pBuffer->pData = NULL;
                        }
                        free(pTemp->pBuffer);
                        pTemp->pBuffer = NULL;
                    }
                    free(pTemp);
                    pTemp = NULL;
                }
            }
            mst->mstdata.pSpecailMSGTableListEnd[i] = NULL;
        }
    }
}

