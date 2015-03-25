#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/earlysuspend.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/aee.h>
#include <linux/timer.h>
#include <linux/disp_assert_layer.h>
#include <linux/xlog.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>

#include <asm/mach/map.h>
#include <mach/sync_write.h>
#include <mach/mt_irq.h>
#include <mach/mt_clkmgr.h>
#include <mach/irqs.h>
//#include <mach/mt_boot.h>
#include <asm/cacheflush.h>
#include <asm/system.h>
#include <linux/mm.h>
#include <linux/pagemap.h>
#include <linux/m4u_profile.h>

#include "m4u_internal.h"
#include "m4u_reg.h"
#include <linux/debugfs.h>
#include <linux/proc_fs.h>
#include <mach/mt_smi.h>

//==========================================================================================//
#define M4U_ASSERT(x) if(!(x)){xlog_printk(ANDROID_LOG_ERROR, "M4U", "assert fail, file:%s, line:%d", __FILE__, __LINE__);}

#define MTK_M4U_DBG
#ifdef MTK_M4U_DBG
#define M4UDBG(string, args...)	xlog_printk(ANDROID_LOG_DEBUG, "M4U", "[pid=%d]"string,current->tgid,##args);
#else
#define M4UDBG(string, args...)
#endif


bool gM4uLogFlag = false;

#define M4ULOG(string, args...) do { \
	if(gM4uLogFlag){ \
	xlog_printk(ANDROID_LOG_INFO, "M4U", "[pid=%d] "string,current->tgid,##args); } \
}while(0)

#define MTK_M4U_MSG
#ifdef MTK_M4U_MSG
#define M4UMSG(string, args...)	xlog_printk(ANDROID_LOG_INFO, "M4U", "[pid=%d]"string,current->tgid,##args)
#else
#define M4UMSG(string, args...)
#endif

#define M4UERR(string, args...) do {\
	xlog_printk(ANDROID_LOG_ERROR, "M4U", "[pid=%d]error_assert_fail: "string,current->tgid,##args);  \
	aee_kernel_exception("M4U", "[M4U] error:"string,##args);  \
}while(0)

static char m4u_name[100];
#define M4UERR_WITH_OWNER(string, who, args...) do {                           \
	xlog_printk(ANDROID_LOG_ERROR, "M4U", "[pid=%d]error_assert_fail: "string,current->tgid,##args);  \
	sprintf(m4u_name, "[M4U] owner:%s",who); \
  aee_kernel_exception(m4u_name, "[M4U] error:"string,##args);  \
}while(0)


#define m4u_aee_print(string, args...) do{\
	snprintf(m4u_name,100, "[M4U]"string, ##args); \
  aee_kernel_warning(m4u_name, "[M4U] error:"string,##args);  \
}while(0)


#define M4U_USE_ONE_PAGETABLE  //NO Support Secure Domain in MT6572
//#define M4U_COPY_NONSEC_PT_TO_SEC
//#define M4U_PRINT_RANGE_DETAIL  // dump range infro when no available range can be found
//#define M4U_INT_INVALIDATION_DONE //enable interrupt of invalidation done
#define M4U_LOG_ALLOCATE_SIZE
#define M4U_CMD_DEBUG
#define M4U_PROC_DEBUG
//==========================================================================================//
// garbage collect related
#define MVA_REGION_FLAG_NONE 0x0
#define MVA_REGION_HAS_TLB_RANGE 0x1
#define MVA_REGION_REGISTER    0x2

#define M4U_4M_PAGETABLE

// list element, each element record mva's size, start addr info
// if user process dose not call mva_alloc() and mva_dealloc() in pair
// we will help to call mva_dealloc() according to elements' info
typedef struct
{
    struct list_head link;
    unsigned int bufAddr;
    unsigned int mvaStart;
    unsigned int size;
    M4U_MODULE_ID_ENUM eModuleId;
    unsigned int flags;    
    int security;
    int cache_coherent;

} garbage_list_t;

// per-file-handler structure, allocated in M4U_Open, used to 
// record calling of mva_alloc() and mva_dealloc()    
typedef struct
{
    struct mutex dataMutex;
    pid_t open_pid;
    pid_t open_tgid;
    unsigned int OwnResource;
    struct list_head mvaList;
    int isM4uDrvConstruct;
    int isM4uDrvDeconstruct;
} garbage_node_t;

//------------------------------------Defines & Data for alloc mva-------------
//----------------------------------------------------------------------
/// macros to handle M4u Page Table processing
#define M4U_MVA_MAX (TOTAL_MVA_RANGE-1)
#define M4U_PAGE_MASK 0xfff
#define M4U_PAGE_SIZE   0x1000 //4KB
#define DEFAULT_PAGE_SIZE   0x1000 //4KB
#define M4U_PTE_MAX (M4U_GET_PTE_OFST_TO_PT_SA(TOTAL_MVA_RANGE-1))
#define mva_pteAddr_nonsec(mva) ((unsigned int *)pPT_nonsec+((mva) >> 12))
#define mva_pteAddr_sec(mva) ((unsigned int *)pPT_sec+((mva) >> 12))
#define mva_pteAddr(mva) mva_pteAddr_nonsec(mva)

//  ((va&0xfff)+size+0xfff)>>12
#define M4U_GET_PAGE_NUM(va,size) ((((unsigned int)(va)&(M4U_PAGE_SIZE-1))+(size)+(M4U_PAGE_SIZE-1))>>12)

#define mva_pageOffset(mva) ((mva)&0xfff)

#define MVA_BLOCK_SIZE_ORDER     18     //256K
#define MVA_MAX_BLOCK_NR        ((TOTAL_MVA_RANGE >> MVA_BLOCK_SIZE_ORDER)-1)

#define MVA_BLOCK_SIZE      (1<<MVA_BLOCK_SIZE_ORDER)  //0x40000 
#define MVA_BLOCK_ALIGN_MASK (MVA_BLOCK_SIZE-1)        //0x3ffff
#define MVA_BLOCK_NR_MASK   (MVA_MAX_BLOCK_NR)      //0xfff
#define MVA_BUSY_MASK       (1<<15)                 //0x8000

#define MVA_IS_BUSY(index) ((mvaGraph[index]&MVA_BUSY_MASK)!=0)
#define MVA_SET_BUSY(index) (mvaGraph[index] |= MVA_BUSY_MASK)
#define MVA_SET_FREE(index) (mvaGraph[index] & (~MVA_BUSY_MASK))
#define MVA_GET_NR(index)   (mvaGraph[index] & MVA_BLOCK_NR_MASK)

#define MVAGRAPH_INDEX(mva) (mva>>MVA_BLOCK_SIZE_ORDER)

//==========================================================================================//

static short mvaGraph[MVA_MAX_BLOCK_NR+1];
static unsigned char moduleGraph[MVA_MAX_BLOCK_NR+1];
#ifdef M4U_LOG_ALLOCATE_SIZE
static unsigned int mvasizeGraph[MVA_MAX_BLOCK_NR+1];
#endif

static PFN_TF_T m4u_tfcallback[M4U_CLNTMOD_MAX];

static DEFINE_SPINLOCK(gMvaGraph_lock);
static int m4u_cache_sync_init(void);


//==========================================================================================//
//-------------------------------------Global variables------------------------------------------------//

#define MAX_BUF_SIZE_TO_GET_USER_PAGE (200*1024*1024)  //200MB at most for single time alloc

extern unsigned char *pMlock_cnt;
extern unsigned int mlock_cnt_size;
// record memory usage
int* pmodule_max_size=NULL;
int* pmodule_current_size=NULL;
int* pmodule_locked_pages=NULL;
bool* pmodule_in_freemva=NULL;


//unsigned int gLarbBaseAddr[SMI_LARB_NR] = {LARB0_BASE}; 

unsigned int gM4UBaseAddr[TOTAL_M4U_NUM] = {M4U_BASE};
unsigned int g4M4UTagCount[TOTAL_M4U_NUM]  = {M4U_MAIN_TLB_NR};
unsigned int g4M4UWrapCount[TOTAL_M4U_NUM] = {M4U_WRAP_NR};
static volatile unsigned int FreeSEQRegs[TOTAL_M4U_NUM] = {M4U_SEQ_NR};
static volatile unsigned int FreeWrapRegs[TOTAL_M4U_NUM]= {M4U_WRAP_NR};

unsigned int m4u_port_size_limit[M4U_PORT_NR] = {};

unsigned int pt_pa_nonsec;    //Page Table Physical Address, 64K align
unsigned int *pPT_nonsec;
unsigned int pt_pa_sec;
unsigned int *pPT_sec;


#define TF_PROTECT_BUFFER_SIZE 128
unsigned int ProtectPA = 0;
unsigned int *pProtectVA = NULL; 

//unsigned int gM4U_align_page_va = 0;
//unsigned int gM4U_align_page_pa = 0;

//#define BACKUP_REG_SIZE (M4U_REG_SIZE*TOTAL_M4U_NUM)
#define BACKUP_REG_SIZE 640
static unsigned int* pM4URegBackUp = 0;

static M4U_RANGE_DES_T *pRangeDes = NULL;
static M4U_WRAP_DES_T *pWrapDes = 0;
#define RANGE_DES_ADDR 0x11



static spinlock_t gM4u_reg_lock;
static DEFINE_MUTEX(gM4uMutex);
static DEFINE_MUTEX(gM4uMutexPower);

#define MTK_M4U_DEV_MAJOR_NUMBER 188
static struct cdev * g_pMTKM4U_CharDrv = NULL;
static dev_t g_MTKM4Udevno = MKDEV(MTK_M4U_DEV_MAJOR_NUMBER,0);
#define M4U_DEVNAME "M4U_device"

extern void init_mlock_cnt(void);

extern unsigned int m4u_user_v2p(unsigned int va);

extern int is_pmem_range(unsigned long* base, unsigned long size);
extern int m4u_get_user_pages(int eModuleID, struct task_struct *tsk, struct mm_struct *mm, unsigned long start, int nr_pages, int write, int force, struct page **pages, struct vm_area_struct **vmas);
extern void  smp_inner_dcache_flush_all(void);



static unsigned int gModuleMaxMVASize[M4U_CLIENT_MODULE_NUM] = {
    M4U_CLNTMOD_SZ_MDP     ,
    M4U_CLNTMOD_SZ_DISP    ,
    M4U_CLNTMOD_SZ_VIDEO   ,
    M4U_CLNTMOD_SZ_CAM     ,
    M4U_CLNTMOD_SZ_CMDQ    ,
    M4U_CLNTMOD_SZ_LCDC_UI ,    
    M4U_CLNTMOD_SZ_RESERVED     
};


typedef enum
{
	M4U_TEST_LEVEL_USER = 0,  // performance best, least verification
	M4U_TEST_LEVEL_ENG = 1,   // SQC used, more M4UMSG and M4UERR
	M4U_TEST_LEVEL_STRESS= 2  // stricker verification ,may use M4UERR instead M4UMSG sometimes, used for our own internal test
} M4U_TEST_LEVEL_ENUM;
M4U_TEST_LEVEL_ENUM gTestLevel = M4U_TEST_LEVEL_ENG;    

#define M4U_POW_ON_TRY(eModuleID) 
#define M4U_POW_OFF_TRY(eModuleID) 

struct timer_list perf_timer;

//--------------------------------------Functions-----------------------------------------------------//

/*****************************************************************************
 * FUNCTION
 *    m4u_port_2_module
 * DESCRIPTION
 *    Query Module ID about specified Port ID.
 * PARAMETERS
 *    param1 : [IN]  const M4U_PORT_ID_ENUM portID
 *                Port ID.    
 * RETURNS
 *    M4U_MODULE_ID_ENUM. Module ID
 ****************************************************************************/
static M4U_MODULE_ID_ENUM m4u_port_2_module(const M4U_PORT_ID_ENUM portID)
{
    M4U_MODULE_ID_ENUM moduleID = M4U_CLNTMOD_UNKNOWN;
    switch(portID)
    {
        case M4U_PORT_MDP_RDMA               :
        case M4U_PORT_MDP_WDMA               :
        case M4U_PORT_MDP_ROTO               :
        case M4U_PORT_MDP_ROTCO              :
        case M4U_PORT_MDP_ROTVO              :
            moduleID = M4U_CLNTMOD_MDP;
            break;
        
        case M4U_PORT_LCD_OVL                :
        case M4U_PORT_LCD_R                  :
        case M4U_PORT_LCD_W                  :
        case M4U_PORT_LCD_DBI                :
            moduleID = M4U_CLNTMOD_DISP;
            break;

        
        case M4U_PORT_VENCMC                 :
        case M4U_PORT_VENC_REC_VDEC_WDMA     :
        case M4U_PORT_VENC_CDMA_VDEC_CDMA    :
        case M4U_PORT_VENC_MVQP              :
        case M4U_PORT_VENC_BSDMA_VDEC_POST0  :			
            moduleID = M4U_CLNTMOD_VIDEO;
            break;

        case M4U_PORT_CAM_WDMA               :
            moduleID = M4U_CLNTMOD_CAM;
            break;

        case M4U_PORT_CMDQ                :
            moduleID = M4U_CLNTMOD_CMDQ;
            break;
        default:
        	M4UERR("m4u_port_2_module() fail, invalid portID=%d", portID);
    }	
  
    return moduleID;    
}


/*****************************************************************************
 * FUNCTION
 *    m4u_invalid_tlb
 * DESCRIPTION
 *    Invalid TLB implement.
 * PARAMETERS
 *    param1 : [IN]  const bool isInvAll
 *                Invalid all flag.  
 *    param2 : [IN]  const unsigned int mva_start
 *                Start invalid mva for range invalid.  
 *    param3 : [IN]  const unsigned int mva_end
 *                End invalid mva for range invalid.   
 * RETURNS
 *    None.
 ****************************************************************************/
static void m4u_invalid_tlb(const bool isInvAll, const unsigned int mva_start, const unsigned int mva_end)
{
    unsigned int reg = 0;
    reg |= F_MMUg_CTRL_INV_EN0;

    COM_WriteReg32(REG_MMUg_CTRL, reg); 
    
    if(isInvAll)
    {
        COM_WriteReg32(REG_MMUg_INVLD, F_MMUg_INV_ALL);
    }
    else 
    {

        if(mva_end < mva_start)
        {
            M4UERR("[M4U] TLB Invalid parameter error!");
        }
        COM_WriteReg32(REG_MMUg_INVLD_SA ,mva_start & (~0xfff));
        COM_WriteReg32(REG_MMUg_INVLD_EA, mva_end&(~0xfff));
        COM_WriteReg32(REG_MMUg_INVLD, F_MMUg_INV_RANGE);  //auto clear after invalid
    }    
}

/*****************************************************************************
 * FUNCTION
 *    m4u_invalid_tlb_all
 * DESCRIPTION
 *    Invalid all TLB.
 * PARAMETERS
 *    None.
 * RETURNS
 *    None.
 ****************************************************************************/
static void m4u_invalid_tlb_all(void)
{
    m4u_invalid_tlb(true, 0, 0);
}

/*****************************************************************************
 * FUNCTION
 *    m4u_invalid_tlb_by_range
 * DESCRIPTION
 *    Invalid TLB by range.
 * PARAMETERS
 *    param1 : [IN]  const unsigned int mva_start
 *                Start invalid mva.  
 *    param2 : [IN]  const unsigned int mva_end
 *                End invalid mva.   
 * RETURNS
 *    None.
 ****************************************************************************/
void m4u_invalid_tlb_by_range(const unsigned int mva_start, const unsigned int mva_end)
{
    M4ULOG("m4u_invalid_tlb_by_range Start=0x%x, End=0x%x\n", mva_start, mva_end);

    m4u_invalid_tlb(false, mva_start, mva_end);
}

/*****************************************************************************
 * FUNCTION
 *    m4u_invalid_tlb_sec_by_range
 * DESCRIPTION
 *    Invalid TLB by security range.
 * PARAMETERS
 *    param1 : [IN]  const unsigned int mva_start
 *                Start invalid mva.  
 *    param2 : [IN]  const unsigned int mva_end
 *                End invalid mva.   
 * RETURNS
 *    None.
 ****************************************************************************/
#ifndef M4U_USE_ONE_PAGETABLE
void m4u_invalid_tlb_sec_by_range(const unsigned int mva_start, 
                                        const unsigned int mva_end)
{    
    unsigned int reg = 0;
    reg |= F_MMUg_CTRL_SEC_INV_EN0;

    m4uHw_set_field_by_mask(0, REG_MMUg_CTRL_SEC, F_MMUg_CTRL_SEC_INV_EN0_MSK, reg);
    
    COM_WriteReg32(REG_MMUg_INVLD_SA, mva_start & (~0xfff));
    COM_WriteReg32(REG_MMUg_INVLD_EA, mva_end&(~0xfff));
    COM_WriteReg32(REG_MMUg_INVLD_SEC, F_MMUg_INV_SEC_RANGE);
     
}
#endif

static int m4u_dump_maps(unsigned int addr)
{
    struct vm_area_struct *vma;
    
    M4UMSG("addr=0x%x, name=%s,pid=0x%x,", addr, current->comm, current->pid);

    vma = find_vma(current->mm, addr);
    if(vma == NULL)
    {
        M4UMSG("dump_maps fail: find_vma return NULL\n");
        return -1;
    }

    M4UMSG("find vma: 0x%08x-0x%08x\n", (unsigned int)(vma->vm_start), (unsigned int)(vma->vm_end));

    return 0;
}

/*****************************************************************************
* FUNCTION
*	 MTK_M4U_open
* DESCRIPTION
*	 File operations - open
*      1. allocate private data as garbage_node_t
*      2. initial garbage_node_t
* PARAMETERS
*	 param1 : [IN] struct inode * a_pstInode
*				 inode structure.
*	 param2 : [IN] struct file * a_pstFile
*				 file structure.
* RETURNS
*	 Type: Integer. zero mean sucess and other mean error.
****************************************************************************/
static int MTK_M4U_open(struct inode * a_pstInode, struct file * a_pstFile)
{
    garbage_node_t * pNode;

    M4ULOG("enter MTK_M4U_open() process:%s\n",current->comm);

    //Allocate and initialize private data
    a_pstFile->private_data = kmalloc(sizeof(garbage_node_t) , GFP_ATOMIC|__GFP_ZERO);

    if(NULL == a_pstFile->private_data)
    {
        M4UMSG("Not enough memory for MTK_M4U_open\n");
        return -ENOMEM;
    }

    pNode = (garbage_node_t *)a_pstFile->private_data;
    mutex_init(&(pNode->dataMutex));
    mutex_lock(&(pNode->dataMutex));
    pNode->open_pid = current->pid;
    pNode->open_tgid = current->tgid;  
    INIT_LIST_HEAD(&(pNode->mvaList));
    mutex_unlock(&(pNode->dataMutex));

    return 0;
}

/*****************************************************************************
* FUNCTION
*	 MTK_M4U_release
* DESCRIPTION
*	 File operations - release
*      1. deallocate all mva in list of garbage_node_t
*      2. free private_data
* PARAMETERS
*	 param1 : [IN] struct inode * a_pstInode
*				 inode structure.
*	 param2 : [IN] struct file * a_pstFile
*				 file structure.
* RETURNS
*	 Type: Integer. always zero.
****************************************************************************/
static int MTK_M4U_release(struct inode * a_pstInode, struct file * a_pstFile)
{
    struct list_head *pListHead, *ptmp;
    garbage_node_t *pNode = a_pstFile->private_data;
    garbage_list_t *pList;
    M4ULOG("enter MTK_M4U_release() process:%s\n",current->comm);

    mutex_lock(&(pNode->dataMutex));

    if(pNode->isM4uDrvConstruct==0 || pNode->isM4uDrvDeconstruct==0)
    {
        M4UDBG("warning on close: construct=%d, deconstruct=%d, open_pid=%d, cur_pid=%d\n",
            pNode->isM4uDrvConstruct, pNode->isM4uDrvDeconstruct,
            pNode->open_pid, current->pid);
        M4UDBG("open->tgid=%d, cur->tgid=%d, cur->mm=0x%x\n",
            pNode->open_tgid, current->tgid, current->mm);
    }
    
    pListHead = pNode->mvaList.next;
    while(pListHead!= &(pNode->mvaList))
    {
        ptmp = pListHead;
        pListHead = pListHead->next;
        pList = container_of(ptmp, garbage_list_t, link);
        M4UDBG("warnning: clean garbage at m4u close: module=%s,va=0x%x,mva=0x%x,size=%d\n",
            m4u_get_module_name(pList->eModuleId),pList->bufAddr,pList->mvaStart,pList->size);

        //if registered but never has chance to query this buffer (we will allocate mva in query_mva)
        //then the mva will be 0, and MVA_REGION_REGISTER flag will be set.
        //we don't call deallocate for this mva, because it's 0 ...
        if(pList->mvaStart != 0)        
        {
            m4u_dealloc_mva(pList->eModuleId, pList->bufAddr, pList->size, pList->mvaStart);
        }
        else
        {
            if(!(pList->flags&MVA_REGION_REGISTER))
                M4UERR("warning: in garbage reclaim: mva==0, but MVA_REGION_REGISTER is not set!! flag=0x%x\n", pList->flags);
        }
        list_del(ptmp);
        kfree(pList);
    }
 
    mutex_unlock(&(pNode->dataMutex));
    
    if(NULL != a_pstFile->private_data)
    {
        kfree(a_pstFile->private_data);
        a_pstFile->private_data = NULL;
    }
        
    return 0;
}

/*****************************************************************************
* FUNCTION
*	 MTK_M4U_ioctl
* DESCRIPTION
*	 File operations - unlocked_ioctl
*      1. call copy_from_user to get user space parameter
*      2. call internal function by operation.
*      3. call copy_to_user to set return data.
* PARAMETERS
*	 param1 : [IN] struct file * a_pstFile
*				 file structure*.
*	 param2 : [IN] unsigned int a_Command
*				 operation command.
*	 param3 : [IN] unsigned long a_Param
*				 parameter of ioctl.
* RETURNS
*	 Type: long. zero mean sucess and other mean error.
****************************************************************************/
static long MTK_M4U_ioctl(struct file * a_pstFile,
								unsigned int a_Command,
								unsigned long a_Param)
{
    int ret = 0;
    M4U_MOUDLE_STRUCT m4u_module;
    M4U_PORT_STRUCT m4u_port;
    M4U_PORT_STRUCT_ROTATOR m4u_port_rotator;
    M4U_PORT_ID_ENUM PortID;
    M4U_MODULE_ID_ENUM ModuleID;
    M4U_WRAP_DES_T m4u_wrap_range;
    M4U_CACHE_STRUCT m4u_cache_data;
    garbage_node_t *pNode = a_pstFile->private_data;

    M4ULOG("enter MTK_M4U_ioctl() command:%u\n",a_Command);

    switch(a_Command)
    {
        case MTK_M4U_T_POWER_ON :
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&ModuleID, (void*)a_Param , sizeof(unsigned int));
            if(ret)
            {
            	M4UERR(" MTK_M4U_T_POWER_ON, copy_from_user failed, %d\n", ret);
            	return -EFAULT;
            }  
            ret = m4u_power_on();
        break;

        case MTK_M4U_T_POWER_OFF :
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&ModuleID, (void*)a_Param , sizeof(unsigned int));
            if(ret)
            {
            	M4UERR(" MTK_M4U_T_POWER_OFF, copy_from_user failed, %d\n", ret);
            	return -EFAULT;
            }  
            ret = m4u_power_off();
        break;

        case MTK_M4U_T_ALLOC_MVA :			
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&m4u_module, (void*)a_Param , sizeof(M4U_MOUDLE_STRUCT));
            if(ret)
            {
            	M4UERR(" MTK_M4U_T_ALLOC_MVA, copy_from_user failed: %d\n", ret);
            	return -EFAULT;
            }  

            if(m4u_module.MVAStart == -1) //work around for wrap layer
            {
                m4u_module.MVAStart = m4u_user_v2p(m4u_module.BufAddr);
                M4UMSG("alloc_mva_pmem: module=%d,va=0x%x, pa=0x%x\n",
                    m4u_module.eModuleID, m4u_module.BufAddr, m4u_module.MVAStart);
                ret = 0;
            }
            else
            {
                ret = m4u_alloc_mva(m4u_module.eModuleID, 
            			  m4u_module.BufAddr, 
            			  m4u_module.BufSize, 
            			  m4u_module.security,
            			  m4u_module.cache_coherent,
            			  &(m4u_module.MVAStart)); 

                if(ret)
                {
                	M4UMSG(" MTK_M4U_T_ALLOC_MVA, m4u_alloc_mva failed: %d\n", ret);
                	return -EFAULT;
                }  
                else
                {
                    m4u_add_to_garbage_list(a_pstFile, m4u_module.MVAStart, 
                        m4u_module.BufSize, m4u_module.eModuleID, m4u_module.BufAddr, 
                        MVA_REGION_FLAG_NONE, m4u_module.security, m4u_module.cache_coherent);      
                }

            }
            
            ret = copy_to_user(&(((M4U_MOUDLE_STRUCT*)a_Param)->MVAStart), &(m4u_module.MVAStart) , sizeof(unsigned int));
            if(ret)
            {
            	M4UERR(" MTK_M4U_T_ALLOC_MVA, copy_from_user failed: %d\n", ret);
            	return -EFAULT;
            }  
        break;

        case MTK_M4U_T_QUERY_MVA :			
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&m4u_module, (void*)a_Param , sizeof(M4U_MOUDLE_STRUCT));
            if(ret)
            {
            	M4UERR(" MTK_M4U_T_QUERY_MVA, copy_from_user failed: %d\n", ret);
            	return -EFAULT;
            }  
            M4ULOG("-MTK_M4U_T_QUERY_MVA, module_id=%d, BufAddr=0x%x, BufSize=%d \r\n",
            		m4u_module.eModuleID, m4u_module.BufAddr, m4u_module.BufSize );			
            
            m4u_query_mva(m4u_module.eModuleID, 
            			  m4u_module.BufAddr, 
            			  m4u_module.BufSize, 
            			  &(m4u_module.MVAStart),
            			  a_pstFile); 
                       
            ret = copy_to_user(&(((M4U_MOUDLE_STRUCT*)a_Param)->MVAStart), &(m4u_module.MVAStart) , sizeof(unsigned int));
            if(ret)
            {
            	M4UERR(" MTK_M4U_T_QUERY_MVA, copy_from_user failed: %d\n", ret);
            	return -EFAULT;
            }  
            M4ULOG("MTK_M4U_T_QUERY_MVA,  m4u_module.MVAStart=0x%x \n", m4u_module.MVAStart);
        break;
        
        case MTK_M4U_T_DEALLOC_MVA :
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&m4u_module, (void*)a_Param , sizeof(M4U_MOUDLE_STRUCT));
            if(ret)
            {
            	M4UERR(" MTK_M4U_T_DEALLOC_MVA, copy_from_user failed: %d\n", ret);
            	return -EFAULT;
            } 
            M4ULOG("MTK_M4U_T_DEALLOC_MVA, eModuleID:%d, VABuf:0x%x, Length:%d, MVAStart=0x%x \r\n",
            	m4u_module.eModuleID, m4u_module.BufAddr, m4u_module.BufSize, m4u_module.MVAStart); 


            ret = m4u_delete_from_garbage_list(&m4u_module, a_pstFile);
            
            if(ret!=0)
            {
                M4UMSG("error to dealloc mva: id=%s,va=0x%x,size=%d,mva=0x%x\n", 
                    m4u_get_module_name(m4u_module.eModuleID), m4u_module.BufAddr,
                    m4u_module.BufSize, m4u_module.MVAStart);
                m4u_print_mva_list(a_pstFile, "in deallocate");
            }
            else
            {
                //if user register a buffer without query it,
                //then we never allocated a real mva for it,
                //when deallocate, m4u_module.MVAStart==0, we think this is right.
                if(m4u_module.MVAStart!=0)
                {
                    m4u_dealloc_mva(m4u_module.eModuleID, 
                    				m4u_module.BufAddr, 
                    				m4u_module.BufSize,
                    				m4u_module.MVAStart);
                }
                else
                {
                    M4UMSG("warning: deallocat a registered buffer, before any query!\n");
                    M4UMSG("error to dealloc mva: id=%s,va=0x%x,size=%d,mva=0x%x\n", 
                        m4u_get_module_name(m4u_module.eModuleID), m4u_module.BufAddr,
                        m4u_module.BufSize, m4u_module.MVAStart);
                }
            }

            				
        break;
            
        case MTK_M4U_T_MANUAL_INSERT_ENTRY :
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&m4u_module, (void*)a_Param , sizeof(M4U_MOUDLE_STRUCT));
            if(ret)
            {
            	M4UERR(" MTK_M4U_Manual_Insert_Entry, copy_from_user failed: %d\n", ret);
            	return -EFAULT;
            } 
            M4ULOG(" ManualInsertTLBEntry, eModuleID:%d, Entry_MVA:0x%x, locked:%d\r\n", 
            	m4u_module.eModuleID, m4u_module.EntryMVA, m4u_module.Lock);
            
            ret = m4u_manual_insert_entry(m4u_module.eModuleID,
                                          m4u_module.EntryMVA,
                                          !!(m4u_module.security),
                                          m4u_module.Lock);
        break;

        case MTK_M4U_T_INSERT_TLB_RANGE :
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&m4u_module, (void*)a_Param , sizeof(M4U_MOUDLE_STRUCT));
            if(ret)
            {
            	M4UERR("m4u_insert_seq_range , copy_from_user failed: %d\n", ret);
            	return -EFAULT;
            } 

            ret = m4u_insert_seq_range(m4u_module.eModuleID, 
            				  m4u_module.MVAStart, 
            				  m4u_module.MVAEnd, 
            				  m4u_module.entryCount);
        break;

        case MTK_M4U_T_INVALID_TLB_RANGE :
        M4U_ASSERT(a_Param);
        ret = copy_from_user(&m4u_module, (void*)a_Param , sizeof(M4U_MOUDLE_STRUCT));
        if(ret)
        {
        	M4UERR(" MTK_M4U_Invalid_TLB_Range, copy_from_user failed: %d\n", ret);
        	return -EFAULT;
        } 
        M4ULOG("MTK_M4U_Invalid_TLB_Range(), eModuleID:%d, MVAStart=0x%x, MVAEnd=0x%x \n", 
        		m4u_module.eModuleID, m4u_module.MVAStart, m4u_module.MVAEnd);
                  	
        ret = m4u_invalid_seq_range(m4u_module.eModuleID,
                                        m4u_module.MVAStart, 
            							 m4u_module.MVAEnd);
        break;

        case MTK_M4U_T_INVALID_TLB_ALL :
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&ModuleID, (void*)a_Param , sizeof(unsigned int));
            if(ret)
            {
            	M4UERR(" MTK_M4U_Invalid_TLB_Range, copy_from_user failed, %d\n", ret);
            	return -EFAULT;
            }           		
            //ret = m4u_invalid_tlb_all();
        break;

        case MTK_M4U_T_DUMP_REG :
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&ModuleID, (void*)a_Param , sizeof(unsigned int));
            if(ret)
            {
            	M4UERR(" MTK_M4U_Invalid_TLB_Range, copy_from_user failed, %d\n", ret);
            	return -EFAULT;
            } 
            m4u_dump_main_tlb_tags();
            ret = m4u_dump_reg();
            
        break;

        case MTK_M4U_T_DUMP_INFO :
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&ModuleID, (void*)a_Param , sizeof(unsigned int));
            if(ret)
            {
            	M4UERR(" MTK_M4U_Invalid_TLB_Range, copy_from_user failed, %d\n", ret);
            	return -EFAULT;
            } 
            ret = m4u_dump_info();
            m4u_dump_pagetable(ModuleID);
            
        break;

        case MTK_M4U_T_CACHE_SYNC :
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&m4u_cache_data, (void*)a_Param , sizeof(M4U_CACHE_STRUCT));
            if(ret)
            {
            	M4UERR(" MTK_M4U_T_CACHE_INVALID_AFTER_HW_WRITE_MEM, copy_from_user failed: %d\n", ret);
            	return -EFAULT;
            } 
            M4ULOG("MTK_M4U_T_CACHE_INVALID_AFTER_HW_WRITE_MEM(), moduleID=%d, eCacheSync=%d, buf_addr=0x%x, buf_length=0x%x \n", 
            		m4u_cache_data.eModuleID, m4u_cache_data.eCacheSync, m4u_cache_data.BufAddr, m4u_cache_data.BufSize);

            switch(m4u_cache_data.eCacheSync)  
            {
            	case M4U_CACHE_FLUSH_BEFORE_HW_WRITE_MEM:
                case M4U_CACHE_FLUSH_BEFORE_HW_READ_MEM:
                		ret = m4u_dma_cache_maint(m4u_cache_data.eModuleID, (unsigned int*)(m4u_cache_data.BufAddr), m4u_cache_data.BufSize, M4U_DMA_READ_WRITE);
                		break;

                case M4U_CACHE_CLEAN_BEFORE_HW_READ_MEM:
                		ret = m4u_dma_cache_maint(m4u_cache_data.eModuleID, (unsigned int*)(m4u_cache_data.BufAddr), m4u_cache_data.BufSize, M4U_DMA_READ);
                		break;
                		
                case M4U_CACHE_INVALID_AFTER_HW_WRITE_MEM:
                		ret = m4u_dma_cache_maint(m4u_cache_data.eModuleID, (unsigned int*)(m4u_cache_data.BufAddr), m4u_cache_data.BufSize, M4U_DMA_WRITE);
                		break;
                default:
                	M4UMSG("error: MTK_M4U_T_CACHE_SYNC, invalid eCacheSync=%d, module=%d \n", m4u_cache_data.eCacheSync, m4u_cache_data.eModuleID);  
            }            
        break;

        case MTK_M4U_T_CONFIG_PORT :
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&m4u_port, (void*)a_Param , sizeof(M4U_PORT_STRUCT));
            if(ret)
            {
            	M4UERR(" MTK_M4U_T_CONFIG_PORT, copy_from_user failed: %d \n", ret);
            	return -EFAULT;
            } 
            M4ULOG("ePortID=%d, Virtuality=%d, Security=%d, Distance=%d, Direction=%d \n",
                m4u_port.ePortID, m4u_port.Virtuality, m4u_port.Security, m4u_port.Distance, m4u_port.Direction);
            
            ret = m4u_config_port(&m4u_port);
        break;                                

        case MTK_M4U_T_CONFIG_PORT_ROTATOR:
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&m4u_port_rotator, (void*)a_Param , sizeof(M4U_PORT_STRUCT_ROTATOR));
            if(ret)
            {
            	M4UERR(" MTK_M4U_T_CONFIG_PORT_ROTATOR, copy_from_user failed: %d \n", ret);
            	return -EFAULT;
            } 
            ret = m4u_config_port_rotator(&m4u_port_rotator);
        break; 
      
        case MTK_M4U_T_CONFIG_ASSERT :
            // todo
        break;

        case MTK_M4U_T_INSERT_WRAP_RANGE :
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&m4u_wrap_range, (void*)a_Param , sizeof(M4U_WRAP_DES_T));
            if(ret)
            {
            	M4UERR(" MTK_M4U_T_INSERT_WRAP_RANGE, copy_from_user failed: %d \n", ret);
            	return -EFAULT;
            } 
            M4ULOG("PortID=%d, eModuleID=%d, MVAStart=0x%x, MVAEnd=0x%x \n",
                    m4u_wrap_range.ePortID, 
                    m4u_wrap_range.eModuleID,
                    m4u_wrap_range.MVAStart, 
                    m4u_wrap_range.MVAEnd );
            
            ret = m4u_insert_wrapped_range(m4u_wrap_range.eModuleID,
                                  m4u_wrap_range.ePortID, 
                                  m4u_wrap_range.MVAStart, 
                                  m4u_wrap_range.MVAEnd);
        break;   

        case MTK_M4U_T_MONITOR_START :
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&PortID, (void*)a_Param , sizeof(unsigned int));
            if(ret)
            {
            	M4UERR(" MTK_M4U_T_MONITOR_START, copy_from_user failed, %d\n", ret);
            	return -EFAULT;
            } 
            ret = m4u_monitor_start();

        break;

        case MTK_M4U_T_MONITOR_STOP :
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&PortID, (void*)a_Param , sizeof(unsigned int));
            if(ret)
            {
            	M4UERR(" MTK_M4U_T_MONITOR_STOP, copy_from_user failed, %d\n", ret);
            	return -EFAULT;
            } 
            ret = m4u_monitor_stop();
        break;

        case MTK_M4U_T_RESET_MVA_RELEASE_TLB :
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&ModuleID, (void*)a_Param , sizeof(ModuleID));
            if(ret)
            {
              M4UERR(" MTK_M4U_T_RESET_MVA_RELEASE_TLB, copy_from_user failed: %d\n", ret);
              return -EFAULT;
            }             
            ret = m4u_reset_mva_release_tlb(ModuleID);            
        break;

        case MTK_M4U_T_M4UDrv_CONSTRUCT:
            mutex_lock(&(pNode->dataMutex));
            pNode->isM4uDrvConstruct = 1;
            mutex_unlock(&(pNode->dataMutex));

        break;
        
        case MTK_M4U_T_M4UDrv_DECONSTRUCT:
            mutex_lock(&(pNode->dataMutex));
            pNode->isM4uDrvDeconstruct = 1;
            mutex_unlock(&(pNode->dataMutex));
        break;

        case MTK_M4U_T_DUMP_PAGETABLE:
        do{
            unsigned int mva, va, page_num, size, i;

            M4U_ASSERT(a_Param);
            ret = copy_from_user(&m4u_module, (void*)a_Param , sizeof(M4U_MOUDLE_STRUCT));
            if(ret)
            {
            	M4UERR(" MTK_M4U_T_ALLOC_MVA, copy_from_user failed: %d\n", ret);
            	return -EFAULT;
            }  
            mva = m4u_module.MVAStart;
            va = m4u_module.BufAddr;
            size = m4u_module.BufSize;
            page_num = (size + (va&0xfff))/DEFAULT_PAGE_SIZE;

            M4UMSG("M4U dump pagetable in ioctl: mva=0x%x, size=0x%x===>\n", mva,size);
            m4u_dump_pagetable_range(mva, page_num);
            
            M4UMSG("M4U dump PA by VA in ioctl: va=0x%x, size=0x%x===>\n", va,size);
            M4UMSG("0x%08x: ", va);
            for(i=0; i<page_num; i++)
            {
                M4UMSG("0x%08x, ", m4u_user_v2p(va+i*M4U_PAGE_SIZE));
                if((i+1)%8==0)
                {
                	 M4UMSG("\n 0x%08x: ", (va+((i+1)<<12)));
                }
            }
            M4UMSG("\n"); 


            M4UMSG("=========  compare these automaticly =======>\n");
            for(i=0; i<page_num; i++)
            {
                unsigned int pa, entry;
                pa = m4u_user_v2p(va+i*M4U_PAGE_SIZE);
                entry = *(unsigned int*)mva_pteAddr_nonsec((mva+i*M4U_PAGE_SIZE));

                if((pa&(~0xfff)) != (pa&(~0xfff)))
                {
                    M4UMSG("warning warning!! va=0x%x,mva=0x%x, pa=0x%x,entry=0x%x\n",
                        va+i*M4U_PAGE_SIZE, mva+i*M4U_PAGE_SIZE, pa, entry);
                }
            }

        }while(0);
            
        break;
        
        case MTK_M4U_T_REGISTER_BUFFER:
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&m4u_module, (void*)a_Param , sizeof(M4U_MOUDLE_STRUCT));
            if(ret)
            {
            	M4UERR(" MTK_M4U_T_ALLOC_MVA, copy_from_user failed: %d\n", ret);
            	return -EFAULT;
            }  
            M4ULOG("-MTK_M4U_T_REGISTER_BUF, module_id=%d, BufAddr=0x%x, BufSize=%d \r\n",
            		m4u_module.eModuleID, m4u_module.BufAddr, m4u_module.BufSize );			
            
            m4u_add_to_garbage_list(a_pstFile, 0, m4u_module.BufSize, 
                m4u_module.eModuleID, m4u_module.BufAddr, MVA_REGION_REGISTER,
                m4u_module.security, m4u_module.cache_coherent);

        break;

        case MTK_M4U_T_CACHE_FLUSH_ALL:
            m4u_dma_cache_flush_all();
        break;

        default :
            M4UMSG("MTK M4U ioctl : No such command!!\n");
            ret = -EINVAL;
        break;        
    }

    return ret;
}

