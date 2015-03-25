#ifndef _MT6572_M4U_REG_H__
#define _MT6572_M4U_REG_H__

#include <asm/io.h>

#define M4U_BASE                 0xf0203000
#define M4U_BASEg                0xf0203000

#define LARB0_BASE 	0xf4010000

#define SMI_COMMON_EXT_BASE 0xf4011000

// SMI ALWAYS ON        
#define SMI_COMMON_AO_BASE		0xf000E000

//=================================================
//common macro definitions
#define F_VAL(val,msb,lsb) (((val)&((1<<(msb-lsb+1))-1))<<lsb)
#define F_MSK(msb, lsb)     F_VAL(0xffffffff, msb, lsb)
#define F_BIT_SET(bit)          (1<<(bit))
#define F_BIT_VAL(val,bit)  ((!!(val))<<(bit))
#define F_MSK_SHIFT(regval,msb,lsb) (((regval)&F_MSK(msb,lsb))>>lsb)


//=====================================================
//M4U register definition
//=====================================================

#define REG_MMU_PROG_EN                 0x10
    #define F_MMU_PROG_EN               1
#define REG_MMU_PROG_VA                 0x14
    #define F_PROG_VA_LOCK_BIT             (1<<11)
    #define F_PROG_VA_SECURE_BIT           (1<<10)
    #define F_PROG_VA_MASK            0xfffff000

#define REG_MMU_PROG_DSC                0x18

#define REG_MMU_SQ_START_0_7(x)          (0x20+((x)<<3))
#define REG_MMU_SQ_END_0_7(x)           (0x24+((x)<<3))
#define REG_MMU_SQ_START_8_15(x)        (0x500+(((x)-8)<<3))  //No this register in 6572
#define REG_MMU_SQ_END_8_15(x)          (0x504+(((x)-8)<<3))  //No this register in 6572

#define REG_MMU_SQ_START(x)             (((x)<8) ? REG_MMU_SQ_START_0_7(x): REG_MMU_SQ_START_8_15(x))
    #define F_SQ_VA_MASK                0xfffc0000
    #define F_SQ_EN_BIT                 (1<<17)
    #define F_SQ_MULTI_ENTRY_VAL(x)     (((x)&0xf)<<13)
#define REG_MMU_SQ_END(x)               (((x)<8) ? REG_MMU_SQ_END_0_7(x): REG_MMU_SQ_END_8_15(x))

#define REG_MMU_PFH_DIST0           0x80
#define REG_MMU_PFH_DIST1           0x84

#define REG_MMU_PFH_DIST(port)      (0x80+(((port)>>3)<<2))
    #define F_MMU_PFH_DIST_VAL(port,val) ((val&0xf)<<(((port)&0x7)<<2))
    #define F_MMU_PFH_DIST_MASK(port)     F_MMU_PFH_DIST_VAL((port), 0xf)

#define REG_MMU_PFH_DIR0         0xF0
#define REG_MMU_PFH_DIR1         0xF4
#define REG_MMU_PFH_DIR(port)   (((port)<32) ? REG_MMU_PFH_DIR0: REG_MMU_PFH_DIR1)
    #define F_MMU_PFH_DIR_VAL(port,val) ((!!(val))<<((port)&0x1f))
	#define F_MMU_PFH_DIR_MASK(port) (1<<((port)&0x1f))

#define REG_MMU_MAIN_TAG_0_31(x)       (0x100+((x)<<2))
#define REG_MMU_MAIN_TAG_32_63(x)      (0x400+(((x)-32)<<2)) //No this register in 6572
#define REG_MMU_MAIN_TAG(x) (((x)<32) ? REG_MMU_MAIN_TAG_0_31(x): REG_MMU_MAIN_TAG_32_63(x))
    #define F_MAIN_TLB_LOCK_BIT     (1<<11)
    #define F_MAIN_TLB_VALID_BIT    (1<<10)
    #define F_MAIN_TLB_SQ_EN_BIT    (1<<9)
    #define F_MAIN_TLB_SQ_INDEX_MSK (0xf<<5)
    #define F_MAIN_TLB_INV_DES_BIT      (1<<4)
    #define F_MAIN_TLB_VA_MSK           F_MSK(31, 12)


