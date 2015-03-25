/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cutils/log.h>
#include "m4u_lib.h"
#include <string.h>
#include <errno.h>
#include <assert.h>

#undef LOG_TAG
#define LOG_TAG "M4U_L"

#define M4UMSG(...) ALOGE(__VA_ARGS__)
#define M4UINFO(...) ALOGI(__VA_ARGS__)


#define __M4U_WRAP_LAYER_EN__

#ifdef __M4U_WRAP_LAYER_EN__
    bool m4u_en[M4U_CLNTMOD_MAX] = {false};
#endif



//#define __DUMP_BACKTRACE_ON_ERROR__
#ifdef __DUMP_BACKTRACE_ON_ERROR__
extern "C" void rtt_dump_caller_backtrace(const char* tag);
extern "C" int rtt_dump_backtrace(pid_t pid, pid_t tid, const char* file_path); 
static void m4u_dump_backtrace(void)
{
    char name[35] = "/data/m4u_dump";
    //sprintf(name, "/data/m4u_dump_%d_%d.txt", getpid(), gettid());
    M4UMSG("m4u dump back trace when error============> \n");
    rtt_dump_backtrace(getpid(), gettid(), name);

    FILE *fp = fopen(name, "r");
    if(fp)
    {
        char tmp[101] = "";
        int cnt=0;
        while( (fgets(tmp, 100, fp) != NULL) && (cnt<500) )
        {
            cnt++;
            M4UMSG("[%d]: %s", cnt, tmp);
        }
       fclose(fp);
    }
    //unlink(name);
}
#else
static void m4u_dump_backtrace(void)
{
    //assert(0);
}
#endif


/******************************************************************************
*
*******************************************************************************/
MTKM4UDrv::MTKM4UDrv(void)
{

    mFileDescriptor = -1;
 
    mFileDescriptor = open("/proc/M4U_device", O_RDONLY);
    if(mFileDescriptor<0)
    {
        M4UMSG("Open file failed mFileDescriptor=%d, error=%d :%s", mFileDescriptor, errno, strerror(errno));
        m4u_dump_backtrace();
    }
    else
    {
        if(ioctl(mFileDescriptor, MTK_M4U_T_M4UDrv_CONSTRUCT, NULL))
        {
            M4UMSG(" ioctl MTK_M4U_T_M4UDrv_CONSTRUCT fail! fd=%d, %d, %s \n", mFileDescriptor, errno, strerror(errno));
            m4u_dump_backtrace();
        }
        M4UINFO("MTKM4UDrv fd = %d, obj=0x%x\n", mFileDescriptor, (unsigned int)&mFileDescriptor);
    }

}

MTKM4UDrv::~MTKM4UDrv(void)
{    
    if(-1!=mFileDescriptor)
    {
        if(ioctl(mFileDescriptor, MTK_M4U_T_M4UDrv_DECONSTRUCT, NULL))
        {
            M4UMSG(" ioctl MTK_M4U_T_M4UDrv_DECONSTRUCT fail! fd=%d, %d, %s \n", mFileDescriptor, errno, strerror(errno));
            m4u_dump_backtrace();
        }
        M4UINFO("~MTKM4UDrv close m4u fd=%d, obj=0x%x\n", mFileDescriptor, (unsigned int)&mFileDescriptor);
    	close(mFileDescriptor);
    }
    else
    {
        M4UMSG("warning: ~MTKM4UDrv fd=%d, obj=0x%x\n", mFileDescriptor, (unsigned int)&mFileDescriptor);
    }
    mFileDescriptor = -1;
}


/**
 * @brief : 
 * @param 
 * @return 
 */
M4U_STATUS_ENUM MTKM4UDrv::m4u_power_on(M4U_MODULE_ID_ENUM eModuleID)
{

    if(!m4u_check_m4u_en(eModuleID))
    {
        return M4U_STATUS_OK;
    }
    
    if(mFileDescriptor<0 )
    {
        M4UMSG("m4u_power_on failed \n");
        m4u_dump_backtrace();
        return M4U_STATUS_INVALID_HANDLE;
    }

    if(ioctl(mFileDescriptor, MTK_M4U_T_POWER_ON, &eModuleID))
    {
        M4UMSG(" ioctl MTK_M4U_T_POWER_ON fail! fd=%d, %d, %s \n", mFileDescriptor, errno, strerror(errno));
        m4u_dump_backtrace();
        return M4U_STATUS_KERNEL_FAULT;
    }
    else
    {
        return M4U_STATUS_OK;
    }
}