static const struct file_operations g_stMTK_M4U_fops = 
{
	.owner = THIS_MODULE,
	.open = MTK_M4U_open,
	.release = MTK_M4U_release,
	.unlocked_ioctl = MTK_M4U_ioctl
};

/*****************************************************************************
* FUNCTION
*	 MTK_M4U_isr
* DESCRIPTION
*	 M4U interrupt handler.
* Context
*      ISR
* PARAMETERS
*	 param1 : [IN] int irq
*				 irq number.
*	 param2 : [IN] void *dev_id
*				 No use in this function.
* RETURNS
*	 irqreturn_t. IRQ_HANDLED mean success and -1 mean error.
****************************************************************************/
static irqreturn_t MTK_M4U_isr(int irq, void *dev_id)
{
    unsigned int m4u_base, m4u_index;  								  
    
    switch(irq)
    {
        case MT_M4U0_IRQ_ID:
        	m4u_base = M4U_BASE;
        	m4u_index = 0;
          break;

        default:
          //M4UMSG("MTK_M4U_isr(), Invalid irq number \n");
          M4UERR("MTK_M4U_isr(), Invalid irq number \n");
          return -1;                              	 	
    }

    
    {
        unsigned int IntrSrc = M4U_ReadReg32(m4u_base, REG_MMU_FAULT_ST) & 0x1FFF; 		
        unsigned int faultMva;
        unsigned int regval;
        int portID, larbID;
        faultMva = M4U_ReadReg32(m4u_base, REG_MMU_FAULT_VA);


        M4UMSG("m4u isr: intrSrc=0x%x ==>\n", IntrSrc);

#ifdef M4U_INT_INVALIDATION_DONE 		
        if(IntrSrc&F_INT_INVALIDATION_DONE)
        {
            M4ULOG("TLB Invalidation done! \n");
        }else
#endif        
        {
            if(F_INT_PFH_DMA_FIFO_OVERFLOW != IntrSrc)        
            {
               if((IntrSrc&F_INT_TRANSLATION_FAULT) || IntrSrc&F_INT_INVALID_PHYSICAL_ADDRESS_FAULT)
               {
				   
                   regval = M4U_ReadReg32(m4u_base, REG_MMU_INT_ID);
                   portID = F_INT_ID_TF_PORT_ID(regval);
                   larbID = F_INT_ID_TF_LARB_ID(regval) - 1;
			
                   M4UMSG("translation fault: port=%s, fault_mva=0x%x\n", m4u_get_port_name(larb_port_2_m4u_port(larbID, portID)), faultMva);
               }
				
                m4u_print_active_port();
                //m4u_memory_usage(); 
                m4u_dump_mva_info();

                m4u_dump_main_tlb_des();
                m4u_dump_pfh_tlb_des();				
            }
        }
		
        if(0==IntrSrc)
        {
            M4UMSG("warning: MTK_M4U_isr, IntrSrc=0, need check REG_MMU_FAULT_ST\n");
            m4u_clear_intr(m4u_base);
            return IRQ_HANDLED;
        }
                
        if(IntrSrc&F_INT_TRANSLATION_FAULT)
        {
            unsigned int *faultPTE;
            unsigned int i=0;
            
            regval = M4U_ReadReg32(m4u_base, REG_MMU_INT_ID);
            portID = F_INT_ID_TF_PORT_ID(regval);
            larbID = F_INT_ID_TF_LARB_ID(regval) - 1;
			/*
			    2'b01 => larb0
			    2'b00 => GPU
			*/

            faultPTE = mva_pteAddr(faultMva);

            if(true == pmodule_in_freemva[m4u_port_2_module(portID)])
            {
                M4UMSG("%s in free mva!!!\n", m4u_get_module_name(m4u_port_2_module(portID)));
            }

#if 1
            M4UERR_WITH_OWNER("translation fault: larb=%d,port=%s, fault_mva=0x%x, mva_pteAddr: 0x%x\n", m4u_get_module_name(m4u_port_2_module(portID)), 
                larbID, m4u_get_port_name(larb_port_2_m4u_port(larbID, portID)), faultMva, faultPTE);				
#else
            M4UERR("translation fault: larb=%d,port=%s, fault_mva=0x%x, mva_pteAddr: 0x%x\n", 
                larbID, m4u_get_port_name(larb_port_2_m4u_port(larbID, portID)), faultMva, faultPTE);
#endif			

            M4UMSG(" Protect Buffer Content=====>:\n");
            for(i=0;i<4;i++)
            {
                M4UMSG(" 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x \n", 
                    *(pProtectVA+8*i+0), *(pProtectVA+8*i+1), 
                    *(pProtectVA+8*i+2), *(pProtectVA+8*i+3), 
                    *(pProtectVA+8*i+4), *(pProtectVA+8*i+5), 
                    *(pProtectVA+8*i+6), *(pProtectVA+8*i+7));
            }
        
            if(0 != m4u_tfcallback[m4u_port_2_module(portID)])
            {
                (*m4u_tfcallback[m4u_port_2_module(portID)])();
            }
		
        } 
        if(IntrSrc&F_INT_TLB_MULTI_HIT_FAULT)
        {            
            M4UERR("multi-hit error! \n");
        } 
        if(IntrSrc&F_INT_INVALID_PHYSICAL_ADDRESS_FAULT)
        {
            if(!(IntrSrc&F_INT_TRANSLATION_FAULT))
            {
                regval = M4U_ReadReg32(m4u_base, REG_MMU_INT_ID);
                portID = F_INT_ID_TF_PORT_ID(regval);
                larbID = F_INT_ID_TF_LARB_ID(regval) - 1;
			
                m4u_aee_print("invalid PA:0x%x->0x%x, port: %s\n", faultMva, M4U_ReadReg32(m4u_base, REG_MMU_INVLD_PA), m4u_get_port_name(larb_port_2_m4u_port(larbID, portID))); 

                if(0 != m4u_tfcallback[m4u_port_2_module(portID)])
                {
                    (*m4u_tfcallback[m4u_port_2_module(portID)])();
                }				
            }

            M4UMSG("error: Invalid physical address fault! invalid PA:0x%x->0x%x\n", faultMva, M4U_ReadReg32(m4u_base, REG_MMU_INVLD_PA));
        } 
        if(IntrSrc&F_INT_ENTRY_REPLACEMENT_FAULT)
        {
            unsigned int i=0;  
            unsigned char lock_cnt[M4U_CLNTMOD_MAX] = {0};
            M4UERR("error: Entry replacement fault! No free TLB, TLB are locked by: ");
            for(i=0;i<M4U_MAIN_TLB_NR;i++)
            {
               lock_cnt[mva2module(M4U_ReadReg32(m4u_base, REG_MMU_MAIN_TAG(i))&(~0xfff))]++;
            } 
            for(i=0;i<M4U_CLNTMOD_MAX;i++)
            {
               if(0!=lock_cnt[i])
               {
                   M4UMSG("%s(lock=%d), ", m4u_get_module_name(i), lock_cnt[i]);	
               }
            }   
            M4UMSG("\n");       
        } 
        if(IntrSrc&F_INT_TABLE_WALK_FAULT)
        {
            M4UERR("error:  Table walk fault! pageTable start addr:0x%x, 0x%x\n", pt_pa_nonsec, pt_pa_sec);        	    
        } 
        if(IntrSrc&F_INT_TLB_MISS_FAULT)
        {
            M4UERR("error:  TLB miss fault! \n");        	  	
        } 
        if(IntrSrc&F_INT_PFH_DMA_FIFO_OVERFLOW)
        {
            M4UMSG("Prefetch DMA fifo overflow fault! \n");
        } 
        if(IntrSrc&F_INT_MISS_DMA_FIFO_OVERFLOW)
        {
            M4UERR("error:  DMA fifo overflow fault! \n");
        } 

        
        // reset TLB for TLB-related error 
        if((IntrSrc&F_INT_ENTRY_REPLACEMENT_FAULT) ||
        	 (IntrSrc&F_INT_TLB_MULTI_HIT_FAULT)      )
        {
            m4u_invalid_tlb_all();            	  
        }        

        //diable error hang
        //m4u_enable_error_hang(false);
        m4u_clear_intr(m4u_base);   

        // TODO: DON'T NEED TO ENABLE? cloud
    }
    
    return IRQ_HANDLED;
}


unsigned int SMI_reg_init(void)
{
    return 0;
}

int L1_CACHE_SYNC_BY_RANGE_ONLY = 1;

#ifdef M4U_CMD_DEBUG
struct dentry *gM4Udebugfs = NULL;
static char m4u_dbg_buf[512];
static char m4u_cmd_buf[512];
static char M4U_HELP[] =
    "\n"
    "USAGE\n"
    "        echo [ACTION]... > m4u\n"
    "\n"
    "ACTION\n"
    "        mmprofile:[1]\n"
    "             enable mmprofile\n"
    "        log:[0|1]\n"
    "             disable/enable more debug log\n"    
    "        monitor:[0|1]\n"
    "             stop/start monitor\n"     
    "        cache:[0|1]\n"
    "             0: force enalbe all cache flush\n"         
    "             1: force enalbe range cache flush\n"             
    "        test_level:[0|1|2]\n"
    "             0: Set level to user\n"         
    "             1: Set level to eng\n"      
    "             2: Set level to stress\n"          
    "        dump:[#]\n"        
    "             1: dump mva info\n"      
    "             2: dump m4u register\n"       
    "             3: dump seq range\n"    
    "             4: dump wrap range\n"    
    "             5: dump main tlb des\n"    
    "             6: dump pfh tlb des\n"        
    "             7: dump active port\n"   
    "        read_reg:[#]\n"
    "             #: hexadecimal\n"
    "             read register\n"
    "\n";


static void process_dbg_opt(const char *opt)
{
    char *buf = m4u_dbg_buf + strlen(m4u_dbg_buf);
    if (0 == strncmp(opt, "mmprofile:", 10))
    {
        char *p = (char *)opt + 10;
        unsigned int enable = (unsigned int) simple_strtoul(p, &p, 10);
        if (enable)
        {
            m4u_profile_init();
            sprintf(buf, "MMProfile: 1\n");
        }
    }
    else if (0 == strncmp(opt, "log:", 4))
    {
        char *p = (char *)opt + 4;
        unsigned int enable = (unsigned int) simple_strtoul(p, &p, 10);
        if (enable)
            m4u_log_on();
		else
			m4u_log_off();			

        sprintf(buf, "gM4uLogFlag: %d\n", gM4uLogFlag);
    }
    else if (0 == strncmp(opt, "monitor:", 8))
    {
        char *p = (char *)opt + 8;
        unsigned int enable = (unsigned int) simple_strtoul(p, &p, 10);
        if (enable)
            m4u_monitor_start();
		else
			m4u_monitor_stop();			

        sprintf(buf, "Monitor: %d\n", enable);
    }	
    else if (0 == strncmp(opt, "cache:", 6))
    {
        char *p = (char *)opt + 6;
        unsigned int enable = (unsigned int) simple_strtoul(p, &p, 10);
        if (enable)
        {
            L1_CACHE_SYNC_BY_RANGE_ONLY = 1;        
            sprintf(buf, "Cache flush by range only\n");		        
        }
        else
        {
            L1_CACHE_SYNC_BY_RANGE_ONLY = 0;
            sprintf(buf, "Enable cache flush all\n");        			
        }

    }	
    else if (0 == strncmp(opt, "test_level:", 11))
    {
        char *p = (char *)opt + 11;
        unsigned int enable = (unsigned int) simple_strtoul(p, &p, 10);
        if (M4U_TEST_LEVEL_USER == enable)
        {
            gTestLevel = M4U_TEST_LEVEL_USER;        
            sprintf(buf, "Set level to user\n");		        
        }
        else if (M4U_TEST_LEVEL_ENG == enable)
        {
            gTestLevel = M4U_TEST_LEVEL_ENG;
            sprintf(buf, "Set level to eng\n");        			
        }
		else if(M4U_TEST_LEVEL_STRESS == enable)
        {
            gTestLevel = M4U_TEST_LEVEL_STRESS;
            sprintf(buf, "Set level to stress\n");        			
        }
    }	
    else if (0 == strncmp(opt, "dump:", 5))
    {
        char *p = (char *)opt + 5;
        unsigned int enable = (unsigned int) simple_strtoul(p, &p, 10);
        if (1 == enable)
        {
			M4UMSG("cmd dump mva info\n");		
			m4u_dump_mva_info();
        }    
        else if (2 == enable)
        {
			M4UMSG("cmd dump register\n");		
			m4u_dump_reg();
        }   		
        else if (3 == enable)
        {
			M4UMSG("cmd dump seq range\n");		
			m4u_dump_seq_range_info();
        }   		
        else if (4 == enable)
        {
			M4UMSG("cmd dump wrap range\n");		
			m4u_dump_wrap_range_info();
        }   
        else if (5 == enable)
        {
			M4UMSG("cmd dump main tlb des\n");		
            m4u_dump_main_tlb_des();
        }  
        else if (6 == enable)
        {
			M4UMSG("cmd dump pfh tlb des\n");		
			m4u_dump_pfh_tlb_des();
        }  		
        else if (7 == enable)
        {
			M4UMSG("cmd dump active port\n");		
			m4u_print_active_port();
        } 

    }
    else if (0 == strncmp(opt, "read_reg:", 9))
    {
        char *p = (char *)opt + 9;
        unsigned int addr = (unsigned int) simple_strtoul(p, &p, 16);
        {
            M4UMSG("register 0x%x: 0x:%x\n", addr,  COM_ReadReg32(addr));		
        }  
    }
    else
    {
        M4UMSG("[M4U VERSION] %s %s\n", __DATE__, __TIME__);    
    }
	
}

static void process_dbg_cmd(char *cmd)
{
    char *tok;
    
    M4UMSG("[m4u] %s\n", cmd);
    
    while ((tok = strsep(&cmd, " ")) != NULL)
    {
        process_dbg_opt(tok);
    }
}


static ssize_t m4u_debug_read(struct file *file,
                          char __user *ubuf, size_t count, loff_t *ppos)
{
    if (strlen(m4u_dbg_buf))
        return simple_read_from_buffer(ubuf, count, ppos, m4u_dbg_buf, strlen(m4u_dbg_buf));
    else
        return simple_read_from_buffer(ubuf, count, ppos, M4U_HELP, strlen(M4U_HELP));
        
}

static ssize_t m4u_debug_write(struct file *file,
                           const char __user *ubuf, size_t count, loff_t *ppos)
{
    const int debug_bufmax = sizeof(m4u_cmd_buf) - 1;
    size_t ret;
  
    ret = count;
    
    if (count > debug_bufmax) 
        count = debug_bufmax;
    
    if (copy_from_user(&m4u_cmd_buf, ubuf, count))
        return -EFAULT;
    
    m4u_cmd_buf[count] = 0;

    process_dbg_cmd(m4u_cmd_buf);

    return ret;
}



static struct file_operations m4u_debug_fops = {
    .read  = m4u_debug_read,
    .write = m4u_debug_write,
};

#endif

#ifdef M4U_PROC_DEBUG
static int m4u_proc_show(struct seq_file *seq, void *v)
{

    unsigned int *pteStart;
    int i;	
    short index=1, nr=0;
    unsigned int addr=0;	
	
    unsigned int regval;	

    M4UMSG("m4u_proc_read\n");

    seq_printf(seq, "\n********** dump active ports *********\n");	
    {
        for(i=0;i<M4U_PORT_NR;i++)
        {            
            regval = m4uHw_get_field_by_mask(0, REG_SMI_SECUR_CON_OF_PORT(i), F_SMI_SECUR_CON_VIRTUAL(i));
            if(regval)
            {
                seq_printf(seq, "%s\n", m4u_get_port_name(i));
            }
        }
    }		


    seq_printf(seq, "\n********** dump mva allocated info *********\n");
    seq_printf(seq, "module	 mva_start	mva_end  block_num	legal_size\n");

    for(index=1; index<MVA_MAX_BLOCK_NR+1; index += nr)
    {
        addr = index << MVA_BLOCK_SIZE_ORDER;
        nr = MVA_GET_NR(index);
        if(MVA_IS_BUSY(index))
        {			 
#ifndef M4U_LOG_ALLOCATE_SIZE        
            seq_printf(seq, "%s, 0x%-8x, 0x%-8x, %d	\n", 
            m4u_get_module_name(moduleGraph[index]), addr, addr+nr*MVA_BLOCK_SIZE, nr);
#else
            seq_printf(seq, "%s, 0x%-8x, 0x%-8x, %d,  %d  \n", 
            m4u_get_module_name(moduleGraph[index]), addr, addr+nr*MVA_BLOCK_SIZE, nr, mvasizeGraph[index]);			
#endif			
        }
    }

    seq_printf(seq, "\n********** dump pagetable *********\n");

    pteStart = pPT_nonsec;

    for(i=0; i<PT_TOTAL_ENTRY_NUM; i++)
    {
        if(i%8==0)
        {
        	 seq_printf(seq, "\n 0x%08x: ", (i<<12));
        }    
        seq_printf(seq, "0x%08x, ", pteStart[i]);
    }
	
	return 0;
}

static int m4u_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, m4u_proc_show, inode->i_private);
}

static const struct file_operations M4U_proc_fops = {
	.open		= m4u_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release
};

#endif

void M4U_debug_init(void)
{
    struct proc_dir_entry *entry;
    struct proc_dir_entry *m4u_dir;

#ifdef M4U_CMD_DEBUG
    gM4Udebugfs = debugfs_create_file("m4u",S_IFREG|S_IRUGO, NULL, (void *)0, &m4u_debug_fops);	
#endif	


#ifdef M4U_PROC_DEBUG
    m4u_dir = proc_mkdir("m4u", NULL);

    if (!m4u_dir)
    {
	    M4UMSG("[%s]: fail to mkdir /proc/m4u\n", __func__);
    
        return;
    }

    entry = proc_create("mva", 0444, m4u_dir, &M4U_proc_fops);
    if (!entry)
        return;

#endif

}


void M4U_debug_exit(void)
{
#ifdef M4U_CMD_DEBUG
    debugfs_remove(gM4Udebugfs);
#endif	
}

#if 0
static struct class *pM4uClass = NULL;
static struct device* m4uDevice = NULL;
#endif

/*****************************************************************************
 * FUNCTION
 *    m4u_probe
 * DESCRIPTION
 *    1. Register M4U Device Number.
 *    2. Allocate and Initial M4U cdev struct.
 *    3. Add M4U device to kernel. (call cdev_add)
 *    4. Initial internal variable. (call m4u_struct_init)
 *    5. Initial mva allocation management array (m4u_mvaGraph_init)
 *    6. Setup IRQ
 * PARAMETERS
 *	  param1 : [IN] struct platform_device *pdev
 *				  No used in this function. 
 * RETURNS
 *    Type: Integer. always zero.
 ****************************************************************************/
