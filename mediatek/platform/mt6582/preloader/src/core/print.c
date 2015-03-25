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

#include "typedefs.h"
#include "platform.h"
#include "print.h"
#include "uart.h"
#include "dram_buffer.h"

#define C_LOG_SRAM_BUF_SIZE (20480)
static char log_sram_buf[C_LOG_SRAM_BUF_SIZE];

#define LOG_BUFFER_MAX_SIZE             (0x10000)
#define log_dram_buf g_dram_buf->log_dram_buf

char *  log_ptr;
char *  log_hdr ;
char *  log_end ;
static int  g_log_drambuf = 1;
static int  g_log_disable = 0;


static int  g_log_miss_chrs = 0;

static void outchar(const char c)
{
    if (g_log_disable) {
        if (log_ptr < log_end)
            *log_ptr++ = (char)c;        
        else 
            g_log_miss_chrs++;
    } else {
        PutUARTByte(c);
    }
}

static void outstr(const unsigned char *s)
{
    while (*s) {
        if (*s == '\n')
            outchar('\r');
        outchar(*s++);
    }
}

static void outdec(unsigned long n)
{
    if (n >= 10) {
        outdec(n / 10);
        n %= 10;
    }
    outchar((unsigned char)(n + '0'));
}

static void outhex(unsigned long n, long depth)
{
    if (depth)
        depth--;
    
    if ((n & ~0xf) || depth) {
        outhex(n >> 4, depth);
        n &= 0xf;
    }
    
    if (n < 10) {
        outchar((unsigned char)(n + '0'));
    } else {
        outchar((unsigned char)(n - 10 + 'A'));
    }
}

void log_buf_ctrl(int drambuf)
{    
    if (drambuf) {
        if ((g_log_disable) && (!g_log_drambuf)) {
            char *buf_ptr = log_hdr;
            U32 buf_len = log_ptr - log_hdr;

            log_hdr = (char*)log_dram_buf;
            log_end = log_hdr + LOG_BUFFER_MAX_SIZE;    
            log_ptr = log_hdr;
            if (buf_len) {
                memcpy(log_hdr, buf_ptr, buf_len);
                log_ptr = log_hdr + buf_len;
            }
            if (g_log_miss_chrs) {
                outstr("\n{MISS: ");
                outdec(g_log_miss_chrs);
                outstr(" chars}\n");
                g_log_miss_chrs = 0;
            }
        } else if (!g_log_disable) {
            log_hdr = (char*)log_dram_buf;
            log_end = log_hdr + LOG_BUFFER_MAX_SIZE;        
            log_ptr = log_hdr;
        }
    } else {
        log_hdr = (char*)log_sram_buf;
        log_end = log_hdr + C_LOG_SRAM_BUF_SIZE;
        log_ptr = log_hdr;        
    }
    
    g_log_drambuf = drambuf ? 1 : 0;
}

void log_ctrl(int enable)
{
    u32 len;
    char *ptr;

    g_log_disable = enable ? 0 : 1;

    /* flush log and reset log buf ptr */
    if (enable) {
        ptr = (char*)log_hdr;
        len = (u32)log_ptr - (u32)ptr;
        for (;len;len--) {
            outchar(*ptr++);
        }
        log_ptr = log_hdr;
    }
}

int log_status(void)
{
    return g_log_disable == 0 ? 1 : 0;
}

void dbg_print(char *fmt, ...)
{
    print(fmt);
}

void vprint(char *fmt, va_list vl)
{
    unsigned char c;
    unsigned int reg = 1; /* argument register number (32-bit) */

    while (*fmt) {
        c = *fmt++;
        switch (c)
        {
        case '%':
            c = *fmt++;
            switch (c)
            {
            case 'x':
                outhex(va_arg(vl, unsigned long), 0);
                break;
            case 'B':
                outhex(va_arg(vl, unsigned long), 2);
                break;
            case 'H':
                outhex(va_arg(vl, unsigned long), 4);
                break;
            case 'X':
                outhex(va_arg(vl, unsigned long), 8);
                break;
            case 'l':
                if (*fmt == 'l' && *(fmt+1) == 'x') {
                    u32 ltmp;
                    u32 htmp;

                    #ifdef __ARM_EABI__
                    /* Normally, compiler uses r0 to r6 to pass 32-bit or 64-bit 
                     * arguments. But with EABI, 64-bit arguments will be aligned 
                     * to an _even_ numbered register. for example:
                     *
                     *   int foo(int a, long long b, int c)
                     *
                     *   EABI: r0: a, r1: unused, r2-r3: b, r4: c
                     *   Normal: r0: a, r1-r2: b, r3:c
                     * 
                     * For this reason, need to align to even numbered register
                     * to retrieve 64-bit argument.
                     */

                    /* odd and unused argument */
                    if (reg & 0x1) {
                        /* 64-bit argument starts from next 32-bit register */
                        reg++;
                        /* ignore this 32-bit register */
                        ltmp = va_arg(vl, unsigned int);
                    }
                    reg++; /* 64-bit argument uses one more 32-bit register */
                    #endif
                    ltmp = va_arg(vl, unsigned int);
                    htmp = va_arg(vl, unsigned int);                    
                    
                    outhex(htmp, 8);
                    outhex(ltmp, 8);
                    fmt += 2;
                }
                break;
            case 'd':
                {
                    long l;

                    l = va_arg(vl, long);
                    if (l < 0) {
                        outchar('-');
                        l = -l;
                    }
                    outdec((unsigned long) l);
                }
                break;
            case 'u':
                outdec(va_arg(vl, unsigned long));
                break;
            case 's':
                outstr((const unsigned char *)va_arg(vl, char *));
                break;
            case '%':
                outchar('%');
                break;
            case 'c':
                c = va_arg(vl, int);
                outchar(c);
                break;
            default:
                outchar(' ');
                break;
            }
            reg++; /* one argument uses 32-bit register */
            break;
        case '\r':
            if (*fmt == '\n')
                fmt++;
            c = '\n';
        // fall through
        case '\n':
            outchar('\r');
        // fall through
        default:
            outchar(c);
        }
    }
}

void print(char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vprint(fmt, args);
    va_end(args);
}
