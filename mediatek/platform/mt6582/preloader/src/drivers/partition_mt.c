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

/******************************************************************************
*
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2001
*
*******************************************************************************/
#include "typedefs.h"
#include "mtk_nand_core.h"
#include "mtk_nand.h"
#include "platform.h"
#include "cust_part.h"
//#include "mt6577_utils.h"
//#include "mt6577_partition.h"
//#include "mt6577_download.h"
#include "pmt.h"
#define PAGE_4K 1 

#if CFG_PMT_SUPPORT

//#ifdef PAGE_4K
//#define LPAGE 2048
//char page_buf[LPAGE+64];
//char page_readbuf[LPAGE];
//#else
#define LPAGE 4096
char page_buf[LPAGE+128];
char page_readbuf[LPAGE];
//#endif
extern unsigned char g_nand_spare[64];
extern struct nand_chip g_nand_chip;

static int total_size;
static pt_info pi;
static u8 sig_buf[PT_SIG_SIZE];

//pt_resident old_part[PART_MAX_COUNT];
//extern flashdev_info devinfo;
//extern u32 BLOCK_SIZE;
//extern u32 PAGE_SIZE;

static pt_resident new_part[PART_MAX_COUNT];
static pt_resident lastest_part[PART_MAX_COUNT];
static part_t tempart;
#ifdef MTK_EMMC_SUPPORT
#define CFG_EMMC_PMT_SIZE 0xc00000
extern u64 g_emmc_size;
pt_resident32 lastest_part32[PART_MAX_COUNT];
static int load_pt_from_fixed_addr(u8 *buf);
static u32 PAGE_SIZE=512;
static inline u32 PAGE_NUM (u64 logical_size)
{
        return ((u64) (logical_size) / PAGE_SIZE);
}

#else
extern u32 BLOCK_SIZE;
extern u32 PAGE_SIZE;
static inline u32 PAGE_NUM (u32 logical_size)
{
        return ((unsigned long) (logical_size) / PAGE_SIZE);
}
extern int new_part_tab(u8 *buf);
extern int update_part_tab(void);

extern int mtk_nand_write_page_hwecc (unsigned int logical_addr,
                                         char *buf);
extern int mtk_nand_read_page_hwecc (unsigned int logical_addr, char *buf);

#endif
extern int load_exist_part_tab(u8 *buf);


//DM_PARTITION_INFO_PACKET dm;  //for test
void get_part_tab_from_complier(void)
{
#ifdef MTK_EMMC_SUPPORT
	int index=0;
	part_t *part = cust_part_tbl();
	MSG (INIT, "get_pt_from_complier \n");
	while(part->flags!= PART_FLAG_END)
	{
    		
		memcpy(lastest_part[index].name, part->name, MAX_PARTITION_NAME_LEN);
		lastest_part[index].size = (u64)part->blks * PAGE_SIZE;
		lastest_part[index].offset = (u64)part->startblk * PAGE_SIZE;
		lastest_part[index].mask_flags = PART_FLAG_NONE;  //this flag in kernel should be fufilled even though in flash is 0.
		print("get_ptr  %s %llx \n",lastest_part[index].name,lastest_part[index].offset);
		index++; part++;
	}
#else
	int index=0;
	part_t *part = cust_part_tbl();
	MSG (INIT, "get_pt_from_complier \n");
	while(part->flags!= PART_FLAG_END)
	{
    		
		memcpy(lastest_part[index].name, part->name, MAX_PARTITION_NAME_LEN);
		lastest_part[index].size = part->blks * PAGE_SIZE;
		lastest_part[index].offset = part->startblk * PAGE_SIZE;
		lastest_part[index].mask_flags = PART_FLAG_NONE;  //this flag in kernel should be fufilled even though in flash is 0.
		MSG (INIT, "get_ptr  %s %x \n",lastest_part[index].name,lastest_part[index].offset);
		index++; part++;
	}
#endif
}

