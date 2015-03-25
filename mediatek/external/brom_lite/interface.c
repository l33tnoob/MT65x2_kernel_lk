/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005
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
/*******************************************************************************
* Filename:
* ---------
*  interface.c
*
* Project:
* --------
*   Standalone Flash downloader sample code
*
* Description:
* ------------
*   This module contains port interface
*
* Author:
* -------
*  Kevin Lim (mtk60022)
*
*==============================================================================
*           HISTORY
* Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
*------------------------------------------------------------------------------
* $Revision$
* $Modtime$
* $Log$
*
* 05 03 2013 monster.huang
* [STP100006902]  [FlashTool] v5.1312/v5.1314 maintenance
* [BROM Lite]
* Support SF/SPI NAND download flow via compiler option.
* Support USB/UART download.
* Support arbitrary number of download items.
* Receive data with retry 1000 times per 1ms.
*
* 04 12 2013 monster.huang
* [STP100006902]  [FlashTool] v5.1312/v5.1314 maintenance
* [BROM LITE] Support combo memory and automatic generate eppParam.
* Fetch bmt address by acquiring linux partitions.
 *
 * 10 01 2010 kevin.lim
 * [STP100004187]  Upload the BROM-lite source code on P4 server.
 * 10 01 2010 Kevin Lim
 * [STP100004187] BROM Lite v1.1037.2 release
*
*
*------------------------------------------------------------------------------
* Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
*==============================================================================
*******************************************************************************/

#include <stdarg.h>
#include "interface.h"
#if defined(__GNUC__)
#include "GCC_Utility.h"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#endif


#if defined(__linux__)
static struct termios gOriginalTTYAttrs;
#endif

//added by xiaohui 4-23
//char dev_path[64],*pipe_buf;
//int pipe_fd;

void gsm_reset()
{
    //log_output("[gsm] Resetting GSM...\n");
}

#define EXT_MD_IOC_MAGIC        'E'
#define EXT_MD_IOCTL_R8_DOWNLOAD    _IO(EXT_MD_IOC_MAGIC, 106)
#define EXT_MD_MONITOR_DEV "/dev/ext_md_ctl0"

static int _r8_download_init(void)
{
    int rc = -1;
    int fd = -1;
    
    if ((fd = open(EXT_MD_MONITOR_DEV, O_RDWR)) < 0) {
        log_output("[r8down] open %s failed: %d, %s\n", EXT_MD_MONITOR_DEV, errno, strerror(errno));
        return -1;
    }

    if ((rc = ioctl(fd, EXT_MD_IOCTL_R8_DOWNLOAD, NULL)) != 0)
        log_output("[r8down] ioctl %s failed: %d, %s\n", EXT_MD_MONITOR_DEV, errno, strerror(errno));

    close(fd);
    return rc;
}