#define REG_MMU_PFH_TAG_0_31(x)       (0x180+((x)<<2))
#define REG_MMU_PFH_TAG_32_63(x)      (0x480+(((x)-32)<<2)) //No this register in 6572
#define REG_MMU_PFH_TAG(x) (((x)<32) ? REG_MMU_PFH_TAG_0_31(x): REG_MMU_PFH_TAG_32_63(x))
    #define F_PFH_TAG_VA_MSK            F_MSK(31, 13)
    #define F_PFH_TAG_VALID_MSK         F_MSK(12, 11)
    #define F_PFH_TAG_DESC_VALID_MSK    F_MSK(10, 9)
    #define F_PFH_TAG_REQUEST_BY_PFH    F_BIT_SET(8)
    #define F_PFH_TAG_SEQ_EN            F_BIT_SET(7)	        

#define REG_MMU_READ_ENTRY       0x200
    #define F_READ_ENTRY_TLB_SEL_PFH        F_VAL(1,12,12)
    #define F_READ_ENTRY_TLB_SEL_MAIN       F_VAL(0,12,12)
    #define F_READ_ENTRY_INDEX_VAL(idx)     F_VAL(idx,9,5)
    #define F_READ_ENTRY_PFH_IDX(idx)       F_VAL(idx,4,4)
    #define F_READ_ENTRY_READ_EN_BIT        F_BIT_SET(0)
    
#define REG_MMU_DES_RDATA        0x204


#define REG_MMU_CTRL_REG         0x210
    #define F_MMU_CTRL_PFH_DIS(dis)         F_BIT_VAL(dis, 0)
    #define F_MMU_CTRL_TLB_WALK_DIS(dis)    F_BIT_VAL(dis, 1)
    #define F_MMU_CTRL_MONITOR_EN(en)       F_BIT_VAL(en, 2)
    #define F_MMU_CTRL_MONITOR_CLR(clr)       F_BIT_VAL(clr, 3)
    #define F_MMU_CTRL_PFH_RT_RPL_MODE(mod)   F_BIT_VAL(mod, 4)
    #define F_MMU_CTRL_TF_PROT_VAL(prot)    F_VAL(prot, 6, 5)
    #define F_MMU_CTRL_TF_PROT_MSK           F_MSK(6,5)
    #define F_MMU_CTRL_INT_HANG_en(en)       F_BIT_VAL(en, 7)
    #define F_MMU_CTRL_COHERE_EN(en)        F_BIT_VAL(en, 8)





#define REG_MMU_IVRP_PADDR       0x214
#define REG_MMU_INT_CONTROL      0x220
    #define F_INT_CLR_BIT (1<<12)
#define REG_MMU_FAULT_ST         0x224
    #define F_INT_TRANSLATION_FAULT                 F_BIT_SET(0)
    #define F_INT_TLB_MULTI_HIT_FAULT               F_BIT_SET(1)
    #define F_INT_INVALID_PHYSICAL_ADDRESS_FAULT    F_BIT_SET(2)
    #define F_INT_ENTRY_REPLACEMENT_FAULT           F_BIT_SET(3)
    #define F_INT_TABLE_WALK_FAULT                  F_BIT_SET(4)
    #define F_INT_TLB_MISS_FAULT                    F_BIT_SET(5)
    #define F_INT_PFH_DMA_FIFO_OVERFLOW             F_BIT_SET(6)
    #define F_INT_MISS_DMA_FIFO_OVERFLOW            F_BIT_SET(7)
    #define F_INT_PFH_FIFO_OUT_ERR                  F_BIT_SET(8)
    #define F_INT_PFH_FIFO_IN_ERR                   F_BIT_SET(9)	
    #define F_INT_MISS_FIFO_OUT_ERR                 F_BIT_SET(10)
    #define F_INT_MISS_FIFO_IN_ERR                  F_BIT_SET(11)	
    #define F_INT_INVALIDATION_DONE                 F_BIT_SET(12)	
    