void pmt_init(void)
{
	
#ifdef MTK_EMMC_SUPPORT
	int ret = 0;
    int i = 0;
	
    memset(&new_part,0, PART_MAX_COUNT * sizeof(pt_resident));
    memset(&lastest_part,0, PART_MAX_COUNT * sizeof(pt_resident));
    memset(&lastest_part32,0,PART_MAX_COUNT*sizeof(pt_resident32));
	ret = load_pt_from_fixed_addr((u8 *)&lastest_part);
    if(ret == ERR_NO_EXIST)
    ret = load_exist_part_tab((u8 *)&lastest_part);
    if (ret == ERR_NO_EXIST) { //first run preloader before dowload
        //and valid mirror last download or first download 
        get_part_tab_from_complier(); //get from complier
    } else {
        print("find pt\n");
        for (i = 0; i < PART_MAX_COUNT; i++) {   
			if(lastest_part[i].size == 0)
				break;
            print("part %s size %llx %llx\n", lastest_part[i].name, 
                lastest_part[i].offset, lastest_part[i].size);
        }
    }
#else
    int ret = 0;
    int i = 0;

    memset(&new_part,0, PART_MAX_COUNT * sizeof(pt_resident));
    memset(&lastest_part,0, PART_MAX_COUNT * sizeof(pt_resident));

    ret = load_exist_part_tab((u8 *)&lastest_part);
    if (ret == ERR_NO_EXIST) { //first run preloader before dowload
        //and valid mirror last download or first download 
        get_part_tab_from_complier(); //get from complier
    } else {
        print("find pt\n");
        for (i = 0; i < PART_MAX_COUNT; i++) {   
			if(lastest_part[i].size == 0)
				break;
            print("part %s size %x %x\n", lastest_part[i].name, 
                lastest_part[i].offset, lastest_part[i].size);
        }
    }
 #endif
}

part_t *pmt_get_part(part_t *part, int index)
{
    if (index >= PART_MAX_COUNT)
        return NULL;

    tempart.name=part->name;
    //when download get partitin used new,orther wise used latest
    if (new_part[0].size != 0) {
        tempart.startblk = PAGE_NUM(new_part[index].offset);
        tempart.blks=PAGE_NUM(new_part[index].size);
    } else {
        tempart.startblk = PAGE_NUM(lastest_part[index].offset);
        tempart.blks=PAGE_NUM(lastest_part[index].size);
    }
    tempart.flags=part->flags;
    tempart.size=part->size;
    return &tempart;
}
#ifndef MTK_EMMC_SUPPORT
extern unsigned char g_nand_spare[];
int find_empty_page_from_top(int start_addr)
{
	int page_offset;
	int current_add;
	int i;
	memset(page_buf,0xFF,PAGE_SIZE);
	//mtk_nand_erase(start_addr); //for test
	for(page_offset=0;page_offset<(BLOCK_SIZE/PAGE_SIZE);page_offset++)
	{
		current_add = start_addr+(page_offset*PAGE_SIZE);
		if(!mtk_nand_read_page_hwecc(current_add, page_readbuf))
		{
			MSG (INIT, "find_emp read failed %x \n",current_add);
			continue;
		}
		else
		{
			if(memcmp(page_readbuf, page_buf,PAGE_SIZE)||memcmp(g_nand_spare,page_buf,32))
			{       
				continue;
			}
			else
			{
				MSG (INIT, "find_emp  at %x \n",page_offset);
				break;
			}
			
		}
	}
	MSG (INIT, "find_emp find empty at %x \n",page_offset);
	if(page_offset==(BLOCK_SIZE/PAGE_SIZE))
	{
		MSG (INIT, "find_emp no empty \n");
		if(!mtk_nand_erase(start_addr)) 
		{//no good block for used in replace pool
			MSG (INIT, "find_emp erase mirror failed %x\n",start_addr);
			pi.mirror_pt_has_space=0;
			return 0xFFFF;
		}
		else
		{
			return 0; //the first page is empty
		}
	}
	else
	{
		return page_offset;
	}
}


