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

/*
 ** Copyright 2003-2010, VisualOn, Inc.
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

/*******************************************************************************
	File:		mem_align.c

	Content:	Memory alloc alignments functions

*******************************************************************************/


#include	"memalign.h"
#ifdef _MSC_VER
#include	<stddef.h>
#else
#include	<stdint.h>
#endif

/*****************************************************************************
*
* function name: mem_malloc
* description:  malloc the alignments memory
* returns:      the point of the memory
*
**********************************************************************************/
void *
mem_malloc(VO_MEM_OPERATOR *pMemop, unsigned int size, unsigned char alignment, unsigned int CodecID)
{
	int ret;
	unsigned char *mem_ptr;
	VO_MEM_INFO MemInfo;

	if (!alignment) {

		MemInfo.Flag = 0;
		MemInfo.Size = size + 1;
		ret = pMemop->Alloc(CodecID, &MemInfo);
		if(ret != 0)
			return 0;
		mem_ptr = (unsigned char *)MemInfo.VBuffer;

		pMemop->Set(CodecID, mem_ptr, 0, size + 1);

		*mem_ptr = (unsigned char)1;

		return ((void *)(mem_ptr+1));
	} else {
		unsigned char *tmp;

		MemInfo.Flag = 0;
		MemInfo.Size = size + alignment;
		ret = pMemop->Alloc(CodecID, &MemInfo);
		if(ret != 0)
			return 0;

		tmp = (unsigned char *)MemInfo.VBuffer;

		pMemop->Set(CodecID, tmp, 0, size + alignment);

		mem_ptr =
			(unsigned char *) ((intptr_t) (tmp + alignment - 1) &
					(~((intptr_t) (alignment - 1))));

		if (mem_ptr == tmp)
			mem_ptr += alignment;

		*(mem_ptr - 1) = (unsigned char) (mem_ptr - tmp);

		return ((void *)mem_ptr);
	}

	return(0);
}


/*****************************************************************************
*
* function name: mem_free
* description:  free the memory
*
*******************************************************************************/
void
mem_free(VO_MEM_OPERATOR *pMemop, void *mem_ptr, unsigned int CodecID)
{

	unsigned char *ptr;

	if (mem_ptr == 0)
		return;

	ptr = mem_ptr;

	ptr -= *(ptr - 1);

	pMemop->Free(CodecID, ptr);
}