/**
 * @brief : 
 * @param 
 * @return 
 */
M4U_STATUS_ENUM MTKM4UDrv::m4u_power_off(M4U_MODULE_ID_ENUM eModuleID)
{

    if(!m4u_check_m4u_en(eModuleID))
    {
        return M4U_STATUS_OK;
    }    
    
    if(mFileDescriptor<0 )
    {
        M4UMSG("m4u_power_off failed \n");
        m4u_dump_backtrace();
        return M4U_STATUS_INVALID_HANDLE;
    }

    if(ioctl(mFileDescriptor, MTK_M4U_T_POWER_OFF, &eModuleID))
    {
        M4UMSG(" ioctl MTK_M4U_T_POWER_OFF fail! fd=%d, %d, %s \n", mFileDescriptor, errno, strerror(errno));
        m4u_dump_backtrace();
        return M4U_STATUS_KERNEL_FAULT;
    }
    else
    {
        return M4U_STATUS_OK;
    }
}


M4U_STATUS_ENUM MTKM4UDrv::m4u_alloc_mva(M4U_MODULE_ID_ENUM eModuleID, 
								  const unsigned int BufAddr, 
								  const unsigned int BufSize,
								  int security,
								  int cache_coherent,
								  unsigned int *pRetMVAAddr)
{
    M4U_MOUDLE_STRUCT m4u_module;
    m4u_module.eModuleID = eModuleID;
    m4u_module.BufAddr = BufAddr;
    m4u_module.BufSize = BufSize;
    m4u_module.security = security;
    m4u_module.cache_coherent = cache_coherent;


    if(!m4u_check_m4u_en(eModuleID))
    {
        //just work around, tell kernel it's pmem, and use wrap layer!!
        m4u_module.MVAStart = -1;
        if(mFileDescriptor<0 )
        {
            M4UMSG("m4u_alloc_mva failed \n");
            m4u_dump_backtrace();
            return M4U_STATUS_INVALID_HANDLE;
        }
        if(ioctl(mFileDescriptor, MTK_M4U_T_ALLOC_MVA, &m4u_module))
        {
            M4UMSG(" ioctl MTK_M4U_T_ALLOC_MVA fail! fd=%d, %d, %s \n", mFileDescriptor, errno, strerror(errno));
            m4u_dump_backtrace();
            return M4U_STATUS_KERNEL_FAULT;
        }
        else
        {
            *pRetMVAAddr = m4u_module.MVAStart;
            return M4U_STATUS_OK;
        }

        return M4U_STATUS_OK;
    }
    else
    {
        m4u_module.MVAStart = 0;

        if(mFileDescriptor<0 )
        {
            M4UMSG("m4u_alloc_mva failed \n");
            m4u_dump_backtrace();
            return M4U_STATUS_INVALID_HANDLE;
        }

        //mlock this buffer
        if(mlock((void*)BufAddr, BufSize))
        {
            M4UMSG("mlock fail! va=0x%x, size=%d, err=%d, %s \n",
                BufAddr, BufSize, errno, strerror(errno));
            *pRetMVAAddr = 0;
            m4u_dump_backtrace();
            return M4U_STATUS_INVALID_PARAM;
        }
        
        if(ioctl(mFileDescriptor, MTK_M4U_T_ALLOC_MVA, &m4u_module))
        {
            M4UMSG(" ioctl MTK_M4U_T_ALLOC_MVA fail! fd=%d, %d, %s \n", mFileDescriptor, errno, strerror(errno));
            m4u_dump_backtrace();
            return M4U_STATUS_KERNEL_FAULT;
        }
        else
        {
            *pRetMVAAddr = m4u_module.MVAStart;
            return M4U_STATUS_OK;
        }
    }
}

 	        