static int m4u_probe(struct platform_device *pdev)
{


    struct proc_dir_entry *m4u_entry;

    M4UMSG("MTK_M4U_Init\n");

#if 0
    ret = register_chrdev_region(g_MTKM4Udevno, 1, M4U_DEVNAME);	
    if(ret)
        M4UMSG("error: can't get major number for m4u device\n");
    else
        M4UMSG("Get M4U Device Major number (%d)\n", ret);

    g_pMTKM4U_CharDrv = cdev_alloc();
    g_pMTKM4U_CharDrv->owner = THIS_MODULE;
    g_pMTKM4U_CharDrv->ops = &g_stMTK_M4U_fops;
    ret = cdev_add(g_pMTKM4U_CharDrv, g_MTKM4Udevno, 1);	

    //create /dev/M4U_device automaticly
    pM4uClass = class_create(THIS_MODULE, M4U_DEVNAME);
    if (IS_ERR(pM4uClass)) {
        int ret = PTR_ERR(pM4uClass);
        M4UMSG("Unable to create class, err = %d", ret);
        return ret;
    }
    m4uDevice = device_create(pM4uClass, NULL, g_MTKM4Udevno, NULL, M4U_DEVNAME);
#else
    m4u_entry = create_proc_entry("M4U_device", 0, NULL);
    if(m4u_entry)
    {
    	m4u_entry -> proc_fops = &g_stMTK_M4U_fops;
    }
#endif

    M4U_debug_init();

    pmodule_current_size = (int*)kmalloc(M4U_CLIENT_MODULE_NUM*4, GFP_KERNEL|__GFP_ZERO);
    pmodule_max_size = (int*)kmalloc(M4U_CLIENT_MODULE_NUM*4, GFP_KERNEL|__GFP_ZERO);
    pmodule_locked_pages = (int*)kmalloc(M4U_CLIENT_MODULE_NUM*4, GFP_KERNEL|__GFP_ZERO);
    pmodule_in_freemva = (bool*)kmalloc(M4U_CLIENT_MODULE_NUM, GFP_KERNEL|__GFP_ZERO);

    m4u_struct_init(); //init related structures

    m4u_mvaGraph_init();
        
    // add SMI reg init here
    SMI_reg_init();
    
    //m4u_dump_reg();
    m4u_memory_usage(); 

    //m4u_print_active_port();
      
    spin_lock_init(&gM4u_reg_lock);
    
  
    //Set IRQ   
    if(request_irq(MT_M4U0_IRQ_ID , MTK_M4U_isr, IRQF_TRIGGER_LOW, M4U_DEVNAME , NULL))
    {
        M4UERR("request M4U0 IRQ line failed\n");
        return -ENODEV;
    }
    disable_irq(MT_M4U_SEC_IRQ_ID);

    M4UMSG("init done\n");

    memset(m4u_tfcallback, 0, sizeof(PFN_TF_T)*M4U_CLNTMOD_MAX);

    // m4u_profile_init();
    m4u_cache_sync_init();    

// M4U UT-----------------------------------
#if 0
{
	M4U_PORT_STRUCT M4uPort;

    unsigned int BufSize[2] = {5*1024, 512*1024};
    unsigned int * BufAddr[2] = {NULL, NULL};
    unsigned int BufMVA[2];
	unsigned int pa_BufAddr = NULL;
	unsigned int Invalid_Start = 0;
	unsigned int Invalid_End = 0;
	
    //m4u_perf_timer_on();

    M4uPort.ePortID = M4U_PORT_MDP_ROTO;
    M4uPort.Virtuality = 1;						   
    M4uPort.Security = 0;
    M4uPort.domain = 0;
    M4uPort.Distance = 3;
    M4uPort.Direction = 0;

    m4u_config_port(&M4uPort);

	BufAddr[0] = kmalloc(BufSize[0], GFP_KERNEL|__GFP_ZERO);
    memset(BufAddr[0], 20, 16);
    memset(BufAddr[0]+16, 21, 16);	
    memset(BufAddr[0]+16*2, 22, 16);	
    memset(BufAddr[0]+16*3, 23, 16);		

    m4u_alloc_mva(M4U_CLNTMOD_DISP,     // Module ID
                       (const unsigned int)BufAddr[0],              // buffer virtual start address
                       BufSize[0],              // buffer size
                       0,
                       0,
                       &BufMVA[0]);

	M4UDBG("kmalloc after m4u_alloc_mva(), BufMVA=0x%x \r\n", BufMVA[0]);


    BufAddr[1] = vmalloc(BufSize[1]);
    memset(BufAddr[1], 30, 16);
    memset(BufAddr[1]+16, 31, 16);	
    memset(BufAddr[1]+16*2, 32, 16);	
    memset(BufAddr[1]+16*3, 33, 16);	

    m4u_alloc_mva(M4U_CLNTMOD_VIDEO,     // Module ID
                       (const unsigned int)BufAddr[1],              // buffer virtual start address
                       BufSize[1],              // buffer size
                       0,
                       0,
                       &BufMVA[1]);

    M4UDBG("vmalloc after m4u_alloc_mva(), BufMVA=0x%x \r\n", BufMVA[1]);	

    m4u_memory_usage();
    m4u_print_active_port();
    m4u_dump_pagetable(M4U_CLNTMOD_DISP);
    m4u_dump_pagetable(M4U_CLNTMOD_VIDEO);	
    m4u_mvaGraph_dump();


	m4u_dealloc_mva(M4U_CLNTMOD_DISP,BufAddr[0], BufSize[0],BufMVA[0]);

    m4u_memory_usage();
    m4u_mvaGraph_dump();

    BufSize[0] =128;
	BufAddr[0] = vmalloc(BufSize[0]);
    m4u_alloc_mva(M4U_CLNTMOD_CMDQ,     // Module ID
                       (const unsigned int)BufAddr[0],              // buffer virtual start address
                       BufSize[0],              // buffer size
                       0,
                       0,
                       &BufMVA[0]);	
	

    m4u_memory_usage();
	m4u_mvaGraph_dump();

    m4u_manual_insert_entry(M4U_PORT_CMDQ,BufMVA[0], 0, 1);
    m4u_manual_insert_entry(M4U_PORT_VENCMC,BufMVA[1], 0, 1);	
    m4u_manual_insert_entry(M4U_PORT_VENCMC,BufMVA[1]+4*1024, 0, 0);	

    m4u_dump_main_tlb_des();	

    m4u_insert_seq_range(M4U_CLNTMOD_VIDEO, BufMVA[1], BufMVA[1] + 512*1024, 2);

    //m4u_insert_seq_range(M4U_CLNTMOD_VIDEO, BufMVA[1]+2*4*1024, BufMVA[1] + 3*4*1024, 3);
    m4u_insert_seq_range(M4U_CLNTMOD_CMDQ, BufMVA[0], BufMVA[0] + 128, 3);

    //m4u_invalid_seq_all(M4U_CLNTMOD_VIDEO);	
    m4u_invalid_seq_range(M4U_CLNTMOD_VIDEO, BufMVA[1], BufMVA[1]+ 512*1024);

    m4u_insert_wrapped_range(M4U_CLNTMOD_VIDEO,M4U_PORT_VENCMC,BufMVA[1],BufMVA[1]+256);
    m4u_dump_wrap_range_info();
	
    //m4u_invalid_wrapped_range(M4U_CLNTMOD_VIDEO,M4U_PORT_VENCMC,BufMVA[1],BufMVA[1]+256*1024-1);
	//m4u_dump_wrap_range_info();
	
    m4u_insert_wrapped_range(M4U_CLNTMOD_VIDEO,M4U_PORT_VENCMC,BufMVA[1],BufMVA[1]+512*1024-1);	
	m4u_dump_wrap_range_info();

    //m4u_invalid_tlb_all();
    //m4u_invalid_tlb_by_range(Invalid_Start, Invalid_End);
    m4u_invalidate_and_check(BufMVA[1], BufMVA[1]+4*1024);
    

    m4u_dump_main_tlb_des();		

    M4UDBG(" finish...");
}
#endif
//------------------------------------------
    return 0;


}


/*****************************************************************************
 * FUNCTION
 *    m4u_remove
 * DESCRIPTION
 *    1. Remove M4U device.
 *    2. Un-register M4U Device Number.
 *    3. Free IRQ
 * PARAMETERS
 *	  param1 : [IN] struct platform_device *pdev
 *				  No used in this function. 
 * RETURNS
 *    Type: Integer. always zero.
 ****************************************************************************/
static int m4u_remove(struct platform_device *pdev)
{
    M4ULOG("MT6572_M4U_Exit() \n");
    
    cdev_del(g_pMTKM4U_CharDrv);
    unregister_chrdev_region(g_MTKM4Udevno, 1);

    //Release IRQ
    free_irq(MT_M4U0_IRQ_ID , NULL);

    //memory allocated in probe function (including m4u_struct_init and m4u_hw_init) don't free.

    M4U_debug_exit();

    return 0;
}

/*****************************************************************************
 * FUNCTION
 *    m4u_dump_reg
 * DESCRIPTION
 *    Dump all m4u register
 * PARAMETERS
 *    None.
 * RETURNS
 *    Type: Integer. always zero.
 ****************************************************************************/
int m4u_dump_reg(void) 
{
    // M4U related
    unsigned int i=0;
    unsigned int m4u_base = M4U_BASE;
    
    M4UMSG(" M4U Register Start ======= \n");
    for(i=0;i<M4U_REG_SIZE/8;i+=4)
    {
    	M4UMSG("+0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x \n", 8*i, 
    	M4U_ReadReg32(m4u_base, 8*i + 4*0), M4U_ReadReg32(m4u_base, 8*i + 4*1),
    	M4U_ReadReg32(m4u_base, 8*i + 4*2), M4U_ReadReg32(m4u_base, 8*i + 4*3),
    	M4U_ReadReg32(m4u_base, 8*i + 4*4), M4U_ReadReg32(m4u_base, 8*i + 4*5),
    	M4U_ReadReg32(m4u_base, 8*i + 4*6), M4U_ReadReg32(m4u_base, 8*i + 4*7));
    }
    M4UMSG(" M4U Register End ========== \n");
    
    //M4UMSG(" SMI Register End====== \n"); 	
    
    return 0;
}


/*****************************************************************************
* FUNCTION
*	 m4u_confirm_range_invalidated
* DESCRIPTION
*	 TLB invalidate check by range. 
* PARAMETERS
*	 param1 : [IN] const unsigned int MVAStart
*				 Start invalid mva  
*	 param2 : [IN] const unsigned int MVAEnd
*				 End invalid mva. 
* RETURNS
*	 Integer. zero mean success and others mean fail.
****************************************************************************/
static int m4u_confirm_range_invalidated(const unsigned int MVAStart, const unsigned int MVAEnd)
{
    unsigned int i;
    unsigned int regval;
    unsigned int m4u_base = gM4UBaseAddr[0];
    int result = 0;
   

    if(gTestLevel==M4U_TEST_LEVEL_USER)
    {
    	  return 0;    	  
    }    

    ///> check Main TLB part
    MMProfileLogEx(M4U_MMP_Events[PROFILE_MAIN_TLB_MON], MMProfileFlagStart, MVAStart, MVAEnd);    
    for(i=0;i<M4U_MAIN_TLB_NR;i++)
    {
        regval = M4U_ReadReg32(m4u_base, REG_MMU_MAIN_TAG(i));
        
        if(regval & (F_MAIN_TLB_VALID_BIT))
        {
            unsigned int mva = regval & F_MAIN_TLB_VA_MSK;
            unsigned int sa_align = MVAStart & F_MAIN_TLB_VA_MSK;
            unsigned int ea_align = MVAEnd & F_MAIN_TLB_VA_MSK;

            if(mva>=sa_align && mva<=ea_align)
            {
                if(gTestLevel==M4U_TEST_LEVEL_STRESS)
                {
                    M4UERR("m4u_confirm_range_invalidated fail main: i=%d, MVAStart=0x%x, MVAEnd=0x%x, RegValue=0x%x\n",
                        i, MVAStart, MVAEnd, regval);
                    m4u_dump_reg(); 
                }
                else if(gTestLevel==M4U_TEST_LEVEL_ENG)
                {
                    M4UMSG("m4u_confirm_range_invalidated fail main: i=%d, MVAStart=0x%x, MVAEnd=0x%x, RegValue=0x%x \n",
                        i, MVAStart, MVAEnd, regval);
                }
                result = -1;
            }
        }
    }
    MMProfileLogEx(M4U_MMP_Events[PROFILE_MAIN_TLB_MON], MMProfileFlagEnd, MVAStart, MVAEnd);   


    if(result < 0)
        return result;

    ///> check Prefetch TLB part
    MMProfileLogEx(M4U_MMP_Events[PROFILE_PREF_TLB_MON], MMProfileFlagStart, MVAStart, MVAEnd);       
    for(i=0;i<M4U_PRE_TLB_NR;i++)
    {
        regval = M4U_ReadReg32(m4u_base, REG_MMU_PFH_TAG(i));
        
        if(regval & F_PFH_TAG_VALID_MSK)  ///> a valid Prefetch TLB entry
        {

            unsigned int mva = regval & F_PFH_TAG_VA_MSK;
            unsigned int sa_align = MVAStart& F_PFH_TAG_VA_MSK;
            unsigned int ea_align = MVAEnd & F_PFH_TAG_VA_MSK;
            
            if(mva>=sa_align && mva<=ea_align)
            {
                if(gTestLevel==M4U_TEST_LEVEL_STRESS)
                {
                    M4UERR("m4u_confirm_range_invalidated fail prefetch: i=%d, MVAStart=0x%x, MVAEnd=0x%x, RegValue=0x%x\n",
                        i, MVAStart, MVAEnd, regval);
                    m4u_dump_reg(); 
                }
                else if(gTestLevel==M4U_TEST_LEVEL_ENG)
                {
                    M4UMSG("m4u_confirm_range_invalidated fail prefetch: i=%d, MVAStart=0x%x, MVAEnd=0x%x, RegValue=0x%x\n",
                        i, MVAStart, MVAEnd, regval);
                }
                result = -1;
            }
            
        }
    }
    MMProfileLogEx(M4U_MMP_Events[PROFILE_PREF_TLB_MON], MMProfileFlagEnd, MVAStart, MVAEnd);       	
    
    return result;
}


 /*****************************************************************************
 * FUNCTION
 *    m4u_get_main_descriptor
 * DESCRIPTION
 *    Get main TLB descriptor implement. 
 * PARAMETERS
 *    param1 : [IN]  const unsigned int m4u_base
 *                M4U base register.  
 *    param2 : [IN]  const unsigned int idx
 *                Main TLB index. 
 * RETURNS
 *    Unsigned Integer. Target descriptor.
 ****************************************************************************/
static unsigned int m4u_get_main_descriptor(const unsigned int m4u_base, const unsigned int idx)
{
    unsigned int regValue=0;
    regValue = F_READ_ENTRY_TLB_SEL_MAIN \
             | F_READ_ENTRY_INDEX_VAL(idx)\
             | F_READ_ENTRY_READ_EN_BIT;
    
    M4U_WriteReg32(m4u_base, REG_MMU_READ_ENTRY, regValue);

    //M4UMSG("m4u_get_main_descriptor: index=%d, REG_MMU_READ_ENTRY = 0x%x\n", idx, regValue);
	
    return M4U_ReadReg32(m4u_base, REG_MMU_DES_RDATA);
}

/*****************************************************************************
* FUNCTION
*	 m4u_get_pfh_descriptor
* DESCRIPTION
*	 Get prefetch TLB descriptor implement. 
* PARAMETERS
*	 param1 : [IN] const unsigned int m4u_base
*				 M4U base register.  
*	 param2 : [IN] const int tlbIndex
*				 Pefetch TLB index. 
*	 param3 : [IN] const int tlbSelect
*				 Selected index in prefetch TLB entry . 
* RETURNS
*	 Unsigned Integer. Target descriptor.
****************************************************************************/
static unsigned int m4u_get_pfh_descriptor(const unsigned int m4u_base, const int tlbIndex, const int tlbSelect)
{
    unsigned regValue=0; 
    regValue = F_READ_ENTRY_TLB_SEL_PFH \
             | F_READ_ENTRY_INDEX_VAL(tlbIndex)\
             | F_READ_ENTRY_PFH_IDX(tlbSelect)\
             | F_READ_ENTRY_READ_EN_BIT;
   
    M4U_WriteReg32(m4u_base, REG_MMU_READ_ENTRY, regValue);
    return M4U_ReadReg32(m4u_base, REG_MMU_DES_RDATA);
}

/*****************************************************************************
 * FUNCTION
 *    m4u_dump_main_tlb_tags
 * DESCRIPTION
 *    Dump main TLB tags. 
 * PARAMETERS
 *    None.
 * RETURNS
 *    None.
 ****************************************************************************/
static void m4u_dump_main_tlb_tags(void) 
{
    unsigned int i=0;
    unsigned int m4u_base = gM4UBaseAddr[0];
    M4UMSG("dump main tlb tags=======>\n");
    for(i=0;i<M4U_MAIN_TLB_NR;i++)
    {
        M4UMSG("0x%x  ", M4U_ReadReg32(m4u_base, REG_MMU_MAIN_TAG(i)));
        
        if((i+1)%4==0)
            M4UMSG("\n");
    }    
}
#if 0
/*****************************************************************************
 * FUNCTION
 *    m4u_dump_pfh_tlb_tags
 * DESCRIPTION
 *    Dump prefetch TLB tags. 
 * PARAMETERS
 *    None.
 * RETURNS
 *    None.
 ****************************************************************************/
static void m4u_dump_pfh_tlb_tags(void)
{
    unsigned int i=0;
    unsigned int m4u_base = gM4UBaseAddr[0];
    M4UMSG("dump pfh tags=======>\n");
    for(i=0;i<M4U_PRE_TLB_NR;i++)
    {
        M4UMSG("0x%x ", M4U_ReadReg32(m4u_base, REG_MMU_PFH_TAG(i)));
        if((i+1)%4==0)
            M4UMSG("\n");
    }
    M4UMSG("\n");
}
#endif

/*****************************************************************************
 * FUNCTION
 *    m4u_dump_main_tlb_des
 * DESCRIPTION
 *    Dump main TLB descriptor. 
 * PARAMETERS
 *    None.
 * RETURNS
 *    Type: Integer. always zero.
 ****************************************************************************/
int m4u_dump_main_tlb_des() 
{
    unsigned int i=0;
    unsigned int m4u_base = gM4UBaseAddr[0];
    M4UMSG("dump main tlb descriptor ====>\n");
    for(i=0;i<M4U_MAIN_TLB_NR;i++)
    {
        M4UMSG("%d:  0x%x:  0x%x\n", i,
            M4U_ReadReg32(m4u_base, REG_MMU_MAIN_TAG(i)),
            m4u_get_main_descriptor(m4u_base,i));
        
    }
    
    return 0;
}

/*****************************************************************************
 * FUNCTION
 *    m4u_dump_pfh_tlb_des
 * DESCRIPTION
 *    Dump prefetch TLB descriptor. 
 * PARAMETERS
 *    None.
 * RETURNS
 *    Type: Integer. always zero.
 ****************************************************************************/
int m4u_dump_pfh_tlb_des() 
{
    unsigned int i=0;
    unsigned int m4u_base = gM4UBaseAddr[0];
    M4UMSG("dump pfh tlb descriptor ====>\n");
    for(i=0;i<M4U_PRE_TLB_NR;i++)
    {
        M4UMSG("%d:0x%x:0x%x,0x%x \n", i,
            M4U_ReadReg32(m4u_base, REG_MMU_PFH_TAG(i)),
            m4u_get_pfh_descriptor(m4u_base,i, 0),
            m4u_get_pfh_descriptor(m4u_base,i, 1)
            );
    }
    
    return 0;
}


/*****************************************************************************
* FUNCTION
*	 m4u_dump_pagetable_range
* DESCRIPTION
*	 Dump M4U page table by range. 
* PARAMETERS
*	 param1 : [IN] unsigned int mvaStart
*				 Start dump page table mva.  
*	 param2 : [IN] const unsigned int nr
*				 Dump count. 
* RETURNS
*	 None.
****************************************************************************/
void m4u_dump_pagetable_range(unsigned int mvaStart, const unsigned int nr)
{
    unsigned int *pteStart;
    int i;

    pteStart = mva_pteAddr(mvaStart);
    mvaStart &= ~0xfff;

    M4UMSG("m4u dump pagetable by range: start=0x%x, nr=%d ==============>\n", mvaStart, nr);

    for(i=0; i<nr; i++)
    {
        if(i%8==0)
        {
        	 M4UMSG("\n 0x%08x: ", (mvaStart+(i<<12)));
        }    
        M4UMSG("0x%08x, ", pteStart[i]);
    }
    
    M4UMSG("\nm4u dump pagetable done==============<\n");
}


/*****************************************************************************
* FUNCTION
*	 m4u_dump_pagetable
* DESCRIPTION
*	 Dump M4U page table by Module ID.
* PARAMETERS
*	 param1 : [IN] const M4U_MODULE_ID_ENUM eModuleID
*				 Module ID.
* RETURNS
*	 None.
****************************************************************************/
void m4u_dump_pagetable(const M4U_MODULE_ID_ENUM eModuleID)
{
    unsigned int addr=0;
    short index=1, nr=0;
    
    M4UMSG("[M4U_K] dump pagetable by module: %s, page_num=%d ========>\n", 
        m4u_get_module_name(eModuleID), pmodule_locked_pages[eModuleID]);

//  this function may be called in ISR
//  spin_lock(&gMvaGraph_lock);
    for(index=1; index<MVA_MAX_BLOCK_NR+1; index += nr)
    {
        nr = MVA_GET_NR(index);    
        if(MVA_IS_BUSY(index) && moduleGraph[index] == eModuleID)
        {
            addr = index << MVA_BLOCK_SIZE_ORDER;        
            // M4UMSG("start a mva region for module %d===>\n", eModuleID);
            m4u_dump_pagetable_range(addr, ((nr<<MVA_BLOCK_SIZE_ORDER)>>12));
        }
    }
//  spin_unlock(&gMvaGraph_lock);
    M4UMSG("[M4U_K]  dump pagetable by module done =========================<\n");

}


/*****************************************************************************
* FUNCTION
*	 m4u_dump_pagetable_nearby
* DESCRIPTION
*	 Dump M4U page table by Module ID nearby user specified address.
* PARAMETERS
*	 param1 : [IN] const M4U_MODULE_ID_ENUM eModuleID
*				 Module ID.
* RETURNS
*	 None.
****************************************************************************/
void m4u_dump_pagetable_nearby(const M4U_MODULE_ID_ENUM eModuleID, const unsigned int mva_addr)
{
    unsigned int addr=0;
    short index=1, nr=0;
    
    M4UMSG("[M4U_K] dump pagetable by module: %s, page_num=%d ========>\n", 
        m4u_get_module_name(eModuleID), pmodule_locked_pages[eModuleID]);

//  this function may be called in ISR
//  spin_lock(&gMvaGraph_lock);
    for(index=1; index<MVA_MAX_BLOCK_NR+1; index += nr)
    {
        addr = index << MVA_BLOCK_SIZE_ORDER;
        nr = MVA_GET_NR(index);
        if(MVA_IS_BUSY(index) && 
		   moduleGraph[index] == eModuleID && 
		  ((mva_addr>=addr && mva_addr-addr<=2*DEFAULT_PAGE_SIZE)||
		   (mva_addr<=addr && addr-mva_addr<=2*DEFAULT_PAGE_SIZE))   )
        {
            // M4UMSG("start a mva region for module %d===>\n", eModuleID);
            m4u_dump_pagetable_range(addr, ((nr<<MVA_BLOCK_SIZE_ORDER)>>12));
        }
    }
//  spin_unlock(&gMvaGraph_lock);
    M4UMSG("[M4U_K]  dump pagetable by module done =========================<\n");

}

/*****************************************************************************
 * FUNCTION
 *    m4u_dump_mva_info
 * DESCRIPTION
 *    Dump all used mva block. 
 * PARAMETERS
 *    None.
 * RETURNS
 *    Type: Integer. always zero.
 ****************************************************************************/
int m4u_dump_mva_info()
{  
    short index=1, nr=0;
    unsigned int addr=0;
    
    M4UMSG(" dump mva allocated info =====>\n");
    M4UMSG("module   mva_start  mva_end  block_num  legal_size\n");
    for(index=1; index<MVA_MAX_BLOCK_NR+1; index += nr)
    {
        addr = index << MVA_BLOCK_SIZE_ORDER;
        nr = MVA_GET_NR(index);
        if(MVA_IS_BUSY(index))
        {            
#ifndef M4U_LOG_ALLOCATE_SIZE        
            M4UMSG("%s, 0x%-8x, 0x%-8x, %d  \n", 
                m4u_get_module_name(moduleGraph[index]), addr, addr+nr*MVA_BLOCK_SIZE, nr);
#else
            M4UMSG("%s, 0x%-8x, 0x%-8x, %d,  %d  \n", 
                m4u_get_module_name(moduleGraph[index]), addr, addr+nr*MVA_BLOCK_SIZE, nr, mvasizeGraph[index]);			
#endif			
        }
    }
    M4UMSG("<===== dump mva allocated info done\n");
    
    return 0;
}


int m4u_power_on()
{
    return 0;
}

int m4u_power_off()
{
    return 0;
}


int m4u_clock_on(void)
{	//MT6572, M4U and SMI infra clock follow MM MTCMOS
    return 0;
}

int m4u_clock_off(void)
{
    return 0;
}

/*****************************************************************************
 * FUNCTION
 *    m4u_mvaGraph_init
 * DESCRIPTION
 *    Initial Mva allocation management array. 
 * PARAMETERS
 *    None.
 * RETURNS
 *    None.
 ****************************************************************************/
static void m4u_mvaGraph_init(void)
{
    spin_lock(&gMvaGraph_lock);
    memset(mvaGraph, 0, sizeof(short)*(MVA_MAX_BLOCK_NR+1));
    memset(moduleGraph, 0, sizeof(unsigned char)*(MVA_MAX_BLOCK_NR+1));
    mvaGraph[0] = 1|MVA_BUSY_MASK;
    moduleGraph[0] = M4U_CLNTMOD_UNKNOWN;
    mvaGraph[1] = MVA_MAX_BLOCK_NR;
    moduleGraph[1] = M4U_CLNTMOD_UNKNOWN;
    mvaGraph[MVA_MAX_BLOCK_NR] = MVA_MAX_BLOCK_NR;
    moduleGraph[MVA_MAX_BLOCK_NR] = M4U_CLNTMOD_UNKNOWN;
    
    spin_unlock(&gMvaGraph_lock);
}

/*****************************************************************************
 * FUNCTION
 *    m4u_mvaGraph_dump_raw
 * DESCRIPTION
 *    Dump Mva allocation management array. 
 * PARAMETERS
 *    None.
 * RETURNS
 *    None.
 ****************************************************************************/
void m4u_mvaGraph_dump_raw(void)
{
    int i;
    spin_lock(&gMvaGraph_lock);
    M4UMSG("[M4U_K] dump raw data of mvaGraph:============>\n");
    for(i=0; i<MVA_MAX_BLOCK_NR+1; i++)
        M4UMSG("0x%4x: 0x%08x   ID:%d\n", i, mvaGraph[i], moduleGraph[i]); 
    spin_unlock(&gMvaGraph_lock);
}

/*****************************************************************************
 * FUNCTION
 *    m4u_mvaGraph_dump
 * DESCRIPTION
 *  Dump Mva allocation management array information. 
 *    1. used and free block by user.
 *    2. free count by order
 * PARAMETERS
 *    None.
 * RETURNS
 *    None.
 ****************************************************************************/
void m4u_mvaGraph_dump(void)
{
    unsigned int addr=0, size=0;
    int index, nr=0;
    M4U_MODULE_ID_ENUM moduleID;
    char *pMvaFree = "FREE";
    char *pOwner = NULL;
    int i,max_bit;    
    short frag[12] = {0};
    short nr_free=0, nr_alloc=0;
    
    M4UMSG("[M4U_K] mva allocation info dump:====================>\n");
    M4UMSG("start      size     blocknum    owner       \n");

    spin_lock(&gMvaGraph_lock);
    for(index=0; index<MVA_MAX_BLOCK_NR+1; index += nr)
    {
        addr = index << MVA_BLOCK_SIZE_ORDER;
        nr = MVA_GET_NR(index);
        size = nr << MVA_BLOCK_SIZE_ORDER;
        if(MVA_IS_BUSY(index))
        {
            moduleID = (M4U_MODULE_ID_ENUM)moduleGraph[index];
            pOwner = m4u_get_module_name(moduleID);
            nr_alloc += nr;
        }
        else    // mva region is free
        {
            pOwner = pMvaFree;
            nr_free += nr;

            max_bit=0;
            for(i=0; i<12; i++)
            {
                if(nr & (1<<i))
                    max_bit = i;
            }
            frag[max_bit]++; 
        }

        M4UMSG("0x%08x  0x%08x  %4d    %s\n", addr, size, nr, pOwner);

     }

    spin_unlock(&gMvaGraph_lock);

    M4UMSG("\n");
    M4UMSG("[M4U_K] mva alloc summary: (unit: blocks)========================>\n");
    M4UMSG("free: %d , alloc: %d, total: %d \n", nr_free, nr_alloc, nr_free+nr_alloc);
    M4UMSG("[M4U_K] free region fragments in 2^x blocks unit:===============\n");
    M4UMSG("  0     1     2     3     4     5     6     7     8     9     10    11    \n");
    M4UMSG("%4d  %4d  %4d  %4d  %4d  %4d  %4d  %4d  %4d  %4d  %4d  %4d  \n",
        frag[0],frag[1],frag[2],frag[3],frag[4],frag[5],frag[6],frag[7],frag[8],frag[9],frag[10],frag[11]);
    M4UMSG("[M4U_K] mva alloc dump done=========================<\n");
    
}


/*****************************************************************************
 * FUNCTION
 *    mva2module
 * DESCRIPTION
 *    Get Module ID from a mva.
 * PARAMETERS
 *    param1 : [IN]  const unsigned int mva
 *                Query mva.  
 * RETURNS
 *    Type: M4U_MODULE_ID_ENUM. Module ID.
 ****************************************************************************/