#define REG_MMU_FAULT_VA         0x228
#define REG_MMU_INVLD_PA         0x22C
#define REG_MMU_ACC_CNT          0x230
#define REG_MMU_MAIN_MSCNT       0x234
#define REG_MMU_PF_MSCNT         0x238
#define REG_MMU_PF_CNT           0x23C

#define REG_MMU_WRAP_SA(x)       (0x300+((x)<<3))
#define REG_MMU_WRAP_EA(x)       (0x304+((x)<<3))

#define REG_MMU_WRAP_EN0    0x340
#define REG_MMU_WRAP_EN1    0x344

#define REG_MMU_WRAP_EN(port)         (0x340+(((port)>>3)<<2))
    #define F_MMU_WRAP_SEL_VAL(port, val)  (((val)&0xf)<<(((port)&0x7)<<2))



#define REG_MMU_PFQ_BROADCAST_EN  0x364
#define REG_MMU_NON_BLOCKING_DIS    0x380
    #define F_MMU_NON_BLOCK_DISABLE_BIT 1
#define REG_MMU_RS_PERF_CNT         0x384

#define REG_MMU_INT_ID              0x388
    #define F_INT_ID_TF_PORT_ID(regval)     F_MSK_SHIFT(regval,12, 8)
    #define F_INT_ID_TF_LARB_ID(regval)     F_MSK_SHIFT(regval,14, 13)


#define MMU_TOTAL_RS_NR         8
#define REG_MMU_RSx_VA(x)      (0x550+((x)<<2))
#define REG_MMU_RSx_PA(x)      (0x570+((x)<<2))
#define REG_MMU_RSx_ST(x)      (0x5A0+((x)<<2))




//================================================================
// SMI larb
//================================================================

#define SMI_LARB_STAT            (0x0  )
#define SMI_LARB_CON             (0x10 ) 
    #define F_SMI_LARB_CON_MAU_IRQ_EN(en)   F_BIT_VAL(en, 14)
#define SMI_LARB_CON_SET         (0x14 ) 
#define SMI_LARB_CON_CLR         (0x18 ) 
#define SMI_ARB_CON              (0x20 ) 
#define SMI_ARB_CON_SET          (0x24 ) 
#define SMI_ARB_CON_CLR          (0x28 ) 
#define SMI_BW_EXT_CON0          (0x30 ) 
#define SMI_STARV_CON0           (0x40 ) 
#define SMI_STARV_CON1           (0x44 ) 
#define SMI_STARV_CON2           (0x48 ) 
#define SMI_INT_PATH_SEL         (0x54 ) 
#define SMI_LARB_BWFILTER_EN     (0x60 ) 
#define SMI_LARB_OSTD_CTRL_EN    (0x64 ) 
#define SMI_BW_VC0               (0x80 ) 
#define SMI_BW_VC1               (0x84 ) 
#define SMI_BW_VC2               (0x88 ) 
#define SMI_LARB_CMD_THRT_LMT    (0x8C )
#define SMI_SHARE_EN             (0xf0 )  // register address change in 6572, but no use in 6572
#define SMI_EBW_PORT             (0x100)
#define SMI_LARB_BWL_PORT        (0x180)
#define SMI_LARB_OSTD_PORT       (0x200)
#define SMI_SUB_PINFO            (0x280)

//====== mau registers ========
#if 0
#define SMI_MAU_ENTR_START(x)      (0x300+(x)*0x10)
    #define F_MAU_START_WR(en)      F_BIT_VAL(en, 31)
    #define F_MAU_START_RD(en)      F_BIT_VAL(en, 30)
    #define F_MAU_START_ADD_MSK     F_MSK(29, 0)
    #define F_MAU_START_ADD(addr)    F_MSK_SHIFT(addr, 31, 2)
    #define F_MAU_START_IS_WR(regval)   F_MSK_SHIFT(regval, 31, 31)
    #define F_MAU_START_IS_RD(regval)   F_MSK_SHIFT(regval, 30, 30)
    #define F_MAU_START_ADDR_VAL(regval)  (F_MSK_SHIFT(regval, 29, 0)<<2)
