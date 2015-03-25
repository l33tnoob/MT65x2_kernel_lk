#include <mach/m4u.h>

#define __M4U_CACHE_SYCN_USING_KERNEL_MAP__

 static void m4u_mvaGraph_init(void);
 void m4u_mvaGraph_dump_raw(void);
 void m4u_mvaGraph_dump(void);
 static int m4u_dealloc_mva_dynamic(const M4U_MODULE_ID_ENUM eModuleID, 
									 const unsigned int BufAddr, 
									 const unsigned int BufSize,
									 const unsigned int mvaRegionAddr,
                                   struct sg_table* sg_table);
 static unsigned int m4u_do_mva_alloc(const M4U_MODULE_ID_ENUM eModuleID, 
								   const unsigned int BufAddr, 
								   const unsigned int BufSize);
 static int m4u_do_mva_free(const M4U_MODULE_ID_ENUM eModuleID, 
								 const unsigned int BufAddr,
								 const unsigned int BufSize,
								 const unsigned int mvaRegionStart) ;
 static M4U_MODULE_ID_ENUM mva2module(const unsigned int mva);
 static int m4u_invalid_seq_range_by_mva(unsigned int MVAStart, unsigned int MVAEnd);
 void m4u_dump_pagetable(const M4U_MODULE_ID_ENUM eModuleID);
 static int m4u_confirm_range_invalidated(const unsigned int MVAStart, const unsigned int MVAEnd);
 
 static bool m4u_struct_init(void);
 static int m4u_hw_init(void);
									 
 static int m4u_get_pages(const M4U_MODULE_ID_ENUM eModuleID,
					 const unsigned int BufAddr, 
					 const unsigned int BufSize, 
					 unsigned int* const pPhys);
 static int m4u_get_pages_sg(M4U_MODULE_ID_ENUM eModuleID, 
 	                 unsigned int BufAddr, 
 	                 unsigned int BufSize, 
                     struct sg_table* sg_table, unsigned int* pPhys);
								
 static void m4u_release_pages(const M4U_MODULE_ID_ENUM eModuleID,
					 const unsigned int BufAddr, 
					 const unsigned int BufSize,
					 const unsigned int MVA,
					 struct sg_table* sg_table);
 #ifndef __M4U_CACHE_SYCN_USING_KERNEL_MAP__
 static M4U_DMA_DIR_ENUM m4u_get_dir_by_module(M4U_MODULE_ID_ENUM eModuleID);
 #endif
 static void m4u_clear_intr(const unsigned int m4u_base);
#define  m4u_port_2_m4u_id(portID) 0  // TODO: FIXME! cloud
 static void m4u_memory_usage(void);
 void m4u_print_active_port(void);
 static M4U_MODULE_ID_ENUM m4u_port_2_module(M4U_PORT_ID_ENUM portID);
 static char* m4u_get_port_name(const M4U_PORT_ID_ENUM portID);
 static char* m4u_get_module_name(const M4U_MODULE_ID_ENUM moduleID);
 void m4u_get_power_status(void);
 unsigned int m4u_get_pa_by_mva(unsigned int mva);
 int m4u_dump_user_addr_register(unsigned int m4u_index);
 static int m4u_add_to_garbage_list(const struct file * a_pstFile,
										 const unsigned int mvaStart, 
										 const unsigned int bufSize,
										 const M4U_MODULE_ID_ENUM eModuleID,
										 const unsigned int va,
										 const unsigned int flags,
										 const int security,
										 const int cache_coherent);
 static int m4u_delete_from_garbage_list(const M4U_MOUDLE_STRUCT* p_m4u_module, const struct file * a_pstFile);
 M4U_PORT_ID_ENUM m4u_get_error_port(unsigned int m4u_index, unsigned int mva);
 int m4u_dump_mva_info(void);
 int m4u_get_write_mode_by_module(M4U_MODULE_ID_ENUM moduleID);
 void m4u_dump_pagetable_range(unsigned int vaStart, const unsigned int nr);
 void m4u_print_mva_list(struct file *filep, const char *pMsg);
 int m4u_dma_cache_flush_all(void);
 extern void mlock_vma_page(struct page *page);
 extern void munlock_vma_page(struct page *page);
 static void m4u_dump_main_tlb_tags(void) ;
 int m4u_dump_main_tlb_des(void); 
// static void m4u_dump_pfh_tlb_tags(void);
 int m4u_dump_pfh_tlb_des(void);
// static void m4u_enable_error_hang(const bool fgEnable);
 static void m4u_invalidate_and_check(unsigned int start, unsigned int end);
 int m4u_query_mva(const M4U_MODULE_ID_ENUM eModuleID, 
						const unsigned int BufAddr, 
						const unsigned int BufSize, 
						unsigned int *pRetMVABuf,
						const struct file * a_pstFile);

//static int m4u_invalid_seq_all(const M4U_MODULE_ID_ENUM eModuleID);
static void m4u_dump_wrap_range_info(void);
//static int m4u_perf_timer_on(void);
static void m4u_profile_init(void);
static int m4u_log_on(void);
static int m4u_log_off(void);
static void m4u_dump_seq_range_info(void);
static void m4u_invalid_tlb_all(void);

 