COM_STATUS com_open(COM_HANDLE *com_handle, unsigned int baudrate)
{
    log_output("[com] Opening the communication port: baudrate=%u\n", baudrate);

#if defined(__linux__)
    {
        char dev_path[64] = "/dev/ttyACM0";
        struct termios tty;
        log_output("[com] Opening the communication device path: %s\n", dev_path);

        if (access(dev_path, R_OK) != 0) {
            log_output("[com] create dev %s, please wait ...\n", dev_path);
            if (_r8_download_init() != 0)
                return COM_STATUS_ERROR;
            Sleep(2500);
        }

        *com_handle = open(dev_path, O_RDWR | O_NOCTTY | O_NONBLOCK | O_SYNC );
        if(*com_handle == -1)
        {
            return COM_STATUS_ERROR;
        }

        tcgetattr(*com_handle,&tty);
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
        tty.c_iflag &= ~IGNBRK;
        tty.c_lflag = 0;
        tty.c_oflag = 0;
        tty.c_cc[VMIN]  = 0;
        tty.c_cc[VTIME] = 5;
        tty.c_iflag &= ~(IXON | IXOFF | IXANY);
        tty.c_cflag |= (CLOCAL | CREAD);
        tty.c_cflag &= ~(PARENB | PARODD);
        tty.c_cflag |= 0; // No parity
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;
        tcsetattr (*com_handle, TCSANOW, &tty);
        com_change_baudrate(*com_handle, baudrate);
    }
#elif defined(_MSC_VER)

    {
         //Comport Setting
        const unsigned int COM_PORT_NUM = 3;//30;//39;
        char str[16];
        log_output("[com] Opening the communication port number: %d\n", COM_PORT_NUM);

        //assert(com_handle != NULL);

        sprintf(str, "\\\\.\\COM%u", COM_PORT_NUM);
        *com_handle = CreateFile(str, GENERIC_READ | GENERIC_WRITE,
                                 0, NULL, OPEN_EXISTING, 0, NULL);

        if (*com_handle == INVALID_COM_HANDLE)
        {
            return COM_STATUS_ERROR;
        }
    }

    {
        DWORD com_error = 0;
        COMSTAT com_state = { 0 };

        if (!ClearCommError(*com_handle, &com_error, &com_state))
        {
            return COM_STATUS_ERROR;
        }

        if (!SetupComm(*com_handle, 8192, 8192))
        {
            return COM_STATUS_ERROR;
        }
    }

    {
        DCB dcb = { 0 };

        if (!GetCommState(*com_handle, &dcb))
        {
            return COM_STATUS_ERROR;
        }

        dcb.BaudRate = baudrate;
        dcb.fAbortOnError = FALSE;

        // Set 8/N/1
        dcb.ByteSize = 8;
        dcb.fParity = FALSE;
        dcb.Parity = NOPARITY;
        dcb.StopBits = ONESTOPBIT;

        // Disable H/W flow control
        dcb.fDtrControl = DTR_CONTROL_ENABLE;
        dcb.fRtsControl = RTS_CONTROL_ENABLE;
        dcb.fOutxCtsFlow = FALSE;
        dcb.fOutxDsrFlow = FALSE;
        dcb.fDsrSensitivity = FALSE;

        // Disable S/W flow control
        dcb.fOutX = FALSE;
        dcb.fInX = FALSE;

        if (!SetCommState(*com_handle, &dcb))
        {
            return COM_STATUS_ERROR;
        }
    }

    {
        if (!ClearCommBreak(*com_handle))
        {
            return COM_STATUS_ERROR;
        }

        if (!PurgeComm(*com_handle, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR))
        {
            return COM_STATUS_ERROR;
        }

    }
    {
        if (com_change_timeout(*com_handle, 20, 5000) != COM_STATUS_DONE)
        {
            return COM_STATUS_ERROR;
        }
    }
#endif

    return COM_STATUS_DONE;
}


COM_STATUS com_close(COM_HANDLE *com_handle)
{
    //assert(com_handle != NULL);

    log_output("[com] Closing the communication port\n");
#if defined(__linux__)
    if ( *com_handle < 0 )
    {
        *com_handle = 0;
    }
    else
    {
        tcsetattr(*com_handle, TCSANOW, &gOriginalTTYAttrs);
        close(*com_handle);
    }
#elif defined(_MSC_VER)
    if (*com_handle != INVALID_COM_HANDLE)
    {
        CloseHandle(*com_handle);
        *com_handle = INVALID_COM_HANDLE;
    }
#endif
    return COM_STATUS_DONE;
}


COM_STATUS com_change_timeout(COM_HANDLE com_handle,
                                                            unsigned int read_timeout_in_ms,
                                                            unsigned int write_timeout_in_ms)
{

#if defined(__linux__)
    log_output("[com] Unsupported function on Linux platform\n");
    return COM_STATUS_ERROR;
#elif defined(_MSC_VER)
    COMMTIMEOUTS timeout = { 0 };

    timeout.ReadIntervalTimeout         = 0;
    timeout.ReadTotalTimeoutMultiplier  = 1;
    timeout.ReadTotalTimeoutConstant    = read_timeout_in_ms;
    timeout.WriteTotalTimeoutMultiplier = 1;
    timeout.WriteTotalTimeoutConstant   = write_timeout_in_ms;

    if (!SetCommTimeouts(com_handle, &timeout))
    {
        return COM_STATUS_ERROR;
    }
    return COM_STATUS_DONE;
#endif

}