#define SMI_MAU_ENTR_END(x)        (0x304+(x)*0x10)
    #define F_MAU_END_VIR(en)      F_BIT_VAL(en, 30)
    #define F_MAU_END_ADD(addr)    F_MSK_SHIFT(addr, 31, 2)
    #define F_MAU_END_IS_VIR(regval) F_MSK_SHIFT(regval, 30, 30)
    #define F_MAU_END_ADDR_VAL(regval) (F_MSK_SHIFT(regval, 29, 0)<<2)
    
#define SMI_MAU_ENTR_GID(x)      (0x308+(x)*0x10)
#define SMI_MAU_ENTR_STAT(x)       (0x500+(x)*0x4)
    #define F_MAU_STAT_ASSERT(regval)   F_MSK_SHIFT(regval, 5, 5)
    #define F_MAU_STAT_ID(regval)       F_MSK_SHIFT(regval, 4, 0)


#define SMI_LARB_MON_ENA         (0x400)
#define SMI_LARB_MON_CLR         (0x404)
#define SMI_LARB_MON_PORT        (0x408)
#define SMI_LARB_MON_TYPE        (0x40c)
#define SMI_LARB_MON_CON         (0x410)
#define SMI_LARB_MON_ACT_CNT     (0x420)
#define SMI_LARB_MON_REQ_CNT     (0x424)
#define SMI_LARB_MON_IDL_CNT     (0x428)
#define SMI_LARB_MON_BEA_CNT     (0x42c)
#define SMI_LARB_MON_BYT_CNT     (0x430)
#define SMI_LARB_MON_CP_CNT      (0x434)
#define SMI_LARB_MON_DP_CNT      (0x438)
#define SMI_LARB_MON_CDP_MAX     (0x43c)
#define SMI_LARB_MON_COS_MAX     (0x440)
#define SMI_LARB_MON_BUS_REQ0    (0x450)
#define SMI_LARB_MON_BUS_REQ1    (0x454)
#define SMI_LARB_MON_WDT_CNT     (0x460)
#define SMI_LARB_MON_RDT_CNT     (0x464)
#define SMI_LARB_MON_OST_CNT     (0x468)
#define SMI_LARB_MON_STALL_CNT   (0x46c)


#define SMI_IRQ_STATUS           (0x520)
#define SMI_LARB_FIFO_STAT0      (0x600)
#define SMI_LARB_FIFO_STAT1      (0x604)
#define SMI_LARB_BUS_STAT0       (0x610)
#define SMI_LARB_BUS_STAT1       (0x614)
#define SMI_LARB_DBG_MODE0       (0x700)
#define SMI_LARB_DBG_MODE1       (0x704)
#define SMI_LARB_TST_MODE0       (0x780)
#endif

/* ===============================================================
 * 					  SMI COMMON
 * =============================================================== */

#if 0
#define REG_SMI_MON_AXI_ENA             (0x1a0+SMI_COMMON_EXT_BASE)	
#define REG_SMI_MON_AXI_CLR             (0x1a4+SMI_COMMON_EXT_BASE)	
#define REG_SMI_MON_AXI_TYPE            (0x1ac+SMI_COMMON_EXT_BASE)	
#define REG_SMI_MON_AXI_CON	            (0x1b0+SMI_COMMON_EXT_BASE)	
#define REG_SMI_MON_AXI_ACT_CNT         (0x1c0+SMI_COMMON_EXT_BASE)	
#define REG_SMI_MON_AXI_REQ_CNT         (0x1c4+SMI_COMMON_EXT_BASE)	
#define REG_SMI_MON_AXI_IDL_CNT         (0x1c8+SMI_COMMON_EXT_BASE)	
#define REG_SMI_MON_AXI_BEA_CNT         (0x1cc+SMI_COMMON_EXT_BASE)	
#define REG_SMI_MON_AXI_BYT_CNT         (0x1d0+SMI_COMMON_EXT_BASE)	
#define REG_SMI_MON_AXI_CP_CNT          (0x1d4+SMI_COMMON_EXT_BASE)	
#define REG_SMI_MON_AXI_DP_CNT          (0x1d8+SMI_COMMON_EXT_BASE)	
#define REG_SMI_MON_AXI_CP_MAX          (0x1dc+SMI_COMMON_EXT_BASE)	
#define REG_SMI_MON_AXI_COS_MAX         (0x1e0+SMI_COMMON_EXT_BASE)	
#define REG_SMI_L1LEN	                (0x200+SMI_COMMON_EXT_BASE)	
#define REG_SMI_L1ARB0	                (0x204+SMI_COMMON_EXT_BASE)	
#define REG_SMI_L1ARB1	                (0x208+SMI_COMMON_EXT_BASE)	

