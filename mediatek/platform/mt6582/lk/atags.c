/* Copyright (c) 2009-2012, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Code Aurora nor
 *       the names of its contributors may be used to endorse or promote
 *       products derived from this software without specific prior written
 *       permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <reg.h>
#include <debug.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <platform/mt_typedefs.h>
#include <platform/boot_mode.h>
#include <platform/mt_reg_base.h>
#include <platform/sec_devinfo.h>
#include <platform/env.h>
#include <dfo_boot.h>
extern int g_nr_bank;
extern BOOT_ARGUMENT *g_boot_arg;
extern BI_DRAM bi_dram[MAX_NR_BANK];



struct tag_header {
	u32 size;
	u32 tag;
};

#define tag_size(type)	((sizeof(struct tag_header) + sizeof(struct type)) >> 2)

#define SIZE_1M             (1024 * 1024)
#define SIZE_2M             (2 * SIZE_1M)
#define SIZE_256M           (256 * SIZE_1M)
#define SIZE_512M           (512 * SIZE_1M)

/* The list must start with an ATAG_CORE node */
#define ATAG_CORE	0x54410001
struct tag_core {
	u32 flags;		/* bit 0 = read-only */
	u32 pagesize;
	u32 rootdev;
};

/* it is allowed to have multiple ATAG_MEM nodes */
#define ATAG_MEM  0x54410002
typedef struct {
	uint32_t size;
	uint32_t start_addr;
}mem_info;

/* command line: \0 terminated string */
#define ATAG_CMDLINE	0x54410009
struct tag_cmdline {
	char	cmdline[1];	/* this is the minimum size */
};

/* describes where the compressed ramdisk image lives (physical address) */
#define ATAG_INITRD2	0x54420005
struct tag_initrd {
	u32 start;	/* physical start address */
	u32 size;	/* size of compressed ramdisk image in bytes */
};

#define ATAG_VIDEOLFB	0x54410008
struct tag_videolfb {
	u16		lfb_width;
	u16		lfb_height;
	u16		lfb_depth;
	u16		lfb_linelength;
	u32		lfb_base;
	u32		lfb_size;
	u8		red_size;
	u8		red_pos;
	u8		green_size;
	u8		green_pos;
	u8		blue_size;
	u8		blue_pos;
	u8		rsvd_size;
	u8		rsvd_pos;
};

/* boot information */
#define ATAG_BOOT	0x41000802
struct tag_boot {
	u32 bootmode;
};

/*META com port information*/
#define ATAG_META_COM 0x41000803
struct tag_meta_com {
	u32 meta_com_type; /* identify meta via uart or usb */
    u32 meta_com_id;  /* multiple meta need to know com port id */
};

/*device information*/
#define ATAG_DEVINFO_DATA         0x41000804
#define ATAG_DEVINFO_DATA_SIZE    22
struct tag_devinfo_data{
    u32 devinfo_data[ATAG_DEVINFO_DATA_SIZE];
    u32 devinfo_data_size;
};

#define ATAG_MDINFO_DATA 0x41000806
struct tag_mdinfo_data{
   u8 md_type[4];
};



/* The list ends with an ATAG_NONE node. */
#define ATAG_NONE	0x00000000


unsigned *target_atag_boot(unsigned *ptr)
{
  *ptr++ = tag_size(tag_boot);
  *ptr++ = ATAG_BOOT;
  *ptr++ = g_boot_mode;
  
  return ptr;
}

unsigned *target_atag_devinfo_data(unsigned *ptr)
{
  int i = 0;
  *ptr++ = tag_size(tag_devinfo_data);
  *ptr++ = ATAG_DEVINFO_DATA;

#if defined (TRUSTONIC_TEE_SUPPORT)
  for (i=0;i<ATAG_DEVINFO_DATA_SIZE;i++)
  {
    if(20==i)
    {
        *ptr++ = g_boot_arg->chip_ver;        
    }
    else
    {
        *ptr++ = get_devinfo_with_index(i);
    }
  }
#else
  for (i=0;i<ATAG_DEVINFO_DATA_SIZE;i++){  
    *ptr++ = get_devinfo_with_index(i);
  }
#endif

  *ptr++ = ATAG_DEVINFO_DATA_SIZE;  
  printf("SSSS:0x%x\n", get_devinfo_with_index(1));
  printf("SSSS:0x%x\n", get_devinfo_with_index(2));
  printf("SSSS:0x%x\n", get_devinfo_with_index(3));
  printf("SSSS:0x%x\n", get_devinfo_with_index(4));
  printf("SSSS:0x%x\n", g_boot_arg->chip_ver);
  printf("SSSS:0x%x\n", get_devinfo_with_index(21));

  return ptr;
}


unsigned *target_atag_mem(unsigned *ptr)
{
  int i;
  
  for (i = 0; i < g_nr_bank; i++) {
    *ptr++ = 4; //tag size
    *ptr++ = ATAG_MEM; //tag name
    *ptr++ = bi_dram[i].size;
    *ptr++ = bi_dram[i].start;
	}
  return ptr;
}