M4U_STATUS_ENUM MTKM4UDrv::m4u_query_mva(M4U_MODULE_ID_ENUM eModuleID, 
								  const unsigned int BufAddr, 
								  const unsigned int BufSize, 
								  unsigned int *pRetMVAAddr)
{
    M4U_MOUDLE_STRUCT m4u_module;
    m4u_module.eModuleID = eModuleID;
    m4u_module.BufAddr = BufAddr;
    m4u_module.BufSize = BufSize;

    *pRetMVAAddr = 0;

    if(!m4u_check_m4u_en(eModuleID))
    {
        return M4U_STATUS_OK;
    }
 	        
    if(mFileDescriptor<0 )
    {
        M4UMSG("m4u_query_mva failed \n");
        return M4U_STATUS_INVALID_HANDLE;
    }
    if(ioctl(mFileDescriptor, MTK_M4U_T_QUERY_MVA, &m4u_module))
    {
        M4UMSG(" ioctl MTK_M4U_T_QUERY_MVA fail! fd=%d, %d, %s \n", mFileDescriptor, errno, strerror(errno));
        return M4U_STATUS_KERNEL_FAULT;
    }
    else
    {
        *pRetMVAAddr = m4u_module.MVAStart;
        return M4U_STATUS_OK;
    }
}

M4U_STATUS_ENUM MTKM4UDrv::m4u_dealloc_mva(M4U_MODULE_ID_ENUM eModuleID, 
									const unsigned int BufAddr, 
									const unsigned int BufSize,
                                    const unsigned int MVAStart)
{
    M4U_MOUDLE_STRUCT m4u_module;
    m4u_module.eModuleID = eModuleID;
    m4u_module.BufAddr = BufAddr;
    m4u_module.BufSize = BufSize;
    m4u_module.MVAStart = MVAStart;



    if(!m4u_check_m4u_en(eModuleID))
    {
        return M4U_STATUS_OK;
    }
	        
    if(mFileDescriptor<0 )
    {
        M4UMSG("m4u_dealloc_mva failed \n");
        m4u_dump_backtrace();
        return M4U_STATUS_INVALID_HANDLE;
    }
    if(ioctl(mFileDescriptor, MTK_M4U_T_DEALLOC_MVA, &m4u_module))
    {
        M4UMSG(" ioctl MTK_M4U_T_DEALLOC_MVA fail! fd=%d, %d, %s \n", mFileDescriptor, errno, strerror(errno));
        m4u_dump_backtrace();
        return M4U_STATUS_KERNEL_FAULT;
    }
    else
    {
        return M4U_STATUS_OK;
    }

}


M4U_STATUS_ENUM MTKM4UDrv::m4u_insert_wrapped_range(M4U_MODULE_ID_ENUM eModuleID, 
                                  M4U_PORT_ID_ENUM portID, 
								  const unsigned int MVAStart, 
								  const unsigned int MVAEnd)
{
    M4U_WRAP_DES_T m4u_wrap;
    m4u_wrap.eModuleID = eModuleID;
    m4u_wrap.ePortID = portID;
    m4u_wrap.MVAStart = MVAStart;
    m4u_wrap.MVAEnd = MVAEnd;
	      

    if(!m4u_check_m4u_en(eModuleID))
    {
        return M4U_STATUS_OK;
    }

    if(mFileDescriptor<0 )
    {
        M4UMSG("m4u_insert_wrapped_range failed \n");
        m4u_dump_backtrace();
        return M4U_STATUS_INVALID_HANDLE;
    }
    if(ioctl(mFileDescriptor, MTK_M4U_T_INSERT_TLB_RANGE, &m4u_wrap))
    {
        M4UMSG(" ioctl MTK_M4U_T_INSERT_TLB_RANGE fail! fd=%d, %d, %s \n", mFileDescriptor, errno, strerror(errno));
        m4u_dump_backtrace();
        return M4U_STATUS_KERNEL_FAULT;
    }
    else
    {
        return M4U_STATUS_OK;
    }	
}

