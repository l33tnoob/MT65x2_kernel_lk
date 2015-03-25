#ifndef _MT6572_SMI_REG_H__
#define _MT6572_SMI_REG_H__

#include <asm/io.h>

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


/* ===============================================================
 * 					  SMI COMMON
 * =============================================================== */


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


#define REG_SMI_SECUR_CON(x)       	  (0x05C0+SMI_COMMON_AO_BASE+((x)<<2))
#define REG_SMI_SECUR_CON_OF_PORT(port)     REG_SMI_SECUR_CON(((m4u_port_2_smi_port(port))>>3))  //compute the port setting register.
    #define F_SMI_SECUR_CON_SECURE(port)        ((1)<<(((m4u_port_2_smi_port(port))&0x7)<<2))
    #define F_SMI_SECUR_CON_DOMAIN(port, val)   (((val)&0x3)<<((((m4u_port_2_smi_port(port))&0x7)<<2)+1))
    #define F_SMI_SECUR_CON_VIRTUAL(port)       ((1)<<((((m4u_port_2_smi_port(port))&0x7)<<2)+3))
    #define F_SMI_SECUR_CON_MASK(port)          ((0xf)<<((((m4u_port_2_smi_port(port))&0x7)<<2)))

#define REG_SMI_SECUR_CON_G3D	  	  (0x05E8+SMI_COMMON_AO_BASE)	
                             	

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