bool find_mirror_pt_from_bottom(int *start_addr)
{
	int mpt_locate;
	int mpt_start_addr= total_size +BLOCK_SIZE;//MPT_LOCATION*BLOCK_SIZE-PAGE_SIZE;
	int current_start_addr=0;
	char pmt_spare[4];
	
	for(mpt_locate=(BLOCK_SIZE/PAGE_SIZE);mpt_locate>0;mpt_locate--)
	{
		memset(pmt_spare,0xFF,PT_SIG_SIZE);
		memset(g_nand_spare,0xFF,64);
		current_start_addr = mpt_start_addr+mpt_locate*PAGE_SIZE;
		if(!mtk_nand_read_page_hwecc(current_start_addr, page_readbuf))
		{
			MSG (INIT, "find_mirror read  failed %x %x \n",current_start_addr,mpt_locate);
		}
		memcpy(pmt_spare,&g_nand_spare[1] ,PT_SIG_SIZE);
		//need enhance must be the larget sequnce number
		#if 0
		{
			int i;
			for(i=0;i<4;i++)
			MSG (INIT, " %x %x \n",page_readbuf[i],g_nand_spare[1+i]);	
			MSG (INIT,"%x %x " , *((u32*)page_readbuf), *((u32*)pmt_spare));
			MSG (INIT,"%x " , is_valid_mpt(page_readbuf));
			MSG (INIT,"%x \n" , is_valid_mpt(pmt_spare));
		}
		#endif
		if(is_valid_mpt(page_readbuf)&&is_valid_mpt(&pmt_spare))
		{
		      //if no pt, pt.has space is 0;
			pi.sequencenumber =  g_nand_spare[5];
			MSG (INIT, "find_mirror find valid pt at %x sq %x \n",current_start_addr,pi.sequencenumber);
			break;
		}
		else
		{
			continue;
		}
	}
	if(mpt_locate==0)
	{
		MSG (INIT, "no valid mirror page\n");
		pi.sequencenumber =  0;
		return FALSE;
	}
	else
	{
		*start_addr = current_start_addr;
		return TRUE;
	}
}
#endif

#ifdef MTK_EMMC_SUPPORT
static u32 get_pmt_fixed_addr()
{
		int i;
		part_t *part = cust_part_tbl();
		while(part->flags!= PART_FLAG_END){
			if(!strcmp(part->name,PART_PMT)){
					return (u32)(part->startblk * PAGE_SIZE);
				}	
			part++;	
		}

		return 0;
}