M4U_STATUS_ENUM MTKM4UDrv::m4u_insert_tlb_range(M4U_MODULE_ID_ENUM eModuleID, 
										unsigned int MVAStart, 
										const unsigned int MVAEnd, 
										M4U_RANGE_PRIORITY_ENUM ePriority,
										unsigned int entryCount) 
{
    M4U_MOUDLE_STRUCT m4u_module;
    m4u_module.eModuleID = eModuleID;
    m4u_module.MVAStart = MVAStart;
    m4u_module.MVAEnd = MVAEnd;
    m4u_module.ePriority = ePriority;
    m4u_module.entryCount = entryCount;  


    if(!m4u_check_m4u_en(eModuleID))
    {
        return M4U_STATUS_OK;
    }
	      
    if(mFileDescriptor<0 )
    {
        M4UMSG("m4u_insert_tlb_range failed \n");
        m4u_dump_backtrace();
        return M4U_STATUS_INVALID_HANDLE;
    }
    if(ioctl(mFileDescriptor, MTK_M4U_T_INSERT_TLB_RANGE, &m4u_module))
    {
        M4UMSG(" ioctl MTK_M4U_T_INSERT_TLB_RANGE fail! fd=%d, %d, %s \n", mFileDescriptor, errno, strerror(errno));
        m4u_dump_backtrace();
        return M4U_STATUS_KERNEL_FAULT;
    }
    else
    {
        return M4U_STATUS_OK;
    }

}

M4U_STATUS_ENUM MTKM4UDrv::m4u_invalid_tlb_range(M4U_MODULE_ID_ENUM eModuleID,
										  unsigned int MVAStart, 
										  unsigned int MVAEnd)
{
    M4U_MOUDLE_STRUCT m4u_module;
    m4u_module.eModuleID = eModuleID;
    m4u_module.MVAStart = MVAStart;
    m4u_module.MVAEnd = MVAEnd;


    if(!m4u_check_m4u_en(eModuleID))
    {
        return M4U_STATUS_OK;
    }

    if(mFileDescriptor<0 )
    {
        M4UMSG("m4u_invalid_tlb_range failed \n");
        m4u_dump_backtrace();
        return M4U_STATUS_INVALID_HANDLE;
    }
    if(ioctl(mFileDescriptor, MTK_M4U_T_INVALID_TLB_RANGE, &m4u_module))
    {
        M4UMSG(" ioctl MTK_M4U_T_INVALID_TLB_RANGE fail! fd=%d, %d, %s \n", mFileDescriptor, errno, strerror(errno));
        m4u_dump_backtrace();
        return M4U_STATUS_KERNEL_FAULT;
    }
    else
    {
        return M4U_STATUS_OK;
    }
}

M4U_STATUS_ENUM MTKM4UDrv::m4u_invalid_tlb_all(M4U_MODULE_ID_ENUM eModuleID)
{

    if(!m4u_check_m4u_en(eModuleID))
    {
        return M4U_STATUS_OK;
    }
	        
    if(mFileDescriptor<0 )
    {
        M4UMSG("m4u_invalid_tlb_all failed \n");
        m4u_dump_backtrace();
        return M4U_STATUS_INVALID_HANDLE;
    }
    if(ioctl(mFileDescriptor, MTK_M4U_T_INVALID_TLB_ALL, &eModuleID))
    {
        M4UMSG(" ioctl MTK_M4U_T_INVALID_TLB_ALL fail! fd=%d, %d, %s \n", mFileDescriptor, errno, strerror(errno));
        m4u_dump_backtrace();
        return M4U_STATUS_KERNEL_FAULT;
    }
    else
    {
        return M4U_STATUS_OK;
    }
}


M4U_STATUS_ENUM MTKM4UDrv::m4u_manual_insert_entry(M4U_MODULE_ID_ENUM eModuleID,
									unsigned int EntryMVA, 
									bool Lock)	
{


    if(!m4u_check_m4u_en(eModuleID))
    {
        return M4U_STATUS_OK;
    }

    M4U_MOUDLE_STRUCT m4u_module;

    if(0)
    {
        m4u_module.eModuleID = eModuleID;
        m4u_module.EntryMVA = EntryMVA;
        m4u_module.Lock = Lock;
    	        
        if(mFileDescriptor<0 )
        {
            M4UMSG("m4u_insert_entry failed \n");
            m4u_dump_backtrace();
            return M4U_STATUS_INVALID_HANDLE;
        }
        if(ioctl(mFileDescriptor, MTK_M4U_T_MANUAL_INSERT_ENTRY, &m4u_module))
        {
            M4UMSG(" ioctl MTK_M4U_T_MANUAL_INSERT_ENTRY fail! fd=%d, %d, %s \n", mFileDescriptor, errno, strerror(errno));
            m4u_dump_backtrace();
            return M4U_STATUS_KERNEL_FAULT;
        }
        else
        {
            return M4U_STATUS_OK;
        }	
    }
    else
    {
        return M4U_STATUS_OK;
    }
}