static M4U_MODULE_ID_ENUM mva2module(const unsigned int mva)
{

    M4U_MODULE_ID_ENUM eModuleId = M4U_CLNTMOD_UNKNOWN;
    int index;

    index = MVAGRAPH_INDEX(mva);
    if(index==0 || index>MVA_MAX_BLOCK_NR)
    {
        M4UMSG("mvaGraph index is 0. mva=0x%x\n", mva);
        return M4U_CLNTMOD_UNKNOWN;
    }
    
    spin_lock(&gMvaGraph_lock);

    //find prev head/tail of this region
    while(mvaGraph[index]==0)
        index--;

    if(MVA_IS_BUSY(index))
    {
        eModuleId = moduleGraph[index];
    }
	
    spin_unlock(&gMvaGraph_lock);
    return eModuleId;
    
}

/*****************************************************************************
 * FUNCTION
 *    m4u_do_mva_alloc
 * DESCRIPTION
 *    Allocate a mva implement.
 * PARAMETERS
 *    param1 : [IN]  const M4U_MODULE_ID_ENUM eModuleID
 *                Module ID.  
 *    param2 : [IN]  const unsigned int BufAddr
 *                User allocated virtual memory address.  
 *    param3 : [IN]  const unsigned int BufSize
 *                User allocated memory size.   
 * RETURNS
 *    Type: Integer. mva and zero mean fail.
 ****************************************************************************/
static unsigned int m4u_do_mva_alloc(const M4U_MODULE_ID_ENUM eModuleID, 
								  const unsigned int BufAddr, 
								  const unsigned int BufSize)
{
    short s,end;
    short new_start, new_end;
    short nr = 0;
    unsigned int mvaRegionStart;
    unsigned int startRequire, endRequire, sizeRequire; 

    if(BufSize == 0) return 0;

    MMProfileLogEx(M4U_MMP_Events[PROFILE_ALLOC_MVA_REGION], MMProfileFlagStart, eModuleID, BufAddr);
    ///-----------------------------------------------------
    ///calculate mva block number
    startRequire = BufAddr & (~M4U_PAGE_MASK);
    endRequire = (BufAddr+BufSize-1)| M4U_PAGE_MASK;
    sizeRequire = endRequire-startRequire+1;
    nr = (sizeRequire+MVA_BLOCK_ALIGN_MASK)>>MVA_BLOCK_SIZE_ORDER;//(sizeRequire>>MVA_BLOCK_SIZE_ORDER) + ((sizeRequire&MVA_BLOCK_ALIGN_MASK)!=0);
#ifdef M4U_4M_PAGETABLE
    if (unlikely(M4U_CLNTMOD_LCDC_UI == eModuleID))
    {
        M4UMSG("LCDCUI Comes to register 0x%x for size 0x%x", BufAddr, BufSize);
        {
            MTK_MAU_CONFIG mau_disp;
            mau_disp.entry = 1;
            mau_disp.larb = 0;
            mau_disp.start = TOTAL_MVA_RANGE;
            mau_disp.end = BufAddr;
            mau_disp.virt = 1;
            mau_disp.port_msk = 0xFFFF;
            mau_disp.monitor_read = 1;
            mau_disp.monitor_write = 1;
            mau_config(&mau_disp);

            mau_disp.entry = 2;
            mau_disp.start = (BufAddr+BufSize);
            mau_disp.end = 0xFFFFFFFF;
            mau_config(&mau_disp);
        }

        /* Free unused page table pages */
        {
            struct page* pg;
            struct page* pg2;
            M4UMSG("before release PageTable, remaining %d pages", zone_page_state(&contig_page_data.node_zones[0], NR_FREE_PAGES));
            pg = (struct page*)virt_to_page(mva_pteAddr_nonsec(TOTAL_MVA_RANGE));
            pg2 = (struct page*)virt_to_page(mva_pteAddr_nonsec(BufAddr));
            M4UMSG("release partI from 0x%x to 0x%x", mva_pteAddr_nonsec(TOTAL_MVA_RANGE), mva_pteAddr_nonsec(BufAddr));
            while (pg != pg2)
                __free_page(pg++);
            M4UMSG("After release PageTable, remaining %d pages", zone_page_state(&contig_page_data.node_zones[0], NR_FREE_PAGES));
            pg = (struct page*)virt_to_page(mva_pteAddr_nonsec(BufAddr+BufSize-1));
            pg++;
            pg2 = (struct page*)virt_to_page(mva_pteAddr_nonsec(0x100000000));
            M4UMSG("release partII from 0x%x to 0x%x", mva_pteAddr_nonsec(BufAddr+BufSize-1), mva_pteAddr_nonsec(0x100000000));
            while (pg != pg2)
                __free_page(pg++);
            M4UMSG("After release PageTable, remaining %d pages", zone_page_state(&contig_page_data.node_zones[0], NR_FREE_PAGES));
        }
        return BufAddr;
    }
#endif
    spin_lock(&gMvaGraph_lock);

    ///-----------------------------------------------
    ///find first match free region
    for(s=1; (s<(MVA_MAX_BLOCK_NR+1))&&(mvaGraph[s]<nr); s+=(mvaGraph[s]&MVA_BLOCK_NR_MASK))
        ;
    if(s > MVA_MAX_BLOCK_NR)
    {
        spin_unlock(&gMvaGraph_lock);
        M4UERR("mva_alloc error: no available MVA region for %d blocks!\n", nr);
        return 0;
    }

    ///-----------------------------------------------
    ///alloc a mva region
    end = s + mvaGraph[s] - 1;

    if(unlikely(nr == mvaGraph[s]))
    {
        MVA_SET_BUSY(s);
        MVA_SET_BUSY(end);
        moduleGraph[s] = eModuleID;
        moduleGraph[end] = eModuleID;
    }
    else
    {
        new_end = s + nr - 1;
        new_start = new_end + 1; //next free start
        //note: new_start may equals to end
        mvaGraph[new_start] = (mvaGraph[s]-nr);
        mvaGraph[new_end] = nr | MVA_BUSY_MASK;
        mvaGraph[s] = mvaGraph[new_end];
        mvaGraph[end] = mvaGraph[new_start];

        moduleGraph[s] = eModuleID;
        moduleGraph[new_end] = eModuleID;
        //moduleGraph[new_start] = M4U_CLNTMOD_UNKNOWN;
        //moduleGraph[end] = M4U_CLNTMOD_UNKNOWN;
    }

#ifdef M4U_LOG_ALLOCATE_SIZE
        mvasizeGraph[s] = BufSize;
#endif

    spin_unlock(&gMvaGraph_lock);

    mvaRegionStart = (unsigned int)s;
    
    MMProfileLogEx(M4U_MMP_Events[PROFILE_ALLOC_MVA_REGION], MMProfileFlagEnd, eModuleID, BufSize);
    return (mvaRegionStart<<MVA_BLOCK_SIZE_ORDER) + mva_pageOffset(BufAddr);

}



#define RightWrong(x) ( (x) ? "correct" : "error")


/*****************************************************************************
 * FUNCTION
 *    m4u_do_mva_free
 * DESCRIPTION
 *    free a mva implement.
 * PARAMETERS
 *    param1 : [IN]  const M4U_MODULE_ID_ENUM eModuleID
 *                Module ID.  
 *    param2 : [IN]  const unsigned int BufAddr
 *                User free virtual memory address.  
 *    param3 : [IN]  const unsigned int BufSize
 *                User free memory size.  
 *    param4 : [IN]  const unsigned int mvaRegionStart
 *                User free mva. 
 * RETURNS
 *    Type: Integer. zero mean success and -1 mean fail.
 ****************************************************************************/
static int m4u_do_mva_free(const M4U_MODULE_ID_ENUM eModuleID, 
                                const unsigned int BufAddr,
								const unsigned int BufSize,
								const unsigned int mvaRegionStart) 
{
    short startIdx = mvaRegionStart >> MVA_BLOCK_SIZE_ORDER;
    short nr = mvaGraph[startIdx] & MVA_BLOCK_NR_MASK;
    short endIdx = startIdx + nr - 1;
    unsigned int startRequire, endRequire, sizeRequire;
    short nrRequire;
#ifdef M4U_4M_PAGETABLE
    if (unlikely(M4U_CLNTMOD_LCDC_UI == eModuleID))
    {
        M4UMSG("LCDCUI Comes to free 0x%x, 0x%x", BufAddr, mvaRegionStart);
        return 0;
    }
#endif
    spin_lock(&gMvaGraph_lock);
    ///--------------------------------
    ///check the input arguments
    ///right condition: startIdx is not NULL && region is busy && right module && right size 
    startRequire = BufAddr & (~M4U_PAGE_MASK);
    endRequire = (BufAddr+BufSize-1)| M4U_PAGE_MASK;
    sizeRequire = endRequire-startRequire+1;
    nrRequire = (sizeRequire+MVA_BLOCK_ALIGN_MASK)>>MVA_BLOCK_SIZE_ORDER;//(sizeRequire>>MVA_BLOCK_SIZE_ORDER) + ((sizeRequire&MVA_BLOCK_ALIGN_MASK)!=0);
    if(!(   startIdx != 0           //startIdx is not NULL
            && MVA_IS_BUSY(startIdx)               // region is busy
            && (moduleGraph[startIdx]==eModuleID) //right module
            && (nr==nrRequire)       //right size
          )
       )
    {

        spin_unlock(&gMvaGraph_lock);
        m4u_aee_print("free mva error, module=%s\n", m4u_get_module_name(eModuleID));
        M4UMSG("error to free mva========================>\n");
        M4UMSG("ModuleID=%s (expect %s) [%s]\n", 
                m4u_get_module_name(eModuleID), m4u_get_module_name(moduleGraph[startIdx]),RightWrong(eModuleID==moduleGraph[startIdx]));
        M4UMSG("BufSize=%d(unit:0x%xBytes) (expect %d) [%s]\n", 
                nrRequire, MVA_BLOCK_SIZE, nr, RightWrong(nrRequire==nr));
        M4UMSG("mva=0x%x, (IsBusy?)=%d (expect %d) [%s]\n",
                mvaRegionStart, MVA_IS_BUSY(startIdx),1, RightWrong(MVA_IS_BUSY(startIdx)));
        m4u_mvaGraph_dump();
        //m4u_mvaGraph_dump_raw();
        return -1;
    }

    moduleGraph[startIdx] = M4U_CLNTMOD_UNKNOWN;
    moduleGraph[endIdx] = M4U_CLNTMOD_UNKNOWN;

    ///--------------------------------
    ///merge with followed region
    if( (endIdx+1 <= MVA_MAX_BLOCK_NR)&&(!MVA_IS_BUSY(endIdx+1)))
    {
        nr += mvaGraph[endIdx+1];
        mvaGraph[endIdx] = 0;
        mvaGraph[endIdx+1] = 0;
    }

    ///--------------------------------
    ///merge with previous region
    if( (startIdx-1>0)&&(!MVA_IS_BUSY(startIdx-1)) )
    {
        int pre_nr = mvaGraph[startIdx-1];
        mvaGraph[startIdx] = 0;
        mvaGraph[startIdx-1] = 0;
        startIdx -= pre_nr;
        nr += pre_nr;
    }
    ///--------------------------------
    ///set region flags
    mvaGraph[startIdx] = nr;
    mvaGraph[startIdx+nr-1] = nr;

    spin_unlock(&gMvaGraph_lock);
    return 0;    

}

/*****************************************************************************
 * FUNCTION
 *    m4u_profile_init
 * DESCRIPTION
 *    Initial profile event
 * PARAMETERS
 *    None.
 * RETURNS
 *    None.
 ****************************************************************************/
static void m4u_profile_init(void)
{
    MMP_Event M4U_Event;
    M4U_Event = MMProfileRegisterEvent(MMP_RootEvent, "M4U");
    M4U_MMP_Events[PROFILE_ALLOC_MVA] = MMProfileRegisterEvent(M4U_Event, "Alloc MVA");
    M4U_MMP_Events[PROFILE_ALLOC_MVA_REGION] = MMProfileRegisterEvent(M4U_MMP_Events[PROFILE_ALLOC_MVA], "Alloc MVA Region");
    M4U_MMP_Events[PROFILE_GET_PAGES] = MMProfileRegisterEvent(M4U_MMP_Events[PROFILE_ALLOC_MVA], "Get Pages");
    M4U_MMP_Events[PROFILE_FOLLOW_PAGE] = MMProfileRegisterEvent(M4U_MMP_Events[PROFILE_GET_PAGES], "Follow Page");
    M4U_MMP_Events[PROFILE_FORCE_PAGING] = MMProfileRegisterEvent(M4U_MMP_Events[PROFILE_GET_PAGES], "Force Paging");
    M4U_MMP_Events[PROFILE_MLOCK] = MMProfileRegisterEvent(M4U_MMP_Events[PROFILE_GET_PAGES], "MLock");
    M4U_MMP_Events[PROFILE_ALLOC_FLUSH_TLB] = MMProfileRegisterEvent(M4U_MMP_Events[PROFILE_ALLOC_MVA], "Alloc Flush TLB");
    M4U_MMP_Events[PROFILE_DEALLOC_MVA] = MMProfileRegisterEvent(M4U_Event, "DeAlloc MVA");
    M4U_MMP_Events[PROFILE_RELEASE_PAGES] = MMProfileRegisterEvent(M4U_MMP_Events[PROFILE_DEALLOC_MVA], "Release Pages");
    M4U_MMP_Events[PROFILE_MUNLOCK] = MMProfileRegisterEvent(M4U_MMP_Events[PROFILE_RELEASE_PAGES], "MUnLock");
    M4U_MMP_Events[PROFILE_PUT_PAGE] = MMProfileRegisterEvent(M4U_MMP_Events[PROFILE_RELEASE_PAGES], "Put Page");
    M4U_MMP_Events[PROFILE_RELEASE_MVA_REGION] = MMProfileRegisterEvent(M4U_MMP_Events[PROFILE_DEALLOC_MVA], "Release MVA Region");
    M4U_MMP_Events[PROFILE_QUERY] = MMProfileRegisterEvent(M4U_Event, "Query MVA");
    M4U_MMP_Events[PROFILE_INSERT_TLB] = MMProfileRegisterEvent(M4U_Event, "Insert TLB");
    M4U_MMP_Events[PROFILE_DMA_MAINT_ALL] = MMProfileRegisterEvent(M4U_Event, "Cache Maintain");
    M4U_MMP_Events[PROFILE_DMA_CLEAN_RANGE] = MMProfileRegisterEvent(M4U_MMP_Events[PROFILE_DMA_MAINT_ALL], "Clean Range");
    M4U_MMP_Events[PROFILE_DMA_CLEAN_ALL] = MMProfileRegisterEvent(M4U_MMP_Events[PROFILE_DMA_MAINT_ALL], "Clean All");
    M4U_MMP_Events[PROFILE_DMA_INVALID_RANGE] = MMProfileRegisterEvent(M4U_MMP_Events[PROFILE_DMA_MAINT_ALL], "Invalid Range");
    M4U_MMP_Events[PROFILE_DMA_INVALID_ALL] = MMProfileRegisterEvent(M4U_MMP_Events[PROFILE_DMA_MAINT_ALL], "Invalid All");
    M4U_MMP_Events[PROFILE_DMA_FLUSH_RANGE] = MMProfileRegisterEvent(M4U_MMP_Events[PROFILE_DMA_MAINT_ALL], "Flush Range");
    M4U_MMP_Events[PROFILE_DMA_FLUSH_ALL] = MMProfileRegisterEvent(M4U_MMP_Events[PROFILE_DMA_MAINT_ALL], "Flush All");
    M4U_MMP_Events[PROFILE_CACHE_FLUSH_ALL] = MMProfileRegisterEvent(M4U_Event, "Cache Flush All");
    M4U_MMP_Events[PROFILE_CONFIG_PORT] = MMProfileRegisterEvent(M4U_Event, "Config Port");
    M4U_MMP_Events[PROFILE_MAIN_TLB_MON] = MMProfileRegisterEvent(M4U_Event, "Main TLB Monitor");
    M4U_MMP_Events[PROFILE_PREF_TLB_MON] = MMProfileRegisterEvent(M4U_Event, "PreFetch TLB Monitor");
    M4U_MMP_Events[PROFILE_M4U_REG] = MMProfileRegisterEvent(M4U_Event, "M4U Registers");
    M4U_MMP_Events[PROFILE_M4U_ERROR] = MMProfileRegisterEvent(M4U_Event, "M4U ERROR");
}


/*****************************************************************************
 * FUNCTION
 *    m4u_query_mva
 * DESCRIPTION
 *    Query mva by va
 * PARAMETERS
 *    param1 : [IN]  const M4U_MODULE_ID_ENUM eModuleID
 *                Module ID.  
 *    param2 : [IN]  const unsigned int BufAddr
 *                User allocated virtual memory address.  
 *    param3 : [IN]  const unsigned int BufSize
 *                User allocated memory size . 
 *    param4 : [OUT]  unsigned int *pRetMVABuf
 *                Return mva. 
 *    param5 : [IN]  const struct file * a_pstFile
 *                A private data contain garbage node. 
 * RETURNS
 *    Type: Integer. zero mean success and else mean fail.
 ****************************************************************************/
int m4u_query_mva(const M4U_MODULE_ID_ENUM eModuleID, 
                       const unsigned int BufAddr, 
                       const unsigned int BufSize, 
                       unsigned int *pRetMVABuf,
                       const struct file * a_pstFile) 
{
    struct list_head *pListHead;
    garbage_list_t *pList = NULL;
    garbage_node_t *pNode = (garbage_node_t*)(a_pstFile->private_data);
    unsigned int query_start = BufAddr;
    unsigned int query_end = BufAddr + BufSize - 1;
    unsigned int s,e;
    int ret, err = 0;
    
    *pRetMVABuf = 0;                 
    
    if(pNode==NULL)
    {
        M4UMSG("error: m4u_query_mva, pNode is NULL, va=0x%x, module=%s! \n", BufAddr, m4u_get_module_name(eModuleID));
        return -1;
    }  

    MMProfileLogEx(M4U_MMP_Events[PROFILE_QUERY], MMProfileFlagStart, eModuleID, BufAddr);
    mutex_lock(&(pNode->dataMutex));              
    list_for_each(pListHead, &(pNode->mvaList))
    {
        pList = container_of(pListHead, garbage_list_t, link);
        s = pList->bufAddr;
        e = s + pList->size - 1;

        if((pList->eModuleId==eModuleID) &&
        	 (query_start>=s && query_end<=e))
        {
            if(pList->mvaStart > 0) //here we have allocated mva for this buffer
            {
                *pRetMVABuf = pList->mvaStart + (query_start-s);
            }
            else    // here we have not allocated mva (this buffer is registered, and query for first time)
            {
                M4U_ASSERT(pList->flags&MVA_REGION_REGISTER);
                //we should allocate mva for this buffer

                ret = m4u_alloc_mva(pList->eModuleId, pList->bufAddr, pList->size,
                            pList->security, pList->cache_coherent, pRetMVABuf);
                if(ret)
                {
                	M4UMSG("m4u_alloc_mva failed when query for it: %d\n", ret);
                	err = -EFAULT;
                } 
                else
                {
                    pList->flags &= ~(MVA_REGION_REGISTER);
                    pList->mvaStart = *pRetMVABuf;
                    *pRetMVABuf = pList->mvaStart + (query_start-s);
                }
                M4ULOG("allocate for first query: id=%s, addr=0x%08x, size=%d, mva=0x%x \n", 
                      m4u_get_module_name(eModuleID), BufAddr,  BufSize, *pRetMVABuf);
            }
    		break;
        }
    }
    mutex_unlock(&(pNode->dataMutex));
    MMProfileLogEx(M4U_MMP_Events[PROFILE_QUERY], MMProfileFlagEnd, eModuleID, BufSize);

    M4ULOG("m4u_query_mva: id=%s, addr=0x%08x, size=%d, mva=0x%x \n", 
                    m4u_get_module_name(eModuleID), BufAddr,  BufSize, *pRetMVABuf);

    return err;

}

/*****************************************************************************
 * FUNCTION
 *    m4u_invalidate_and_check
 * DESCRIPTION
 *    1. Call m4u_invalid_tlb_by_range to invalidate TLB.
 *    2. Call m4u_confirm_range_invalidated to check invalidate result
 *    3. If invalidate result is not ok, call m4u_invalid_tlb_by_range again.
 *    4. If invalidate result is not ok again, call m4u_invalid_tlb_all.
 *    5. If invalidate result is not ok again, KE...
 *    PS: Invalidation done status register need enable invalidation done Interrupt.
 * PARAMETERS
 *    param1 : [IN]  unsigned int start
 *                Invalidate region start address.  
 *    param2 : [IN]  unsigned int end
 *                Invalidate region end address.  
 * RETURNS
 *    None.
 ****************************************************************************/
static void m4u_invalidate_and_check(unsigned int start, unsigned int end)
{
    spin_lock(&gM4u_reg_lock);
    m4u_invalid_tlb_by_range(start, end);
    if(0!=m4u_confirm_range_invalidated(start, end)) // first time fail, invalidate range again
    {
        m4u_invalid_tlb_by_range(start, end);
    	if(0!=m4u_confirm_range_invalidated(start, end)) // again failed, invalidate all
    	{
    		M4UMSG("invalidate range twice, also fail! \n");
            m4u_invalid_tlb_all();
            if(0!=m4u_confirm_range_invalidated(start, end)) // invalidate all failed, die
            {
                M4UERR("invalidate all fail! \n");
            }
    	}
    } 
	spin_unlock(&gM4u_reg_lock);
}

/*****************************************************************************
 * FUNCTION
 *    m4u_dealloc_mva_dynamic
 * DESCRIPTION
 *    1. Call m4u_release_pages to release page.
 *    2. Clean page table
 *    3. Invalid TLB
 *    4. Call m4u_do_mva_free to free mva 
 * PARAMETERS
 *    param1 : [IN]  const M4U_MODULE_ID_ENUM eModuleID
 *                Module ID.  
 *    param2 : [IN]  const unsigned int BufAddr
 *                User free virtual memory address.  
 *    param3 : [IN]  const unsigned int BufSize
 *                User free memory size.  
 *    param4 : [IN]  const unsigned int MVA
 *                User free mva. 
 * RETURNS
 *    Type: Integer. zero mean success and -1 mean fail.
 ****************************************************************************/
static int m4u_dealloc_mva_dynamic(const M4U_MODULE_ID_ENUM eModuleID, 
                                   const unsigned int BufAddr, 
                                   const unsigned int BufSize,
                                   const unsigned int mvaRegionAddr,
                                   struct sg_table* sg_table) 
{			
    int ret;
    unsigned int pteStart;
    unsigned int page_num;
//	unsigned int align_page_num;

    M4ULOG("mva dealloc: ID=%s, VA=0x%x, size=%d, mva=0x%x\n", m4u_get_module_name(eModuleID), BufAddr, BufSize, mvaRegionAddr);

    page_num = M4U_GET_PAGE_NUM(BufAddr, BufSize);

    mutex_lock(&gM4uMutex);

    MMProfileLogEx(M4U_MMP_Events[PROFILE_RELEASE_PAGES], MMProfileFlagStart, eModuleID, BufAddr);
    m4u_release_pages(eModuleID,BufAddr,BufSize,mvaRegionAddr, sg_table);

    //==================================
    // fill pagetable with 0
    {
        pteStart= (unsigned int)mva_pteAddr_nonsec(mvaRegionAddr); // get offset in the page table  
        memset((void*)pteStart, 0, page_num<<2); // one page entery size is 4 bytes.
#ifdef M4U_4M_PAGETABLE
    {
        dma_addr_t dma_handle;
        dma_handle = dma_map_single(NULL, (void *)pteStart, page_num<<2, DMA_TO_DEVICE);
        dma_sync_single_for_device(NULL, dma_handle, page_num<<2, DMA_TO_DEVICE);
        dma_unmap_single(NULL, dma_handle, page_num<<2, DMA_TO_DEVICE);
    }
#endif
        m4u_invalidate_and_check(mvaRegionAddr, mvaRegionAddr+BufSize-1);        
    }
    // TODO: no clear sec page table?? cloud
	
    MMProfileLogEx(M4U_MMP_Events[PROFILE_RELEASE_PAGES], MMProfileFlagEnd, eModuleID, BufSize);
    mutex_unlock(&gM4uMutex);


    MMProfileLogEx(M4U_MMP_Events[PROFILE_RELEASE_MVA_REGION], MMProfileFlagStart, eModuleID, BufAddr);

    ret = m4u_do_mva_free(eModuleID, BufAddr, BufSize, mvaRegionAddr);
    
    MMProfileLogEx(M4U_MMP_Events[PROFILE_RELEASE_MVA_REGION], MMProfileFlagEnd, eModuleID, BufSize);
    //m4u_mvaGraph_dump();

    return ret;
}


/*****************************************************************************
 * FUNCTION
 *    m4u_fill_pagetable
 * DESCRIPTION
 *    1. call m4u_get_pages to fill page table.
 *    2. flush tlb entries in this mva range.
 *    3. record memory usage.
 * PARAMETERS
 *    param1 : [IN]  const M4U_MODULE_ID_ENUM eModuleID
 *                Module ID.  
 *    param2 : [IN]  const unsigned int BufAddr
 *                User allocated virtual memory address.  
 *    param3 : [IN]  const unsigned int BufSize
 *                User allocated memory size.  
 *    param4 : [IN]  const unsigned int mvaStart
 *                MVA start address. 
 *    param5 : [IN]  const unsigned int entry_flag
 *                 Flag of specified memory. 
 * RETURNS
 *    Type: Integer. zero mean fail and others mean success.
 ****************************************************************************/
static int m4u_fill_pagetable(const M4U_MODULE_ID_ENUM eModuleID, const unsigned int BufAddr, 
                const unsigned int BufSize, const unsigned int mvaStart, const unsigned int entry_flag,
                struct sg_table* sg_table)
{
    int i;
    int page_num = M4U_GET_PAGE_NUM(BufAddr, BufSize);
    
    unsigned int *pPagetable_nonsec = mva_pteAddr_nonsec(mvaStart);
//    unsigned int *pPagetable_sec = mva_pteAddr_sec(mvaStart);
    unsigned int *pPhys;

    MMProfileLogEx(M4U_MMP_Events[PROFILE_GET_PAGES], MMProfileFlagStart, eModuleID, BufAddr);
    
    pPhys = (unsigned int*)vmalloc(page_num*sizeof(unsigned int*));
    if(pPhys == NULL)
    {
        MMProfileLogEx(M4U_MMP_Events[PROFILE_GET_PAGES], MMProfileFlagEnd, eModuleID, BufSize);
        M4UMSG("m4u_fill_pagetable: error to vmalloc %d*4 size\n", page_num);
        return 0;
    }
    if(sg_table != NULL)
        page_num = m4u_get_pages_sg(eModuleID, BufAddr, BufSize, sg_table, pPhys);
    else
        page_num = m4u_get_pages(eModuleID, BufAddr, BufSize, pPhys);
    if(page_num<=0)
    {
        MMProfileLogEx(M4U_MMP_Events[PROFILE_GET_PAGES], MMProfileFlagEnd, eModuleID, BufSize);
        M4UMSG("Error: m4u_get_pages failed \n");        
        vfree(pPhys);		
        return 0;
    }

    mutex_lock(&gM4uMutex);    

    //fill page table
    for(i=0;i<page_num;i++)
    {
        unsigned int pa = pPhys[i];
        
        pa |= entry_flag;
        
    #ifdef M4U_USE_ONE_PAGETABLE
        *(pPagetable_nonsec+i) = pa;
    #else
        if(!(entry_flag&0x8)) // secure bit
        {
            *(pPagetable_sec+i) = pa;
        }
        else
        {
            #ifdef M4U_COPY_NONSEC_PT_TO_SEC
                *(pPagetable_nonsec+i) = pa;
                *(pPagetable_sec+i) = pa;
            #else
                *(pPagetable_nonsec+i) = pa;
            #endif
        }
    #endif
    } 

#ifdef M4U_4M_PAGETABLE
    {
        dma_addr_t dma_handle;
        dma_handle = dma_map_single(NULL, pPagetable_nonsec, page_num<<2, DMA_TO_DEVICE);
        dma_sync_single_for_device(NULL, dma_handle, page_num<<2, DMA_TO_DEVICE);
        dma_unmap_single(NULL, dma_handle, page_num<<2, DMA_TO_DEVICE);
    }
#endif

    vfree(pPhys);


    mb();
    MMProfileLogEx(M4U_MMP_Events[PROFILE_GET_PAGES], MMProfileFlagEnd, eModuleID, BufSize);
        
    ///-------------------------------------------------------
    ///flush tlb entries in this mva range
    MMProfileLogEx(M4U_MMP_Events[PROFILE_ALLOC_FLUSH_TLB], MMProfileFlagStart, eModuleID, BufAddr);
    
    m4u_invalidate_and_check(mvaStart, mvaStart+BufSize-1);
    
    MMProfileLogEx(M4U_MMP_Events[PROFILE_ALLOC_FLUSH_TLB], MMProfileFlagEnd, eModuleID, BufSize);
    

    // record memory usage
    pmodule_current_size[eModuleID] += BufSize;
    if(pmodule_current_size[eModuleID]>gModuleMaxMVASize[eModuleID])
    {    	 
    	 M4UERR_WITH_OWNER("MVA overflow, module=%s, Current alloc MVA=0x%x, Max MVA size=0x%x \n",  m4u_get_module_name(eModuleID), 
         m4u_get_module_name(eModuleID), pmodule_current_size[eModuleID], gModuleMaxMVASize[eModuleID]);
         m4u_memory_usage();
    }
    if(pmodule_current_size[eModuleID]> pmodule_max_size[eModuleID])
    {
        pmodule_max_size[eModuleID] = pmodule_current_size[eModuleID];
    }

    mutex_unlock(&gM4uMutex);

    return page_num;
    
}