static int load_pt_from_fixed_addr(u8 *buf)
{

          int reval = ERR_NO_EXIST;
          int index = 0;
          int i,j;
          int len=0;
          char *buf_p;
          u32 pt_start;
          u32 mpt_start;
      	  int pt_size = 2048;
      	  int buffer_size = pt_size;
      	  u8 *page_buf = (u8 *)EMMC_PMT_BUFFER_START;
      	  int pt_next = 0;
		  int pt_addr = 0;
 		  blkdev_t *dev= blkdev_get(BOOTDEV_SDMMC);
          int pn_per_pt = (g_emmc_size<0x100000000)?(pt_size/sizeof(pt_resident32)):(pt_size/sizeof(pt_resident));
          PAGE_SIZE = 512;
                  
          pt_start = get_pmt_fixed_addr(); 
          mpt_start = pt_start + pt_size;
          //if(false == bSecRead)
          {
              printf("============func=%s===scan pmt from %x=====\n", __func__,pt_start);
          }
          /* try to find the pmt at fixed address, signature:0x50547631 */

              dev->bread(dev,pt_start/PAGE_SIZE,buffer_size/PAGE_SIZE,(u8*)page_buf);
            
             	if(is_valid_pt(page_buf)&&is_valid_pt(&page_buf[pt_size-4])){                
             		  printf("find pt at %x \n",pt_start);
	                pt_addr = pt_start;
                  memcpy(page_readbuf,&page_buf[PT_SIG_SIZE],pt_size-PT_SIG_SIZE);
                	reval=DM_ERR_OK;
                	goto find;
              }else{
              	 dev->bread(dev,mpt_start/PAGE_SIZE,buffer_size/PAGE_SIZE,(u8*)page_buf);
              	 if(is_valid_mpt(page_buf)&&is_valid_mpt(&page_buf[pt_size-4])){
	              	 	printf("find mpt at %x \n",mpt_start);
		                pt_addr = mpt_start;
	                  memcpy(page_readbuf,&page_buf[PT_SIG_SIZE],pt_size-PT_SIG_SIZE);
	                	reval=DM_ERR_OK;
	                	goto find;
              	 	}
              	
              }

 
          printf("find no pt or mpt\n");
          return reval;
 find:
	pt_next = (page_readbuf[pt_size-11]>>4)&0x0F;
	printf("next pt %d\n",pt_next);
	
	if(pt_next == 0x1)
	{
		 dev->bread(dev,(pt_addr+pt_size)/PAGE_SIZE,pt_size/PAGE_SIZE,(u8*)page_buf);
		 printf("next_pt addr 0x%x: 0x%x",pt_addr+pt_size,*((u32 *)page_buf));
		 if((is_valid_pt(page_buf)&&is_valid_pt(&page_buf[pt_size-4]))||(is_valid_mpt(page_buf)&&is_valid_mpt(&page_buf[pt_size-4]))){
				pt_next = 1;
				printf("find next pt\n");
				if(g_emmc_size<0x100000000){
					memcpy(&page_readbuf[pn_per_pt*sizeof(pt_resident32)],&page_buf[4],pt_size-8);
				}else{
					memcpy(&page_readbuf[pn_per_pt*sizeof(pt_resident)],&page_buf[4],pt_size-8);
				}
		}else{
			printf("can not find next pt, error\n");
		}
	}
	if(g_emmc_size<0x100000000){ //32bit
		printf("32bit parse PMT\n");
		memcpy(&lastest_part32,page_readbuf,PART_MAX_COUNT*sizeof(pt_resident32));
		
		memset(&lastest_part,0,PART_MAX_COUNT*sizeof(pt_resident));
		for(i=0;i<PART_MAX_COUNT;i++)
		{
			if(lastest_part32[i].size!=0){
				memcpy(lastest_part[i].name,lastest_part32[i].name,MAX_PARTITION_NAME_LEN);
				lastest_part[i].size= lastest_part32[i].size;
				lastest_part[i].offset= lastest_part32[i].offset;
				lastest_part[i].mask_flags= lastest_part32[i].mask_flags;
			}
		}
	}else{
		memcpy(buf,page_readbuf,PART_MAX_COUNT*sizeof(pt_resident));
		printf("64bit parse PMT\n");
	}
return reval;       
}
#endif
int load_exist_part_tab(u8 *buf)
{
        #ifdef MTK_EMMC_SUPPORT
            int reval = ERR_NO_EXIST;
            int index = 0;
            int i,j;
            int len=0;
            char *buf_p;
            int pt_start;
            int mpt_start;
        		int pt_size = 2048;
        		int buffer_size = EMMC_PMT_BUFFER_SIZE;
        		u8 *page_buf = (u8 *)EMMC_PMT_BUFFER_START;
        		int pt_next = 0;
						int pt_addr = 0;
  
            int pn_per_pt = (g_emmc_size<0x100000000)?(pt_size/sizeof(pt_resident32)):(pt_size/sizeof(pt_resident));
             PAGE_SIZE = 512;
            
        
            blkdev_t *dev= blkdev_get(BOOTDEV_SDMMC);
            pt_start = dev->offset * dev->blksz + 1024; /* user area offset + 1024 */
            mpt_start = pt_start + pt_size;
            //if(false == bSecRead)
            {
                printf("============func=%s===scan pmt from %x=====\n", __func__,pt_start);
            }
            /* try to find the pmt at fixed address, signature:0x50547631 */
            for(i=0;i<CFG_EMMC_PMT_SIZE/buffer_size;i++)
            {
                buf_p = page_buf;
                dev->bread(dev,(pt_start + i*buffer_size)/PAGE_SIZE,buffer_size/PAGE_SIZE,(u8*)page_buf);
                for(j=0;j<buffer_size/PAGE_SIZE;j++){
                
            //  printf("search %x %x\n",buf_p,pt_start + i*4096+j*PAGE_SIZE);
                    if(is_valid_pt(buf_p)){
                
                        //if(false == bSecRead)
                        {
                            printf("find h-pt at %x \n",pt_start + i*buffer_size+j*PAGE_SIZE);
                        }
                        if((buffer_size-j*PAGE_SIZE) < pt_size){
                            len = buffer_size- j*PAGE_SIZE;

                            //if(false == bSecRead)
                            {
                                printf("left %d j=%d\n",len,j);
                            }
                            memcpy(page_readbuf,&buf_p[PT_SIG_SIZE],len-PT_SIG_SIZE);
                            dev->bread(dev,(pt_start + (i+1)*buffer_size)/PAGE_SIZE,pt_size/PAGE_SIZE,(u8*)page_buf);
                            if(is_valid_pt(&page_buf[pt_size-4-len])){
                                //if(false == bSecRead)
                                {
                                    printf("find pt at %x \n",pt_start + i*buffer_size+j*PAGE_SIZE);
                                }
                                pt_addr = pt_start + i*buffer_size+j*PAGE_SIZE;
                                memcpy(&page_readbuf[len-PT_SIG_SIZE],page_buf,pt_size-len);
                               // memcpy(buf,page_readbuf,sizeof(lastest_part));
                                reval=DM_ERR_OK;
                                goto find;
                               // return reval;
                            }
                            
                        }else{
                            if(is_valid_pt(&buf_p[pt_size-4])){
                                //if(false == bSecRead)
                                {
                                    printf("find pt at %x \n",pt_start + i*buffer_size+j*PAGE_SIZE);
                                }
                                pt_addr = pt_start + i*buffer_size+j*PAGE_SIZE;
                               // memcpy(buf,&buf_p[PT_SIG_SIZE],sizeof(lastest_part));
                              	memcpy(page_readbuf,&buf_p[PT_SIG_SIZE],pt_size-PT_SIG_SIZE);
                                reval=DM_ERR_OK;
                                goto find;
                                //return reval;
                            }
                        }
                        break;
                    }
                buf_p += PAGE_SIZE;
              }
            }
            if(i == CFG_EMMC_PMT_SIZE/buffer_size)
            {
                for(i=0;i<CFG_EMMC_PMT_SIZE/buffer_size;i++){
                /* try to find the backup pmt at fixed address, signature:0x4d505431 */
                buf_p = page_buf;
                dev->bread(dev,(mpt_start + i*buffer_size)/PAGE_SIZE,buffer_size/PAGE_SIZE,(u8*)page_buf);
                
                for(j=0;j<buffer_size/PAGE_SIZE;j++){

                    if(is_valid_mpt(buf_p)){
                
                        //if(false == bSecRead)
                        {
                            printf("find h-pt at %x \n",mpt_start + i*buffer_size+j*PAGE_SIZE);
                        }
                        if((buffer_size - j*PAGE_SIZE) < pt_size){
                            len = buffer_size- j*PAGE_SIZE;
                            //if(false == bSecRead)
                            {
                                printf("left %d j=%d\n",len,j);
                            }
                            memcpy(page_readbuf,&buf_p[PT_SIG_SIZE],len-PT_SIG_SIZE);
                            dev->bread(dev,(mpt_start + (i+1)*buffer_size)/PAGE_SIZE,pt_size/PAGE_SIZE,(u8*)page_buf);
                            if(is_valid_mpt(&page_buf[pt_size-4-len])){
                                //if(false == bSecRead)
                                {
                                    printf("find mpt at %x \n",mpt_start + i*buffer_size+j*PAGE_SIZE);
                                }
                                pt_addr = mpt_start + i*buffer_size+j*PAGE_SIZE;
                                memcpy(&page_readbuf[len-PT_SIG_SIZE],page_buf,pt_size-len+PT_SIG_SIZE);
                               // memcpy(buf,page_readbuf,sizeof(lastest_part));
                                reval=DM_ERR_OK;
                                goto find;
                               // return reval;
                            }
                            
                        }else{
                            if(is_valid_mpt(&buf_p[pt_size-4])){
                                //if(false == bSecRead)
                                {
                                    printf("find mpt at %x \n",mpt_start + i*buffer_size+j*PAGE_SIZE);
                                }
                                 pt_addr = mpt_start + i*buffer_size+j*PAGE_SIZE;
                                memcpy(page_readbuf,&buf_p[PT_SIG_SIZE],pt_size-PT_SIG_SIZE);
                               // memcpy(buf,&buf_p[PT_SIG_SIZE],sizeof(lastest_part));
                                reval=DM_ERR_OK;
                               goto find;
                               // return reval;
                            }
                        }
                        break;
                    }
                    buf_p += PAGE_SIZE;
                }
                }
                
                }
            if(i == CFG_EMMC_PMT_SIZE/buffer_size)
            {
                //if(false == bSecRead)
                {   
                    printf("find no pt or mpt\n");
                }
            }

            return reval;
   find:
		pt_next = (page_readbuf[pt_size-11]>>4)&0x0F;
		printf("next pt %d\n",pt_next);
		
		if(pt_next == 0x1)
		{
			 dev->bread(dev,(pt_addr+pt_size)/PAGE_SIZE,pt_size/PAGE_SIZE,(u8*)page_buf);
			 printf("next_pt addr 0x%x: 0x%x",pt_addr+pt_size,*((u32 *)page_buf));
			 if((is_valid_pt(page_buf)&&is_valid_pt(&page_buf[pt_size-4]))||(is_valid_mpt(page_buf)&&is_valid_mpt(&page_buf[pt_size-4]))){
					pt_next = 1;
					printf("find next pt\n");
					if(g_emmc_size<0x100000000){
						memcpy(&page_readbuf[pn_per_pt*sizeof(pt_resident32)],&page_buf[4],pt_size-8);
					}else{
						memcpy(&page_readbuf[pn_per_pt*sizeof(pt_resident)],&page_buf[4],pt_size-8);
					}
			}else{
				printf("can not find next pt, error\n");
			}
		}
		if(g_emmc_size<0x100000000){ //32bit
			printf("32bit parse PMT\n");
			memcpy(&lastest_part32,page_readbuf,PART_MAX_COUNT*sizeof(pt_resident32));
			
			memset(&lastest_part,0,PART_MAX_COUNT*sizeof(pt_resident));
			for(i=0;i<PART_MAX_COUNT;i++)
			{
				if(lastest_part32[i].size!=0){
					memcpy(lastest_part[i].name,lastest_part32[i].name,MAX_PARTITION_NAME_LEN);
					lastest_part[i].size= lastest_part32[i].size;
					lastest_part[i].offset= lastest_part32[i].offset;
					lastest_part[i].mask_flags= lastest_part32[i].mask_flags;
				}
			}
		}else{
			memcpy(buf,page_readbuf,PART_MAX_COUNT*sizeof(pt_resident));
			printf("64bit parse PMT\n");
		}
	return reval;       

#else
	int pt_start_addr;
	int pt_cur_addr;
	int pt_locate;
	int reval=DM_ERR_OK;
	int mirror_address;
	char pmt_spare[PT_SIG_SIZE];
	
	PAGE_SIZE = (u32) g_nand_chip.page_size;
	BLOCK_SIZE = (u32) g_nand_chip.erasesize;
	total_size = (int)(g_nand_chip.chipsize);
	
	pt_start_addr = total_size;

	MSG (INIT, "load_pt from %x\n",pt_start_addr);
	memset(&pi,0xFF,sizeof(pi));
	for(pt_locate=0;pt_locate<(BLOCK_SIZE/PAGE_SIZE);pt_locate++)
	{
		pt_cur_addr = pt_start_addr+pt_locate*PAGE_SIZE;
		memset(pmt_spare,0xFF,PT_SIG_SIZE);
		memset(g_nand_spare,0xFF,64);
		if(!mtk_nand_read_page_hwecc(pt_cur_addr, page_readbuf))
		{
			MSG (INIT, "load_pt read pt failded\n");
		}
		memcpy(pmt_spare,&g_nand_spare[1] ,PT_SIG_SIZE);
		if(is_valid_pt(page_readbuf)&&is_valid_pt(pmt_spare))
		{
			pi.sequencenumber = g_nand_spare[5];
			MSG (INIT, "load_pt find valid pt at %x sq %x \n",pt_start_addr,pi.sequencenumber);
			break;
		}
		else
		{
			continue;
		}
	}

	if(pt_locate==(BLOCK_SIZE/PAGE_SIZE))
	{
		//first download or download is not compelte after erase or can not download last time
		MSG (INIT, "load_pt find pt failed \n");
		pi.pt_has_space = 0; //or before download pt power lost
		
		if(!find_mirror_pt_from_bottom(&mirror_address))
		{
			MSG (INIT, "First time download \n");
			reval=ERR_NO_EXIST;
			return reval;
		}
		else
		{
			//used the last valid mirror pt, at lease one is valid.
			mtk_nand_read_page_hwecc(mirror_address, page_readbuf);
		}
	}
	memcpy(buf,&page_readbuf[PT_SIG_SIZE],sizeof(lastest_part));

	return reval;
#endif
}
#ifndef MTK_EMMC_SUPPORT
int new_part_tab(u8 *buf)
{
	DM_PARTITION_INFO_PACKET  *dm_part= (DM_PARTITION_INFO_PACKET *)buf;
	int part_num,change_index,i;
	int retval;
	int pageoffset;
	int start_addr=total_size+BLOCK_SIZE;
	int current_addr=0;
	
	pi.pt_changed =0;
	pi.tool_or_sd_update = 1;

	MSG (INIT, "new_pt par_nub  enter \n");
	//the first image is ?
	
	for(part_num=0;part_num<PART_MAX_COUNT;part_num++)
	{
		memcpy(new_part[part_num].name,dm_part->part_info[part_num].part_name,MAX_PARTITION_NAME_LEN);
		new_part[part_num].offset=dm_part->part_info[part_num].start_addr;
		new_part[part_num].size=dm_part->part_info[part_num].part_len;
		new_part[part_num].mask_flags=0;
		//MSG (INIT, "DM_PARTITION_INFO_PACKET %s size %x %x \n",dm_part->part_info[part_num].part_name,dm_part->part_info[part_num].part_len,part_num);
		MSG (INIT, "new_pt %s size %x \n",new_part[part_num].name,new_part[part_num].size);
		if(dm_part->part_info[part_num].part_len ==0)
		{
			MSG (INIT, "new_pt last %x \n",part_num);
			break;
		}
	}
	MSG (INIT, "new_pt par_nub %x \n",part_num);
	#if 1
	//++++++++++for test
	#if 0
	part_num=13;
	memcpy(&new_part[0],&lastest_part[0],sizeof(new_part));
	MSG (INIT, "new_part  %x size  \n",sizeof(new_part));
	for(i=0;i<part_num;i++)
	{
		MSG (INIT, "npt partition %s size  \n",new_part[i].name);
		//MSG (INIT, "npt %x size  \n",new_part[i].offset);
		//MSG (INIT, "npt %x size  \n",lastest_part[i].offset);
		//MSG (INIT, "npt %x size  \n",new_part[i].size);
		dm_part->part_info[5].part_visibility =1;
		dm_part->part_info[5].dl_selected =1;
		new_part[5].size = lastest_part[5].size+0x100000;
	}
	#endif
	//------------for test
	//Find the first changed partition, whether is visible
	for(change_index=0;change_index<=part_num;change_index++)
	{
		if((new_part[change_index].size!=lastest_part[change_index].size)||(new_part[change_index].offset!=lastest_part[change_index].offset))
		{
			MSG (INIT, "new_pt %x size changed from %x to %x\n",change_index,lastest_part[change_index].size,new_part[change_index].size);
			pi.pt_changed =1;
			break;
		}
	}

      if(pi.pt_changed==1)
      	{
		//Is valid image update
		for(i=change_index;i<=part_num;i++)
		{
			if(dm_part->part_info[i].dl_selected==0&&dm_part->part_info[i].part_visibility==1)
			{
				
				MSG (INIT, "Full download is need %x \n",i);
				retval=DM_ERR_NO_VALID_TABLE;
				return retval;
			}
		}

		pageoffset=find_empty_page_from_top(start_addr);
		//download partition used the new partition
		//write mirror at the same 2 page
		memset(page_buf,0xFF,PAGE_SIZE+64);
		*(int *)sig_buf = MPT_SIG;
		memcpy(page_buf,&sig_buf,PT_SIG_SIZE);
		memcpy(&page_buf[PT_SIG_SIZE],&new_part[0],sizeof(new_part));		
		memcpy(&page_buf[PAGE_SIZE],&sig_buf,PT_SIG_SIZE);
		pi.sequencenumber+=1;
		memcpy(&page_buf[PAGE_SIZE+PT_SIG_SIZE],&pi,PT_SIG_SIZE);
		#if 0
             for(i=0;i<8;i++)
             {
             	MSG (INIT, "%x\n",page_buf[i]);
             }	
		#endif	 
		if(pageoffset!=0xFFFF)                                                      
		{
			if((pageoffset%2)!=0)
			{
				MSG (INIT, "new_pt mirror block may destroy last time%x\n",pageoffset);
				pageoffset+=1;	
			}
			for(i=0;i<2;i++)
			{
				current_addr=start_addr+(pageoffset+i)*PAGE_SIZE;
				if(!mtk_nand_write_page_hwecc(current_addr, page_buf))
				{
					MSG (INIT, "new_pt write m first page failed %x\n",current_addr);
				}
				else
				{
					MSG (INIT, "new_pt w_mpt at %x\n",current_addr);
					//read back verify
					if((!mtk_nand_read_page_hwecc(current_addr, page_readbuf))||memcmp(page_buf,page_readbuf,PAGE_SIZE))
					{
						MSG (INIT, "new_pt read or verify first mirror page failed %x \n",current_addr);
						memcpy(page_buf,0,PT_SIG_SIZE);
						if(mtk_nand_write_page_hwecc(current_addr,page_buf))
						{
							MSG (INIT, "new_pt mark failed %x\n",current_addr);
						}		
					}
					else
					{
						MSG (INIT, "new_pt w_mpt ok %x\n",i);
						//any one success set this flag?
						pi.mirror_pt_dl=1;
					}
				}
			}
	      	}
	}
	else
	{
		MSG (INIT, "new_part_tab no pt change %x\n",i);
	}
#endif	
	retval=DM_ERR_OK;
//for test
//	retval=DM_ERR_NO_VALID_TABLE;
	return retval;
}