static M4U_MODULE_ID_ENUM m4u_get_module_by_port(M4U_PORT_ID_ENUM portID)
{
    return portID;    
}


///> native
M4U_STATUS_ENUM MTKM4UDrv::m4u_config_port(M4U_PORT_STRUCT* pM4uPort)
{

    M4U_MODULE_ID_ENUM eModuleID = m4u_get_module_by_port(pM4uPort->ePortID);

    if(!m4u_check_m4u_en(eModuleID))
    {
        return M4U_STATUS_OK;
    }

    if(NULL==pM4uPort)
    {
        M4UMSG("m4u_config_port failed, input M4U_PORT_STRUCT* is null! \n");
        m4u_dump_backtrace();
        return M4U_STATUS_INVALID_HANDLE;
    }
	        	        
    if(mFileDescriptor<0)
    {
        M4UMSG("m4u_config_port failed \n");
        m4u_dump_backtrace();
        return M4U_STATUS_INVALID_HANDLE;
    }
    if(ioctl(mFileDescriptor, MTK_M4U_T_CONFIG_PORT, pM4uPort))
    {
        M4UMSG(" ioctl MTK_M4U_T_CONFIG_PORT fail! fd=%d, %d, %s \n", mFileDescriptor, errno, strerror(errno));
        m4u_dump_backtrace();
        return M4U_STATUS_KERNEL_FAULT;
    }
    else
    {
        return M4U_STATUS_OK;
    }	
}

M4U_STATUS_ENUM MTKM4UDrv::m4u_config_port_rotator(M4U_PORT_STRUCT_ROTATOR* pM4uPort)
{

    M4U_MODULE_ID_ENUM eModuleID = m4u_get_module_by_port(pM4uPort->ePortID);
    if(!m4u_check_m4u_en(eModuleID))
    {
        return M4U_STATUS_OK;
    }

    if(NULL==pM4uPort)
    {
        M4UMSG("m4u_config_port_rotator failed, input M4U_PORT_STRUCT_ROTATOR* is null! \n");
        m4u_dump_backtrace();
        return M4U_STATUS_INVALID_HANDLE;
    }
	        	        
    if(mFileDescriptor<0)
    {
        M4UMSG("m4u_config_port_rotator failed \n");
        m4u_dump_backtrace();
        return M4U_STATUS_INVALID_HANDLE;
    }
    if(ioctl(mFileDescriptor, MTK_M4U_T_CONFIG_PORT_ROTATOR, pM4uPort))
    {
        M4UMSG(" ioctl MTK_M4U_T_CONFIG_PORT_ROTATOR fail! fd=%d, %d, %s \n", mFileDescriptor, errno, strerror(errno));
        m4u_dump_backtrace();
        return M4U_STATUS_KERNEL_FAULT;
    }
    else
    {
        return M4U_STATUS_OK;
    }	
}

/**
 * @brief :             
 * @param 
 * @return 
 */
M4U_STATUS_ENUM MTKM4UDrv::m4u_reset_mva_release_tlb(M4U_MODULE_ID_ENUM eModuleID)
{

    if(!m4u_check_m4u_en(eModuleID))
    {
        return M4U_STATUS_OK;
    }

    if(mFileDescriptor<0 )
    {
        M4UMSG("m4u_reset_mva_release_tlb failed \n");
        m4u_dump_backtrace();
        return M4U_STATUS_INVALID_HANDLE;
    }

    if(ioctl(mFileDescriptor, MTK_M4U_T_RESET_MVA_RELEASE_TLB, &eModuleID))
    {
        M4UMSG(" ioctl MTK_M4U_T_RESET_MVA_RELEASE_TLB fail! fd=%d, %d, %s \n", mFileDescriptor, errno, strerror(errno));
        m4u_dump_backtrace();
        return M4U_STATUS_KERNEL_FAULT;
    }
    else
    {
        return M4U_STATUS_OK;
    }
}

