#ifndef __M4U_PRIV_H__
#define __M4U_PRIV_H__
#include <linux/ioctl.h>
#include <linux/fs.h>


#define M4U_PAGE_SIZE    0x1000                                  //4KB

#define M4U_CLIENT_MODULE_NUM        (M4U_PORT_UNKNOWN+1)
#define TOTAL_MVA_RANGE      0x40000000                              //total virtual address range: 1GB


#define PT_TOTAL_ENTRY_NUM    (1*1024*1024) //(TOTAL_MVA_RANGE/DEFAULT_PAGE_SIZE)              //total page table entries


#define M4U_GET_PTE_OFST_TO_PT_SA(MVA)    (((MVA) >> 12) << 2)
#define M4U_PTE_MAX (M4U_GET_PTE_OFST_TO_PT_SA(TOTAL_MVA_RANGE-1))

//==========================
//hw related
//=========================
//m4u global
#define NORMAL_M4U_NUMBER 2
#define TOTAL_M4U_NUM         2
#define M4U_REG_SIZE        0x5e0
#define M4U_PORT_NR         54

//SMI related
#define SMI_LARB_NR         3
#define SMI_PORT_NR         57

//tlb related
#define M4U_MAIN_TLB_NR   40
#define M4U_PRE_TLB_NR    40

//wrap range related
#define M4U_WRAP_NR       4
#define TOTAL_WRAP_NUM    ((M4U_WRAP_NR)*(TOTAL_M4U_NUM))

//IRQ related
#define MT6589_MMU0_IRQ_ID          (105+32)
#define MT6589_MMU1_IRQ_ID          (106+32)
#define MT6589_MMU_L2_IRQ_ID        (107+32)
#define MT6589_MMU_L2_SEC_IRQ_ID    (108+32)

//seq range related
#define M4U_SEQ_NR          16
#define SEQ_RANGE_NUM       M4U_SEQ_NR
#define TOTAL_RANGE_NUM       (M4U_SEQ_NR)*TOTAL_M4U_NUM
#define M4U_SEQ_ALIGN_MSK   (0x40000-1)
#define M4U_SEQ_ALIGN_SIZE  0x40000



//extern unsigned int gM4UBaseAddr[TOTAL_M4U_NUM];

typedef enum
{
    M4U_ID_0 = 0,
    M4U_ID_1,
    M4U_ID_ALL
}M4U_ID_ENUM;

typedef struct _M4U_RANGE_DES  //sequential entry range
{
    unsigned int Enabled;
    M4U_MODULE_ID_ENUM eModuleID;
    unsigned int MVAStart;
    unsigned int MVAEnd;
    unsigned int entryCount;
} M4U_RANGE_DES_T;

typedef struct _M4U_WRAP_DES
{
    unsigned int Enabled;
    M4U_MODULE_ID_ENUM eModuleID;
    M4U_PORT_ID_ENUM ePortID;    
    unsigned int MVAStart;
    unsigned int MVAEnd;
} M4U_WRAP_DES_T;

typedef enum
{
	M4U_DMA_READ_WRITE = 0,
	M4U_DMA_READ = 1,
	M4U_DMA_WRITE = 2,
	M4U_DMA_NONE_OP = 3,

} M4U_DMA_DIR_ENUM;


// module related:  alloc/dealloc MVA buffer
typedef struct _M4U_MOUDLE
{
	// MVA alloc / dealloc
	M4U_MODULE_ID_ENUM eModuleID;	// module ID used inside M4U driver, defined in M4U_PORT_MODULE_ID_ENUM
	unsigned int BufAddr;				// buffer virtual address
	unsigned int BufSize;				// buffer size in byte

	// TLB range invalidate or set uni-upadte range
	unsigned int MVAStart;						 // MVA start address
	unsigned int MVAEnd;							 // MVA end address
	M4U_RANGE_PRIORITY_ENUM ePriority;						 // range priority, 0:high, 1:normal
	unsigned int entryCount;

    // manually insert page entry
	unsigned int EntryMVA;						 // manual insert entry MVA
	unsigned int Lock;							 // manual insert lock or not
	int security;
    int cache_coherent;
}M4U_MOUDLE_STRUCT;


typedef struct _M4U_CACHE
{
    // MVA alloc / dealloc
    M4U_MODULE_ID_ENUM eModuleID;             // module ID used inside M4U driver, defined in M4U_MODULE_ID_ENUM
    M4U_CACHE_SYNC_ENUM eCacheSync;
    unsigned int BufAddr;                  // buffer virtual address
    unsigned int BufSize;                     // buffer size in byte
}M4U_CACHE_STRUCT;

typedef struct _M4U_PERF_COUNT
{
    unsigned int transaction_cnt;
    unsigned int main_tlb_miss_cnt;
    unsigned int pfh_tlb_miss_cnt;
    unsigned int pfh_cnt;
}M4U_PERF_COUNT;


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

} mva_info_t;

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


