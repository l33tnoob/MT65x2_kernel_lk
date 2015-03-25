#ifndef __MT_EMI_BW_LIMITER__
#define __MT_EMI_BW_LIMITER__

/*
 * Define EMI hardware registers.
 */
 
//#define UINT32P unsigned int *
#define EMI_CONN                 (EMI_BASE+0x0068)
#define EMI_ARBA                 (EMI_BASE+0x0100)
#define EMI_ARBB                 (EMI_BASE+0x0108)
#define EMI_ARBC                 (EMI_BASE+0x0110)
#define EMI_ARBD                 (EMI_BASE+0x0118)
#define EMI_ARBE                (EMI_BASE+0x0120)
#define EMI_ARBF                 (EMI_BASE+0x0128)
#define EMI_ARBG                 (EMI_BASE+0x0130)


/*
 * Define constants.
 */

/* define supported DRAM types */
enum
{
    LPDDR1 = 0,
    LPDDR2,
    LPDDR3,
    PCDDR3
};

/* define concurrency scenario ID */
enum 
{
#define X_CON_SCE(con_sce, arba, arbb, arbc, arbd, arbe, arbf, arbg)  con_sce,
#include "mach/con_sce_mddr.h"
#undef X_CON_SCE
    NR_CON_SCE
};

/* define control operation */
enum
{
    ENABLE_CON_SCE = 0,
    DISABLE_CON_SCE = 1
};

#define EN_CON_SCE_STR "ON"
#define DIS_CON_SCE_STR "OFF"

/*
 * Define data structures.
 */

/* define control table entry */
struct emi_bwl_ctrl
{
    unsigned int ref_cnt; 
};

/*
 * Define function prototype.
 */

extern int mtk_mem_bw_ctrl(int sce, int op);
extern int get_ddr_type(void);

#endif  /* !__MT_EMI_BWL_H__ */