///> ------------------- helper function -----------------------------------------------------
/**
 * @brief :             
 * @param 
 * @return 
 */
M4U_STATUS_ENUM MTKM4UDrv::m4u_monitor_start(M4U_PORT_ID_ENUM PortID)
{

    M4U_MODULE_ID_ENUM eModuleID = m4u_get_module_by_port(PortID);

    if(!m4u_check_m4u_en(eModuleID))
    {
        return M4U_STATUS_OK;
    }

    if(mFileDescriptor<0 )
    {
        M4UMSG("m4u_monitor_start failed \n");
        m4u_dump_backtrace();
        return M4U_STATUS_INVALID_HANDLE;
    }

    if(ioctl(mFileDescriptor, MTK_M4U_T_MONITOR_START, &PortID))
    {
        M4UMSG(" ioctl MTK_M4U_T_MONITOR_START fail! fd=%d, %d, %s \n", mFileDescriptor, errno, strerror(errno));
        m4u_dump_backtrace();
        return M4U_STATUS_KERNEL_FAULT;
    }
    else
    {
        return M4U_STATUS_OK;
    }
}

/**
 * @brief :             
 * @param 
 * @return 
 */
M4U_STATUS_ENUM MTKM4UDrv::m4u_monitor_stop(M4U_PORT_ID_ENUM PortID)
{
    M4U_MODULE_ID_ENUM eModuleID = m4u_get_module_by_port(PortID);

    if(!m4u_check_m4u_en(eModuleID))
    {
        return M4U_STATUS_OK;
    }


    if(mFileDescriptor<0 )
    {
        M4UMSG("m4u_monitor_stop failed \n");
        m4u_dump_backtrace();
        return M4U_STATUS_INVALID_HANDLE;
    }

    if(ioctl(mFileDescriptor, MTK_M4U_T_MONITOR_STOP, &PortID))
    {
        M4UMSG(" ioctl MTK_M4U_T_MONITOR_STOP fail! fd=%d, %d, %s \n", mFileDescriptor, errno, strerror(errno));
        m4u_dump_backtrace();
        return M4U_STATUS_KERNEL_FAULT;
    }
    else
    {
        return M4U_STATUS_OK;
    }
}


/**
 * @brief : 
 * @param 
 * @return 
 */
M4U_STATUS_ENUM MTKM4UDrv::m4u_dump_reg(M4U_MODULE_ID_ENUM eModuleID)
{

    if(!m4u_check_m4u_en(eModuleID))
    {
        return M4U_STATUS_OK;
    }

    
    if(mFileDescriptor<0 )
    {
        M4UMSG("m4u_dump_reg failed \n");
        m4u_dump_backtrace();
        return M4U_STATUS_INVALID_HANDLE;
    }

    //M4UMSG("before m4u_dump_reg, module=%d", eModuleID);
    if(ioctl(mFileDescriptor, MTK_M4U_T_DUMP_REG, &eModuleID))
    {
        M4UMSG(" ioctl MTK_M4U_T_DUMP_REG fail! fd=%d, %d, %s \n", mFileDescriptor, errno, strerror(errno));
        m4u_dump_backtrace();
        return M4U_STATUS_KERNEL_FAULT;
    }
    else
    {
        //M4UMSG("after m4u_dump_reg, module=%d", eModuleID);
        return M4U_STATUS_OK;
    }
}

/**
 * @brief : 
 * @param 
 * @return 
 */
M4U_STATUS_ENUM MTKM4UDrv::m4u_dump_info(M4U_MODULE_ID_ENUM eModuleID)
{

    if(!m4u_check_m4u_en(eModuleID))
    {
        return M4U_STATUS_OK;
    }


    if(mFileDescriptor<0 )
    {
        M4UMSG("m4u_dump_info failed \n");
        m4u_dump_backtrace();
        return M4U_STATUS_INVALID_HANDLE;
    }

    if(ioctl(mFileDescriptor, MTK_M4U_T_DUMP_INFO, &eModuleID))
    {
        M4UMSG(" ioctl MTK_M4U_T_DUMP_INFO fail! fd=%d, %d, %s \n", mFileDescriptor, errno, strerror(errno));
        m4u_dump_backtrace();
        return M4U_STATUS_KERNEL_FAULT;
    }
    else
    {
        return M4U_STATUS_OK;
    }

}

