/*
 * Copyright (c) 2008 Travis Geiselbrecht
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <printf.h>
#include <stdlib.h>
#include <string.h>
#include <dev/uart.h>

#include "tz_private/system.h"
#include "tz_private/sys_ipc.h"
#include "tz_private/log.h"
#include "tz_cross/ta_logctrl.h"

#define MTEE_LOG_BUFFER_SIZE  (1024*64)
#define MTEE_LOG_LVL_TAG      0x1

#define MTEE_LOG_FLAG_UART    0x10
#define MTEE_LOG_FLAG_REE     0x20

#define MTEE_LOG_LEVEL        (log_flag & 0xf)
#define MTEE_LOG_TO_UART      (log_flag & MTEE_LOG_FLAG_UART)
#define MTEE_LOG_TO_REE       (log_flag & MTEE_LOG_FLAG_REE)


static unsigned int log_flag = MTEE_LOG_LVL_DEFAULT;

#ifdef MTEE_ENABLE_LOG_IN_BUILD_TIME
static MTEE_SPINLOCK print_lock = MTEE_SPINLOCK_INIT_UNLOCK;
static char mtee_log_buf[MTEE_LOG_BUFFER_SIZE+1];
static int log_read_ptr, log_write_ptr;

static void __dput_uart(char c)
{
    if (c == '\n')
        uart_putc('\r');
    uart_putc(c);
}

static void _dputc_buffer(char c)
{
    mtee_log_buf[log_write_ptr++] = c;

    if (log_write_ptr >= MTEE_LOG_BUFFER_SIZE)
        log_write_ptr = 0;

    if (log_write_ptr == log_read_ptr)
    {
        log_read_ptr++;
        if (log_read_ptr >= MTEE_LOG_BUFFER_SIZE)
            log_read_ptr = 0;
    }
}
#endif

/*
 * Output all buffered log to REE.
 * !!!FIXME!!! Not ready yet.
static void __dputs_ree()
{
#ifdef MTEE_ENABLE_LOG_IN_BUILD_TIME
    int level = MTEE_LOG_LVL_PRINTF;
    int rptr, wptr;
    char buf[REE_SERVICE_BUFFER_SIZE];

    // Output/parse as much as possible to REE.
    // Already lock.
    while (log_read_ptr != log_write_ptr)
    {
        rptr = log_read_ptr;
        wptr = log_write_ptr;
        if (wptr < rptr)
            wptr = MTEE_LOG_BUFFER_SIZE;

        while (rptr < wptr &&
            rptr - log_read_ptr < REE_SERVICE_BUFFER_SIZE - 1)
        {
            if (mtee_log_buf[rptr] == MTEE_LOG_LVL_TAG) break;
            rptr++;
        }

        len = log_read_ptr - rptr;
        log_read

        if (mtee_log_buf[rptr] == MTEE_LOG_LVL_TAG)


        if (level >= MTEE_LOG_LEVEL)
        {
            /
            /
        }


    }
#endif    
}
*/

static void _dputc_lvl(int lvl, char c)
{
#ifdef MTEE_ENABLE_LOG_IN_BUILD_TIME
    uint32_t state;
    char buf[2];

    // Log to UART
    if (MTEE_LOG_TO_UART && lvl >= MTEE_LOG_LEVEL)
        __dput_uart(c);

    // Log to buffer
    state = MTEE_SpinLockMaskIrq(&print_lock);
    _dputc_buffer(c);
    MTEE_SpinUnlockRestoreIrq(&print_lock, state);

    // Log to REE, 
    // Can't be in spinlock, otherwise it might deadlock
    if (MTEE_LOG_TO_REE && lvl >= MTEE_LOG_LEVEL)
    {
        // Bad, no buffer now.
        buf[0] = c;
        buf[1] = 0;
        MTEE_Puts(buf);
    }
#endif
}

void _dputc(char c)
{
    _dputc_lvl(MTEE_LOG_LVL_PRINTF, c);
}

int dgetc(char *c, bool wait)
{
    int _c;

    if ((_c = uart_getc()) < 0)
        return -1;

    *c = _c;
    return 0;
}

