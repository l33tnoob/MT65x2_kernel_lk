/* An example test TA implementation.
 */

#ifndef __TRUSTZONE_TA_M4U__
#define __TRUSTZONE_TA_M4U__



#define TZ_TA_M4U_UUID   "m4u-smi-mau-spc"

/* Data Structure for Test TA */
/* You should define data structure used both in REE/TEE here
   N/A for Test TA */

/* Command for Test TA */
#define M4U_TZCMD_TEST              0
#define M4U_TZCMD_CONFIG_PORT       66
#define M4U_TZCMD_REG_BACKUP        67
#define M4U_TZCMD_REG_RESTORE       68



#define M4U_TZCMD_INVALID_TLB       75
#define M4U_TZCMD_HW_INIT           76
#define M4U_TZCMD_DUMP_REG          77
#define M4U_TZCMD_WAIT_ISR          78
#define M4U_TZCMD_INVALID_CHECK     79
#define M4U_TZCMD_INSERT_SEQ        80


#define M4U_CHECKSELF_VALUE 0x12345678

#define MMU_TOTAL_RS_NR         8
#define M4U_MAIN_TLB_NR         48

typedef struct _M4U_ISR_INFO_
{
    unsigned int u4Check; // fixed is M4U_CHECKSELF_VALUE
    unsigned int u4IrqM4uIndex;
    unsigned int IntrSrc;
    unsigned int faultMva;
    unsigned int port_regval;
    int          portID;
    int          larbID;

    unsigned int invalidPA;

    unsigned int rs_va[MMU_TOTAL_RS_NR];
    unsigned int rs_pa[MMU_TOTAL_RS_NR];
    unsigned int rs_st[MMU_TOTAL_RS_NR];

    unsigned int main_tags[M4U_MAIN_TLB_NR];
    unsigned int pfh_tags[M4U_MAIN_TLB_NR];

    unsigned int main_des[M4U_MAIN_TLB_NR];
    unsigned int pfn_des[M4U_MAIN_TLB_NR*4];
}M4U_ISR_INFO;

#endif /* __TRUSTZONE_TA_TEST__ */
