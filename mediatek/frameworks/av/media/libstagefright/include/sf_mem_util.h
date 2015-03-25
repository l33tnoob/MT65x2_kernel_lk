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

// Morris Yang 20100826
#ifndef PMEM_UTIL_H_
#define PMEM_UTIL_H_


#define INVALID_FD        -1   //!<: invalid file descriptor

struct MemHeapInfo
{
    int     mMemHeapBase;
    void*   base;
    size_t  size;
};

bool sf_memheap_set_info(MemHeapInfo* info);
bool sf_memheap_get_info(void* aPtr, MemHeapInfo* info);
void sf_memheap_remove_info(void* aPtr);

//#ifndef OSCL_PMEM_H_INCLUDED
//!<: Complete pmem information
struct PmemInfo{
    int    fd;          //!<: master file description used to mmap memory
    int    shared_fd;   //!<: shared fd, used for IPC
    void*  base;        //!<: address of pmem chunk
    size_t size;        //!<: size of this memory chunk
    int    offset;      //!<: distance between virtual address and base
};


//!<: External pmem information
struct ExPmemInfo{
    int    shared_fd;   //!<: shared fd, used for IPC
    void*  base;        //!<: virtual address of pmem chunk
    int    offset;		//!<: offset of this sub-chunk
    size_t size;        //!<: size of this sub-chunk
};

//#endif
/**
 * Allocates a memory block which is continuous in physical memory.
 * @param aSize  number of bytes to allocate
 * @return a void pointer to the allocated space, or NULL if there is insufficient
 *         memory available.
 */
void* sf_pmem_alloc(size_t aSize, bool noncache = 0);


/**
 * Deallocates or frees a memory block which is continuous in physical memory.
 * @param aPtr  pointer to previously allocated memory block which is allocated by oscl_pmem_alloc().
 */
void  sf_pmem_free(void* aPtr);


/**
 * Convert the address of a memory block from virtual address to physical address.
 * @param aVirPtr  pointer to previously allocated memory block which is allocated by oscl_pmem_alloc().
 * @return a void pointer contains the physical address, or NULL if aVirPtr is invalid.
 */
void* sf_pmem_vir2phy(void* aVirPtr, int where);


/**
 * Get complete pmem information associated to the address
 * @param aVirPtr  pointer to previously allocated memory block which is allocated by oscl_pmem_alloc().
 * @param info     pointer to PmemInfo, which contains the output information.
 * @return true if succeed; otherwise, false is returned.
 */
bool sf_pmem_get_info(void* aPtr, PmemInfo* info);


/**
 * Register the pmem chunk, which is allocated by another process
 * @param info     pointer to ExPmemInfo.
 * @return true if succeed; otherwise, false is returned.
 */
bool sf_pmem_register(ExPmemInfo* info);


/**
 * Unregister the pmem chunk, which is allocated by another process
 * @param info     pointer to ExPmemInfo.
 * @return true if succeed; otherwise, false is returned.
 */
bool sf_pmem_unregister(ExPmemInfo* info);


/**
 * Shoe the pmem sAddrMap
 * @param void
 * @return true if succeed; otherwise, false is returned.
 */
bool sf_pmem_show_addr_map(void);

/**
 * Map the pmem into non-cached
 * @param aVirPtr  pointer to previously allocated memory block which is allocated by oscl_pmem_alloc().
 * @return true if succeed; otherwise, false is returned.
 */


bool sf_pmem_map_into_noncached(void* aVirPtr);
#endif
