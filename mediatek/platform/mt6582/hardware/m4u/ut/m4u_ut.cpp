/* 
 * data path: virtual memory -> m4u -> LCDC_R
 * LCD_R read BufAddr through M4U, then LCD_W write the data to PMEM PA 
 * test APP dump PMEM_VA image to verify
 */

#include "stdio.h"
#include "errno.h"
#include "fcntl.h"
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <cutils/log.h>
#include "m4u_lib.h"

#undef LOG_TAG
#define LOG_TAG "[m4u_ut]"

#define MTKM4UDEBUG
#ifdef MTKM4UDEBUG
  #define M4UDBG(string, args...) printf("[M4U_N]"string,##args)
#else
  #define M4UDBG(string, args...)
#endif

#define TEST_START() M4UDBG("--------------test start: %s ---------------- \n", __FUNCTION__)
#define TEST_END() M4UDBG("--------------test end: %s ---------------- \n", __FUNCTION__)



//#define M4U_MEM_USE_NEW
#define M4U_MEM_USE_PMEM



int vAllocate_Deallocate_basic(int BufSize)
{
    int i, ret;
    unsigned char* BufAddr;
    unsigned int BufMVA;
    MTKM4UDrv CM4u;

    printf("m4u basic test size=0x%x\n", BufSize);

    BufAddr = (unsigned char *)malloc(BufSize);
    if(BufAddr == NULL)
    {
        printf("error to allocate buffer size=0x%x\n", BufSize);
        return -1;
    }

    for(i=0; i<BufSize; i++)
        BufAddr[i] = 0x55;

    CM4u.m4u_enable_m4u_func(DISP_OVL_0);

    ret = CM4u.m4u_alloc_mva(DISP_OVL_0,     
                       (unsigned int)BufAddr, BufSize,             
                       0,
                       0,
                       &BufMVA);             
    if(ret)
    {
        printf("allocate mva fail. ret=0x%x\n", ret);
        return ret;
    }

    ret = CM4u.m4u_cache_sync(DISP_OVL_0, M4U_CACHE_CLEAN_BEFORE_HW_READ_MEM, 
                    (unsigned int)BufAddr, BufSize);
    if(ret)
    {
        printf("cache clean fail. ret=%d,va=0x%x,size=0x%x\n", ret,BufAddr,BufSize);
        return ret;
    }

    ret = CM4u.m4u_cache_sync(DISP_OVL_0, M4U_CACHE_FLUSH_BEFORE_HW_WRITE_MEM, 
                    (unsigned int)BufAddr, BufSize);
    if(ret)
    {
        printf("cache flush fail. ret=%d,va=0x%x,size=0x%x\n", ret,BufAddr,BufSize);
        return ret;
    }
    ret = CM4u.m4u_cache_sync(DISP_OVL_0, M4U_CACHE_INVALID_AFTER_HW_WRITE_MEM, 
                    (unsigned int)BufAddr, BufSize);
    if(ret)
    {
        printf("cache invalid fail. ret=%d,va=0x%x,size=0x%x\n", ret,BufAddr,BufSize);
        return ret;
    }

    /*== OVL0 use mva here ==*/

    ret = CM4u.m4u_dealloc_mva(DISP_OVL_0, (unsigned int)BufAddr, BufSize, BufMVA);
    if(ret)
    {
        printf("m4u_dealloc_mva fail. ret=%d, mva=0x%x\n", ret, BufMVA);
        return -1;
    }

    ret = 0;
    for(i=0; i<BufSize; i++)
    {
        char data = BufAddr[i];
        if(data != 0x55)
        {
            printf("check data fail!!, data=0x%x, i=%d\n", data, i);
            ret = -1;
        }
    }

    free(BufAddr);
        
    return ret;
}



int main (int argc, char *argv[])
{
    int i, j = 0;
    int ret = 0;
    while(1)
    {
        j++;
        ret |= vAllocate_Deallocate_basic(1024);
        ret |= vAllocate_Deallocate_basic(4096);
        ret |= vAllocate_Deallocate_basic(4096*20);
        ret |= vAllocate_Deallocate_basic(4096*100);
        for(i=1; i<80; i+=1)
        {
            ret |= vAllocate_Deallocate_basic(1024*1024*i);
        }

        if(ret)
            break;
    }

    return 0;
}


