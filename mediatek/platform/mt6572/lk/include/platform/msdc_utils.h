/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2010
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

#ifndef _MSDC_UTILS_H_
#define _MSDC_UTILS_H_

#include "msdc_cfg.h"
#include "mmc_types.h"

#if defined(MMC_MSDC_DRV_CTP)
#include <common.h>
#endif

#if defined(MMC_MSDC_DRV_PRELOADER)
#include <mtk_timer.h>
#endif

#if defined(MMC_MSDC_DRV_LK)
#include <mt_gpt.h>
#include <printf.h>
#endif

#define MSDC_WRITE32(addr,data)     (*(volatile uint32*)(addr) = (uint32)(data))
#define MSDC_READ32(addr)           (*(volatile uint32*)(addr))

/* Debug message event */
#define MSG_EVT_NONE    0x00000000  /* No event */
#define MSG_EVT_DMA     0x00000001  /* DMA related event */
#define MSG_EVT_CMD     0x00000002  /* MSDC CMD related event */
#define MSG_EVT_RSP     0x00000004  /* MSDC CMD RSP related event */
#define MSG_EVT_INT     0x00000008  /* MSDC INT event */
#define MSG_EVT_CFG     0x00000010  /* MSDC CFG event */
#define MSG_EVT_FUC     0x00000020  /* Function event */
#define MSG_EVT_OPS     0x00000040  /* Read/Write operation event */
#define MSG_EVT_FIO     0x00000080  /* FIFO operation event */
#define MSG_EVT_OPS_MMC 0x00000100  /* MMC operation event */
#define MSG_EVT_TUNE    0x00000400  /* Tuning event */
#define MSG_EVT_INF     0x01000000  /* information event */
#define MSG_EVT_WRN     0x02000000  /* Warning event */
#define MSG_EVT_ERR     0x04000000  /* Error event */

#define MSG_EVT_ALL     0xffffffff

//#define MSG_EVT_MASK  (MSG_EVT_ALL & ~MSG_EVT_DMA & ~MSG_EVT_WRN & ~MSG_EVT_RSP & ~MSG_EVT_INT & ~MSG_EVT_CMD & ~MSG_EVT_OPS)
//#define MSG_EVT_MASK  (MSG_EVT_ALL & ~MSG_EVT_FIO)
//#define MSG_EVT_MASK  (MSG_EVT_FIO)
//#define MSG_EVT_MASK    (MSG_EVT_ALL)
//#define MSG_EVT_MASK    (MSG_EVT_OPS_MMC|MSG_EVT_OPS)
#define MSG_EVT_MASK    (MSG_EVT_TUNE)

#undef MSG

