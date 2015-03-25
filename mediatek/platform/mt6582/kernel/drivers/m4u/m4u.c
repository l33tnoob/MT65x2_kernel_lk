#include <linux/spinlock.h>
#include <linux/xlog.h>
#include <asm/cacheflush.h>
#include <linux/printk.h>

#include <mach/m4u.h>
#include "m4u_reg.h"
#include <linux/m4u_profile.h>
#include "m4u_priv.h"


int m4u_dump_reg(int m4u_index) 
{
      return 0;
}

int m4u_power_on(int m4u_index)
{
    return 0;
}

int m4u_power_off(int m4u_index)
{
    return 0;
}

// query mva by va
int m4u_query_mva(M4U_MODULE_ID_ENUM eModuleID, 
								  const unsigned int BufAddr, 
								  const unsigned int BufSize, 
								  unsigned int *pRetMVABuf,
								  struct file * a_pstFile) 
{
    return __m4u_query_mva(eModuleID, BufAddr, BufSize, pRetMVABuf, a_pstFile);
}



int m4u_alloc_mva(M4U_MODULE_ID_ENUM eModuleID, 
								  const unsigned int BufAddr, 
								  const unsigned int BufSize, 
								  int security,
								  int cache_coherent,
								  unsigned int *pRetMVABuf)
{
    mva_info_t *pMvaInfo = NULL;
    int ret;
    pMvaInfo=m4u_alloc_garbage_list(0,BufSize,eModuleID,BufAddr,0,security,cache_coherent);
    ret = __m4u_alloc_mva(pMvaInfo, NULL);

    if(ret == 0)
    {
        *pRetMVABuf = pMvaInfo->mvaStart;
    }
    else
    {
        *pRetMVABuf = 0;
    }
    return ret;
}


int m4u_alloc_mva_sg(M4U_MODULE_ID_ENUM eModuleID, 
								  struct sg_table *sg_table, 
								  const unsigned int BufSize, 
								  int security,
								  int cache_coherent,
								  unsigned int *pRetMVABuf)
{
    mva_info_t *pMvaInfo = NULL;
    int ret;
    pMvaInfo=m4u_alloc_garbage_list(0,BufSize,eModuleID,0,0,security,cache_coherent);
    ret = __m4u_alloc_mva(pMvaInfo, sg_table);

    if(ret == 0)
    {
        *pRetMVABuf = pMvaInfo->mvaStart;
    }
    else
    {
        *pRetMVABuf = 0;
    }
    return ret;
}


int m4u_dealloc_mva(M4U_MODULE_ID_ENUM eModuleID, 
									const unsigned int BufAddr, 
									const unsigned int BufSize, 
									const unsigned int MVA) 
{
    return __m4u_dealloc_mva(eModuleID, BufAddr, BufSize, MVA, NULL);
}

int m4u_dealloc_mva_sg(M4U_MODULE_ID_ENUM eModuleID, 
									struct sg_table* sg_table,
									const unsigned int BufSize, 
									const unsigned int MVA) 
{									
    return __m4u_dealloc_mva(eModuleID, 0, BufSize, MVA, sg_table);
}

int m4u_insert_seq_range(M4U_MODULE_ID_ENUM eModuleID, 
                             unsigned int MVAStart, 
                             unsigned int MVAEnd, 
                             M4U_RANGE_PRIORITY_ENUM ePriority,
                             unsigned int entryCount) //0:disable multi-entry, 1,2,4,8,16: enable multi-entry
{

    int ret;
    
    MMProfileLogEx(M4U_MMP_Events[PROFILE_INSERT_TLB], MMProfileFlagStart, eModuleID, MVAStart);
    ret = m4u_do_insert_seq_range(eModuleID, MVAStart, MVAEnd, entryCount);
    MMProfileLogEx(M4U_MMP_Events[PROFILE_INSERT_TLB], MMProfileFlagEnd, eModuleID, MVAEnd-MVAStart+1);
    
    return ret;
    
}


int m4u_invalid_seq_range(M4U_MODULE_ID_ENUM eModuleID, unsigned int MVAStart, unsigned int MVAEnd)
{
    m4u_invalid_seq_range_by_mva(m4u_module_2_m4u_id(eModuleID), MVAStart, MVAEnd);
    return 0;

}

int m4u_insert_wrapped_range(M4U_MODULE_ID_ENUM eModuleID, 
                             M4U_PORT_ID_ENUM portID, 
                             unsigned int MVAStart, 
                             unsigned int MVAEnd)
{
    return 0;
}

int m4u_invalid_wrapped_range(M4U_MODULE_ID_ENUM eModuleID, 
                              M4U_PORT_ID_ENUM portID,
                              unsigned int MVAStart, 
                              unsigned int MVAEnd)
{
    return 0;
}



int m4u_config_port(M4U_PORT_STRUCT* pM4uPort) //native
{
    return m4u_do_config_port(pM4uPort);
}


int m4u_config_port_rotator(M4U_PORT_STRUCT_ROTATOR *pM4uPort)
{ 

    return 0;
}


int m4u_monitor_start(int m4u_id)
{
    return m4u_do_monitor_start(m4u_id);
}

int m4u_monitor_stop(int m4u_id)
{
    return m4u_do_monitor_stop(m4u_id);
}


int m4u_dma_cache_maint(M4U_MODULE_ID_ENUM eModuleID, const void *start, size_t size, int direction)
{
    return m4u_do_dma_cache_maint(eModuleID, start, size, direction);
}



extern void  smp_inner_dcache_flush_all(void);
int m4u_dma_cache_flush_all()
{

   // M4UMSG("cache flush all!!\n")
    //mutex_lock(&gM4uMutex);

    // L1 cache clean before hw read
    smp_inner_dcache_flush_all();
     
	// L2 cache maintenance by physical pages
    outer_flush_all();
    
    //mutex_unlock(&gM4uMutex);
   
    return 0;
}


int m4u_dump_info(int m4u_index) 
{
    return m4u_do_dump_info(m4u_index);
}

int m4u_log_on(void)
{
    return m4u_do_log_on();
}

int m4u_log_off(void)
{
    return m4u_do_log_off();
}


int m4u_mau_check_pagetable(unsigned int start_addr, unsigned int end_addr)
{
    return 0;	
}

int m4u_mva_map_kernel(unsigned int mva, unsigned int size, int sec,
                        unsigned int* map_va, unsigned int* map_size)
{
    return m4u_do_mva_map_kernel(mva, size, sec, map_va, map_size);
}


int m4u_mva_unmap_kernel(unsigned int mva, unsigned int size, unsigned int va)
{
    return m4u_do_mva_unmap_kernel(mva, size, va);
}

int m4u_debug_command(unsigned int command)
{
    return m4u_do_debug_command(command);
}