#define REG_SMI_WRR_REG0                (0x228+SMI_COMMON_EXT_BASE)	
#define REG_SMI_READ_FIFO_TH            (0x230+SMI_COMMON_EXT_BASE)	
#define REG_SMI_M4U_TH                  (0x234+SMI_COMMON_EXT_BASE)	
#define REG_SMI_DCM                     (0x300+SMI_COMMON_EXT_BASE)	
#define REG_SMI_DEBUG0                  (0x400+SMI_COMMON_EXT_BASE)	
#define REG_SMI_DUMMY                   (0x418+SMI_COMMON_EXT_BASE)	
                                	
        	
#define REG_SMI_CON             	  (0x0010+SMI_COMMON_AO_BASE)	
#define REG_SMI_CON_SET         	  (0x0014+SMI_COMMON_AO_BASE)	
#define REG_SMI_CON_CLR         	  (0x0018+SMI_COMMON_AO_BASE)	
#define REG_SMI_SEN             	  (0x0500+SMI_COMMON_AO_BASE)	
#define REG_D_VIO_CON(x)              (0x0550+SMI_COMMON_AO_BASE+((x)<<2))
#define REG_D_VIO_STA(x)           	  (0x0560+SMI_COMMON_AO_BASE+((x)<<2))
#endif

#define REG_SMI_SECUR_CON(x)       	  (0x05C0+SMI_COMMON_AO_BASE+((x)<<2))
#define REG_SMI_SECUR_CON_OF_PORT(port)     REG_SMI_SECUR_CON(((m4u_port_2_smi_port(port))>>3))  //compute the port setting register.
    #define F_SMI_SECUR_CON_SECURE(port)        ((1)<<(((m4u_port_2_smi_port(port))&0x7)<<2))
    #define F_SMI_SECUR_CON_DOMAIN(port, val)   (((val)&0x3)<<((((m4u_port_2_smi_port(port))&0x7)<<2)+1))
    #define F_SMI_SECUR_CON_VIRTUAL(port)       ((1)<<((((m4u_port_2_smi_port(port))&0x7)<<2)+3))
    #define F_SMI_SECUR_CON_MASK(port)          ((0xf)<<((((m4u_port_2_smi_port(port))&0x7)<<2)))

#define REG_SMI_SECUR_CON_G3D	  	  (0x05E8+SMI_COMMON_AO_BASE)	
                             	

/* =============================	==================================
 * 					  M4U global        	
 * =============================	================================== */
#define REG_MMUg_CTRL           	 (0x5D8  + M4U_BASE)
 #define F_MMUg_CTRL_INV_EN0    	 (1<<0)
                                	
#define REG_MMUg_INVLD          	 (0x5C0  + M4U_BASE)    
 #define F_MMUg_INV_ALL         	 0x2   
 #define F_MMUg_INV_RANGE       	 0x1   
                                	    
#define REG_MMUg_INVLD_SA        	 (0x5C4  + M4U_BASE)     
#define REG_MMUg_INVLD_EA            (0x5C8  + M4U_BASE)    
#define REG_MMUg_PT_BASE             (0x0    + M4U_BASE)
 #define F_MMUg_PT_VA_MSK        0xffff0000 //64kB alignment
    
#define REG_MMUg_DCM               (0x5F0 + M4U_BASEg)
    #define F_MMUg_DCM_ON(on)       F_BIT_VAL(on, 0)