COM_STATUS com_enable_hardware_flow_control(COM_HANDLE com_handle)
{
#if defined(__linux__)
    log_output("[com] Unsupported function on Linux platform\n");
    return COM_STATUS_ERROR;
#elif defined(_MSC_VER)
        DCB dcb;

    log_output("[com] Enabling HW flow control on the communication port\n");

    if (!GetCommState(com_handle, &dcb))
    {
        return COM_STATUS_ERROR;
    }


    dcb.fDtrControl = DTR_CONTROL_DISABLE;
    dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
    dcb.fOutxCtsFlow = TRUE;
    dcb.fOutxDsrFlow = FALSE;
    dcb.fDsrSensitivity = FALSE;
    dcb.fTXContinueOnXoff = FALSE;

    if (!SetCommState(com_handle, &dcb))
    {
        return COM_STATUS_ERROR;
    }
    return COM_STATUS_DONE;
#endif

}


COM_STATUS com_change_baudrate(COM_HANDLE com_handle, unsigned int baudrate)
{
#if defined(__linux__)
    struct termios newtio;
    struct termios oldtio;
    speed_t baudrate_t = B9600;
    int ret = 0;
    log_output("[com] Changing baudrate of the communication port: "
               "baudrate=%u\n", baudrate);
    tcgetattr(com_handle, &gOriginalTTYAttrs);
    newtio = gOriginalTTYAttrs;
    switch(baudrate)
    {
    case 1800:
        baudrate_t = B1800;
        break;
    case 2400:
        baudrate_t = B2400;
        break;
    case 4800:
        baudrate_t = B4800;
        break;
    case 9600:
        baudrate_t = B9600;
        break;
    case 19200:
        baudrate_t = B19200;
        break;
    case 38400:
        baudrate_t = B38400;
        break;
    case 57600:
        baudrate_t = B57600;
        break;
    case 115200:
        baudrate_t = B115200;
        break;
    case 921600:
        baudrate_t = B921600;
        break;
    default:
        break;
    }

    cfmakeraw(&newtio);

    newtio.c_cc[VTIME] = 5;
    newtio.c_cc[VMIN] = 0;
    cfsetospeed (&newtio, baudrate_t);
    cfsetispeed (&newtio, baudrate_t);
    tcflush(com_handle, TCIFLUSH);

    ret = tcsetattr(com_handle, TCSANOW, &newtio);

    if(ret == -1)
        log_output("[com] Set tty attributes error\n");
    return COM_STATUS_DONE;
#elif defined(_MSC_VER)
    DCB dcb;

    log_output("[com] Changing baudrate of the communication port: "
               "baudrate=%u\n", baudrate);

    if (!GetCommState(com_handle, &dcb))
    {
        return COM_STATUS_ERROR;
    }

    dcb.BaudRate = baudrate;

    if (!SetCommState(com_handle, &dcb))
    {
        return COM_STATUS_ERROR;
    }
    return COM_STATUS_DONE;
#endif

}


COM_STATUS com_send_data(COM_HANDLE com_handle,
                         const unsigned char *data, unsigned int len)
{
    DWORD num_bytes_written = 0;

    if (!WriteFile(com_handle, data, len, &num_bytes_written, NULL))
    {
        printf("COM_STATUS_ERROR\n");
        return COM_STATUS_ERROR;
    }

    if (num_bytes_written == 0)
    {
        printf("num_bytes_written == 0\n");
        return COM_STATUS_WRITE_TIMEOUT;
    }

    if (num_bytes_written != len)
    {
        printf("num_bytes_written %d != len %d\n", num_bytes_written, len);
        return COM_STATUS_ERROR;
    }

#if defined(_MSC_VER)
    if (!FlushFileBuffers(com_handle))
    {
        return COM_STATUS_ERROR;
    }
#endif

    return COM_STATUS_DONE;
}


COM_STATUS com_send_byte(COM_HANDLE com_handle, unsigned char data)
{
    return com_send_data(com_handle, &data, 1);
}


COM_STATUS com_send_word(COM_HANDLE com_handle, unsigned short data)
{
    unsigned char tmp[2];

    tmp[0] = (data >> 8) & 0xFF;
    tmp[1] = data & 0xFF;

    return com_send_data(com_handle, tmp, 2);
}


COM_STATUS com_send_dword(COM_HANDLE com_handle, unsigned int data)
{
    unsigned char tmp[4];

    tmp[0] = (data >> 24) & 0xFF;
    tmp[1] = (data >> 16) & 0xFF;
    tmp[2] = (data >> 8) & 0xFF;
    tmp[3] = data & 0xFF;

    return com_send_data(com_handle, tmp, 4);
}