M4U_STATUS_ENUM MTKM4UDrv::m4u_cache_sync(M4U_MODULE_ID_ENUM eModuleID,
	                                    M4U_CACHE_SYNC_ENUM eCacheSync,
		                                  unsigned int BufAddr, 
		                                  unsigned int BufSize)
{

    if(!m4u_check_m4u_en(eModuleID))
    {
        return M4U_STATUS_OK;
    }


    M4U_CACHE_STRUCT m4u_cache;
    
    if(mFileDescriptor<0 )
    {
        M4UMSG("m4u_cache_sync failed \n");
        m4u_dump_backtrace();
        return M4U_STATUS_INVALID_HANDLE;
    }
	        
    m4u_cache.eModuleID = eModuleID;
    m4u_cache.eCacheSync = eCacheSync;
    m4u_cache.BufAddr = BufAddr;
    m4u_cache.BufSize = BufSize;
    if(ioctl(mFileDescriptor, MTK_M4U_T_CACHE_SYNC, &m4u_cache))
    {
        M4UMSG(" ioctl MTK_M4U_T_CACHE_SYNC fail! fd=%d, %d, %s \n", mFileDescriptor, errno, strerror(errno));
        m4u_dump_backtrace();
        return M4U_STATUS_KERNEL_FAULT;
    }
    else
    {
        return M4U_STATUS_OK;
    }
}


M4U_STATUS_ENUM MTKM4UDrv::m4u_dump_pagetable(M4U_MODULE_ID_ENUM eModuleID, 
								  const unsigned int BufAddr, 
								  const unsigned int BufSize, 
								  unsigned int MVAStart)
{


    if(!m4u_check_m4u_en(eModuleID))
    {
        return M4U_STATUS_OK;
    }

    M4U_MOUDLE_STRUCT m4u_module;
    m4u_module.eModuleID = eModuleID;
    m4u_module.BufAddr = BufAddr;
    m4u_module.BufSize = BufSize;
    m4u_module.MVAStart = MVAStart;
 	        
    if(mFileDescriptor<0 )
    {
        M4UMSG("m4u_dump_pagetable failed \n");
        m4u_dump_backtrace();
        return M4U_STATUS_INVALID_HANDLE;
    }
    if(ioctl(mFileDescriptor, MTK_M4U_T_DUMP_PAGETABLE, &m4u_module))
    {
        M4UMSG(" ioctl MTK_M4U_T_DUMP_PAGETABLE fail! fd=%d, %d, %s \n", mFileDescriptor, errno, strerror(errno));
        m4u_dump_backtrace();
        return M4U_STATUS_KERNEL_FAULT;
    }
    else
    {
        return M4U_STATUS_OK;
    }
}


M4U_STATUS_ENUM MTKM4UDrv::m4u_register_buffer(M4U_MODULE_ID_ENUM eModuleID, 
								  const unsigned int BufAddr, 
								  const unsigned int BufSize,
								  int security,
								  int cache_coherent,
								  unsigned int *pRetMVAAddr)
{


    if(!m4u_check_m4u_en(eModuleID))
    {
        return M4U_STATUS_OK;
    }

    M4U_MOUDLE_STRUCT m4u_module;
    m4u_module.eModuleID = eModuleID;
    m4u_module.BufAddr = BufAddr;
    m4u_module.BufSize = BufSize;
    m4u_module.security = security;
    m4u_module.cache_coherent = cache_coherent;
 	        
    if(mFileDescriptor<0 )
    {
        M4UMSG("m4u_register_buffer failed \n");
        m4u_dump_backtrace();
        return M4U_STATUS_INVALID_HANDLE;
    }
    if(ioctl(mFileDescriptor, MTK_M4U_T_REGISTER_BUFFER, &m4u_module))
    {
        M4UMSG(" ioctl MTK_M4U_T_REGISTER_BUFFER fail! fd=%d, %d, %s \n", mFileDescriptor, errno, strerror(errno));
        m4u_dump_backtrace();
        return M4U_STATUS_KERNEL_FAULT;
    }
    else
    {
        *pRetMVAAddr = 0;
        return M4U_STATUS_OK;
    }
}


