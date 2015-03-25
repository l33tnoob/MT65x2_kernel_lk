#include "GCC_Utility.h"
#if defined(__GNUC__)


#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

#include <unistd.h>
#include <stdlib.h>
#include <strings.h>
#if defined (__linux__)
#include <time.h>
#endif

#include <ctype.h>

#define _tcsrchr strrchr

DWORD GetCurrentProcessId()
{
    return (DWORD)getpid();;
}

DWORD GetCurrentThreadId()
{
    return 0;
}

void GetLocalTime(PSYSTEMTIME p_systime)
{
    struct timeval curTime;
    struct tm *p;

    gettimeofday(&curTime, NULL);
    p=localtime(&curTime.tv_sec);

    p_systime->wYear = 1900+p->tm_year;
    p_systime->wDayOfWeek = p->tm_wday;
    p_systime->wMonth = 1+p->tm_mon;
    p_systime->wDay = p->tm_mday;
    p_systime->wHour = p->tm_hour;
    p_systime->wMinute = p->tm_min;
    p_systime->wSecond = p->tm_sec;
    p_systime->wMilliseconds = curTime.tv_usec / 1000;

}

/*void OutputDebugString(const char* c)
{
}*/

FILE *_fsopen (const char *fname, const char *mode, int shflag)
{
    //without setting share mode flag
    FILE *f;
    f = fopen(fname, mode);
    return f;
}

static char* xbuf(char* buf, const char* data, size_t size)
{
    if (buf && data && size > 0) {
        int i = 0;
        size_t len = size > 8 ? 8 : size;
        for (i = 0; i < len; i++) {
            sprintf(buf + i * 3, "%02x ", data[i]);
        }
        if (size > len) {
            sprintf(buf + i * 3, "...");
        }
    }
    return buf;
}

long long int _get_ms64(void)
{
    struct timespec tp = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return tp.tv_sec * 1000 + tp.tv_nsec / (1000*1000);
}

extern int PSIZ;
bool WriteFile(HANDLE hFile, LPCVOID buffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, void* dummyforAPI)
{
    int written_len = 0;
    //usleep(3000);
    fd_set writefd;
    //int state;

    struct timeval timeout;
   // timeout.tv_sec = 0;
   // timeout.tv_usec = 500000;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    bool is_written = false;

    LPCVOID obuf = buffer;
    DWORD onum = nNumberOfBytesToWrite;

    //LOG("%d, %p, %d, %p\n", hFile, buffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten);
    while(!is_written)
    {
        FD_ZERO(&writefd);
        FD_SET(hFile, &writefd);
        if( select(hFile+1, NULL , &writefd, NULL, &timeout) > 0)
        {
            //int ifd;
            //for(ifd=0; ifd < hFile+1; ifd++)
            {
                if (FD_ISSET(hFile, &writefd))
                {
                    //if(ifd == hFile)
                    {
/*#define PSIZ 64*/
                        //printf("To  ttyACM0 write data....\n");
                        ssize_t wlen = write(hFile, buffer, (size_t)nNumberOfBytesToWrite > PSIZ ? PSIZ : (size_t)nNumberOfBytesToWrite);
                        /*printf("%d written\n", written_len);
                        {
                            int i = 0;
                            while(i < written_len)
                            {
                                printf("%x ",((const char*)buffer)[i++]);
                            }
                            printf("\n");
                        }*/
                        if (wlen > 0) {
                            nNumberOfBytesToWrite -= wlen;
                            buffer += wlen;
                            written_len += wlen;
                        }
                        if (nNumberOfBytesToWrite == 0 || wlen <= 0) {
                            is_written = true;
                            char buf[BUFSIZ] = "";
                            LOG("fd %d W %d/%u >>> %s\n", hFile, written_len, onum, xbuf(buf, obuf, onum));
                        }
                    }
                }
            }
       }
        else
       {
            printf("write timeout\n");
            break;
       }
    }

    if(written_len >= 0)
    {
        if(lpNumberOfBytesWritten != NULL)
        {
            *lpNumberOfBytesWritten = written_len;
        }
        return true;
    }
    else
    {
        if(lpNumberOfBytesWritten != NULL)
        {
            *lpNumberOfBytesWritten = 0;
        }
        return false;
    }
}

bool ReadFile(HANDLE hFile, LPVOID buffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, void* dummyforAPI)
{
    int read_len = 0;

    //usleep(3000);
    fd_set    readfd;
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    bool is_read = false;
    while(!is_read)
    {
        FD_ZERO(&readfd);
        FD_SET(hFile, &readfd);
        if( select(hFile+1, &readfd , NULL, NULL, &timeout) > 0)
        {
            //int ifd;
            //for(ifd=0; ifd < hFile+1; ifd++)
            {
                if (FD_ISSET(hFile, &readfd))
                {
                    //if(ifd == hFile)
                    {
                        //printf("Read from ttyACM0...\n");
                        read_len = read(hFile, buffer, nNumberOfBytesToRead);
                        char buf[BUFSIZ] = "";
                        LOG("fd %d R %d/%u  <<< %s\n", hFile, read_len, nNumberOfBytesToRead, xbuf(buf, buffer, read_len));
                        is_read = true;
                        /*{
                            int i = 0;
                            while(i < read_len)
                            {
                                printf("%x ",((const char*)buffer)[i++]);
                            }
                            printf("\n");
                        }*/
                    }
                }
            }
        }
        else
        {
            printf("read timeout 5s? %p\n", __builtin_return_address(0));
            break;
        }
    }

    if(read_len >= 0)
    {
        if(lpNumberOfBytesRead != NULL)
        {
            *lpNumberOfBytesRead = read_len;
        }
        return true;
    }
    else
    {
        if(lpNumberOfBytesRead != NULL)
        {
            *lpNumberOfBytesRead = 0;
        }
        return false;
    }
}

bool ChangeBaudRate(HANDLE hCOM, DWORD  baudrate) {

    struct termios newtio;
    struct termios oldtio;
    speed_t baudrate_t = B9600;

    tcgetattr(hCOM, &oldtio);
    bzero(&newtio, sizeof(newtio));
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
    default:
        break;
    }


    newtio.c_cflag = baudrate_t|CS8|CLOCAL|CREAD;
    //newtio.c_cflag |= baudrate;
    newtio.c_oflag = oldtio.c_oflag;
    newtio.c_iflag = oldtio.c_oflag;
    newtio.c_lflag = oldtio.c_oflag;
    //[TODO] set timeout by default
    newtio.c_cc[VTIME]=5;
    newtio.c_cc[VMIN]=0;

    tcflush(hCOM, TCIFLUSH);
    tcsetattr(hCOM, TCSANOW, &newtio);

    return true;
}

ULONG GetTickCount()
{
    ULONG currentTime;
#if defined(_MSC_VER)
    currentTime = ::GetTickCount();

#elif defined(__GNUC__)
    struct timeval current;
    gettimeofday(&current, NULL);
    currentTime = current.tv_sec * 1000 + current.tv_usec/1000;

#endif

    return currentTime;
}
ULONG GetTickCount2()
{
    ULONG currentTime;
#if defined(_MSC_VER)
    currentTime = ::GetTickCount();

#elif defined(__GNUC__)
    struct timeval current;
    gettimeofday(&current, NULL);
    currentTime = current.tv_sec * 1000000 + current.tv_usec;

#endif

    return currentTime;
}

char *_strupr( char *str )
{
    int str_size = strlen(str);
    int i = 0;
    while (i < str_size)
    {
        str[i] = (char)toupper(str[i]);
        i++;
    }
    return str;
}

#endif