#ifdef MTK_M4U_EXT_PAGE_TABLE 
/* add for ovl: 
   this function will build pagetable for framebuffer,
   its mva==pa. so when switch from LK to kernel, 
   ovl just switch to virtual mode, no need to modify address register

!!!! NOTES:
    1. only be used by ovl for frame buffer 
    2. currently, total mva is 1G
        frame buffer pa is > 0xf0000000
        so it won't be corrupted by other m4u_alloc_mva()
*/
int m4u_fill_linear_pagetable(unsigned int pa, unsigned int size)
{
    int page_num, i;
    unsigned int mva = pa&(~M4U_PAGE_MASK);
    unsigned int *pPt = mva_pteAddr_nonsec(mva);

    page_num = M4U_GET_PAGE_NUM(pa, size);
    pa = mva;   //page align

    if(pa < 0x20000000)
    {
        M4UMSG("error: m4u_fill_linear_pagetable fail: pa=0x%x < 0x20000000\n", pa);
        return -1;
    }

    for(i=0; i<page_num; i++)
    {
        pPt[i] = pa | F_DESC_VALID | F_DESC_NONSEC(1);
        pa += M4U_PAGE_SIZE;
    }

    return 0;
}

int m4u_erase_linear_pagetable(unsigned int pa, unsigned int size)
{
    int page_num, i;
    unsigned int *pPt = mva_pteAddr_nonsec(pa);
    
    page_num = M4U_GET_PAGE_NUM(pa, size);

    for(i=0; i<page_num; i++)
    {
        pPt[i] = 0;
    }

    return 0;
}

#endif

#if 0
static int m4u_perf_timer_on(void)
{
    M4ULOG("m4u_perf_timer_on is called!, %d, %d %d %d\n", jiffies, HZ);
    perf_timer.expires = (jiffies + HZ); //every 1 second
    //perf_timer.data = 0;
    //perf_timer.function = m4u_get_performance;
    perf_timer.data = 1;
    perf_timer.function = m4u_memory_usage;    
    add_timer(&perf_timer);    	
    M4ULOG("m4u_perf_timer_on");
    return 0;
}
#endif

/*****************************************************************************
 * FUNCTION
 *    m4u_alloc_mva
 * DESCRIPTION
 *    1. allocate a mva.
 *    2. fill page table
 * PARAMETERS
 *    param1 : [IN]  const M4U_MODULE_ID_ENUM eModuleID
 *                Module ID.  
 *    param2 : [IN]  const unsigned int BufAddr
 *                User allocated virtual memory address.  
 *    param3 : [IN]  const unsigned int BufSize
 *                User allocated memory size.  
 *    param4 : [IN]  const int security
 *                User specified memory security type. 
 *    param5 : [IN]  const int cache_coherent
 *                 User specified memory cache coherent type. 
 *    param6 : [OUT]  unsigned int *pRetMVABuf
 *                Allocated MVA.  
 * RETURNS
 *    Type: Integer. zero mean success and -1 mean fail.
 ****************************************************************************/
int __m4u_alloc_mva(const M4U_MODULE_ID_ENUM eModuleID, 
                    const unsigned int BufAddr, 
                    const unsigned int BufSize, 
                    const int security,
                    const int cache_coherent,
                    unsigned int *pRetMVABuf,
                    struct sg_table *sg_table)
{
    unsigned int page_num;
    //unsigned int align_page_num;
    unsigned int mvaStart;
    unsigned int entry_flag = F_DESC_VALID | F_DESC_NONSEC(!security) | F_DESC_SHARE(cache_coherent);

    MMProfileLogEx(M4U_MMP_Events[PROFILE_ALLOC_MVA], MMProfileFlagStart, eModuleID, BufAddr);

    page_num = M4U_GET_PAGE_NUM(BufAddr, BufSize);
    M4ULOG("m4u_alloc_mva: id=%s, addr=0x%x, size=%d, page_num=%d\n", 
                    m4u_get_module_name(eModuleID), BufAddr,  BufSize, page_num);
	
    mvaStart= m4u_do_mva_alloc(eModuleID, BufAddr, BufSize);		
    if(0 == mvaStart)
    {
        M4UERR("mva_alloc error: no available MVA region for %d bytes!\n", BufSize);
        m4u_mvaGraph_dump();
        *pRetMVABuf = 0;
        return -1;
    }

    page_num = m4u_fill_pagetable(eModuleID, BufAddr, BufSize, mvaStart, entry_flag, sg_table);
    if(page_num==0)
    {
        M4UMSG("Error: m4u_fill_pagetable failed");
        goto error_alloc_mva;
    }

    *pRetMVABuf = mvaStart;

    if(gM4uLogFlag)
    {
        M4UMSG("m4u_alloc_mva done: id=%s, addr=0x%x, size=%d, mva=0x%x, mva_end=0x%x\n", 
                    m4u_get_module_name(eModuleID), BufAddr, BufSize, *pRetMVABuf, *pRetMVABuf+BufSize-1);
	}
	else
    {
        M4UDBG("m4u_alloc_mva done: id=%s, addr=0x%x, size=%d, mva=0x%x, mva_end=0x%x\n", 
                    m4u_get_module_name(eModuleID), BufAddr, BufSize, *pRetMVABuf, *pRetMVABuf+BufSize-1);
    }
    MMProfileLogEx(M4U_MMP_Events[PROFILE_ALLOC_MVA], MMProfileFlagEnd, eModuleID, BufSize);

    return 0;

error_alloc_mva:
    m4u_do_mva_free(eModuleID, BufAddr, BufSize, mvaStart);
    M4UMSG("alloc_mva error: id=%s, addr=0x%x, size=%d, sec=%d\n", 
                    m4u_get_module_name(eModuleID), BufAddr,  BufSize, security);

    *pRetMVABuf = 0;
    return -1;
}

int m4u_alloc_mva(const M4U_MODULE_ID_ENUM eModuleID, 
                      const unsigned int BufAddr, 
                      const unsigned int BufSize, 
                      const int security,
                      const int cache_coherent,
                      unsigned int *pRetMVABuf)
{
    return __m4u_alloc_mva(eModuleID, BufAddr, BufSize, security, cache_coherent, pRetMVABuf, NULL);
}

int m4u_alloc_mva_sg(M4U_MODULE_ID_ENUM eModuleID, 
                     struct sg_table *sg_table, 
                     const unsigned int BufSize, 
                     int security,
                     int cache_coherent,
                     unsigned int *pRetMVABuf)
{

    return __m4u_alloc_mva(eModuleID, 0, BufSize, security, cache_coherent, pRetMVABuf, sg_table);
}
EXPORT_SYMBOL(m4u_alloc_mva_sg);


/*****************************************************************************
 * FUNCTION
 *    m4u_dealloc_mva
 * DESCRIPTION
 *    1. call m4u_invalid_seq_range_by_mva to invalid sequential range about this range. 
 *    2. call m4u_dealloc_mva_dynamic to free a mva.
 * PARAMETERS
 *    param1 : [IN]  M4U_MODULE_ID_ENUM eModuleID
 *                Module ID.  
 *    param2 : [IN]  const unsigned int BufAddr
 *                User allocated virtual memory address.  
 *    param3 : [IN]  const unsigned int BufSize
 *                User allocated memory size.  
 *    param4 : [IN]  const unsigned int MVA
 *                User specified mva. 
 * RETURNS
 *    Type: Integer. zero mean success and -1 mean fail.
 ****************************************************************************/
int __m4u_dealloc_mva(const M4U_MODULE_ID_ENUM eModuleID, 
                      const unsigned int BufAddr, 
                      const unsigned int BufSize, 
                      const unsigned int MVA,
                      struct sg_table* sg_table) 
{									

    int ret;

    MMProfileLogEx(M4U_MMP_Events[PROFILE_DEALLOC_MVA], MMProfileFlagStart, eModuleID, BufAddr);

    if(gM4uLogFlag)
    {
        M4UMSG("m4u_dealloc_mva, module=%s, addr=0x%x, size=%d, MVA=0x%x, mva_end=0x%x\n",
        m4u_get_module_name(eModuleID), BufAddr, BufSize, MVA, MVA+BufSize-1 );    
    }
    else
    {
        M4UDBG("m4u_dealloc_mva, module=%s, addr=0x%x, size=%d, MVA=0x%x, mva_end=0x%x\n",
        m4u_get_module_name(eModuleID), BufAddr, BufSize, MVA, MVA+BufSize-1 );
    }

    pmodule_in_freemva[eModuleID] = true;

    //if(eModuleID!=M4U_CLNTMOD_RDMA_GENERAL && eModuleID!=M4U_CLNTMOD_ROT_GENERAL)
    {
        if(m4u_invalid_seq_range_by_mva(MVA, MVA+BufSize-1)==0)
        {
            M4UMSG("warning: dealloc mva without invalid tlb range!! id=%s,add=0x%x,size=0x%x,mva=0x%x\n",
                m4u_get_module_name(eModuleID), BufAddr, BufSize, MVA);
        }
    }
    
    ret = m4u_dealloc_mva_dynamic(eModuleID, BufAddr, BufSize, MVA, sg_table);

    pmodule_in_freemva[eModuleID] = false;

    MMProfileLogEx(M4U_MMP_Events[PROFILE_DEALLOC_MVA], MMProfileFlagEnd, eModuleID, BufSize);
    return ret;

}