M4U_STATUS_ENUM MTKM4UDrv::m4u_cache_flush_all(M4U_MODULE_ID_ENUM eModuleID)
{


    if(!m4u_check_m4u_en(eModuleID))
    {
        return M4U_STATUS_OK;
    }

    if(mFileDescriptor<0 )
    {
        M4UMSG("m4u_cache_sync failed \n");
        m4u_dump_backtrace();
        return M4U_STATUS_INVALID_HANDLE;
    }
	        
    if(ioctl(mFileDescriptor, MTK_M4U_T_CACHE_FLUSH_ALL, NULL))
    {
        M4UMSG(" ioctl MTK_M4U_T_CACHE_FLUSH_ALL fail! fd=%d, %d, %s \n", mFileDescriptor, errno, strerror(errno));
        m4u_dump_backtrace();
        return M4U_STATUS_KERNEL_FAULT;
    }
    else
    {
        return M4U_STATUS_OK;
    }
}


unsigned int MTKM4UDrv::m4u_set_reg(unsigned int addr, unsigned int val)
{

    if(mFileDescriptor<0 )
    {
        M4UMSG("m4u_reset_mva_release_tlb failed \n");
        m4u_dump_backtrace();
        return M4U_STATUS_INVALID_HANDLE;
    }

    unsigned int para[2];
    para[0] = addr;
    para[1] = val;
    if(ioctl(mFileDescriptor, MTK_M4U_T_REG_SET, &para))
    {
        M4UMSG(" ioctl MTK_M4U_T_RESET_MVA_RELEASE_TLB fail! fd=%d, %d, %s \n", mFileDescriptor, errno, strerror(errno));
        m4u_dump_backtrace();
        return M4U_STATUS_KERNEL_FAULT;
    }
    else
    {
        return val;
    }
}

unsigned int MTKM4UDrv::m4u_get_reg(unsigned int addr)
{

    if(mFileDescriptor<0 )
    {
        M4UMSG("m4u_reset_mva_release_tlb failed \n");
        m4u_dump_backtrace();
        return M4U_STATUS_INVALID_HANDLE;
    }

    unsigned int para[2];
    para[0] = addr;
    if(ioctl(mFileDescriptor, MTK_M4U_T_REG_GET, &para))
    {
        M4UMSG(" ioctl MTK_M4U_T_RESET_MVA_RELEASE_TLB fail! fd=%d, %d, %s \n", mFileDescriptor, errno, strerror(errno));
        m4u_dump_backtrace();
        return M4U_STATUS_KERNEL_FAULT;
    }
    else
    {
        return para[1];
    }
}



#ifdef __PMEM_WRAP_LAYER_EN__

    bool MTKM4UDrv::mUseM4U[M4U_CLNTMOD_MAX] = {false};

    bool MTKM4UDrv::m4u_enable_m4u_func(M4U_MODULE_ID_ENUM eModuleID)
    {
        bool ret = mUseM4U[eModuleID];
        mUseM4U[eModuleID] = true;
        return ret;
    }
    
    bool MTKM4UDrv::m4u_disable_m4u_func(M4U_MODULE_ID_ENUM eModuleID)
    {
        bool ret = mUseM4U[eModuleID];
        mUseM4U[eModuleID] = false;
        return ret;
    }
    bool MTKM4UDrv::m4u_print_m4u_enable_status()
    {
        for(int i=0; i<M4U_CLNTMOD_MAX; i++)
        {
            M4UMSG("module(%d)\t:\t%d\n", i, mUseM4U[i]);
        }
        return true;
    }
    bool MTKM4UDrv::m4u_check_m4u_en(M4U_MODULE_ID_ENUM eModuleID)
    {
        if(!mUseM4U[eModuleID])
        {
            M4UMSG("warning: call m4u function but didnot enable m4u function. ID: %d\n", eModuleID);
        }
        return mUseM4U[eModuleID];
    }
#endif 




