#include <stdio.h>
#include <cutils/log.h>
#include <cutils/pmem.h>
#include <cutils/ashmem.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cutils/pmem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "m4u_lib.h"

#include "rgb565_256x256.h"
#include "disp_config_api.h"
#include "DpEngineGroup.h"
#include "DpEngine_datatype.h"

#define SRC_W 256
#define SRC_H 256
#define SRC_FORMAT eFMT_RGB565
#define SRC_BPP 2
#define DST_W 256             
#define DST_H 256             
#define DST_FORMAT eFMT_RGB565
#define DST_BPP 2



int start_engine()
{
    return 0;
}




#define __USE_M4U__
int cache_coherent = 1;
int main()
{
    printf("enter m4u test main()! \n");	
    
    // 1. alloc memory
    unsigned char *pSrc = NULL;
    unsigned char *pDst = NULL;

    unsigned int srcsize = SRC_W*SRC_H*SRC_BPP;
    unsigned int dstsize = DST_W*DST_H*DST_BPP;
#ifdef __USE_M4U__
    pSrc = new unsigned char[SRC_W*SRC_H*SRC_BPP];
    pDst = new unsigned char[DST_W*DST_H*DST_BPP];
    unsigned int srcPa = 0;
    unsigned int dstPa = 0;
    // alloc MVA ...
    MTKM4UDrv CM4u;

    CM4u.m4u_enable_m4u_func(M4U_CLNTMOD_ROT);
    CM4u.m4u_enable_m4u_func(M4U_CLNTMOD_WDMA);


    ///> --------------------4. config LCD port                       
    // config port 
    M4U_PORT_STRUCT M4uPort;
    M4uPort.ePortID = M4U_PORT_ROT_EXT;
    M4uPort.Virtuality = 1;						   
    M4uPort.Security = 0;
    M4uPort.domain = 0;
    M4uPort.Distance = 1;
    M4uPort.Direction = 0;
    CM4u.m4u_config_port(&M4uPort);
    

    M4uPort.ePortID = M4U_PORT_WDMA0;
    M4uPort.Virtuality = 1;						   
    M4uPort.Security = 0;
    M4uPort.domain = 0;
    M4uPort.Distance = 1;
    M4uPort.Direction = 0;
    CM4u.m4u_config_port(&M4uPort);

    
    // allocate MVA buffer for m4u module                    
    CM4u.m4u_alloc_mva(M4U_CLNTMOD_ROT,     // Module ID
                       (unsigned int)pSrc,              // buffer virtual start address
                       srcsize,              // buffer size
                       0,
                       cache_coherent,
                       &srcPa);             // return MVA address


    CM4u.m4u_dump_info(M4U_CLNTMOD_ROT);

    CM4u.m4u_insert_tlb_range(M4U_CLNTMOD_ROT, 
            srcPa, srcPa+srcsize-1, SEQ_RANGE_LOW_PRIORITY, 1);

    
    
    CM4u.m4u_alloc_mva(M4U_CLNTMOD_WDMA,     // Module ID
                       (unsigned int)pDst,              // buffer virtual start address
                       dstsize,              // buffer size
                       0,
                       cache_coherent,
                       &dstPa);             // return MVA address

    CM4u.m4u_dump_info(M4U_CLNTMOD_WDMA);
    
    CM4u.m4u_insert_tlb_range(M4U_CLNTMOD_WDMA, 
            dstPa, dstPa+dstsize-1, SEQ_RANGE_LOW_PRIORITY, 1);



    //srcPa = 0x5000;
    //dstPa = 0x2000;
    printf("psrc=0x%x, srcPa=0x%x, pDst=0x%x, disPa=0x%x\n", pSrc, srcPa, pDst, dstPa);

    
#else
    unsigned int srcPa = 0;
    unsigned int dstPa = 0;
    int src_ID, dst_ID;     
    pSrc = (unsigned char *) pmem_alloc(SRC_W*SRC_H*SRC_BPP , &src_ID);
    srcPa = (unsigned int)pmem_get_phys(src_ID);
    pDst = (unsigned char *) pmem_alloc(DST_W*DST_H*DST_BPP , &dst_ID);
    dstPa = (unsigned int)pmem_get_phys(dst_ID);

    printf("psrc=0x%x, srcPa=0x%x, pDst=0x%x, disPa=0x%x\n", pSrc, srcPa, pDst, dstPa);
#endif    

    CM4u.m4u_monitor_start(M4U_PORT_ROT_EXT);

    if(pSrc==NULL || pDst==NULL)
    {
    	  printf("error: alloc mem fail! \n");
    	  return 0;
    }
    memcpy(pSrc, rgb565_256x256, SRC_W*SRC_H*SRC_BPP);
    memset(pDst, 0, DST_W*DST_H*DST_BPP);

    printf("copy done\n");

    int m4u_fd = open("/dev/M4U_device", O_RDWR);
    unsigned int regbase;
    regbase = (unsigned int)mmap((void*)0x50000000, 0x14000, (PROT_READ | PROT_WRITE | PROT_EXEC), MAP_SHARED, m4u_fd, 0x14000000);


    if(regbase != 0x50000000)
    {
        printf("error to mmap register!! 0x%x\n", regbase);
        printf("\n");
        //while(1);
    }
    
    printf("mapped regbase = 0x%x, fd=%d\n", regbase, m4u_fd);

    #ifdef __USE_M4U__

    if(!cache_coherent)
    {
        //CM4u.m4u_cache_flush_all(M4U_CLNTMOD_ROT);
        CM4u.m4u_cache_sync(M4U_CLNTMOD_ROT, M4U_CACHE_FLUSH_BEFORE_HW_READ_MEM,
                (unsigned int)pSrc, srcsize);
        CM4u.m4u_cache_sync(M4U_CLNTMOD_WDMA, M4U_CACHE_FLUSH_BEFORE_HW_WRITE_MEM,
                (unsigned int)pDst, dstsize);
    }
    
    #endif
    
    disp_subsys_reg_base_init(regbase);
    printf("-1\n");
    // config path
    int module_list[] = {DISP_ROT, DISP_SCL, DISP_WDMA0};    
    // get mutex
    disp_mutex_lock(0, module_list, 3, SINGLE_MODE);
    printf("0\n");
    // set path
    disp_set_path(module_list, 3);    

    printf("1\n");
    DpConfiguration dc;
    DpCommandRecorder cr;

    dc.w = SRC_W;
    dc.h = SRC_H;
    dc.scalingW = DST_W;
    dc.scalingH = DST_H;
    dc.roi.x = 0;
    dc.roi.y = 0;
    dc.roi.w = SRC_W;
    dc.roi.h = SRC_H;
    dc.enDither = false;
    dc.enTTD = false;
    dc.enBLS = false;
    dc.addr[0] = srcPa;
    //dc.addr[1] = 0;
    //dc.addr[2] = 0;
    dc.inFormat = SRC_FORMAT;
    dc.outFormat = eFMT_YUV_444_1P;//DST_FORMAT;    
    dc.enSrcPitch = false;
    dc.srcPitch = 0;
    dc.enDstPitch = false;
    dc.dstPitch = 0;
    dc.rot = ROT_DEGREE_0;           
    dc.flip = ROT_FLIP_DISABLE;          
    dc.interlaced = false;    
    dc.fieldID = 0;       
     printf("2\n");
   
    // 2. define DDP engines
    //DpEngine *pRot = DpEngine::Factory(tROT);
    DpEngine_ROT *pRot = new DpEngine_ROT;

    DpEngine_SCL *pScl = new DpEngine_SCL;
    DpEngine_WDMA *pWdma0 = new DpEngine_WDMA(0);
        printf("3\n");

    // 3. config DDP engines
    DpCommandRecorder cmdRecord;
    
    pRot->onReset(cmdRecord);	
    pRot->onConfig(dc,cmdRecord);
    pRot->onEnable(cmdRecord);

    dc.inFormat = eFMT_YUV_444_1P;
    dc.outFormat = eFMT_YUV_444_1P;//DST_FORMAT;    

    pScl->onReset(cmdRecord);	
    pScl->onConfig(dc,cmdRecord);
    pScl->onEnable(cmdRecord);
    printf("4\n");

    dc.addr[0] = dstPa;


    dc.inFormat = eFMT_YUV_444_1P;
    dc.outFormat = DST_FORMAT;//DST_FORMAT;    
    pWdma0->onReset(cmdRecord);	
    pWdma0->onConfig(dc,cmdRecord);
    pWdma0->onEnable(cmdRecord);

    printf("5\n");
    disp_mutex_unlock(0);
    pWdma0->onWaitDone();

    printf("engine finished\n");

#ifdef __USE_M4U__
    CM4u.m4u_monitor_stop(M4U_PORT_ROT_EXT);

    CM4u.m4u_invalid_tlb_range(M4U_CLNTMOD_WDMA, 
            dstPa, dstPa+dstsize-1);
    CM4u.m4u_invalid_tlb_range(M4U_CLNTMOD_ROT, 
            srcPa, srcPa+srcsize-1);

    CM4u.m4u_dealloc_mva(M4U_CLNTMOD_ROT, (unsigned int)pSrc, (unsigned int)srcsize, srcPa);
    CM4u.m4u_dealloc_mva(M4U_CLNTMOD_WDMA, (unsigned int)pDst, (unsigned int)dstsize, dstPa);
    
#else
    pmem_free(pSrc, srcsize, src_ID);
    pmem_free(pDst, dstsize, dst_ID);
    
#endif

    
}