//IOCTL commnad
#define MTK_M4U_MAGICNO 'g'
#define MTK_M4U_T_POWER_ON            _IOW(MTK_M4U_MAGICNO, 0, int)
#define MTK_M4U_T_POWER_OFF           _IOW(MTK_M4U_MAGICNO, 1, int)
#define MTK_M4U_T_DUMP_REG            _IOW(MTK_M4U_MAGICNO, 2, int)
#define MTK_M4U_T_DUMP_INFO           _IOW(MTK_M4U_MAGICNO, 3, int)
#define MTK_M4U_T_ALLOC_MVA           _IOWR(MTK_M4U_MAGICNO,4, int)
#define MTK_M4U_T_DEALLOC_MVA         _IOW(MTK_M4U_MAGICNO, 5, int)
#define MTK_M4U_T_INSERT_TLB_RANGE    _IOW(MTK_M4U_MAGICNO, 6, int)
#define MTK_M4U_T_INVALID_TLB_RANGE   _IOW(MTK_M4U_MAGICNO, 7, int)
#define MTK_M4U_T_INVALID_TLB_ALL     _IOW(MTK_M4U_MAGICNO, 8, int)
#define MTK_M4U_T_MANUAL_INSERT_ENTRY _IOW(MTK_M4U_MAGICNO, 9, int)
#define MTK_M4U_T_CACHE_SYNC          _IOW(MTK_M4U_MAGICNO, 10, int)
#define MTK_M4U_T_CONFIG_PORT         _IOW(MTK_M4U_MAGICNO, 11, int)
#define MTK_M4U_T_CONFIG_ASSERT       _IOW(MTK_M4U_MAGICNO, 12, int)
#define MTK_M4U_T_INSERT_WRAP_RANGE   _IOW(MTK_M4U_MAGICNO, 13, int)
#define MTK_M4U_T_MONITOR_START       _IOW(MTK_M4U_MAGICNO, 14, int)
#define MTK_M4U_T_MONITOR_STOP        _IOW(MTK_M4U_MAGICNO, 15, int)
#define MTK_M4U_T_RESET_MVA_RELEASE_TLB  _IOW(MTK_M4U_MAGICNO, 16, int)
#define MTK_M4U_T_CONFIG_PORT_ROTATOR _IOW(MTK_M4U_MAGICNO, 17, int)
#define MTK_M4U_T_QUERY_MVA           _IOW(MTK_M4U_MAGICNO, 18, int)
#define MTK_M4U_T_M4UDrv_CONSTRUCT    _IOW(MTK_M4U_MAGICNO, 19, int)
#define MTK_M4U_T_M4UDrv_DECONSTRUCT  _IOW(MTK_M4U_MAGICNO, 20, int)
#define MTK_M4U_T_DUMP_PAGETABLE      _IOW(MTK_M4U_MAGICNO, 21, int)
#define MTK_M4U_T_REGISTER_BUFFER     _IOW(MTK_M4U_MAGICNO, 22, int)
#define MTK_M4U_T_CACHE_FLUSH_ALL     _IOW(MTK_M4U_MAGICNO, 23, int)
#define MTK_M4U_T_REG_SET             _IOW(MTK_M4U_MAGICNO, 24, int)
#define MTK_M4U_T_REG_GET             _IOW(MTK_M4U_MAGICNO, 25, int)


int __m4u_query_mva(M4U_MODULE_ID_ENUM eModuleID, 
								  const unsigned int BufAddr, 
								  const unsigned int BufSize, 
								  unsigned int *pRetMVABuf,
								  struct file * a_pstFile) ;



mva_info_t* m4u_alloc_garbage_list(   unsigned int mvaStart, 
                                          unsigned int bufSize,
                                          M4U_MODULE_ID_ENUM eModuleID,
                                          unsigned int va,
                                          unsigned int flags,
                                          int security,
                                          int cache_coherent);
int __m4u_alloc_mva(mva_info_t *pMvaInfo, struct sg_table *sg_table);

int __m4u_dealloc_mva(M4U_MODULE_ID_ENUM eModuleID, 
									const unsigned int BufAddr, 
									const unsigned int BufSize, 
									const unsigned int MVA,
									struct sg_table* sg_table) ;
int m4u_invalid_seq_range_by_mva(int m4u_index, unsigned int MVAStart, unsigned int MVAEnd);

int m4u_do_insert_seq_range(M4U_PORT_ID_ENUM eModuleID, 
                             unsigned int MVAStart, 
                             unsigned int MVAEnd, 
                             unsigned int entryCount);
int m4u_module_2_m4u_id(M4U_MODULE_ID_ENUM emoduleID);
int m4u_do_config_port(M4U_PORT_STRUCT* pM4uPort);

int m4u_do_monitor_start(int m4u_id);
int m4u_do_monitor_stop(int m4u_id);

int m4u_do_log_on(void);
int m4u_do_log_off(void);

int m4u_do_dump_info(int m4u_index);

int m4u_do_dma_cache_maint(M4U_MODULE_ID_ENUM eModuleID, const void *start, size_t size, int direction);
int m4u_do_mva_map_kernel(unsigned int mva, unsigned int size, int sec,
                        unsigned int* map_va, unsigned int* map_size);
int m4u_do_mva_unmap_kernel(unsigned int mva, unsigned int size, unsigned int va);
int m4u_do_debug_command(unsigned int command);

int m4u_dma_cache_flush_all();
M4U_MODULE_ID_ENUM mva2module(unsigned int mva);
#endif