COM_STATUS com_recv_data(COM_HANDLE com_handle,
                         unsigned char *data, unsigned int len)
{
    DWORD num_bytes_read = 0;
    DWORD total_bytes_read = 0;
    int retry = 0;

    while(total_bytes_read<len)
    {
        if (!ReadFile((HANDLE)com_handle, data+total_bytes_read, len-total_bytes_read, &num_bytes_read, NULL))
        {
                return COM_STATUS_ERROR;
        }
        total_bytes_read += num_bytes_read;
        if (total_bytes_read == len)
        {
            break;
        }
        if (num_bytes_read > 0)
            retry = 0;
        else
            retry++;
        if (retry > 12)
            break;
        Sleep(1);
    }

    if (total_bytes_read == 0)
    {
        return COM_STATUS_READ_TIMEOUT;
    }

    if (total_bytes_read != len)
    {
        // if the len > 512, it maybe retry to get remain data
        return COM_STATUS_ERROR;
    }

    return COM_STATUS_DONE;
}

COM_STATUS com_recv_data_chk_len(COM_HANDLE com_handle,
                         unsigned char *data, unsigned int len)
{
    DWORD num_bytes_read = 0;
    DWORD total_bytes_read = 0;

    while(total_bytes_read<len)
    {
        if (!ReadFile((HANDLE)com_handle, data+total_bytes_read, len-total_bytes_read, &num_bytes_read, NULL))
        {
                return COM_STATUS_ERROR;
        }
        total_bytes_read += num_bytes_read;
    }

    if (total_bytes_read == 0)
    {
        return COM_STATUS_READ_TIMEOUT;
    }

    if (total_bytes_read != len)
    {
        return COM_STATUS_ERROR;
    }

    return COM_STATUS_DONE;
}




COM_STATUS com_recv_byte_without_retry(COM_HANDLE com_handle, unsigned char *data)

{
    DWORD num_bytes_read = 0;
    if (!ReadFile((HANDLE)com_handle, data, 1, &num_bytes_read, NULL))
    {
            return COM_STATUS_ERROR;
    }
    if (num_bytes_read == 0)
    {
        return COM_STATUS_READ_TIMEOUT;
    }

    if (num_bytes_read != 1)
    {
        return COM_STATUS_ERROR;
    }
    return COM_STATUS_DONE;
}

COM_STATUS com_recv_byte(COM_HANDLE com_handle, unsigned char *data)
{
    return com_recv_data(com_handle, data, 1);
}



COM_STATUS com_recv_word(COM_HANDLE com_handle, unsigned short *data)
{
    unsigned char tmp[2];
    const COM_STATUS com_status = com_recv_data(com_handle, tmp, 2);

    if (com_status != COM_STATUS_DONE)
    {
        return com_status;
    }

    ((unsigned char *) data)[0] = tmp[1];
    ((unsigned char *) data)[1] = tmp[0];

    return COM_STATUS_DONE;
}


COM_STATUS com_recv_dword(COM_HANDLE com_handle, unsigned int *data)
{
    unsigned char tmp[4];
    const COM_STATUS com_status = com_recv_data(com_handle, tmp, 4);

    if (com_status != COM_STATUS_DONE)
    {
        return com_status;
    }

    ((unsigned char *) data)[0] = tmp[3];
    ((unsigned char *) data)[1] = tmp[2];
    ((unsigned char *) data)[2] = tmp[1];
    ((unsigned char *) data)[3] = tmp[0];

    return COM_STATUS_DONE;
}


void log_output(const char *format, ...)
{
    char message[512];
    va_list va;

    va_start(va, format);
    _vsnprintf(message, 512, format, va);
    va_end(va);

    printf("%s", message);
}

//log_feedback()
//feed back update infomation to recovery
//added by xiaohui 4-22
void log_feedback(const char *format,...){
#if 0
    int len;
    va_list ap;
    char buffer[200];

    va_start(ap,format);
    //vsnprintf(pipe_buf,PIPE_BUF_SIZE,format,ap);
    vsnprintf(buffer, PIPE_BUF_SIZE, format,ap);
    va_end(ap);

    //len = strlen(pipe_buf);
    //write(pipe_fd,pipe_buf,len+1);
#endif
}

#if !defined(__GNUC__)
void sleep(unsigned int ms)
{
    Sleep(ms);
}
#endif