#if MSG_EVT_MASK!=MSG_EVT_NONE
#define MSG(evt, fmt, args...) \
    do {    \
        if ((MSG_EVT_##evt) & MSG_EVT_MASK) { \
            printf(fmt, ##args); \
        } \
    } while(0)

#define MSG_FUNC(f) MSG(FUC, "<FUNC>: %s\n", __FUNCTION__)
#else
#define MSG(evt, fmt, args...)
#define MSG_FUNC(f)
#endif

#if defined(MMC_MSDC_DRV_PRELOADER)
//printf(fmt, args...)  is defined as print in core/inc/typedefs.h
#define msdc_printf(fmt, args...)     print(fmt, ##args)
#endif

#if defined(MMC_MSDC_DRV_LK)
#include <string.h> //For memcpy, memset
#endif

#if defined(MMC_MSDC_DRV_CTP)
#undef printf //printf may have been defined as sys_printf in inc/CTP_type.h
#define printf(fmt, args...)          dbg_print(fmt, ##args)
#define msdc_printf(fmt, args...)     dbg_print(fmt, ##args)
#endif


#if 0
#define memcpy(dst,src,sz)      KAL_memcpy(dst, src, sz)
#define memset(p,v,s)           KAL_memset(p, v, s)
#define free(p)                 KAL_free(p)
#define malloc(sz)              KAL_malloc(sz,4, KAL_USER_MSDC)
#endif

#undef BUG_ON
#define BUG_ON(x) \
    do { \
        if (x) { \
            printf("[BUG] %s LINE:%d FILE:%s\n", #x, __LINE__, __FILE__); \
            while(1); \
        } \
    }while(0)

#undef WARN_ON
#define WARN_ON(x) \
    do { \
        if (x) { \
            MSG(WRN, "[WARN] %s LINE:%d FILE:%s\n", #x, __LINE__, __FILE__); \
        } \
    }while(0)

#define ERR_EXIT(expr, ret, expected_ret) \
    do { \
        (ret) = (expr);\
        if ((ret) != (expected_ret)) { \
            printf("[ERR] LINE:%d: %s != %d (%d)\n", __LINE__, #expr, expected_ret, ret); \
            goto exit; \
        } \
    } while(0)

#undef ARRAY_SIZE
#define ARRAY_SIZE(x)       (sizeof(x) / sizeof((x)[0]))

/*
 * ffs: find first bit set. This is defined the same way as
 * the libc and compiler builtin ffs routines, therefore
 * differs in spirit from the above ffz (man ffs).
 */

#ifdef MSDC_INLINE_UTILS
static inline uint32 uffs(uint32 x)
{
    int r = 1;

    if (!x)
        return 0;
    if (!(x & 0xffff)) {
        x >>= 16;
        r += 16;
    }
    if (!(x & 0xff)) {
        x >>= 8;
        r += 8;
    }
    if (!(x & 0xf)) {
        x >>= 4;
        r += 4;
    }
    if (!(x & 3)) {
        x >>= 2;
        r += 2;
    }
    if (!(x & 1)) {
        x >>= 1;
        r += 1;
    }
    return r;
}

static inline unsigned int ntohl(unsigned int n)
{
    unsigned int t;
    unsigned char *b = (unsigned char*)&t;
    *b++ = ((n >> 24) & 0xFF);
    *b++ = ((n >> 16) & 0xFF);
    *b++ = ((n >> 8) & 0xFF);
    *b   = ((n) & 0xFF);
    return t;
}

#define set_field(reg,field,val) \
    do {    \
        unsigned int tv = (unsigned int)(*(volatile u32*)(reg)); \
        tv &= ~(field); \
        tv |= ((val) << (uffs((unsigned int)field) - 1)); \
        (*(volatile u32*)(reg) = (u32)(tv)); \
    } while(0)

#define get_field(reg,field,val) \
    do {    \
        unsigned int tv = (unsigned int)(*(volatile u32*)(reg)); \
        val = ((tv & (field)) >> (uffs((unsigned int)field) - 1)); \
    } while(0)

#else
extern unsigned int msdc_uffs(unsigned int x);
extern unsigned int msdc_ntohl(unsigned int n);
extern void msdc_set_field(volatile u32 *reg, u32 field, u32 val);
extern void msdc_get_field(volatile u32 *reg, u32 field, u32 *val);

#define uffs(x)               msdc_uffs(x)
#define ntohl(n)              msdc_ntohl(n)
#define set_field(r,f,v)      msdc_set_field((volatile u32*)r,f,v)
#define get_field(r,f,v)      msdc_get_field((volatile u32*)r,f,&v)

#endif


#ifndef min
#define min(x, y)   (x < y ? x : y)
#endif

#ifndef max
#define max(x, y)   (x > y ? x : y)
#endif

#if defined(MMC_MSDC_DRV_CTP)
#if !defined(__FPGA__)
#define udelay(us) do{GPT_Delay_us(us);}while(0)
#define mdelay(ms) do{GPT_Delay_ms(ms);}while(0)

#else
#define udelay(us)  \
    do { \
        volatile int count = us * 10; \
        while (count--); \
    }while(0)

#define udelay1(us) \
    do{ \
        MSDC_WRITE32(0x10008040,0x31);\
        MSDC_WRITE32(0x10008044,0x0);\
        u32 test_t1 = MSDC_READ32(0x10008048); \
        u32 test_t2; \
        do{ \
            test_t2= MSDC_READ32(0x10008048); \
        } \
        while((test_t2-test_t1) < us * 6);\
    }while(0)

#define mdelay(ms) \
    do { \
        unsigned long i; \
        for (i = 0; i < ms; i++) \
            udelay(1000); \
    }while(0)

#define mdelay1(ms) \
    do { \
        unsigned long i; \
        for (i = 0; i < ms; i++) \
            udelay1(1000); \
    }while(0)
#endif
#endif

#define WAIT_COND(cond,tmo,left) \
    do { \
        volatile u32 t = tmo; \
        u32 infinite_wait; \
        if ( t==0 ) infinite_wait=1; \
        else infinite_wait=0; \
        while (1) { \
            if ( cond ) break; \
            if ( !infinite_wait ) { \
                if ( t==0 ) break; \
                if ( t > 0 ) t--; \
            } \
            udelay(1); \
        } \
        if ( infinite_wait ) left = 1; \
        else left = t; \
        WARN_ON(left == 0); \
    } while(0)

#endif /* _MSDC_UTILS_H_ */