int m4u_dealloc_mva(const M4U_MODULE_ID_ENUM eModuleID, 
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
EXPORT_SYMBOL(m4u_dealloc_mva_sg);

#if 0
/*****************************************************************************
 * FUNCTION
 *    m4u_invalid_seq_all
 * DESCRIPTION
 *    Invalid all sequential range.
 * PARAMETERS
 *    param1 : [IN]  const M4U_MODULE_ID_ENUM eModuleID
 *                Caller Module ID.  
 * RETURNS
 *    Type: Integer. always zero.
 ****************************************************************************/
int m4u_invalid_seq_all(const M4U_MODULE_ID_ENUM eModuleID)
{
    unsigned int i;
    const unsigned int m4u_index = 0;
    unsigned int m4u_base = gM4UBaseAddr[m4u_index];
      
    M4ULOG("m4u_invalid_tlb_all, module:%s \n", m4u_get_module_name(eModuleID)); 
    M4U_POW_ON_TRY(eModuleID);

    spin_lock(&gM4u_reg_lock);

    if(FreeSEQRegs[m4u_index] < SEQ_RANGE_NUM)
    {
        for(i=0;i<SEQ_RANGE_NUM;i++)
        {
            if(pRangeDes[i].Enabled == 1)
            {
                 pRangeDes[i].Enabled = 0;
                 M4U_WriteReg32(m4u_base, REG_MMU_SQ_START(i), 0);
                 M4U_WriteReg32(m4u_base, REG_MMU_SQ_END(i), 0);
                 FreeSEQRegs[m4u_index]++;
            }
        }
    }
    m4u_invalid_tlb_all();
    
    M4U_POW_OFF_TRY(eModuleID);
    spin_unlock(&gM4u_reg_lock);
    
    return 0;

}
#endif

static inline int mva_owner_match(M4U_MODULE_ID_ENUM id, M4U_MODULE_ID_ENUM owner)
{
    if(owner == id)
        return 1;

#if 0
    if(owner==M4U_CLNTMOD_RDMA_GENERAL &&
       (id==M4U_CLNTMOD_RDMA0||id==M4U_CLNTMOD_RDMA1) 
       )
    {
        return 1;
    }
    if(owner==M4U_CLNTMOD_ROT_GENERAL &&
        (id==M4U_CLNTMOD_VDO_ROT0||
        id==M4U_CLNTMOD_RGB_ROT0||
        id==M4U_CLNTMOD_RGB_ROT1||
        id==M4U_CLNTMOD_VDO_ROT1||
        id==M4U_CLNTMOD_RGB_ROT2)
        )
    {
        return 1;
    }
#endif

    return 0;
}

/*****************************************************************************
 * FUNCTION
 *    m4u_manual_insert_entry
 * DESCRIPTION
 *    Insert TLB by user.
 * PARAMETERS
 *    param1 : [IN]  const M4U_PORT_ID_ENUM eModuleID
 *                Port ID.  
 *    param2 : [IN]  unsigned int EntryMVA
 *                MVA entry.  
 *    param3 : [IN]  const int secure_pagetable
 *                Secure pagetable or not.  
 *    param4 : [IN]  const int Lock
 *                Lock TLB flag.  
 * RETURNS
 *    Type: Integer. always zero.
 ****************************************************************************/
int m4u_manual_insert_entry(const M4U_PORT_ID_ENUM eModuleID,
                                 unsigned int EntryMVA, 
                                 const int secure_pagetable,
                                 const int Lock) 
{ 
    unsigned int *pPageAddr = 0;
    unsigned int EntryPA;
    unsigned int m4u_base = gM4UBaseAddr[m4u_port_2_m4u_id(eModuleID)];

    if(secure_pagetable)
    {
        pPageAddr = mva_pteAddr_sec(EntryMVA);
    }       
    else
    {
        pPageAddr = mva_pteAddr_nonsec(EntryMVA);
    }
    
    EntryPA = *pPageAddr; //can't clear bit0~11  

    M4UMSG("m4u_manual_insert_entry, port:%s, MVA:0x%x, Lock:%d, Descriptor: 0x%x\n", 
        m4u_get_port_name(eModuleID), EntryMVA, Lock, EntryPA);	

    EntryMVA &= 0xFFFFF000;	//clear bit0~11

    if(Lock)
    {
        EntryMVA |= F_PROG_VA_LOCK_BIT; 
    }

    if(secure_pagetable && (!(EntryPA&F_DESC_NONSEC(1))))
    {
        EntryMVA |= F_PROG_VA_SECURE_BIT; 
    }
    
    spin_lock(&gM4u_reg_lock);
    M4U_WriteReg32(m4u_base, REG_MMU_PROG_VA, EntryMVA);
    M4U_WriteReg32(m4u_base, REG_MMU_PROG_DSC, EntryPA);
    M4U_WriteReg32(m4u_base, REG_MMU_PROG_EN, F_MMU_PROG_EN);
    spin_unlock(&gM4u_reg_lock);

    return 0;
}


/*****************************************************************************
 * FUNCTION
 *    m4u_do_insert_seq_range
 * DESCRIPTION
 *    Insert a sequential range implement. (every seq range has to align to 256K Bytes)
 * PARAMETERS
 *    param1 : [IN]  M4U_MODULE_ID_ENUM eModuleID
 *                Module ID.  
 *    param2 : [IN]  unsigned int MVAStart
 *                Start seq range.  
 *    param3 : [IN]  unsigned int MVAEnd
 *                End seq range.  
 *    param4 : [IN]   unsigned int entryCount
 *                seq range TLB entry count(0, 1, 2, 4, 8, 16). 
 * RETURNS
 *    Type: Integer. -1 mean range overlap and 0 mean success or no free seq range.
 ****************************************************************************/
#define M4U_INVALID_ID 0x5555
static int m4u_do_insert_seq_range(const M4U_MODULE_ID_ENUM eModuleID, 
                                  unsigned int MVAStart, 
                                  unsigned int MVAEnd, 
                                  unsigned int entryCount)
{
    //causion: we should hold m4u global 
    unsigned int i;
    unsigned int RangeReg_ID = M4U_INVALID_ID;
    unsigned int m4u_index = 0;
    unsigned int m4u_base = gM4UBaseAddr[m4u_index];

    M4U_ASSERT(MVAStart <= MVAEnd);    

    if(entryCount!=1 && entryCount!=2 && entryCount!=4 && entryCount!=8 && entryCount!=16)
        entryCount = 1;

    if(!mva_owner_match(eModuleID, mva2module(MVAStart)))
    {
        
        M4UERR_WITH_OWNER(" m4u_insert_seq_range module=%s, MVAStart=0x%x is %s, MVAEnd=0x%x is %s\n", m4u_get_module_name(eModuleID), m4u_get_module_name(eModuleID),
    	    m4u_get_module_name(eModuleID), MVAStart, m4u_get_module_name(mva2module(MVAStart)),
    	    MVAEnd, m4u_get_module_name(mva2module(MVAEnd)));
        m4u_dump_mva_info();
    }

    M4UDBG("m4u_insert_seq_range, module:%s, MVAStart:0x%x, MVAEnd:0x%x, entryCount=%d\n", 
            m4u_get_module_name(eModuleID), MVAStart, MVAEnd,  entryCount);

    if((MVAEnd - MVAStart) < M4U_PAGE_SIZE)
    {
        M4UDBG("seq_range too small: %d\n", MVAEnd - MVAStart);
        return 0;		
    }

//==================================
//no seq range error
    if(FreeSEQRegs[m4u_index] == 0)
    {
        M4ULOG("No seq range found. module=%s \n", m4u_get_module_name(eModuleID));
#ifdef M4U_PRINT_RANGE_DETAIL
        M4UMSG("m4u_insert_seq_range , module:%s, MVAStart:0x%x, MVAEnd:0x%x, entryCount=%d \r\n", 
                m4u_get_module_name(eModuleID), MVAStart, MVAEnd,  entryCount);
        M4UMSG(" Curent Range Info: \n");
        for(i=0;i<TOTAL_RANGE_NUM;i++)
        {
            M4UMSG("pRangeDes[%d]: Enabled=%d, module=%s, MVAStart=0x%x, MVAEnd=0x%x \n", 
                i, pRangeDes[i].Enabled, m4u_get_module_name(pRangeDes[i].eModuleID), 
                pRangeDes[i].MVAStart, pRangeDes[i].MVAEnd);
        } 
#endif        
        return 0;
    }

//===============================================
    //every seq range has to align to 256K Bytes
    MVAStart &= ~M4U_SEQ_ALIGN_MSK;
    MVAEnd |= M4U_SEQ_ALIGN_MSK;

//==================================================================    
    // check if the range is overlap with previous ones
    for(i=0;i<SEQ_RANGE_NUM;i++)
    {
        if(1==pRangeDes[i].Enabled)
        {
            if(MVAEnd<pRangeDes[i].MVAStart || MVAStart>pRangeDes[i].MVAEnd) //no overlap
            {
            	  continue;
            }
            else
            {
                M4UERR_WITH_OWNER("error: insert tlb range is overlapped with previous ranges, current process=%s,!\n", m4u_get_module_name(eModuleID),  current->comm);	
                M4UMSG("module=%s, mva_start=0x%x, mva_end=0x%x \n", m4u_get_module_name(eModuleID), MVAStart, MVAEnd);
                M4UMSG("overlapped range id=%d, module=%s, mva_start=0x%x, mva_end=0x%x \n", 
                        i, m4u_get_module_name(pRangeDes[i].eModuleID), pRangeDes[i].MVAStart, pRangeDes[i].MVAEnd);
                return 0;
            }
        }
    }
//========================================
    //find a free seq range
    
    if(FreeSEQRegs[m4u_index]>0) 
    {
        for(i=0;i<SEQ_RANGE_NUM;i++)
        {
            if(pRangeDes[i].Enabled == 0)
            {
                RangeReg_ID = i;
                FreeSEQRegs[m4u_index]--;
                break;
            }
        }
    }
    
    if(RangeReg_ID == M4U_INVALID_ID)
    {
        M4ULOG("error: can not find available range \n");
        return 0;  // do not have to return erro to up-layer, nothing will happen even insert tlb range fails
    }

//======================================================
    // write register to insert seq range

    ///> record range information in array
    pRangeDes[RangeReg_ID].Enabled = 1;
    pRangeDes[RangeReg_ID].eModuleID = eModuleID;
    pRangeDes[RangeReg_ID].MVAStart = MVAStart;
    pRangeDes[RangeReg_ID].MVAEnd = MVAEnd;
    pRangeDes[RangeReg_ID].entryCount = entryCount;
    
    ///> set the range register
    MVAStart |= F_SQ_MULTI_ENTRY_VAL(entryCount-1);
    MVAStart |= F_SQ_EN_BIT;
    M4ULOG("Insert seq range register: MVAStart=0x%x, MVAEnd=0x%x \n", MVAStart, MVAEnd);	
    
    spin_lock(&gM4u_reg_lock);
    {
        M4U_POW_ON_TRY(eModuleID);
        M4U_WriteReg32(m4u_base, REG_MMU_SQ_START(RangeReg_ID), MVAStart);
        M4U_WriteReg32(m4u_base, REG_MMU_SQ_END(RangeReg_ID), MVAEnd);
        M4U_POW_OFF_TRY(eModuleID);
    }
    
    spin_unlock(&gM4u_reg_lock);
    
    return 0;
}  //end of vM4USetUniupdateRangeInTLB()

/*****************************************************************************
 * FUNCTION
 *    m4u_insert_seq_range
 * DESCRIPTION
 *    Insert a sequential range.
 * PARAMETERS
 *    param1 : [IN]  M4U_MODULE_ID_ENUM eModuleID
 *                Module ID.  
 *    param2 : [IN]  const unsigned int MVAStart
 *                Start seq range.  
 *    param3 : [IN]  const unsigned int MVAEnd
 *                End seq range.  
 *    param4 : [IN]  const unsigned int entryCount
 *                seq range TLB entry count(0, 1, 2, 4, 8, 16). 
 * RETURNS
 *    Type: Integer. -1 mean range overlap and 0 mean success or no free seq range.
 ****************************************************************************/
int m4u_insert_seq_range(const M4U_MODULE_ID_ENUM eModuleID, 
                              const unsigned int MVAStart, 
                              const unsigned int MVAEnd, 
                              const unsigned int entryCount) //0:disable multi-entry, 1,2,4,8,16: enable multi-entry
{

    int ret;
    
    MMProfileLogEx(M4U_MMP_Events[PROFILE_INSERT_TLB], MMProfileFlagStart, eModuleID, MVAStart);
    mutex_lock(&gM4uMutex);
    ret = m4u_do_insert_seq_range(eModuleID, MVAStart, MVAEnd, entryCount);
    mutex_unlock(&gM4uMutex);
    MMProfileLogEx(M4U_MMP_Events[PROFILE_INSERT_TLB], MMProfileFlagEnd, eModuleID, MVAEnd-MVAStart+1);
    
    return ret;
    
}

/*****************************************************************************
 * FUNCTION
 *    m4u_invalid_seq_range_by_mva
 * DESCRIPTION
 *    Invalid a sequential range implement.
 * PARAMETERS
 *    param1 : [IN]  unsigned int MVAStart
 *                Start seq range.  
 *    param2 : [IN]  unsigned int MVAEnd
 *                End seq range.   
 * RETURNS
 *    Type: Integer. 0 means invalid specified sequential range and -1 means no specified sequential range.
 ****************************************************************************/
static int m4u_invalid_seq_range_by_mva(unsigned int MVAStart, unsigned int MVAEnd)
{
    unsigned int i;
    const int m4u_index = 0;
    unsigned int m4u_base = gM4UBaseAddr[m4u_index];
    int ret=-1;

    MVAStart &= ~M4U_SEQ_ALIGN_MSK;
    MVAEnd |= M4U_SEQ_ALIGN_MSK;
    
    M4ULOG("m4u_invalid_seq_range_by_mva,  MVAStart:0x%x, MVAEnd:0x%x \r\n", MVAStart, MVAEnd);
	      
    if(MVAStart > MVAEnd)
    {
    	  M4UERR("m4u_invalid_seq_range_by_mva MVAStart=0x%x, MVAEnd=0x%x\n",
    	    MVAStart, MVAEnd);
    }
        
    spin_lock(&gM4u_reg_lock); 
    M4U_POW_ON_TRY(m4u_index);

    if(FreeSEQRegs[m4u_index] < SEQ_RANGE_NUM)
    {
        for(i=0;i<SEQ_RANGE_NUM;i++)
        {
            if(pRangeDes[i].Enabled == 1 &&
                pRangeDes[i].MVAStart>=MVAStart && 
                pRangeDes[i].MVAEnd<=MVAEnd)
            {
                 pRangeDes[i].Enabled = 0;
                 M4U_WriteReg32(m4u_base, REG_MMU_SQ_START(i), 0);
                 M4U_WriteReg32(m4u_base, REG_MMU_SQ_END(i), 0);
                 m4u_invalid_tlb_by_range(MVAStart, MVAEnd);

                 FreeSEQRegs[m4u_index]++;
                 ret = 0;
                 break;
            }
        }
    }
    

    spin_unlock(&gM4u_reg_lock);

    return ret;

}


/*****************************************************************************
 * FUNCTION
 *    m4u_invalid_seq_range
 * DESCRIPTION
 *    Invalid a sequential range.
 * PARAMETERS
 *    param1 : [IN]  const M4U_MODULE_ID_ENUM eModuleID
 *                Module ID.  
 *    param2 : [IN]  const unsigned int MVAStart
 *                Start seq range.  
 *    param3 : [IN]  const unsigned int MVAEnd
 *                End seq range.   
 * RETURNS
 *    Type: Integer. always zero.
 ****************************************************************************/
int m4u_invalid_seq_range(const M4U_MODULE_ID_ENUM eModuleID, const unsigned int MVAStart, const unsigned int MVAEnd)
{ 
    M4ULOG(" m4u_invalid_seq_range, module:%s, MVAStart:0x%x, MVAEnd:0x%x \r\n", m4u_get_module_name(eModuleID), MVAStart, MVAEnd);
	      
    if(!mva_owner_match(eModuleID, mva2module(MVAStart)))
    {
        M4UERR_WITH_OWNER(" m4u_invalid_seq_rangemodule=%s, MVAStart=0x%x is %s, MVAEnd=0x%x is %s\n", m4u_get_module_name(eModuleID),
    	    m4u_get_module_name(eModuleID), MVAStart, m4u_get_module_name(mva2module(MVAStart)),
    	    MVAEnd, m4u_get_module_name(mva2module(MVAEnd)));
        m4u_dump_mva_info();
    }
    mutex_lock(&gM4uMutex);
    m4u_invalid_seq_range_by_mva(MVAStart, MVAEnd);
    mutex_unlock(&gM4uMutex);
    return 0;

}


/*****************************************************************************
 * FUNCTION
 *    m4u_insert_wrapped_range
 * DESCRIPTION
 *    Insert a wrap range.
 * PARAMETERS
 *    param1 : [IN]  const M4U_MODULE_ID_ENUM eModuleID
 *                Module ID.  
 *    param2 : [IN]  const M4U_PORT_ID_ENUM portID
 *                Port ID. 
 *    param3 : [IN]  const unsigned int MVAStart
 *                Start wrap range.  
 *    param4 : [IN]  const unsigned int MVAEnd
 *                End wrap range.  
 * RETURNS
 *    Type: Integer. -1 mean range overlap or no free wrap range and 0 mean success.
 ****************************************************************************/
int m4u_insert_wrapped_range(const M4U_MODULE_ID_ENUM eModuleID, 
                                    const M4U_PORT_ID_ENUM portID, 
                                    const unsigned int MVAStart, 
                                    const unsigned int MVAEnd)
{
    unsigned int i;
    unsigned int WrapRangeID = M4U_INVALID_ID;
    unsigned int RegVal;
    unsigned int m4u_index = m4u_port_2_m4u_id(portID);
    unsigned int m4u_base = gM4UBaseAddr[m4u_index];
    
    M4ULOG("m4u_insert_wrapped_range, module:%s, port:%s, MVAStart:0x%x, MVAEnd:0x%x \r\n", 
            m4u_get_module_name(eModuleID), m4u_get_port_name(portID), MVAStart, MVAEnd);
	  
            
    if(FreeWrapRegs[m4u_index] == 0)
    {
        M4UMSG("warning: m4u_insert_wrapped_range, no available wrap range found.\n");
        return -1;
    }    	      
    

    if(mva2module(MVAStart)!=eModuleID || mva2module(MVAEnd)!=eModuleID)
    {
        M4UERR_WITH_OWNER("m4u_insert_wrapped_range module=%s, MVAStart=0x%x is %s, MVAEnd=0x%x is %s\n", m4u_get_module_name(eModuleID),
    	    m4u_get_module_name(eModuleID), MVAStart, m4u_get_module_name(mva2module(MVAStart)),
    	    MVAEnd, m4u_get_module_name(mva2module(MVAEnd)));
        m4u_dump_mva_info();
        return -1;
    }
    
    if(MVAStart > MVAEnd)
    {
    	  M4UMSG("m4u_insert_wrapped_range() module=%s, MVAStart=0x%x, MVAEnd=0x%x\n",
    	    m4u_get_port_name(portID), MVAStart, MVAEnd); 
    	  M4UMSG("error: m4u_insert_wrapped_range parameter invalid! \n");
    	  return -1;
    }
    
        
    // check if the range is overlap with previous ones
    for(i=0;i<g4M4UWrapCount[m4u_index];i++)
    {
        if(1==pWrapDes[i].Enabled)
        {
            if(MVAEnd<pWrapDes[i].MVAStart || MVAStart>pWrapDes[i].MVAEnd) //no overlap
            {
            	  continue;
            }
            else
            {
                M4UMSG("error: insert wrap range is overlapped with previous ranges, current!\n");	
                M4UMSG("module=%s, mva_start=0x%x, mva_end=0x%x \n", m4u_get_module_name(eModuleID), MVAStart, MVAEnd);
                M4UMSG("overlapped range id=%d, module=%s, mva_start=0x%x, mva_end=0x%x \n", 
                        i, m4u_get_module_name(pWrapDes[i].eModuleID), pWrapDes[i].MVAStart, pWrapDes[i].MVAEnd);
                return -1;        
            }
        }
    }

    
    spin_lock(&gM4u_reg_lock);        
    ///> look for an inactive range register
    for(i=0;i<g4M4UWrapCount[m4u_index];i++)
    {
        if(pWrapDes[i].Enabled == 0)
        {
            M4ULOG("wrap range found. rangeID=%d \n", i);
            WrapRangeID = i;
            FreeWrapRegs[m4u_index]--;
            break;
        }
    }
            
    if(WrapRangeID == M4U_INVALID_ID)
    {
        M4U_ASSERT(0);
        M4ULOG("can not find available wrap range \n");
        spin_unlock(&gM4u_reg_lock);
        return -1;
    }
    
    pWrapDes[WrapRangeID].Enabled = 1;
    pWrapDes[WrapRangeID].eModuleID = eModuleID;
    pWrapDes[WrapRangeID].ePortID = portID;
    pWrapDes[WrapRangeID].MVAStart = MVAStart&(~0xfff);
    pWrapDes[WrapRangeID].MVAEnd = MVAEnd|(0xfff);

    M4U_POW_ON_TRY(eModuleID);
    // write registers    
    M4U_WriteReg32(m4u_base, REG_MMU_WRAP_SA(WrapRangeID), pWrapDes[WrapRangeID].MVAStart);
    M4U_WriteReg32(m4u_base, REG_MMU_WRAP_EA(WrapRangeID), pWrapDes[WrapRangeID].MVAEnd);

    RegVal = M4U_ReadReg32(m4u_base, REG_MMU_WRAP_EN(portID));
    RegVal &= ~(F_MMU_WRAP_SEL_VAL(portID, 0xf)); //clear sel field
    RegVal |= F_MMU_WRAP_SEL_VAL(portID, WrapRangeID+1);
    M4U_WriteReg32(m4u_base, REG_MMU_WRAP_EN(portID), RegVal);

    M4U_POW_OFF_TRY(eModuleID);
  
    spin_unlock(&gM4u_reg_lock);
    
    return 0;
}


/*****************************************************************************
 * FUNCTION
 *    m4u_invalid_wrapped_range
 * DESCRIPTION
 *    Invalid a wrap range.
 * PARAMETERS
 *    param1 : [IN]  const M4U_MODULE_ID_ENUM eModuleID
 *                Module ID.  
 *    param2 : [IN]  const M4U_PORT_ID_ENUM portID
 *                Port ID. 
 *    param3 : [IN]  const unsigned int MVAStart
 *                Start wrap range.  
 *    param4 : [IN]  const unsigned int MVAEnd
 *                End wrap range.  
 * RETURNS
 *    Type: Integer. -1 mean parameter error and 0 mean success or no such wrap range.
 ****************************************************************************/
int m4u_invalid_wrapped_range(const M4U_MODULE_ID_ENUM eModuleID, 
                              const M4U_PORT_ID_ENUM portID,
                              const unsigned int MVAStart, 
                              const unsigned int MVAEnd)
{
    unsigned int i;
    unsigned int WrapRangeID = M4U_INVALID_ID;
    unsigned int RegVal;
    const unsigned int m4u_index = 0;
    unsigned int m4u_base = gM4UBaseAddr[m4u_index];
	  
    M4ULOG("m4u_invalid_wrapped_range, module:%s, MVAStart:0x%x, MVAEnd:0x%x \r\n", m4u_get_module_name(eModuleID), MVAStart, MVAEnd);
	      

    if(!mva_owner_match(eModuleID, mva2module(MVAStart)))
    {
        M4UERR_WITH_OWNER("m4u_invalid_wrapped_range module=%s, MVAStart=0x%x is %s, MVAEnd=0x%x is %s\n", m4u_get_module_name(eModuleID), 
    	    m4u_get_module_name(eModuleID), MVAStart, m4u_get_module_name(mva2module(MVAStart)),
    	    MVAEnd, m4u_get_module_name(mva2module(MVAEnd)));
        m4u_dump_mva_info();
        return -1;
    }

    if(MVAStart > MVAEnd)
    {
    	  M4UERR("m4u_invalid_wrapped_range,  module=%s, MVAStart=0x%x, MVAEnd=0x%x\n",
    	    m4u_get_module_name(eModuleID), MVAStart, MVAEnd);
    }  
        
    spin_lock(&gM4u_reg_lock);
    
    if(FreeWrapRegs[m4u_index] < g4M4UWrapCount[m4u_index])
    {
        for(i=0;i<g4M4UWrapCount[m4u_index];i++)
        {
            if(pWrapDes[i].Enabled == 1 &&
                pWrapDes[i].MVAStart>=(MVAStart&(~0xfff)) && 
                pWrapDes[i].MVAEnd<=(MVAEnd|(0xfff)))
            {
                pWrapDes[i].Enabled = 0;
                WrapRangeID = i;
                FreeWrapRegs[m4u_index]++;
            }
        }
    }

    if(WrapRangeID == M4U_INVALID_ID)
    {
        M4ULOG("warning: m4u_invalid_wrapped_range(), does not find wrap range \n");
        spin_unlock(&gM4u_reg_lock);
        return 0;
    }

    pWrapDes[WrapRangeID].Enabled = 0;
    pWrapDes[WrapRangeID].eModuleID = 0;
    pWrapDes[WrapRangeID].ePortID = 0;
    pWrapDes[WrapRangeID].MVAStart = 0;
    pWrapDes[WrapRangeID].MVAEnd = 0;

    M4U_POW_ON_TRY(eModuleID);
    //write register 
    RegVal = M4U_ReadReg32(m4u_base, REG_MMU_WRAP_EN(portID));
    RegVal &= ~(F_MMU_WRAP_SEL_VAL(portID, 0xf)); //clear sel field
    M4U_WriteReg32(m4u_base, REG_MMU_WRAP_EN(portID), RegVal);
	
    M4U_WriteReg32(m4u_base, REG_MMU_WRAP_SA(WrapRangeID), 0);
    M4U_WriteReg32(m4u_base, REG_MMU_WRAP_EA(WrapRangeID), 0);

    M4U_POW_OFF_TRY(eModuleID);
	spin_unlock(&gM4u_reg_lock);
	  
    return 0;

}


/*****************************************************************************
 * FUNCTION
 *    m4u_config_port
 * DESCRIPTION
 *    Config port setting about virtuality, security, domain, and prefetch distance with direction.
 * PARAMETERS
 *    param1 : [IN]  M4U_PORT_STRUCT* pM4uPort
 *                port configuration.  
 * RETURNS
 *    Type: Integer. always zero.
 ****************************************************************************/
int m4u_config_port(M4U_PORT_STRUCT* pM4uPort) //native
{

    M4U_PORT_ID_ENUM PortID = (pM4uPort->ePortID);
    unsigned int m4u_base = gM4UBaseAddr[m4u_port_2_m4u_id(PortID)];
    M4U_MODULE_ID_ENUM eModuleID = m4u_port_2_module(PortID);
    unsigned int sec_con_val = 0;
    
    M4ULOG("m4u_config_port(), port=%s, Virtuality=%d, Distance=%d, Direction=%d \n", 
        m4u_get_port_name(pM4uPort->ePortID), pM4uPort->Virtuality, pM4uPort->Distance, pM4uPort->Direction);

    MMProfileLogEx(M4U_MMP_Events[PROFILE_CONFIG_PORT], MMProfileFlagStart, eModuleID, pM4uPort->ePortID);
    spin_lock(&gM4u_reg_lock);
    M4U_POW_ON_TRY(eModuleID);

    // Direction, one bit for each port, 1:-, 0:+
    m4uHw_set_field_by_mask(m4u_base, REG_MMU_PFH_DIR(PortID),\
                F_MMU_PFH_DIR_MASK(PortID), F_MMU_PFH_DIR_VAL(PortID, pM4uPort->Direction));
        
    // Distance
    if(pM4uPort->Distance>0xf)
    {
        M4ULOG("m4u_config_port() error, port=%s, Distance=%d\n", 
            m4u_get_port_name(pM4uPort->ePortID), pM4uPort->Distance);
    }

    m4uHw_set_field_by_mask(m4u_base, REG_MMU_PFH_DIST(PortID),\
                F_MMU_PFH_DIST_MASK(PortID), F_MMU_PFH_DIST_VAL(PortID,pM4uPort->Distance));

    // Virtuality, 1:V, 0:P
    if(pM4uPort->Virtuality)
    { 
        sec_con_val |= F_SMI_SECUR_CON_VIRTUAL(PortID);
    }

    if(pM4uPort->Security)
    {
#ifdef M4U_USE_ONE_PAGETABLE	
        M4UERR("Error: MT6572 don't support secure domain!");
#else
        sec_con_val |= F_SMI_SECUR_CON_SECURE(PortID);
#endif
    }

#if 1
    if(M4U_PORT_CMDQ == pM4uPort->ePortID && 1 == pM4uPort->Virtuality)
    {
        M4UERR("Error: M4U_PORT_CMDQ, set port virtuality error");
    }
#endif	

    sec_con_val |= F_SMI_SECUR_CON_DOMAIN(PortID, 3);//pM4uPort->domain);

    m4uHw_set_field_by_mask(0, REG_SMI_SECUR_CON_OF_PORT(PortID),\
                F_SMI_SECUR_CON_MASK(PortID), sec_con_val);
    
    M4U_POW_OFF_TRY(eModuleID); 
    spin_unlock(&gM4u_reg_lock);

    MMProfileLogEx(M4U_MMP_Events[PROFILE_CONFIG_PORT], MMProfileFlagEnd, pM4uPort->Virtuality, pM4uPort->ePortID);
    return 0;

}


// config rotator port, need several parameter to improve performance
int m4u_config_port_rotator(M4U_PORT_STRUCT_ROTATOR *pM4uPort)
{ 
      
    return 0;
}


/*****************************************************************************
 * FUNCTION
 *    m4u_get_perf_counter
 * DESCRIPTION
 *    Get monitor data implement.
 * PARAMETERS
 *    None.
 * RETURNS
 *    None.
 ****************************************************************************/
void m4u_get_perf_counter(M4U_PERF_COUNT *pM4U_perf_count)
{
    unsigned int m4u_base = gM4UBaseAddr[0];
    pM4U_perf_count->transaction_cnt= M4U_ReadReg32(m4u_base, REG_MMU_ACC_CNT);    ///> Transaction access count
    pM4U_perf_count->main_tlb_miss_cnt= M4U_ReadReg32(m4u_base, REG_MMU_MAIN_MSCNT); ///> Main TLB miss count
    pM4U_perf_count->pfh_tlb_miss_cnt= M4U_ReadReg32(m4u_base, REG_MMU_PF_MSCNT);   ///> Prefetch TLB miss count
    pM4U_perf_count->pfh_cnt = M4U_ReadReg32(m4u_base, REG_MMU_PF_CNT);     ///> Prefetch count
}

/*****************************************************************************
 * FUNCTION
 *    m4u_monitor_start
 * DESCRIPTION
 *    Start monitor implement.
 * PARAMETERS
 *    None.
 * RETURNS
 *    Type: Integer. always zero.
 ****************************************************************************/
int m4u_monitor_start(void)
{
    unsigned int m4u_base = gM4UBaseAddr[0];
    
    M4ULOG("start monitor\n");
    
    M4U_POW_ON_TRY(0);
    //clear GMC performance counter
    m4uHw_set_field_by_mask(m4u_base, REG_MMU_CTRL_REG, 
                F_MMU_CTRL_MONITOR_CLR(1), F_MMU_CTRL_MONITOR_CLR(1));
    m4uHw_set_field_by_mask(m4u_base, REG_MMU_CTRL_REG, 
                F_MMU_CTRL_MONITOR_CLR(1), F_MMU_CTRL_MONITOR_CLR(0));

    //enable GMC performance monitor
    m4uHw_set_field_by_mask(m4u_base, REG_MMU_CTRL_REG, 
                F_MMU_CTRL_MONITOR_EN(1), F_MMU_CTRL_MONITOR_EN(1));

    M4U_POW_OFF_TRY(0);


    return 0;
}


/*****************************************************************************
 * FUNCTION
 *    m4u_monitor_stop
 * DESCRIPTION
 *    Stop monitor implement.
 * PARAMETERS
 *    None.
 * RETURNS
 *    Type: Integer. always zero.
 ****************************************************************************/
int m4u_monitor_stop(void)
{
    M4U_PERF_COUNT cnt;
    unsigned int m4u_base = gM4UBaseAddr[0];
    
    M4ULOG("stop monitor\n");

    M4U_POW_ON_TRY(0);
    //disable GMC performance monitor
    m4uHw_set_field_by_mask(m4u_base, REG_MMU_CTRL_REG, 
                F_MMU_CTRL_MONITOR_EN(1), F_MMU_CTRL_MONITOR_EN(0));

    m4u_get_perf_counter(&cnt);
    //read register get the count

    M4UMSG("[M4U] total:%d, main miss:%d, pfh miss(walk):%d, auto pfh:%d\n", 
        cnt.transaction_cnt, cnt.main_tlb_miss_cnt, cnt.pfh_tlb_miss_cnt,cnt.pfh_cnt);
    
    
    if(0!=cnt.transaction_cnt)
    {
        M4UMSG("main miss:%d%%, pfh miss:%d%%\n", 
            100*cnt.main_tlb_miss_cnt/cnt.transaction_cnt,
            100*cnt.pfh_tlb_miss_cnt/cnt.transaction_cnt);
    }
    else
    {
        M4UMSG("[M4U] no transaction happened! \r\n");
    }
    M4U_POW_OFF_TRY(0);
    
    return 0;
}


/*****************************************************************************
 * FUNCTION
 *    m4u_print_perf_counter
 * DESCRIPTION
 *    Call m4u_get_perf_counter, and print monitor data.
 * PARAMETERS
 *    param1 : [IN]  const char *msg
 *                User data.  
 * RETURNS
 *    None.
 ****************************************************************************/
void m4u_print_perf_counter(const char *msg)
{
    M4U_PERF_COUNT cnt;
    m4u_get_perf_counter(&cnt);
    M4UMSG("====m4u performance count for %s======\n", msg);
    M4UMSG("total trans=%d, main_miss=%d, pfh_miss=%d, pfh_cnt=%d\n",
        cnt.transaction_cnt, cnt.main_tlb_miss_cnt, cnt.pfh_tlb_miss_cnt, cnt.pfh_cnt);
}

#define M4U_ERR_PAGE_UNLOCKED -101

/*****************************************************************************
 * FUNCTION
 *    m4u_put_unlock_page
 * DESCRIPTION
 *    Unlock user specified page.
 * PARAMETERS
 *    param1 : [IN]  struct page* const page
 *                Specified page.  
 * RETURNS
 *    Type: Integer. zero mean success, and M4U_ERR_PAGE_UNLOCKED  mean page is unlocked.
 ****************************************************************************/
static int m4u_put_unlock_page(struct page* const page)
{
    unsigned int pfn;
    int ret = 0;
    int trycnt;

    pfn = page_to_pfn(page);
    MMProfileLogEx(M4U_MMP_Events[PROFILE_MUNLOCK], MMProfileFlagStart, 0, (unsigned int)(pfn<<12));

    if(pMlock_cnt[pfn])
    {
        if(!PageMlocked(page))
        {
            ret = M4U_ERR_PAGE_UNLOCKED;
        }
        
        pMlock_cnt[pfn]--;
        if(pMlock_cnt[pfn] == 0)
        {
            /* Use retry version to guarantee no leakage */
            trycnt = 3000;
            do {
                if (trylock_page(page)) {
                    munlock_vma_page(page);
                    unlock_page(page);
                    break;
                }
                mdelay(5);
            } while (trycnt-- > 0);

            if(PageMlocked(page)==1)
            {
                M4UMSG(" Can't munlock page: \n");
                dump_page(page);
            }
        }
    }
    else
    {
        M4UMSG("warning pMlock_cnt[%d]==0 !! \n", pfn);
        ret = M4U_ERR_PAGE_UNLOCKED;
    }
    MMProfileLogEx(M4U_MMP_Events[PROFILE_MUNLOCK], MMProfileFlagEnd, 0, 0x1000);
    MMProfileLogEx(M4U_MMP_Events[PROFILE_PUT_PAGE], MMProfileFlagStart, 0, pfn<<12);
    put_page(page);
    MMProfileLogEx(M4U_MMP_Events[PROFILE_PUT_PAGE], MMProfileFlagEnd, 0, 0x1000);

    return ret;
    
}

/*****************************************************************************
 * FUNCTION
 *    m4u_get_pages
 * DESCRIPTION
 *    call internal function to obtain physical address accroding user memory space.
 * PARAMETERS
 *    param1 : [IN]  const M4U_MODULE_ID_ENUM eModuleID
 *                Module ID.  
 *    param2 : [IN]  const unsigned int BufAddr
 *                User allocated virtual memory address.  
 *    param3 : [IN]  const unsigned int BufSize
 *                User allocated memory size.  
 *    param4 : [IN]  unsigned int* const pPhys
 *                Page table address. 
 * RETURNS
 *    Type: Integer. page number, and zero or negative value mean fail.
 ****************************************************************************/
static int m4u_get_pages(const M4U_MODULE_ID_ENUM eModuleID, const unsigned int BufAddr, const unsigned int BufSize, unsigned int* const pPhys)
{
    int ret,i;
    int page_num = M4U_GET_PAGE_NUM(BufAddr, BufSize);
    unsigned int start_pa;    
    unsigned int write_mode = 0;
    struct vm_area_struct *vma = NULL;
    
    
    M4ULOG("^ m4u_get_pages: module=%s, BufAddr=0x%x, BufSize=%d \n", m4u_get_module_name(eModuleID), BufAddr, BufSize);      

    if(M4U_CLNTMOD_LCDC_UI==eModuleID)
    {
        for(i=0;i<page_num;i++)
        {
            pPhys[i] = (BufAddr&0xfffff000) + i*DEFAULT_PAGE_SIZE;
        } 
       
    }  
    else if(BufAddr<PAGE_OFFSET)  // from user space
    {
        start_pa = m4u_user_v2p(BufAddr);
        if(0==start_pa)
        {
            M4ULOG("m4u_user_v2p=0 in m4u_get_pages() \n");
        }
        if(is_pmem_range((unsigned long*)start_pa, BufSize))
        {
            M4UMSG("warning: m4u_get_pages virtual addr from pmem! start_pa=0x%x\n", start_pa);
            for(i=0;i<page_num;i++)
            {
                *(pPhys+i) = m4u_user_v2p((BufAddr&0xfffff000) + i*DEFAULT_PAGE_SIZE);
            }   
        }    
        else 
        {
            if(BufSize>MAX_BUF_SIZE_TO_GET_USER_PAGE)
            {
            	  //M4UMSG("error: m4u_get_pages(), size is bigger than 32MB size=%d \n", BufSize);
            	  M4UERR(": m4u_get_pages(), single time alloc size=0x%x, bigger than limit=0x%x \n", BufSize, MAX_BUF_SIZE_TO_GET_USER_PAGE);
            	  return -EFAULT;
            } 
            
            down_read(&current->mm->mmap_sem);
            
            vma = find_vma(current->mm, BufAddr);
            if(vma == NULL)
            {
                M4UMSG("cannot find vma: module=%s, va=0x%x, size=0x%x\n", 
                    m4u_get_module_name(eModuleID), BufAddr, BufSize);
                m4u_dump_maps(BufAddr);
                up_read(&current->mm->mmap_sem);				
                
                return -1;
            }
            write_mode = (vma->vm_flags&VM_WRITE)?1:0;


            if((vma->vm_flags) & VM_PFNMAP)
            {
                unsigned int bufEnd = BufAddr + BufSize -1;

                if(bufEnd > vma->vm_end)
                {
                    M4UMSG("error: page_num=%d,module=%s, va=0x%x, size=0x%x, vm_flag=0x%x\n", 
                        page_num, m4u_get_module_name(eModuleID), BufAddr, BufSize, vma->vm_flags);                    	
                    M4UMSG("but vma is: vm_start=0x%x, vm_end=0x%x\n", vma->vm_start, vma->vm_end);
                    up_read(&current->mm->mmap_sem);
                    return -1;
                }

                for(i=0; i<page_num; i++)
                {
                    unsigned int va_align = BufAddr&(~M4U_PAGE_MASK);
                    *(pPhys+i) = m4u_user_v2p(va_align + 0x1000*i);
                }

                M4UMSG("alloc_mva VM_PFNMAP module=%s, va=0x%x, size=0x%x, vm_flag=0x%x\n", 
                    m4u_get_module_name(eModuleID), BufAddr, BufSize, vma->vm_flags);                    	
                up_read(&current->mm->mmap_sem);
            }
            else
            {

                ret = m4u_get_user_pages(
                	eModuleID,
                	current,
                	current->mm,
                	BufAddr,
                	page_num,
                	write_mode, //m4u_get_write_mode_by_module(eModuleID),	// 1 /* write */
                	0,	/* force */
                	(struct page**)pPhys,
                	NULL);
    
                up_read(&current->mm->mmap_sem);
                
                if(ret<page_num)  //Error handle
                {
                	  // release pages first
                	for(i=0;i<ret;i++)
                    {
                        m4u_put_unlock_page((struct page*)(*(pPhys+i)));
                    }
                    
                    if(unlikely(fatal_signal_pending(current)))
                    {
                        M4UMSG("error: receive sigkill during get_user_pages(),  page_num=%d, return=%d, module=%s, current_process:%s \n", 
                            page_num, ret, m4u_get_module_name(eModuleID), current->comm);
                    }
                    else
                    {
                        if(ret>0) //return value bigger than 0 but smaller than expected, trigger red screen
                        {
                            M4UMSG("error: page_num=%d, get_user_pages return=%d, module=%s, current_process:%s \n", 
                                page_num, ret, m4u_get_module_name(eModuleID), current->comm);                    	
                            M4UMSG("error hint: maybe the allocated VA size is smaller than the size configured to m4u_alloc_mva()!");
                        }
                        else  // return vaule is smaller than 0, maybe the buffer is not exist, just return error to up-layer
                        {                    	                    
                            M4UMSG("error: page_num=%d, get_user_pages return=%d, module=%s, current_process:%s \n", 
                                page_num, ret, m4u_get_module_name(eModuleID), current->comm);                    	
                            M4UMSG("error hint: maybe the VA is deallocated before call m4u_alloc_mva(), or no VA has be ever allocated!");
                        }
                        m4u_dump_maps(BufAddr);
                    }
                
                    return -EFAULT;                
                }
    
                // add locked pages count, used for debug whether there is memory leakage
                pmodule_locked_pages[eModuleID] += page_num;
                        
                for(i=0;i<page_num;i++)
                {
                    *(pPhys+i) = page_to_phys((struct page*)(*(pPhys+i)))|0x20;
                }		
        
                M4UDBG("[user verify] BufAddr_sv=0x%x, BufAddr_sp=0x%x, BufAddr_ev=0x%x, BufAddr_ep=0x%x \n",
                            BufAddr, 
                            m4u_user_v2p(BufAddr), 
                            BufAddr+BufSize-1, 
                            m4u_user_v2p(BufAddr+BufSize-1));                    
            }
        }
    }
    else // from kernel space
    {
        if(BufAddr>=VMALLOC_START && BufAddr<=VMALLOC_END) // vmalloc
        {
            struct page * ppage;
            for(i=0;i<page_num;i++)
            {          	
                ppage=vmalloc_to_page((unsigned int *)(BufAddr + i*DEFAULT_PAGE_SIZE));            
                *(pPhys+i) = page_to_phys(ppage) & 0xfffff000 ;
            }
            M4UDBG("[kernel verify] vmalloc BufAddr_sv=0x%x, BufAddr_sp=0x%x \n",
                    BufAddr, 
                    *pPhys);			
			
        }
        else // kmalloc
        {
            for(i=0;i<page_num;i++)
            {
                *(pPhys+i) = virt_to_phys((void*)((BufAddr&0xfffff000) + i*DEFAULT_PAGE_SIZE));
            }        	
            M4UDBG("[kernel verify] kmalloc BufAddr_sv=0x%x, BufAddr_sp=0x%x \n",
                    BufAddr, 
                    virt_to_phys((void*)BufAddr));			
        }
        

    }

    return page_num;
}

static int m4u_get_pages_sg(M4U_MODULE_ID_ENUM eModuleID, unsigned int BufAddr, unsigned int BufSize, 
    struct sg_table* sg_table, unsigned int* pPhys)
{
    int i,j;
    int page_num, map_page_num;
    struct scatterlist *sg;
    
    M4ULOG("^ m4u_get_pages_sg: module=%s, BufAddr=0x%x, BufSize=%d \n", m4u_get_module_name(eModuleID), BufAddr, BufSize);
    
    // caculate page number
    page_num = (BufSize + (BufAddr&0xfff))/DEFAULT_PAGE_SIZE;
    if((BufAddr+BufSize)&0xfff)
    {
        page_num++;
    }  

    map_page_num = 0;

	for_each_sg(sg_table->sgl, sg, sg_table->nents, i) 
    {
    	int npages_this_entry = PAGE_ALIGN(sg_dma_len(sg)) / PAGE_SIZE;
    	struct page *page = sg_page(sg);
        for (j = 0; j < npages_this_entry; j++) 
        {
            *(pPhys+map_page_num) = page_to_phys(page++) & 0xfffff000;
            map_page_num++;
            BUG_ON(map_page_num > page_num);
        }
    }

    return map_page_num;
}

/*****************************************************************************
 * FUNCTION
 *    m4u_release_pages
 * DESCRIPTION
 *    1. Get page struct of target memory address from MVA. (if user space memory)
 *    2. Call m4u_put_unlock_page to unlock page. (if user space memory)
 *    3. Record memory usage.
 * PARAMETERS
 *    param1 : [IN]  const M4U_MODULE_ID_ENUM eModuleID
 *                Module ID.  
 *    param2 : [IN]  const unsigned int BufAddr
 *                User allocated virtual memory address.  
 *    param3 : [IN]  const unsigned int BufSize
 *                User allocated memory size.  
 *    param4 : [IN]  const unsigned int MVA
 *                MVA for query page table. 
 * RETURNS
 *    None.
 ****************************************************************************/
void m4u_release_pages(const M4U_MODULE_ID_ENUM eModuleID, const unsigned int BufAddr,
                       const unsigned int BufSize, const unsigned int MVA, struct sg_table* sg_table)
{
    unsigned int page_num = M4U_GET_PAGE_NUM(BufAddr, BufSize);
    unsigned int i=0;
    unsigned int start_pa;
    struct page *page;
    int put_page_err = 0, tmp;
    M4ULOG("m4u_release_pages(),  module=%s, BufAddr=0x%x, BufSize=%d\n", m4u_get_module_name(eModuleID), BufAddr, BufSize);

    if(!mva_owner_match(eModuleID, mva2module(MVA)))
    {
        M4UERR_WITH_OWNER("m4u_release_pages module=%s, MVA=0x%x, expect module is %s \n", m4u_get_module_name(eModuleID), 
    	    m4u_get_module_name(eModuleID), MVA, m4u_get_module_name(mva2module(MVA)));
        m4u_mvaGraph_dump();
    }

    if(M4U_CLNTMOD_LCDC_UI==eModuleID)
    {
        goto RELEASE_FINISH;	
    }

    if(BufAddr<PAGE_OFFSET && sg_table==NULL)  // from user space
    {	
        // put page by finding PA in pagetable
        unsigned int* pPageTableAddr = mva_pteAddr(MVA);    

        for(i=0;i<page_num;i++)
        {
            start_pa = *(pPageTableAddr+i);
            if((start_pa&0x02)==0)
            {
                continue;
            }
            else if(!(start_pa & 0x20))
            {
                continue;
            }

            page = pfn_to_page(__phys_to_pfn(start_pa));
    	  
            //we should check page count before call put_page, because m4u_release_pages() may fail in the middle of buffer
            //that is to say, first several pages may be put successfully in m4u_release_pages()
            if(page_count(page)>0) 
            {
                //to avoid too much log, we only save tha last err here.
                if((tmp=m4u_put_unlock_page(page)))
                    put_page_err = tmp;
            }         
            pmodule_locked_pages[eModuleID]--;   
            *(pPageTableAddr+i) &= (~0x2); 
        }
        if(put_page_err == M4U_ERR_PAGE_UNLOCKED)
        {
            M4UMSG("warning: in m4u_release_page: module=%s, va=0x%x, size=0x%x,mva=0x%x (page is unlocked before put page)\n", 
                m4u_get_module_name(eModuleID), BufAddr, BufSize, MVA);
        }
    } //end of "if(BufAddr<PAGE_OFFSET)"
    M4UDBG("m4u_release_pages() finish!,  module=%s, BufAddr=0x%x, BufSize=%d\n", m4u_get_module_name(eModuleID), BufAddr, BufSize);

RELEASE_FINISH:
    // record memory usage
    if(pmodule_current_size[eModuleID]<BufSize)
    {
        pmodule_current_size[eModuleID] = 0;
        M4UMSG("error pmodule_current_size is less than BufSize, module=%s, current_size=%d, BufSize=%d \n", 
           m4u_get_module_name(eModuleID), pmodule_current_size[eModuleID], BufSize);
    }
    else
    {
        pmodule_current_size[eModuleID] -= BufSize;
    }

}


#define BUFFER_SIZE_FOR_FLUSH_ALL (256*1024) //(864*480*2)

#ifndef __M4U_CACHE_SYCN_USING_KERNEL_MAP__

/*****************************************************************************
 * FUNCTION
 *    m4u_dma_cache_maint
 * DESCRIPTION
 *    Refer to dma_cache_maint()
 *    The function works for user virtual addr 
 * PARAMETERS
 *    param1 : [IN]  const M4U_MODULE_ID_ENUM eModuleID
 *                Moudle ID.  
 *    param2 : [IN]  const void *start
 *                User virtual start address.  
 *    param3 : [IN]  const size_t size
 *                Memory size.  
 *    param4 : [IN]  const int direction
 *                DMA_FROM_DEVICE or DMA_TO_DEVICE or DMA_BIDIRECTIONAL.  
 * RETURNS
 *    Integer. Zero mean success and -1 mean error.
 ****************************************************************************/
int m4u_dma_cache_maint(const M4U_MODULE_ID_ENUM eModuleID, const void *start, const size_t size, const int direction)
{
    void (*outer_op)(phys_addr_t start, phys_addr_t end);
//	void (*outer_op)(unsigned long, unsigned long);
	void (*outer_op_all)(void);
	unsigned int page_start, page_num;
    unsigned int *pPhy = NULL;
    int i, ret=0;
    PROFILE_TYPE ptype=PROFILE_DMA_MAINT_ALL;
    switch (direction) {
	case DMA_FROM_DEVICE:
        if(size < BUFFER_SIZE_FOR_FLUSH_ALL)
            ptype = PROFILE_DMA_INVALID_RANGE;
        else
            ptype = PROFILE_DMA_INVALID_ALL;
		break;
	case DMA_TO_DEVICE:
        if(size < BUFFER_SIZE_FOR_FLUSH_ALL)
            ptype = PROFILE_DMA_CLEAN_RANGE;
        else
            ptype = PROFILE_DMA_CLEAN_ALL;
        break;
	case DMA_BIDIRECTIONAL:
        if(size < BUFFER_SIZE_FOR_FLUSH_ALL)
            ptype = PROFILE_DMA_FLUSH_RANGE;
        else
            ptype = PROFILE_DMA_FLUSH_ALL;
		break;
	default:
        break;
	}
    MMProfileLogEx(M4U_MMP_Events[ptype], MMProfileFlagStart, eModuleID, (unsigned int)start);

    M4ULOG(" m4u_dma_cache_maint():  module=%s, start=0x%x, size=%d, direction=%d \n",
          m4u_get_module_name(eModuleID), (unsigned int)start, size, direction);

    mutex_lock(&gM4uMutex);

    if(0==start)
    {
        M4UERR_WITH_OWNER(" m4u_dma_cache_maint():  module=%s, start=0x%x, size=%d, direction=%d \n", m4u_get_module_name(eModuleID), 
        m4u_get_module_name(eModuleID), (unsigned int)start, size, direction);
        goto out;
    }         

  //To avoid non-cache line align cache corruption, user should make sure
  //cache start addr and size both cache-line-bytes align
  //we check start addr here but size should be checked in memory allocator
  //Rotdma memory is allocated by surfacefligner, address is not easy to modify
  //so do not check them now, should followup after MP
    if( m4u_get_dir_by_module(eModuleID)== M4U_DMA_WRITE &&
        (((unsigned int)start%L1_CACHE_BYTES!=0) || (size%L1_CACHE_BYTES)!=0)
       )
    {
        if(1) //screen red in debug mode
        {
      		M4UERR_WITH_OWNER("error: addr un-align, module=%s, addr=0x%x, size=0x%x, process=%s, align=0x%x\n",  m4u_get_module_name(eModuleID), 
      	        m4u_get_module_name(eModuleID), (unsigned int)start, size, current->comm, L1_CACHE_BYTES);
      	}
      	else
      	{
      		M4UMSG("error: addr un-align, module=%s, addr=0x%x, size=0x%x, process=%s, align=0x%x\n", 
      	        m4u_get_module_name(eModuleID), (unsigned int)start, size, current->comm, L1_CACHE_BYTES);
      	}
    }
          
	switch (direction) {
	case DMA_FROM_DEVICE:		/* invalidate only, HW write to memory */
        //M4UMSG("error: someone call cache maint with DMA_FROM_DEVICE, module=%s\n",m4u_get_module_name(eModuleID));
		outer_op = outer_inv_range;
		outer_op_all = outer_inv_all;  
		break;
	case DMA_TO_DEVICE:		/* writeback only, HW read from memory */
		outer_op = outer_clean_range;
		outer_op_all = outer_flush_all;
		break;
	case DMA_BIDIRECTIONAL:		/* writeback and invalidate */
		outer_op = outer_flush_range;
		outer_op_all = outer_flush_all;
		break;
	default:
		M4UERR("m4u_dma_cache_maint, direction=%d is invalid \n", direction);
                goto out;
	}


//<===========================================================================
//< check whether input buffer is valid (has physical pages allocated)
	page_start = (unsigned int)start & 0xfffff000;
	page_num = M4U_GET_PAGE_NUM((unsigned int)start, size);
    // M4UMSG("start: 0x%x, page_start: 0x%x, page_num:%d\n", start, page_start, page_num);	

    if(size < BUFFER_SIZE_FOR_FLUSH_ALL)
    {
        pPhy = kmalloc(sizeof(int)*page_num, GFP_KERNEL);
        if(pPhy == NULL)
        {
            M4UMSG("error to kmalloc in m4u_cache_maint: module=%s, start=0x%x, size=%d, direction=%d \n", 
                m4u_get_module_name(eModuleID), (unsigned int)start, size, direction);
            goto out;
        }

        if((unsigned int)start<PAGE_OFFSET)  // from user space
        {
            for(i=0; i<page_num; i++,page_start+=DEFAULT_PAGE_SIZE)
            {
                //struct page* page;
                pPhy[i] = m4u_user_v2p(page_start);
                //page = phys_to_page(pPhy[i]);
                // M4UMSG("page_start=0x%x, pPhy[%d]=0x%x\n",(unsigned int)page_start, i, pPhy[i]);
                if((pPhy[i]==0))// || (!PageMlocked(page))) 
                {
                    ret=-1;
                    M4UMSG("error: cache_maint() fail, module=%s, start=0x%x, page_start=0x%x, size=%d, pPhy[%d]=0x%x\n", 
                            m4u_get_module_name(eModuleID), (unsigned int)start, (unsigned int)page_start, size, i, pPhy[i]);
                    //dump_page(page);
                    m4u_dump_maps((unsigned int)start);
                    goto out;
                }
            }
        }
        else if((unsigned int)start>=VMALLOC_START && (unsigned int)start<=VMALLOC_END) // vmalloc
        {

            struct page * ppage;

            for(i=0; i<page_num; i++,page_start+=DEFAULT_PAGE_SIZE)
            {
                ppage=vmalloc_to_page((void *)page_start); 
                if(ppage == NULL) 
                {
                    ret=-1;
                    M4UMSG("error: ppage is 0 in cache_maint of vmalloc!, module=%s, start=0x%x, pagestart=0x%x\n", 
                            m4u_get_module_name(eModuleID), (unsigned int)start,page_start);
                    goto out;
                }
                pPhy[i] = page_to_phys(ppage);
            }
        }
        else // kmalloc
        {
            for(i=0; i<page_num; i++,page_start+=DEFAULT_PAGE_SIZE)
            {
                pPhy[i] = virt_to_phys((void*)page_start);
            }        	
        }
        
    }

//=====================================================================================
// L1 cache clean before hw read
    if(L1_CACHE_SYNC_BY_RANGE_ONLY)
    {
    	if (direction == DMA_TO_DEVICE) 
    	{
            dmac_map_area(start, size, direction);
    	}

    	if (direction == DMA_BIDIRECTIONAL) 
    	{
            dmac_flush_range(start, start+size-1);
    	}

    }
    else
    {
        smp_inner_dcache_flush_all();
    }

//=============================================================================================
	// L2 cache maintenance by physical pages
    if(size<BUFFER_SIZE_FOR_FLUSH_ALL)
    {
        for (i=0; i<page_num; i++) 
        {
    		outer_op(pPhy[i], pPhy[i]+ DEFAULT_PAGE_SIZE);
    	}
    }
    else 
    {
        outer_op_all();
    }
//=========================================================================================      
	// L1 cache invalidate after hw write to memory
    if(L1_CACHE_SYNC_BY_RANGE_ONLY)
    {
    	if (direction == DMA_FROM_DEVICE) 
        {
    	    dmac_unmap_area(start, size, direction);
        }
    }
  
out:
    if(pPhy != NULL)
        kfree(pPhy);

    MMProfileLogEx(M4U_MMP_Events[ptype], MMProfileFlagEnd, eModuleID, size);

    mutex_unlock(&gM4uMutex);
        
    return ret;
}

static int m4u_cache_sync_init(void)
{
    return 0;
}

#else

static unsigned int m4u_cache_v2p(unsigned int va)
{
    unsigned int pageOffset = (va & (PAGE_SIZE - 1));
    pgd_t *pgd;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    unsigned int pa;

    if(NULL==current)
    {
    	  M4UMSG("warning: m4u_user_v2p, current is NULL! \n");
    	  return 0;
    }
    if(NULL==current->mm)
    {
    	  M4UMSG("warning: m4u_user_v2p, current->mm is NULL! tgid=0x%x, name=%s \n", current->tgid, current->comm);
    	  return 0;
    }
        
    pgd = pgd_offset(current->mm, va); /* what is tsk->mm */
    if(pgd_none(*pgd)||pgd_bad(*pgd))
    {
        M4UMSG("m4u_user_v2p(), va=0x%x, pgd invalid! \n", va);
        return 0;
    }

    pud = pud_offset(pgd, va);
    if(pud_none(*pud)||pud_bad(*pud))
    {
        M4UMSG("m4u_user_v2p(), va=0x%x, pud invalid! \n", va);
        return 0;
    }
    
    pmd = pmd_offset(pud, va);
    if(pmd_none(*pmd)||pmd_bad(*pmd))
    {
        M4UMSG("m4u_user_v2p(), va=0x%x, pmd invalid! \n", va);
        return 0;
    }
        
    pte = pte_offset_map(pmd, va);
    if(pte_present(*pte)) 
    { 
        pa=(pte_val(*pte) & (PAGE_MASK)) | pageOffset; 
        pte_unmap(pte);
        return pa; 
    }   

    pte_unmap(pte);


    M4UMSG("m4u_user_v2p(), va=0x%x, pte invalid! \n", va);
    // m4u_dump_maps(va);
    
    return 0;
}

static struct page* m4u_cache_get_page(unsigned int va)
{
    unsigned int pa, start;
    struct page *page;

    start = va & (~M4U_PAGE_MASK);
    pa = m4u_cache_v2p(start);
    if((pa==0))
    {
        M4UMSG("error m4u_get_phys user_v2p return 0 on va=0x%x\n", start);
        //dump_page(page);
        m4u_dump_maps((unsigned int)start);
        return NULL;
    }
    page = phys_to_page(pa);

    return page;
}


static int __m4u_cache_sync_kernel(const void *start, size_t size, int direction)
{
    int ret = 0;

    if (direction == DMA_TO_DEVICE) //clean
    {
        dmac_map_area((void*)start, size, DMA_TO_DEVICE);
    }
    else if (direction == DMA_FROM_DEVICE) // invalid
    {
        dmac_unmap_area((void*)start, size, DMA_FROM_DEVICE);
    }
    else if (direction == DMA_BIDIRECTIONAL) //flush
    {
        dmac_flush_range((void*)start, (void*)(start+size-1));
    }
    return ret;	
}

static struct vm_struct *cache_map_vm_struct = NULL;
static int m4u_cache_sync_init(void)
{
    cache_map_vm_struct = get_vm_area(PAGE_SIZE, VM_ALLOC);
    if (!cache_map_vm_struct)
        return -ENOMEM;

    return 0;
}

static void* m4u_cache_map_page_va(struct page* page)
{
    int ret;
    struct page** ppPage = &page;

    ret = map_vm_area(cache_map_vm_struct, PAGE_KERNEL, &ppPage);
    if(ret)
    {
        M4UMSG("error to map page\n");
        return NULL;
    }
    return cache_map_vm_struct->addr;
}

static void m4u_cache_unmap_page_va(unsigned int va)
{
    unmap_kernel_range((unsigned long)cache_map_vm_struct->addr,  PAGE_SIZE);
}

//lock to protect cache_map_vm_struct
static DEFINE_MUTEX(gM4u_cache_sync_user_lock);

static int __m4u_cache_sync_user(unsigned int start, size_t size, int direction)
{
    unsigned int map_size;
    unsigned int map_start, map_end;
    unsigned int end = start+size;
    struct page* page;
    unsigned int map_va, map_va_align;
    int ret = 0;

    mutex_lock(&gM4u_cache_sync_user_lock);

    if(!cache_map_vm_struct)
    {
        M4UMSG(" error: cache_map_vm_struct is NULL, retry\n");
        m4u_cache_sync_init();
    }
    if(!cache_map_vm_struct)
    {
        M4UMSG("error: cache_map_vm_struct is NULL, no vmalloc area\n");
        ret = -1;
        goto out;
    }

    M4ULOG("__m4u_sync_user: start=0x%x, size=0x%x\n", start, size);

    map_start = start;
    while(map_start < end)
    {
        map_end = min( (map_start&(~M4U_PAGE_MASK))+M4U_PAGE_SIZE, end);
        map_size = map_end - map_start;

        page = m4u_cache_get_page(map_start);
        if(!page)
        {
            ret = -1;
            goto out;
        }

        map_va = (unsigned int)m4u_cache_map_page_va(page);
        if(!map_va)
        {
            ret = -1;
            goto out;
        }

        map_va_align = map_va | (map_start&(M4U_PAGE_SIZE-1));

        M4ULOG("__m4u_sync_user: map_start=0x%x, map_size=0x%x, map_va=0x%x\n", 
            map_start, map_size, map_va_align);
        __m4u_cache_sync_kernel((void*)map_va_align, map_size, direction);

        m4u_cache_unmap_page_va(map_va); 
        map_start = map_end;
    }

    
out:
    mutex_unlock(&gM4u_cache_sync_user_lock);
    
    return ret;
    
}

int m4u_dma_cache_maint(const M4U_MODULE_ID_ENUM eModuleID, const void *start, const size_t size, const int direction)
{
    // By range operation
    unsigned int page_num;
    int ret = 0;
    //MMProfileLogEx(M4U_MMP_Events[PROFILE_DMA_MAINT_ALL], MMProfileFlagStart, start, size);
    //MMProfileLogEx(M4U_MMP_Events[PROFILE_DMA_MAINT_ALL], MMProfileFlagPulse, eModuleID, direction);
    
    if( (((unsigned int)start%L1_CACHE_BYTES!=0) || (size%L1_CACHE_BYTES)!=0))
    {
        M4UMSG("Buffer align error: module=%s,addr=0x%x,size=%d,align=%d\n", 
             m4u_get_module_name(eModuleID), 
             (unsigned int)start, size, L1_CACHE_BYTES);
  		M4UMSG("error: addr un-align, module=%s, addr=0x%x, size=0x%x, process=%s, align=0x%x\n", 
  	        m4u_get_module_name(eModuleID), (unsigned int)start, size, current->comm, L1_CACHE_BYTES);
    }

    page_num = M4U_GET_PAGE_NUM(start, size);

    if((unsigned int)start<PAGE_OFFSET)  // from user space
    {
        ret = __m4u_cache_sync_user((unsigned int)start, size, direction);
    }
    else
    {
        ret = __m4u_cache_sync_kernel(start, size, direction);
    }
    
	M4ULOG("cache_sync: module=%s, addr=0x%x, size=0x%x\n",  m4u_get_module_name(eModuleID), 
        m4u_get_module_name(eModuleID), (unsigned int)start, size);

    //MMProfileLogEx(M4U_MMP_Events[PROFILE_DMA_MAINT_ALL], MMProfileFlagEnd, ((unsigned int)eModuleID<<16)|direction, ret);

    return ret;
}

#endif

/*****************************************************************************
 * FUNCTION
 *    m4u_dma_cache_flush_all
 * DESCRIPTION
 *    1. flush all L1 cache.
 *    2. flush all L2 cache.
 * PARAMETERS
 *    None.  
 * RETURNS
 *    None.
 ****************************************************************************/
int m4u_dma_cache_flush_all()
{

   // M4UMSG("cache flush all!!\n")
    mutex_lock(&gM4uMutex);

    // L1 cache clean before hw read
    smp_inner_dcache_flush_all();
     
	// L2 cache maintenance by physical pages
    outer_flush_all();
    
    mutex_unlock(&gM4uMutex);
   
    return 0;
}

#ifndef __M4U_CACHE_SYCN_USING_KERNEL_MAP__
static M4U_DMA_DIR_ENUM m4u_get_dir_by_module(M4U_MODULE_ID_ENUM eModuleID)
{
    
    M4U_DMA_DIR_ENUM dir;
    switch(eModuleID)  // from user space
    {
        default:
            //M4UMSG("warning: can not get port's direction, module=%s \n", m4u_get_module_name(eModuleID));
            dir = M4U_DMA_READ_WRITE;
            break;
    }

    return dir;
}
#endif


#ifdef M4U_4M_PAGETABLE
#define M4U_PAGE_TABLE_ALIGN 0x3FFFFF
#else
#define M4U_PAGE_TABLE_ALIGN (((PT_TOTAL_ENTRY_NUM*sizeof(unsigned int) - 1)>0xffff)? (PT_TOTAL_ENTRY_NUM*sizeof(unsigned int) - 1) : 0xffff)  //page table addr should (2^16)x align  at least
#endif
#define M4U_PROTECT_BUF_OFFSET (128-1)    // protect buffer start address should be 128x align

#ifdef M4U_4M_PAGETABLE
static struct page* m4u_alloc_pagetable(void)
{
    struct page* tmp_page = NULL;
        
    M4UMSG("MVA space: %d, M4U_PAGE_TABLE_ALIGN:%x\n", TOTAL_MVA_RANGE, M4U_PAGE_TABLE_ALIGN);
    M4UMSG("Before allocate PageTable, remaining %d pages", zone_page_state(&contig_page_data.node_zones[0], NR_FREE_PAGES));	
    tmp_page = alloc_pages(GFP_KERNEL | __GFP_ZERO, 10);
    M4UMSG("After allocate PageTable, remaining %d pages", zone_page_state(&contig_page_data.node_zones[0], NR_FREE_PAGES));		
    if (unlikely(!tmp_page)) {
		M4UERR("%s alloc 2^10 pages failed.\n", __FUNCTION__);
		return NULL;
	}

    if (unlikely(((unsigned int)page_to_pfn(tmp_page) & 0x3FF) != 0))
    {
        M4UERR("PageTable is not aligned by 0x%x", M4U_PAGE_TABLE_ALIGN + 1);
        __free_pages(tmp_page, 10);
        return NULL;      
    }
    else
        split_page(tmp_page, 10);

    return tmp_page;        
}
#endif
/*****************************************************************************
 * FUNCTION
 *    m4u_struct_init
 * DESCRIPTION
 *    1. allocate page table memory.
 *    2. allocate translation fault protection memory.
 *    3. Initial seq range and wrap range structure.
 *    4. Initial register backup memory.
 *    5. Call m4u_hw_init
 * PARAMETERS
 *    None.
 * RETURNS
 *    Type: bool. true mean success and false mean fail.
 ****************************************************************************/
static bool m4u_struct_init(void)
{
    struct page* tmp_page = NULL;
    M4ULOG("MVA space: %d, M4U_PAGE_TABLE_ALIGN:%x\n", TOTAL_MVA_RANGE, M4U_PAGE_TABLE_ALIGN);

    //======= alloc pagetable=======================
#ifdef M4U_4M_PAGETABLE

    tmp_page = m4u_alloc_pagetable();
    if (!tmp_page) {
        return false;
    }
    
    pPT_nonsec = (unsigned int*)page_address(tmp_page);
    pt_pa_nonsec = (unsigned int)page_to_phys(tmp_page);

    M4UMSG("allocate pagetable success! pagetable_va=0x%x, pagetable_pa=0x%x.\n", (unsigned int)pPT_nonsec, (unsigned int)pt_pa_nonsec);
#else
    pPT_nonsec= dma_alloc_coherent(NULL, PT_TOTAL_ENTRY_NUM * sizeof(unsigned int), &pt_pa_nonsec, GFP_KERNEL);
    if(!pPT_nonsec)
    {
        M4UMSG("dma_alloc_coherent error!  dma memory not available.\n");
        return false;
    }
    if((pt_pa_nonsec&M4U_PAGE_TABLE_ALIGN)!=0)
    {
        unsigned int tmp;
        M4UMSG("dma_alloc_coherent memory not align. PageTablePA=0x%x we will try again \n", pt_pa_nonsec);
        dma_free_coherent(NULL, PT_TOTAL_ENTRY_NUM * sizeof(unsigned int), pPT_nonsec, pt_pa_nonsec);
        tmp = (unsigned int)dma_alloc_coherent(NULL, PT_TOTAL_ENTRY_NUM * sizeof(unsigned int)+M4U_PAGE_TABLE_ALIGN, &pt_pa_nonsec, GFP_KERNEL);
        if(!tmp)
        {
            M4UMSG("dma_alloc_coherent error!  dma memory not available.\n");
            return false;
        }
        pPT_nonsec = (unsigned int*)((tmp+M4U_PAGE_TABLE_ALIGN)&(~M4U_PAGE_TABLE_ALIGN));
        pt_pa_nonsec += (unsigned int)pPT_nonsec - tmp;
    }
    
    M4UMSG("dma_alloc_coherent success! pagetable_va=0x%x, pagetable_pa=0x%x.\n", (unsigned int)pPT_nonsec, (unsigned int)pt_pa_nonsec);
    memset((void*)pPT_nonsec, 0, PT_TOTAL_ENTRY_NUM * sizeof(unsigned int));
    //======= alloc pagetable done=======================
#endif 

#ifdef M4U_USE_ONE_PAGETABLE
    pPT_sec = pPT_nonsec;
    pt_pa_sec = pt_pa_nonsec;
#else

    //======= alloc pagetable for security pt=======================
    pPT_sec= dma_alloc_coherent(NULL, PT_TOTAL_ENTRY_NUM * sizeof(unsigned int), &pt_pa_sec, GFP_KERNEL);
    if(!pPT_sec)
    {
        M4UMSG("dma_alloc_coherent error for sec pt!  dma memory not available.\n");
        return false;
    }
    if((pt_pa_sec&M4U_PAGE_TABLE_ALIGN)!=0)
    {
        unsigned int tmp;
        M4UMSG("dma_alloc_coherent memory not align. PageTablePA=0x%x we will try again \n", pt_pa_sec);
        dma_free_coherent(NULL, PT_TOTAL_ENTRY_NUM * sizeof(unsigned int), pPT_sec, pt_pa_sec);
        tmp = (unsigned int)dma_alloc_coherent(NULL, PT_TOTAL_ENTRY_NUM * sizeof(unsigned int)+M4U_PAGE_TABLE_ALIGN, &pt_pa_sec, GFP_KERNEL);
        if(!tmp)
        {
            M4UMSG("dma_alloc_coherent error!  dma memory not available.\n");
            return false;
        }
        pPT_sec = (unsigned int*)((tmp+M4U_PAGE_TABLE_ALIGN)&(~M4U_PAGE_TABLE_ALIGN));
        pt_pa_sec += (unsigned int)pPT_sec - tmp;
    }
    
    M4UMSG("dma_alloc_coherent success! pagetable_va=0x%x, pagetable_pa=0x%x.\n", (unsigned int)pPT_sec, (unsigned int)pt_pa_sec);
    memset((void*)pPT_sec, 0, PT_TOTAL_ENTRY_NUM * sizeof(unsigned int));
    //======= alloc pagetable done=======================
#endif


    init_mlock_cnt();
    if(NULL==pMlock_cnt)
        return false;
          
    // allocate 128 byte for translation fault protection
    // when TF occurs, M4U will translate the physical address to ProtectPA
    pProtectVA = (unsigned int*) kmalloc(TF_PROTECT_BUFFER_SIZE*TOTAL_M4U_NUM+M4U_PROTECT_BUF_OFFSET, GFP_KERNEL|__GFP_ZERO);
    if(NULL==pProtectVA)
    {
        
        M4UMSG("Physical memory not available.\n");
        return false;
    }
    pProtectVA = (unsigned int*)(((unsigned int)pProtectVA+M4U_PROTECT_BUF_OFFSET)&(~M4U_PROTECT_BUF_OFFSET));
    ProtectPA = virt_to_phys(pProtectVA);
    memset((unsigned char*)pProtectVA, 0x55, TF_PROTECT_BUFFER_SIZE*TOTAL_M4U_NUM);

#if 0
    tmp_page = alloc_page(GFP_KERNEL|__GFP_ZERO);
    gM4U_align_page_va = (unsigned int)page_address(tmp_page);
    gM4U_align_page_pa = (unsigned int)page_to_phys(tmp_page);

    M4UMSG("gM4U_align_page_pa is 0x%x\n", gM4U_align_page_pa);
#endif

    M4ULOG("ProtectTablePA:0x%x, ProtectTableVA:0x%x\n", 
        ProtectPA, (unsigned int)pProtectVA);
           
    //initialize global variables
    pRangeDes = kmalloc(sizeof(M4U_RANGE_DES_T) * TOTAL_RANGE_NUM, GFP_KERNEL|__GFP_ZERO);
    if(NULL==pRangeDes)
    {
        
        M4UMSG("Physical memory not available.\n");
        return false;
    }
    
    pWrapDes = kmalloc(sizeof(M4U_WRAP_DES_T) * TOTAL_WRAP_NUM, GFP_KERNEL|__GFP_ZERO);
    if(NULL==pWrapDes)
    {
        
        M4UMSG("Physical memory not available.\n");
        return false;
    }

    pM4URegBackUp = (unsigned int*)kmalloc(BACKUP_REG_SIZE, GFP_KERNEL|__GFP_ZERO);
    if(pM4URegBackUp==NULL)
    {
    	  M4UERR("pM4URegBackUp kmalloc fail \n");
    }    
            
    m4u_hw_init();
	 
    gM4uLogFlag = false; 

    return true;
}


/*****************************************************************************
 * FUNCTION
 *    m4u_hw_init
 * DESCRIPTION
 *    1. Enable M4U and SMI clock.
 *    2. Set SMI register. 
 *    3. Set M4U register. 
 *    4. Invalidate all TLB entry.
 * PARAMETERS
 *    None.
 * RETURNS
 *    Type: Integer. always 1.
 ****************************************************************************/
static int m4u_hw_init(void)
{
    unsigned int i;
    unsigned regval;
    M4ULOG("m4u_hw_init() \n");

    m4u_clock_on();

//=============================================
// SMI registers
//=============================================
    
    // secure register: 
    // all use physical (bypass m4u); domain(3); secure(0)
    for(i=0; i<2; i++)  // domain in MMsys always set 3, for debug purpose.
        COM_WriteReg32(REG_SMI_SECUR_CON(i), 0x66666666);  

   
//=============================================
//  m4u global registers
//============================================
    //set m4u pagetable base address
    COM_WriteReg32(REG_MMUg_PT_BASE, (unsigned int)pt_pa_nonsec);
#ifndef M4U_USE_ONE_PAGETABLE
    COM_WriteReg32(REG_MMUg_PT_BASE_SEC, (unsigned int)pt_pa_sec);
#endif

    COM_WriteReg32(REG_MMUg_DCM, F_MMUg_DCM_ON(1)); //default value


//===============================
// LARB
//===============================
#if 0 //cloud    
        for(i=0; i<SMI_LARB_NR; i++)
        {
            larb_clock_on(i);
            //set SMI_SHARE_EN to 0
            M4U_WriteReg32(gLarbBaseAddr[i], SMI_SHARE_EN, 0x0);  // control transaction go MCI, snoop CPU cache, than if no cache hit go EMI. (transaction coherent)
            //in 6272, SMI_SHARE_EN is not work.

            M4UMSG("larb clock on %d\n", i);

            larb_clock_off(i);
        }
  
#endif //cloud
//=============================================
// m4u registers
//=============================================

    for(i=0;i<TOTAL_M4U_NUM;i++)
    {
        regval = F_MMU_CTRL_PFH_DIS(0)         \
                |F_MMU_CTRL_TLB_WALK_DIS(0)    \
                |F_MMU_CTRL_MONITOR_EN(0)       \
                |F_MMU_CTRL_MONITOR_CLR(0)     \
                |F_MMU_CTRL_PFH_RT_RPL_MODE(0) \
                |F_MMU_CTRL_TF_PROT_VAL(2)    \
                |F_MMU_CTRL_INT_HANG_en(0)      \
                |F_MMU_CTRL_COHERE_EN(0)       ;
        M4U_WriteReg32(gM4UBaseAddr[i], REG_MMU_CTRL_REG, regval);

    //    M4UMSG("ctl = 0x%x\n", M4U_ReadReg32(gM4UBaseAddr[i], REG_MMU_CTRL_REG));

#ifdef M4U_INT_INVALIDATION_DONE        
        M4U_WriteReg32(gM4UBaseAddr[i], REG_MMU_INT_CONTROL, 0x4FF);
#else
        M4U_WriteReg32(gM4UBaseAddr[i], REG_MMU_INT_CONTROL, 0xFF); 
#endif
        
        //disable non-blocking mode
        //M4U_WriteReg32(gM4UBaseAddr[i], REG_MMU_NON_BLOCKING_DIS, F_MMU_NON_BLOCK_DISABLE_BIT);  // TODO: NEED REMOVE in final (cloud)
        
        M4U_WriteReg32(gM4UBaseAddr[i], REG_MMU_IVRP_PADDR, (unsigned int)ProtectPA);

        M4ULOG("init hw OK: %d \n",i);
    }
    //invalidate all TLB entry
    m4u_invalid_tlb_all();

    return 1;
}


/*****************************************************************************
 * FUNCTION
 *    m4u_clear_intr
 * DESCRIPTION
 *    Clear all interrupt
 * Context
*      ISR - MTK_M4U_isr
 * PARAMETERS
 *    param1 : [IN]  const unsigned int m4u_base
 *                M4U base register.  
 * RETURNS
 *    None.
 ****************************************************************************/
void m4u_clear_intr(const unsigned int m4u_base)
{
    unsigned int Temp;
    Temp = M4U_ReadReg32(m4u_base, REG_MMU_INT_CONTROL) | F_INT_CLR_BIT;
    M4U_WriteReg32(m4u_base, REG_MMU_INT_CONTROL, Temp);   
}

/*****************************************************************************
 * FUNCTION
 *    m4u_reg_backup
 * DESCRIPTION
 *    Backup register for system suspend.
 * PARAMETERS
 *    None.
 * RETURNS
 *    Type: Integer. always zero.
 ****************************************************************************/
static int m4u_reg_backup(void)
{
    unsigned int* pReg = pM4URegBackUp;

    int i;

    //flag (for debug)
    *(pReg++) = COM_ReadReg32(REG_MMUg_PT_BASE);

    //m4u reg backup
    {
        unsigned int m4u_base = gM4UBaseAddr[0];
        
        for(i=0; i<M4U_SEQ_NR; i++)
        {
            *(pReg++) = M4U_ReadReg32(m4u_base, REG_MMU_SQ_START(i));
            *(pReg++) = M4U_ReadReg32(m4u_base, REG_MMU_SQ_END(i));
        }
        *(pReg++) = M4U_ReadReg32(m4u_base, REG_MMU_PFH_DIST0);
        *(pReg++) = M4U_ReadReg32(m4u_base, REG_MMU_PFH_DIST1);

        *(pReg++) = M4U_ReadReg32(m4u_base, REG_MMU_PFH_DIR0);

        *(pReg++) = M4U_ReadReg32(m4u_base, REG_MMU_CTRL_REG);
        *(pReg++) = M4U_ReadReg32(m4u_base, REG_MMU_IVRP_PADDR);
        *(pReg++) = M4U_ReadReg32(m4u_base, REG_MMU_INT_CONTROL);

        for(i=0; i<M4U_WRAP_NR; i++)
        {
            *(pReg++) = M4U_ReadReg32(m4u_base, REG_MMU_WRAP_SA(i));
            *(pReg++) = M4U_ReadReg32(m4u_base, REG_MMU_WRAP_EA(i));
        }
        
        *(pReg++) = M4U_ReadReg32(m4u_base, REG_MMU_WRAP_EN0);
        *(pReg++) = M4U_ReadReg32(m4u_base, REG_MMU_WRAP_EN1);
        
        *(pReg++) = M4U_ReadReg32(m4u_base, REG_MMU_PFQ_BROADCAST_EN);
        *(pReg++) = M4U_ReadReg32(m4u_base, REG_MMU_NON_BLOCKING_DIS);
        
    }

    *(pReg++) = COM_ReadReg32(REG_MMUg_CTRL);
    *(pReg++) = COM_ReadReg32(REG_MMUg_DCM);
    *(pReg++) = COM_ReadReg32(REG_MMUg_CTRL_SEC);
    *(pReg++) = COM_ReadReg32(REG_MMUg_PT_BASE_SEC);
   
#if 0    
    //SMI registers
    for(i=0; i<2; i++)
        *(pReg++) = COM_ReadReg32(REG_SMI_SECUR_CON(i));
#endif	

    M4UMSG("register backup buffer needs: %d \n", (unsigned int)pReg-(unsigned int)pM4URegBackUp);

    if(pt_pa_nonsec !=*pM4URegBackUp)
    {
        M4UERR("PT_BASE in memory is error after backup! expect PTPA=0x%x, backupReg=0x%x\n", 
            pt_pa_nonsec, *pM4URegBackUp);
    }

    return 0;
}

/*****************************************************************************
 * FUNCTION
 *    m4u_reg_restore
 * DESCRIPTION
 *    Restore register for system resume.
 * PARAMETERS
 *    None.
 * RETURNS
 *    Type: Integer. always zero.
 ****************************************************************************/
static int m4u_reg_restore(void)
{
    unsigned int* pReg = pM4URegBackUp;

    int i;

    //flag (for debug)
    COM_WriteReg32(REG_MMUg_PT_BASE, *(pReg++));

    //m4u reg backup
    {
        unsigned int m4u_base = gM4UBaseAddr[0];
        
        for(i=0; i<M4U_SEQ_NR; i++)
        {
            M4U_WriteReg32(m4u_base, REG_MMU_SQ_START(i), *(pReg++));
            M4U_WriteReg32(m4u_base, REG_MMU_SQ_END(i)  , *(pReg++));
        }
        M4U_WriteReg32(m4u_base, REG_MMU_PFH_DIST0      , *(pReg++) );
        M4U_WriteReg32(m4u_base, REG_MMU_PFH_DIST1     , *(pReg++) );
                                                                    
        M4U_WriteReg32(m4u_base, REG_MMU_PFH_DIR0      , *(pReg++) );
                                                                   
        M4U_WriteReg32(m4u_base, REG_MMU_CTRL_REG      , *(pReg++) );
        M4U_WriteReg32(m4u_base, REG_MMU_IVRP_PADDR    , *(pReg++) );
        M4U_WriteReg32(m4u_base, REG_MMU_INT_CONTROL   , *(pReg++) );

        for(i=0; i<M4U_WRAP_NR; i++)
        {
            M4U_WriteReg32(m4u_base, REG_MMU_WRAP_SA(i), *(pReg++) );
            M4U_WriteReg32(m4u_base, REG_MMU_WRAP_EA(i), *(pReg++) );
        }
        
        M4U_WriteReg32(m4u_base, REG_MMU_WRAP_EN0        , *(pReg++) );
        M4U_WriteReg32(m4u_base, REG_MMU_WRAP_EN1        , *(pReg++) );
                                                                     
        M4U_WriteReg32(m4u_base, REG_MMU_PFQ_BROADCAST_EN, *(pReg++) );
        M4U_WriteReg32(m4u_base, REG_MMU_NON_BLOCKING_DIS, *(pReg++) );
                                                                     
    }                                                                
                                                                                                             
    COM_WriteReg32(REG_MMUg_CTRL                         , *(pReg++) );         
    COM_WriteReg32(REG_MMUg_DCM                          , *(pReg++) );
    COM_WriteReg32(REG_MMUg_CTRL_SEC                     , *(pReg++) );
    COM_WriteReg32(REG_MMUg_PT_BASE_SEC                  , *(pReg++) );

#if 0
    //SMI registers
    for(i=0; i<2; i++)
        COM_WriteReg32(REG_SMI_SECUR_CON(i), *(pReg++) );
#endif

    if(COM_ReadReg32(REG_MMUg_PT_BASE) != pt_pa_nonsec)
    {
    	  M4UERR("PT_BASE is error after restore! 0x%x != 0x%x\n",
            COM_ReadReg32(REG_MMUg_PT_BASE), pt_pa_nonsec);
    }   
    return 0;
}



/*****************************************************************************
 * FUNCTION
 *    m4u_get_module_name
 * DESCRIPTION
 *    Get Module Name.
 * PARAMETERS
 *    param1 : [IN]  const M4U_MODULE_ID_ENUM moduleID
 *                Module ID.  
 * RETURNS
 *    char*. Module name.
 ****************************************************************************/
static char* m4u_get_module_name(const M4U_MODULE_ID_ENUM moduleID)
{
    switch(moduleID)
    {
        case M4U_CLNTMOD_MDP       :  return "MDP";
        case M4U_CLNTMOD_DISP      :  return "DISP";
        case M4U_CLNTMOD_VIDEO     :  return "VIDEO";
        case M4U_CLNTMOD_CAM       :  return "CAM";
        case M4U_CLNTMOD_CMDQ      :  return "CMDQ";
        case M4U_CLNTMOD_LCDC_UI   :  return "LCDC_UI";	
        default:
             M4UMSG("invalid module id=%d", moduleID);
             return "UNKNOWN";    	    	
    }
}

/*****************************************************************************
 * FUNCTION
 *    m4u_get_port_name
 * DESCRIPTION
 *    Get Port Name.
 * PARAMETERS
 *    param1 : [IN]  const M4U_PORT_ID_ENUM portID
 *                Port ID.  
 * RETURNS
 *    char*. Port name.
 ****************************************************************************/                  
static char* m4u_get_port_name(const M4U_PORT_ID_ENUM portID)
{
    switch(portID)
    {
        case M4U_PORT_LCD_OVL               : return "DISP_OVL";            
        case M4U_PORT_LCD_R                 : return "DISP_RDMA";            
        case M4U_PORT_LCD_W                 : return "DISP_WDMA";            
        case M4U_PORT_LCD_DBI               : return "DISP_DBI";            
        case M4U_PORT_CAM_WDMA              : return "CAM_WDMA";            
        case M4U_PORT_CMDQ                  : return "MM_CMDQ";            
        case M4U_PORT_VENC_BSDMA_VDEC_POST0 : return "VENC_BSDMA_VDEC_POST0";            
        case M4U_PORT_MDP_RDMA              : return "MDP_RDMA";            
        case M4U_PORT_MDP_WDMA              : return "MDP_WDMA";            
        case M4U_PORT_MDP_ROTO              : return "MDP_ROTO";            
        case M4U_PORT_MDP_ROTCO             : return "MDP_ROTCO";            
        case M4U_PORT_MDP_ROTVO             : return "MDP_ROTVO";            
        case M4U_PORT_VENC_MVQP             : return "VENC_MVQP";            
        case M4U_PORT_VENCMC                : return "VENCMC";            
        case M4U_PORT_VENC_CDMA_VDEC_CDMA   : return "VENC_CDMA_VDEC_CDMA";            
        case M4U_PORT_VENC_REC_VDEC_WDMA    : return "VENC_REC_VDEC_WDMA";
        default:
            M4UMSG("invalid port id=%d", portID);
            return "UNKNOWN";    	
    	
    }
}

#if 0
unsigned int m4u_get_pa_by_mva(unsigned int mva)
{
    unsigned int * pPageTableAddr = 0;
    pPageTableAddr = mva_pteAddr_nonsec(mva); 
    if( (*pPageTableAddr & F_DESC_VALID) !=0)
    {
        return *pPageTableAddr;
    }
    else
    {
        M4UMSG("error: pa is invalid, mva=0x%x, pa=0x%x \n", mva, *pPageTableAddr);
        return 0;
    }
}
#endif

/*****************************************************************************
 * FUNCTION
 *    m4u_memory_usage
 * DESCRIPTION
 *    List all module memory usage (max use size, current use size, lock page count)
 * PARAMETERS
 *    None.
 * RETURNS
 *    None.
 ****************************************************************************/
static void m4u_memory_usage()
{
    unsigned int i=0;
    for(i=0;i<M4U_CLIENT_MODULE_NUM;i++)
    {
        M4UMSG("id=%-2d, name=%-10s, max=%-5dKB, current=%-5dKB, locked_page=%-3d \n",
            i, m4u_get_module_name(i), pmodule_max_size[i]/1024, pmodule_current_size[i]/1024, 
            pmodule_locked_pages[i]);
    }    	
#if 0	
    if(m4u_memory_usage == perf_timer.function)
    {
	    mod_timer(&perf_timer, (jiffies+HZ));
    }
#endif	
}    

/*****************************************************************************
 * FUNCTION
 *    m4u_print_active_port
 * DESCRIPTION
 *    List active port (config as use va )
 * PARAMETERS
 *    None.
 * RETURNS
 *    None.
 ****************************************************************************/
void m4u_print_active_port()
{
    unsigned int i=0;
    unsigned int regval;

    M4UMSG("active ports: ");
    {
        for(i=0;i<M4U_PORT_NR;i++)
        {
            
            regval = m4uHw_get_field_by_mask(0, REG_SMI_SECUR_CON_OF_PORT(i), F_SMI_SECUR_CON_VIRTUAL(i));
            if(regval)
            {
                M4UMSG("%s, ", m4u_get_port_name(i));
            }
        }
        M4UMSG("\n");
    }
} 



// used to clear all TLB resource occupied by the module
int m4u_reset_mva_release_tlb(M4U_MODULE_ID_ENUM eModuleID) 
{	
    //todo: implement this func in dynamic alloc mode
    M4ULOG("Have not implemented m4u_reset_mva_release_tlb() in dynamic mva alloc mode! \n");
    
    return 0;
}


/*****************************************************************************
 * FUNCTION
 *    m4u_dump_seq_range_info
 * DESCRIPTION
 *    Dump seq range information.
 * PARAMETERS
 *    None.
 * RETURNS
 *    None.
 ****************************************************************************/
static void m4u_dump_seq_range_info(void)
{
    unsigned int i=0;

    M4UMSG(" MVA Range Info: \n");
    for(i=0;i<TOTAL_RANGE_NUM;i++)
    {
        M4UMSG("pRangeDes[%d]: Enabled=%d, module=%s, MVAStart=0x%x, MVAEnd=0x%x, entrycount=%d \n", 
            i, pRangeDes[i].Enabled, m4u_get_module_name(pRangeDes[i].eModuleID), 
            pRangeDes[i].MVAStart, pRangeDes[i].MVAEnd, pRangeDes[i].entryCount);

    }        
}

/*****************************************************************************
 * FUNCTION
 *    m4u_dump_wrap_range_info
 * DESCRIPTION
 *    Dump wrap range information.
 * PARAMETERS
 *    None.
 * RETURNS
 *    None.
 ****************************************************************************/
static void m4u_dump_wrap_range_info(void)
{
    unsigned int i=0;

    M4UMSG(" Wrap Range Info: \n");
    for(i=0;i<TOTAL_WRAP_NUM;i++)
    {
        M4UMSG("pWrapDes[%d]: Enabled=%d, module=%s, MVAStart=0x%x, MVAEnd=0x%x \n", 
            i, pWrapDes[i].Enabled, m4u_get_port_name(pWrapDes[i].eModuleID), 
            pWrapDes[i].MVAStart, pWrapDes[i].MVAEnd);
    }     
}

/*****************************************************************************
 * FUNCTION
 *    m4u_dump_info
 * DESCRIPTION
 *    Dump seq range, wrap range, and mva use information.
 * PARAMETERS
 *    None.
 * RETURNS
 *    Type: Integer. always zero.
 ****************************************************************************/
int m4u_dump_info(void) 
{  
    m4u_dump_seq_range_info();

    m4u_dump_wrap_range_info();

    m4u_dump_mva_info();
        
    return 0;
}

#if 0 //cloud
void m4u_get_power_status(void)
{

}
#endif

/*****************************************************************************
 * FUNCTION
 *    m4u_log_on
 * DESCRIPTION
 *    Enable log using M4ULOG, and list current infomration about memory usage, seq range, wrap range, and active port.
 * PARAMETERS
 *    None.
 * RETURNS
 *    Type: Integer. always zero.
 ****************************************************************************/
static int m4u_log_on(void)
{
  
    M4UMSG("m4u_log_on is called! \n");  
    gM4uLogFlag = true;
    //m4u_get_power_status();
    
    m4u_memory_usage();
   // m4u_dump_reg();
    m4u_dump_info();
    m4u_print_active_port();    
        
    return 0;
}

/*****************************************************************************
 * FUNCTION
 *    m4u_log_off
 * DESCRIPTION
 *    Disable log using M4ULOG.
 * PARAMETERS
 *    None.
 * RETURNS
 *    Type: Integer. always zero.
 ****************************************************************************/
static int m4u_log_off(void)
{
    M4UMSG("m4u_log_off is called! \n");  
    gM4uLogFlag = false;
    return 0;  	
}


/*****************************************************************************
 * FUNCTION
 *    m4u_enable_prefetch
 * DESCRIPTION
 *    Set TLB prefectch enable register.
 * PARAMETERS
 *    param1 : [IN]  const M4U_PORT_ID_ENUM PortID
 *                Module ID.  
 *    param2 : [IN]  const bool fgEnable
 *                Enable prefetch or not.  
 * RETURNS
 *    None.
 ****************************************************************************/
void m4u_enable_prefetch(const M4U_PORT_ID_ENUM PortID, const bool fgEnable)
{
    const unsigned int m4u_base = gM4UBaseAddr[m4u_port_2_m4u_id(PortID)];
    m4uHw_set_field_by_mask(m4u_base, REG_MMU_CTRL_REG, 
                  F_MMU_CTRL_PFH_DIS(1), F_MMU_CTRL_PFH_DIS(!fgEnable));
}

#if 0
/*****************************************************************************
 * FUNCTION
 *    m4u_enable_error_hang
 * DESCRIPTION
 *    MMU translation hang enable if interrupt occurred.
 * PARAMETERS
 *    param1 : [IN]  const bool fgEnable
 *                Enable error hange or not.  
 * RETURNS
 *    None.
 ****************************************************************************/
static void m4u_enable_error_hang(const bool fgEnable)
{
    const unsigned int m4u_base = gM4UBaseAddr[0];
    m4uHw_set_field_by_mask(m4u_base, REG_MMU_CTRL_REG, 
                  F_MMU_CTRL_INT_HANG_en(1), F_MMU_CTRL_INT_HANG_en(fgEnable));
}
#endif

/*****************************************************************************
 * FUNCTION
 *    m4u_print_mva_list
 * DESCRIPTION
 *    Dump all mva allocation in the Node.
 * PARAMETERS
 *    param1 : [IN]  struct file *filep
 *                A private data contain garbage node.  
 *    param2 : [IN]  const char *pMsg
 *                User message.  
 * RETURNS
 *    None.
 ****************************************************************************/
void m4u_print_mva_list(struct file *filep, const char *pMsg)
{
    garbage_node_t *pNode = filep->private_data;
    garbage_list_t *pList;
    struct list_head *pListHead;

    M4UMSG("print mva list [%s] ================================>\n", pMsg);
    mutex_lock(&(pNode->dataMutex));
    list_for_each(pListHead, &(pNode->mvaList))
    {
        pList = container_of(pListHead, garbage_list_t, link);
        M4UMSG("module=%s, va=0x%x, size=0x%x, mva=0x%x, flags=%d\n", 
            m4u_get_module_name(pList->eModuleId), pList->bufAddr, pList->size, pList->mvaStart, pList->flags);
    }
    mutex_unlock(&(pNode->dataMutex));

    M4UMSG("print mva list done ==========================>\n");
}


/*****************************************************************************
 * FUNCTION
 *    m4u_add_to_garbage_list
 * DESCRIPTION
 *    Add a mva region to garbage list. (for query mva by va)
 * PARAMETERS
 *    param1 : [IN]  const struct file * a_pstFile
 *                A private data contain garbage node.  
 *    param2 : [IN]  const unsigned int mvaStart
 *                User allocated mva.  
 *    param3 : [IN]  const unsigned int bufSize
 *                User allocated memory size.  
 *    param4 : [IN]  const M4U_MODULE_ID_ENUM eModuleID
 *                Module ID.  
 *    param5 : [IN]  const unsigned int va
 *                 User allocated virtual memory address.  
 *    param6 : [IN]  const unsigned int flags
 *                Mva range allocation flag.  
 *    param7 : [IN]  const int security
 *                User specified memory security type.   
 *    param8 : [IN]  const int cache_coherent
 *                User specified memory cache coherent type.  
 * RETURNS
 *    Type: Integer. zero mean success and -1 mean fail.
 ****************************************************************************/
static int m4u_add_to_garbage_list(const struct file * a_pstFile,
                                        const unsigned int mvaStart, 
                                        const unsigned int bufSize,
                                        const M4U_MODULE_ID_ENUM eModuleID,
                                        const unsigned int va,
                                        const unsigned int flags,
                                        const int security,
                                        const int cache_coherent)
{
    garbage_list_t *pList = NULL;
    garbage_node_t *pNode = (garbage_node_t*)(a_pstFile->private_data);
    pList = (garbage_list_t*)kmalloc(sizeof(garbage_list_t), GFP_KERNEL);
    if(pList==NULL || pNode==NULL)
    {
        M4UERR("m4u_add_to_garbage_list(), pList=0x%x, pNode=0x%x \n", (unsigned int)pList, (unsigned int)pNode);
        return -1;
    }

    pList->mvaStart = mvaStart;
    pList->size = bufSize;
    pList->eModuleId = eModuleID;
    pList->bufAddr = va;
    pList->flags = flags;
    pList->security = security;
    pList->cache_coherent = cache_coherent;
    mutex_lock(&(pNode->dataMutex));
    list_add(&(pList->link), &(pNode->mvaList));
    mutex_unlock(&(pNode->dataMutex));
    
    return 0;	
}

/*****************************************************************************
 * FUNCTION
 *    m4u_delete_from_garbage_list
 * DESCRIPTION
 *    Delee a mva region from garbage list.
 * PARAMETERS
 *    param1 : [IN]  const M4U_MOUDLE_STRUCT* p_m4u_module
 *                A structure contain dealloc mva region .  
 *    param2 : [IN]  const struct file * a_pstFile
 *                A private data contain garbage node.  
 * RETURNS
 *    Type: Integer. zero mean success and -1 mean fail.
 ****************************************************************************/
static int m4u_delete_from_garbage_list(const M4U_MOUDLE_STRUCT* p_m4u_module, const struct file * a_pstFile)
{
    struct list_head *pListHead;
    garbage_list_t *pList = NULL;
    garbage_node_t *pNode = (garbage_node_t*)(a_pstFile->private_data);
    int ret=0;

    if(pNode==NULL)
    {
        M4UERR("m4u_delete_from_garbage_list(), pNode is NULL! \n");
        return -1;
    }

    mutex_lock(&(pNode->dataMutex));
    list_for_each(pListHead, &(pNode->mvaList))
    {
        pList = container_of(pListHead, garbage_list_t, link);
        if((pList->mvaStart== p_m4u_module->MVAStart))
        {
            if(    (pList->bufAddr== p_m4u_module->BufAddr)
                && (pList->size == p_m4u_module->BufSize)
                && (pList->eModuleId == p_m4u_module->eModuleID) )
            {                    
                list_del(pListHead);
                kfree(pList);
                ret = 0;
                break;
            }
            else
            {
                ret=-1;
            	M4UMSG("error: input argument isn't valid, can't find the node at garbage list\n");
            }
        }
    }
    if(pListHead == &(pNode->mvaList))
    {
        ret=-1;
        M4UMSG("error: input argument isn't valid, can't find the node at garbage list\n");
    }
    mutex_unlock(&(pNode->dataMutex));
    
    return ret;	
}


/*****************************************************************************
 * FUNCTION
 *    m4u_suspend
 * DESCRIPTION
 *    Call m4u_reg_backup to save register. (for driver suspend API)
 * PARAMETERS
 *    param1 : [IN]  struct platform_device *pdev
 *                No use in this function.
 *    param2 : [IN]  pm_message_t mesg
 *                No use in this function. 
 * RETURNS
 *    Type: Integer. always zero.
 ****************************************************************************/
static int m4u_suspend(struct platform_device *pdev, pm_message_t mesg)
{
    M4UMSG("SMI register backup in suspend \n");
    m4u_reg_backup();    
    return 0;
}

/*****************************************************************************
 * FUNCTION
 *    m4u_resume
 * DESCRIPTION
 *    Call m4u_reg_restore to restore register. (for driver resume API)
 * PARAMETERS
 *    param1 : [IN]  struct platform_device *pdev
 *                No use in this function.
 * RETURNS
 *    Type: Integer. always zero.
 ****************************************************************************/
static int m4u_resume(struct platform_device *pdev)
{
    M4UMSG("SMI register restore in resume \n");
    m4u_reg_restore();
    return 0;
}

/*---------------------------------------------------------------------------*/
#ifdef CONFIG_PM 
/*---------------------------------------------------------------------------*/
static int m4u_pm_suspend(struct device *device)
{
    struct platform_device *pdev = to_platform_device(device);
    BUG_ON(pdev == NULL);

    return m4u_suspend(pdev, PMSG_SUSPEND);
}

static int m4u_pm_resume(struct device *device)
{
    struct platform_device *pdev = to_platform_device(device);
    BUG_ON(pdev == NULL);

    return m4u_resume(pdev);
}

extern void mt_irq_set_sens(unsigned int irq, unsigned int sens);
extern void mt_irq_set_polarity(unsigned int irq, unsigned int polarity);
static int m4u_pm_restore_noirq(struct device *device)
{
    M4ULOG("calling %s()\n", __func__);

    // m4u related irqs
    mt_irq_set_sens(MT_M4U0_IRQ_ID, MT65xx_LEVEL_SENSITIVE);
	mt_irq_set_polarity(MT_M4U0_IRQ_ID, MT65xx_POLARITY_LOW);

    return 0;
}

struct dev_pm_ops m4u_pm_ops = {
    .suspend = m4u_pm_suspend,
    .resume = m4u_pm_resume,
    .freeze = m4u_pm_suspend,
    .thaw = m4u_pm_resume,
    .poweroff = m4u_pm_suspend,
    .restore = m4u_pm_resume,
    .restore_noirq = m4u_pm_restore_noirq,
};
/*---------------------------------------------------------------------------*/
#endif /*CONFIG_PM*/
/*---------------------------------------------------------------------------*/


static struct platform_driver m4uDrv = {
    .probe	= m4u_probe,
    .remove	= m4u_remove,
    .suspend= m4u_suspend,
    .resume	= m4u_resume,
    .driver	= {
    .name	= M4U_DEVNAME,
#ifdef CONFIG_PM
    .pm     = &m4u_pm_ops,
#endif		
    .owner	= THIS_MODULE,
    }
};

/*****************************************************************************
 * FUNCTION
 *    MTK_M4U_Init
 * DESCRIPTION
 *    Call platform_driver_register to register M4U driver
 * PARAMETERS
 *    None.
 * RETURNS
 *    Type: Integer.  zero mean success and others mean fail.
 ****************************************************************************/
static int __init MTK_M4U_Init(void)
{
    if(platform_driver_register(&m4uDrv)){
        M4UMSG("failed to register MAU driver");
        return -ENODEV;
    }

	return 0;
}

/*****************************************************************************
 * FUNCTION
 *    MTK_M4U_Exit
 * DESCRIPTION
 *    Call platform_driver_unregister to unregister M4U driver
 * PARAMETERS
 *    None.
 * RETURNS
 *    None.
 ****************************************************************************/
static void __exit MTK_M4U_Exit(void)
{
    platform_driver_unregister(&m4uDrv);
}

/*****************************************************************************
 * FUNCTION
 *    m4u_set_tf_callback
 * DESCRIPTION
 *    
 * PARAMETERS
 *    None.
 * RETURNS
 *    None.
 ****************************************************************************/
void m4u_set_tf_callback(const M4U_MODULE_ID_ENUM eModuleID, PFN_TF_T ptf)
{
	m4u_tfcallback[eModuleID] = ptf;
}
EXPORT_SYMBOL(m4u_set_tf_callback); 

int m4u_mva_map_kernel(unsigned int mva, unsigned int size, int sec,
                        unsigned int* map_va, unsigned int* map_size)
{
    struct page **pages;
    unsigned int page_num, map_page_num;
    unsigned int kernel_va, kernel_size;

    kernel_va = 0;
    kernel_size = 0;

    page_num = M4U_GET_PAGE_NUM(mva, size);
    pages = vmalloc(sizeof(struct page*)*page_num);
    if(pages == NULL)
    {
        M4UMSG("mva_map_kernel: error to vmalloc for %d\n", sizeof(struct page*)*page_num);
    }

    for(map_page_num=0; map_page_num<page_num; map_page_num++)
    {
        unsigned int pa;
        if(sec)
            pa = *(unsigned int*)mva_pteAddr_sec(mva+map_page_num*M4U_PAGE_SIZE);
        else
            pa = *(unsigned int*)mva_pteAddr_nonsec(mva+map_page_num*M4U_PAGE_SIZE);

        if((pa&F_DESC_VALID) != F_DESC_VALID)
        {
            break;
        }

        pages[map_page_num] = phys_to_page(pa);
    }

    if(map_page_num != page_num)
    {
        M4UMSG("mva_map_kernel: only get %d pages: mva=0x%x, size=0x%x\n", 
            map_page_num, mva, size);
        goto error_out;
    }
    
    kernel_va = (unsigned int)vmap(pages, map_page_num, VM_MAP, PAGE_KERNEL);
    if(kernel_va == 0)
    {
        M4UMSG("mva_map_kernel: vmap fail: page_num=%d\n", map_page_num);
        goto error_out;
    }

    kernel_va += mva & (M4U_PAGE_MASK);
    
    *map_va = kernel_va;
    *map_size = size;

error_out:
    vfree(pages);
    M4ULOG("mva_map_kernel: mva=0x%x,size=0x%x,sec=0x%x,map_va=0x%x,map_size=0x%x\n",
        mva, size, sec, *map_va, *map_size);
    return 0;
    
}

EXPORT_SYMBOL(m4u_mva_map_kernel);

int m4u_mva_unmap_kernel(unsigned int mva, unsigned int size, unsigned int va)
{
    M4ULOG("mva_unmap_kernel: mva=0x%x,size=0x%x,va=0x%x\n", mva, size, va);
    vunmap((void*)(va&(~M4U_PAGE_MASK)));
    return 0;
}
EXPORT_SYMBOL(m4u_mva_unmap_kernel);

//EXPORT_SYMBOL(m4u_dump_reg);  
//EXPORT_SYMBOL(m4u_dump_info); 
EXPORT_SYMBOL(m4u_alloc_mva); 
EXPORT_SYMBOL(m4u_dealloc_mva);
EXPORT_SYMBOL(m4u_insert_seq_range);
EXPORT_SYMBOL(m4u_invalid_seq_range); 
//EXPORT_SYMBOL(m4u_invalid_tlb_all); 
EXPORT_SYMBOL(m4u_manual_insert_entry); 
EXPORT_SYMBOL(m4u_config_port);  
//EXPORT_SYMBOL(m4u_monitor_start);
//EXPORT_SYMBOL(m4u_monitor_stop); 
//EXPORT_SYMBOL(m4u_dma_cache_maint);
//EXPORT_SYMBOL(m4u_log_on);  
//EXPORT_SYMBOL(m4u_log_off); 
//EXPORT_SYMBOL(m4u_dump_pagetable_range);
//EXPORT_SYMBOL(m4u_dump_main_tlb_des); 
//EXPORT_SYMBOL(m4u_dump_pfh_tlb_des);  
//EXPORT_SYMBOL(m4u_print_active_port); 


module_init(MTK_M4U_Init);
module_exit(MTK_M4U_Exit);
                      

MODULE_DESCRIPTION("MTK M4U driver");
MODULE_AUTHOR("MTK80347 <Xiang.Xu@mediatek.com>");
MODULE_LICENSE("GPL");