static void _dputs_lvl(int lvl, const char *str)
{
#ifdef MTEE_ENABLE_LOG_IN_BUILD_TIME
    uint32_t state;
    int len, tleft, rptr, wptr;
    const char *tptr;

    if (!str[0])
        return;

    if (MTEE_LOG_TO_UART && lvl >= MTEE_LOG_LEVEL)
    {
        tptr = str;
        while(*tptr != 0) {
            __dput_uart(*tptr++);
        }
    }

    // Log to buffer
    state = MTEE_SpinLockMaskIrq(&print_lock);
    len = strlen(str);
    tptr = str;

    if (len > MTEE_LOG_BUFFER_SIZE - 8)
        len = MTEE_LOG_BUFFER_SIZE - 8;

    if (lvl >= 0)
    {
        _dputc_buffer(MTEE_LOG_LVL_TAG);
        _dputc_buffer(lvl);
    }

    // Copy log to buffer
    tleft = MTEE_LOG_BUFFER_SIZE - log_write_ptr;
    if (tleft > len) tleft = len;
    if (tleft)
    {
        memcpy(&mtee_log_buf[log_write_ptr], tptr, tleft);
        tptr += tleft;
    }
    if (len - tleft > 0)
    {
        memcpy(&mtee_log_buf[0], tptr, len - tleft);
    }

    // Calculate new read/write ptr
    wptr = log_write_ptr;
    rptr = log_read_ptr;
    if (rptr < wptr)
        rptr += MTEE_LOG_BUFFER_SIZE;

    wptr += len;
    if (wptr >= rptr)
        rptr = wptr + 1;
    log_write_ptr = (wptr & (MTEE_LOG_BUFFER_SIZE-1));
    log_read_ptr = (rptr & (MTEE_LOG_BUFFER_SIZE-1));

    MTEE_SpinUnlockRestoreIrq(&print_lock, state);

    // Log to REE
    // Can't be in spinlock, otherwise it might deadlock
    if (MTEE_LOG_TO_REE && lvl >= MTEE_LOG_LEVEL)
    {
        MTEE_Puts(str);
    }
#endif /* MTEE_ENABLE_LOG_IN_BUILD_TIME */
}

int _dputs(const char *str)
{
    _dputs_lvl(MTEE_LOG_LVL_PRINTF, str);
    return 0;
}

static int _dvprintf_lvl(int level, const char *fmt, va_list ap)
{
    char buf[256];
    int err;

    err = vsnprintf(buf, sizeof(buf), fmt, ap);

    _dputs_lvl(level, buf);

    return err;
}

int _dvprintf(const char *fmt, va_list ap)
{
    return _dvprintf_lvl(MTEE_LOG_LVL_PRINTF, fmt, ap);
}

int _MTEE_LOG(int level, const char *fmt, ...)
{
    int err;

    va_list ap;
    va_start(ap, fmt);
    err = _dvprintf_lvl(level, fmt, ap);
    va_end(ap);

    return err;
}

void MTEE_SetLogLvl(MTEE_LOG_LVL lvl)
{
    log_flag &= 0xfffffff0;
    log_flag |= (lvl&0x0000000f);
}

void MTEE_EnableLogREE(void)
{
    log_flag |= MTEE_LOG_FLAG_REE;
}

void _panic(void *caller, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    _dvprintf(fmt, ap);
    va_end(ap);

    abort();
}

void abort(void)
{
    while(1);
}

int raise(int signal)
{
    while(1);
}

static int MTEE_LogCtrl_Serivce(MTEE_SESSION_HANDLE handle, uint32_t cmd, 
                                uint32_t paramTypes, MTEE_PARAM param[4])
{
    switch (cmd)
    {
        case TZCMD_LOG_CTRL_SET_LVL:
            if( TZ_GetParamTypes(paramTypes, 0) == TZPT_VALUE_INPUT )
            {
                MTEE_SetLogLvl(param[0].value.a);
                return TZ_RESULT_SUCCESS;
            }
            else 
                return TZ_RESULT_ERROR_BAD_FORMAT;
        break;

        default:
            return TZ_RESULT_ERROR_GENERIC;
    }
}

static const struct MTEE_TA_Function mtee_log_ctrl_ta_func =
{
    .TAName       = "Log Ctrl TA",
    .UUID         = TZ_TA_LOG_CTRL_UUID,
    .service_func = MTEE_LogCtrl_Serivce,
};

TA_REGISTER(mtee_log_ctrl_ta_func);