int update_part_tab(void)
{
	int retval=0;
	int retry_w;
	int retry_r;
	int start_addr=total_size;
	int current_addr=0;
	//for test
     // return DM_ERR_NO_SPACE_FOUND;
	memset(page_buf,0xFF,PAGE_SIZE+64);
	if((pi.pt_changed==1||pi.pt_has_space==0)&&pi.tool_or_sd_update== 1)
	{	
		MSG (INIT, "update_pt pt changes\n");

		if(!mtk_nand_erase(start_addr)) 
		{//no good block for used in replace pool
			MSG (INIT, "update_pt erase failed %x\n",start_addr);
			if(pi.mirror_pt_dl==0)
			retval = DM_ERR_NO_SPACE_FOUND;
			return retval;
		}

		for(retry_r=0;retry_r<RETRY_TIMES;retry_r++)
		{
			for(retry_w=0;retry_w<RETRY_TIMES;retry_w++)
			{
				current_addr = start_addr+(retry_w+retry_r*RETRY_TIMES)*PAGE_SIZE;
				*(int *)sig_buf = PT_SIG;
				memcpy(page_buf,&sig_buf,PT_SIG_SIZE);
				memcpy(&page_buf[PT_SIG_SIZE],&new_part[0],sizeof(new_part));		
				memcpy(&page_buf[PAGE_SIZE],&sig_buf,PT_SIG_SIZE);
				memcpy(&page_buf[PAGE_SIZE+PT_SIG_SIZE],&pi,PT_SIG_SIZE);
				
				if(!mtk_nand_write_page_hwecc(current_addr, page_buf))
				{//no good block for used in replace pool . still used the original ones
					MSG (INIT, "update_pt write failed %x\n",retry_w);
					memset(page_buf,0,PT_SIG_SIZE);
					if(!mtk_nand_write_page_hwecc(current_addr, page_buf))
					{
						MSG (INIT, "write error mark failed\n");
						//continue retry
						continue;
					}		
				}
				else
				{
					MSG (INIT, "write pt success %x %x \n",current_addr,retry_w);
					break; // retry_w should not count.
				}
			}
			if(retry_w==RETRY_TIMES)
			{
				MSG (INIT, "update_pt retry w failed\n");
				if(pi.mirror_pt_dl==0)//mirror also can not write down
				{
					retval = DM_ERR_NO_SPACE_FOUND;
					return retval;
				}
				else
				{
					return DM_ERR_OK;
				}
			}
			current_addr = (start_addr+(((retry_w)+retry_r*RETRY_TIMES)*PAGE_SIZE));
			if(!mtk_nand_read_page_hwecc(current_addr, page_readbuf)||memcmp(page_buf,page_readbuf,PAGE_SIZE))
			{
				
				MSG (INIT, "v or r failed %x\n",retry_r);
				memset(page_buf,0,PT_SIG_SIZE);
				if(!mtk_nand_write_page_hwecc(current_addr,page_buf))
				{
					MSG (INIT, "read error mark failed\n");
					//continue retryp
					continue;
				}

			}
			else
			{
				MSG (INIT, "update_pt r&v ok%x\n",current_addr);
				break;
			}
		}		
	}
	else
	{
		MSG (INIT, "update_pt no change \n");
	}
	
	return DM_ERR_OK;

}
#endif
#endif