//registers for security
#define REG_MMUg_CTRL_SEC          (0x5DC + M4U_BASE)
 #define F_MMUg_CTRL_SEC_INV_EN0     (1<<0)
 #define F_MMUg_CTRL_SEC_INV_EN0_MSK (1)
 #define F_MMUg_CTRL_SEC_DBG         (1<<5)
 #define F_SEC_INT_EN                (1<<12)

#define REG_MMUg_INVLD_SEC           (0x5CC+M4U_BASE)
 #define F_MMUg_INV_SEC_RANGE        0x1 
                                     
#define REG_MMUg_PT_BASE_SEC         (0x4 +M4U_BASE)
 #define F_MMUg_PT_VA_MSK_SEC        0xffff0000 //64kB alignment

//=================================================================
//other un-register definitions
#define F_DESC_VALID                F_VAL(0x2,1,0)
#define F_DESC_SHARE(en)            F_BIT_VAL(en,2)
#define F_DESC_NONSEC(non_sec)      F_BIT_VAL(non_sec,3)
#define F_DESC_PA_MSK               F_MSK(31,12)


#if 1
static inline unsigned int M4U_ReadReg32(unsigned int M4uBase, unsigned int Offset) 
{
    unsigned int val;
    val = ioread32(M4uBase+Offset);
    
    //printk("read base=0x%x, reg=0x%x, val=0x%x\n",M4uBase,Offset,val );
    return val;
}
static inline void M4U_WriteReg32(unsigned int M4uBase, unsigned int Offset, unsigned int Val) 
{                   
    //unsigned int read;
    iowrite32(Val, M4uBase+Offset);    
    mb();    
    /*
    read = M4U_ReadReg32(M4uBase, Offset);
    if(read != Val)
    {
        printk("error to write base=0x%x, reg=0x%x, val=0x%x, read=0x%x\n",M4uBase,Offset, Val, read );
    }
    else
    {
        printk("write base=0x%x, reg=0x%x, val=0x%x, read=0x%x\n",M4uBase,Offset, Val, read );
    }
*/

}

static inline unsigned int COM_ReadReg32(unsigned int addr) 
{
    return ioread32(addr);
}
static inline void COM_WriteReg32(unsigned int addr, unsigned int Val)
{          
    iowrite32(Val, addr);    
    mb();    
    /*
    if(COM_ReadReg32(addr) != Val)
    {
        printk("error to write add=0x%x, val=0x%x, read=0x%x\n",addr, Val, COM_ReadReg32(addr) );
    }
    else
    {
        printk("write success add=0x%x, val=0x%x, read=0x%x\n",addr, Val, COM_ReadReg32(addr) );
    }
    */
}

static inline unsigned int m4uHw_set_field(unsigned int M4UBase, unsigned int Reg,
                                      unsigned int bit_width, unsigned int shift,
                                      unsigned int value)
{
    unsigned int mask = ((1<<bit_width)-1)<<shift;
    unsigned int old;
    value = (value<<shift)&mask;
    old = M4U_ReadReg32(M4UBase, Reg);
    M4U_WriteReg32(M4UBase, Reg, (old&(~mask))|value);
    return (old&mask)>>shift;
}

#if 0
static inline unsigned int m4uHw_get_field(unsigned int M4UBase, unsigned int Reg,
                                      unsigned int bit_width, unsigned int shift)
{
    unsigned int mask = ((1<<bit_width)-1);
    unsigned int reg = M4U_ReadReg32(M4UBase, Reg);
    return ( (reg>>shift)&mask);
}
#endif

static inline void m4uHw_set_field_by_mask(unsigned int M4UBase, unsigned int reg,
                                      unsigned int mask, unsigned int val)
{
    unsigned int regval;
    regval = M4U_ReadReg32(M4UBase, reg);
    regval = (regval & (~mask))|val;
    M4U_WriteReg32(M4UBase, reg, regval);
}
static inline unsigned int m4uHw_get_field_by_mask(unsigned int M4UBase, unsigned int reg,
                                      unsigned int mask)
{
    return M4U_ReadReg32(M4UBase, reg)&mask;
}

#endif



#endif 