unsigned *target_atag_meta(unsigned *ptr)
{
  *ptr++ = tag_size(tag_meta_com);
  *ptr++ = ATAG_META_COM;
  *ptr++ = g_boot_arg->meta_com_type;
  *ptr++ = g_boot_arg->meta_com_id;
  printf("meta com type = %d\n", g_boot_arg->meta_com_type);
  printf("meta com id = %d\n", g_boot_arg->meta_com_id);
  return ptr;
}

/* todo: give lk strtoul and nuke this */
static unsigned hex2unsigned(const char *x)
{
    unsigned n = 0;

    while(*x) {
        switch(*x) {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            n = (n << 4) | (*x - '0');
            break;
        case 'a': case 'b': case 'c':
        case 'd': case 'e': case 'f':
            n = (n << 4) | (*x - 'a' + 10);
            break;
        case 'A': case 'B': case 'C':
        case 'D': case 'E': case 'F':
            n = (n << 4) | (*x - 'A' + 10);
            break;
        default:
            return n;
        }
        x++;
    }

    return n;
}

unsigned *target_atag_dfo(unsigned *ptr)
{
  int i, j=0 ;
  tag_dfo_boot *tag_dfo_p;
  char tmp[11];
  unsigned  char *buffer;

  buffer = (unsigned char *)get_env("DFO");

  if(buffer != NULL) {

    *ptr++ = ((sizeof(struct tag_header) + sizeof(tag_dfo_boot)) >> 2);
    *ptr++ = ATAG_DFO_DATA;

    tag_dfo_p = (tag_dfo_boot *) ptr;
    for(i = 0; i < DFO_BOOT_COUNT; i++) {

      do{
        tag_dfo_p->name[i][j] = *buffer;
        j++;
      }while(*buffer++ != ',' && j < 31);

      tag_dfo_p->name[i][j-1] = '\0';
      j = 0;

      do{
        tmp[j] = *buffer;
        j++;
      }while(*buffer++ != ',' && j < 10);

      tmp[j] = '\0';

      if((strncmp("0x", tmp, 2) == 0) || (strncmp("0X", tmp, 2) == 0))
         tag_dfo_p->value[i] = hex2unsigned(&tmp[2]);
      else
         tag_dfo_p->value[i] = atoi(tmp);

      j = 0;
    }

    ptr += sizeof(tag_dfo_boot)/sizeof(int);
    for(i = 0; i < DFO_BOOT_COUNT; i++)
      printf("[DFO-%d] NAME:%s, Value:%d\n",i , tag_dfo_p->name[i], tag_dfo_p->value[i]);

  } else
    printf("DFO = NULL\n");  

  return ptr;
}
unsigned *target_atag_commmandline(unsigned *ptr, char *commandline)
{
  char *p;
  
  if (!commandline)
		return NULL;

  for (p = commandline; *p == ' '; p++);

  if (*p == '\0')
		return NULL;
  
  *ptr++ = (sizeof (struct tag_header) + strlen (p) + 1 + 4) >> 2;; //size
  *ptr++ = ATAG_CMDLINE;
  strcpy((char *)ptr, p);  //copy to atags memory region
  ptr += (strlen (p) + 1 + 4) >> 2;
  return ptr;
}

unsigned *target_atag_initrd(unsigned *ptr, ulong initrd_start, ulong initrd_size)
{
  *ptr++ = tag_size(tag_initrd);
  *ptr++ = ATAG_INITRD2;
//TMP for bring up testing
//  *ptr++ = CFG_RAMDISK_LOAD_ADDR;
 // *ptr++ = 0x1072F9;
  
  *ptr++ = initrd_start;
  *ptr++ = initrd_size;
  return ptr;
}

extern UINT32  mt65xx_disp_get_vram_size(void);
extern unsigned int g_fb_base;
extern unsigned int g_fb_size;
unsigned *target_atag_videolfb(unsigned *ptr)
{
  *ptr++ = tag_size (tag_videolfb);
  *ptr++ = ATAG_VIDEOLFB;
  /*Frambuffer Base. TBD*/
  *(ptr+2) = g_fb_base;
  /*Frambuffer Size. TBD*/
  *(ptr+3) = g_fb_size;
  ptr += (sizeof(struct tag_videolfb)>>2);
  return ptr;
}


unsigned *target_atag_mdinfo(unsigned *ptr)
{
   unsigned char *p;
   *ptr++=tag_size(tag_mdinfo_data);
   *ptr++=ATAG_MDINFO_DATA;
    p=(unsigned char *)ptr;
   *p++=g_boot_arg->md_type[0];
   *p++=g_boot_arg->md_type[1];
   *p++=g_boot_arg->md_type[2];
   *p++=g_boot_arg->md_type[3];
   return (unsigned *)p;
}


void *target_get_scratch_address(void)
{
	return ((void *)SCRATCH_ADDR);
}
