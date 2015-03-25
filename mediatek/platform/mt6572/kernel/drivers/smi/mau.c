#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/earlysuspend.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/sched.h>   //wake_up_process()
#include <linux/kthread.h> //kthread_create()¡¢kthread_run()
#include <mach/irqs.h>
#include <asm/io.h>

#include <mach/mt_smi.h>
#include "smi_reg.h"
#include "smi_common.h"

#include <mach/mt_typedefs.h>
#include <mach/mt_reg_base.h>
#include <mach/mt_clkmgr.h>
#include <mach/mt_irq.h>
#include <mach/m4u.h>

#define SMI_LOG_TAG "MAU"

/*****************************************************************************
 * FUNCTION
 *    mau_enable_interrupt
 * DESCRIPTION
 *    Configure register to enable MAU assert interrupt
 * PARAMETERS
 *	param1 : [IN] const int larb
 *				larb index.
 * RETURNS
 *    None.
 ****************************************************************************/
static void mau_enable_interrupt(const int larb)
{
    M4U_WriteReg32(gLarbBaseAddr[larb], SMI_LARB_CON_SET, F_SMI_LARB_CON_MAU_IRQ_EN(1));
}
#if 0
/*****************************************************************************
 * FUNCTION
 *    mau_disable_interrupt
 * DESCRIPTION
 *    Configure register to disable MAU assert interrupt
 * PARAMETERS
 *	param1 : [IN] const int larb
 *				larb index.
 * RETURNS
 *    None.
 ****************************************************************************/
static void mau_disable_interrupt(const int larb)
{
    M4U_WriteReg32(gLarbBaseAddr[larb], SMI_LARB_CON_CLR, F_SMI_LARB_CON_MAU_IRQ_EN(1));
}
#endif
/*****************************************************************************
 * FUNCTION
 *    mau_isr
 * DESCRIPTION
 *    1. Print MAU status, such as port ID.
 *    2. Clear interrupt status.
 * PARAMETERS
 *	  param1 : [IN] int irq
 *				  irq number.
 *	  param2 : [IN] void *dev_id
 *				  No use in this function.
 * RETURNS
 *    Type: irqreturn_t. IRQ_HANDLED mean success.
 ****************************************************************************/
static irqreturn_t mau_isr(int irq, void *dev_id)
{
    int larb,i;
    unsigned int larb_base;
    unsigned int regval;

    switch(irq)
    {
        case MT_SMI_LARB0_IRQ_ID:
            larb = 0;
            break;
        default :
            larb=0;
            SMIERR("unkown irq(%d)\n",irq);
            
    }

    larb_clock_on(larb, "MAU");
        
    larb_base = gLarbBaseAddr[larb];

    //dump interrupt debug infomation
    for(i=0; i<MAU_ENTRY_NR; i++)
    {
        regval = M4U_ReadReg32(larb_base, SMI_MAU_ENTR_STAT(i));
        if(F_MAU_STAT_ASSERT(regval))
        {
            //violation happens in this entry
            int port =  F_MAU_STAT_ID(regval);
            SMIMSG("[MAU] larb=%d, entry=%d, port=%d\n",larb,i,port);
            regval = M4U_ReadReg32(larb_base, SMI_MAU_ENTR_START(i));
            SMIMSG("start_addr=0x%x, read_en=%d, write_en=%d\n", F_MAU_START_ADDR_VAL(regval), 
                F_MAU_START_IS_RD(regval), F_MAU_START_IS_WR(regval));
            regval = M4U_ReadReg32(larb_base, SMI_MAU_ENTR_END(i));
            SMIMSG("end_addr=0x%x, virtual=%d\n", F_MAU_END_ADDR_VAL(regval), 
                F_MAU_END_IS_VIR(regval));
            smi_aee_print("violation by %s\n",smi_port_name[port]);
        }

        //clear interrupt status
        regval = M4U_ReadReg32(larb_base, SMI_MAU_ENTR_STAT(i));
        M4U_WriteReg32(larb_base, SMI_MAU_ENTR_STAT(i), regval);
    }

    larb_clock_off(larb, "MAU");
    return IRQ_HANDLED;
}

/*****************************************************************************
 * FUNCTION
 *    mau_start_monitor
 * DESCRIPTION
 *    Configure register to set MAU assert
 *    a) Assert range
 *    b) Write and/or read transaction
 *    c) Physical or virtual address
 *    d) port mask
 * PARAMETERS
 *	param1 : [IN] const int larb
 *				larb index.
 *	param2 : [IN] const int entry
 *				Index of MAU in assigned larb.
 *	param3 : [IN] const int rd
 *				0 mean no check read transaction, and others mean check read transaction.
 *	param4 : [IN] const int wr
 *				0 mean no check write transaction, and others mean check write transaction.
 *	param5 : [IN] const int vir
 *				0 mean physical address, and others mean virtual address.
 *	param6 : [IN] const int wr
 *				0 mean no check write transaction, and others mean check write transaction.
 *	param7 : [IN] const unsigned int start
 *				assert start address. (word align)
 *	param8 : [IN] const unsigned int end
 *				assert end address. (word align)
 *	param9 : [IN] const unsigned int port_msk
 *				port mask (bitwise)
 * RETURNS
 *    None.
 ****************************************************************************/
static void mau_start_monitor(const int larb, const int entry, const int rd, const int wr, const int vir, 
            const unsigned int start, const unsigned int end, const unsigned int port_msk)
{
    unsigned int regval;
    unsigned int larb_base = gLarbBaseAddr[larb];
    
    //mau entry i start address
    regval = F_MAU_START_WR(wr)|F_MAU_START_RD(rd)|F_MAU_START_ADD(start);
    M4U_WriteReg32(larb_base, SMI_MAU_ENTR_START(entry), regval);
    regval = F_MAU_END_VIR(vir)|F_MAU_END_ADD(end);
    M4U_WriteReg32(larb_base, SMI_MAU_ENTR_END(entry), regval);

    //start monitor
    regval = M4U_ReadReg32(larb_base, SMI_MAU_ENTR_GID(entry));
    M4U_WriteReg32(larb_base, SMI_MAU_ENTR_GID(entry), regval|port_msk);
}

/*****************************************************************************
 * FUNCTION
 *    mau_config
 * DESCRIPTION
 *    1. Call mau_start_monitor to set MAU related register.
 * PARAMETERS
 *	param1 : [IN] MTK_MAU_CONFIG* pMauConf
 *				MAU setting configuration.
 * RETURNS
 *    Type: Integer.  zero mean success and others mean fail.
 ****************************************************************************/
int mau_config(MTK_MAU_CONFIG* pMauConf)
{

    SMIMSG("mau config: larb=%d,entry=%d,rd=%d,wr=%d,vir=%d,start=0x%x,end=0x%x,port_msk=0x%x\n",
        pMauConf->larb, pMauConf->entry, pMauConf->monitor_read, pMauConf->monitor_write,
        pMauConf->virt,pMauConf->start, pMauConf->end, pMauConf->port_msk);

    if(pMauConf->larb > SMI_LARB_NR ||
        pMauConf->entry > MAU_ENTRY_NR)
    {
        SMIERR("config:larb=%d,entry=%d\n", pMauConf->larb, pMauConf->entry);
        return -1;
    }


    larb_clock_on(pMauConf->larb, "MAU");
    mau_start_monitor(pMauConf->larb,
                      pMauConf->entry,
                      !!(pMauConf->monitor_read),
                      !!(pMauConf->monitor_write),
                      !!(pMauConf->virt),
                      pMauConf->start,
                      pMauConf->end,
                      pMauConf->port_msk); 
    
    larb_clock_off(pMauConf->larb, "MAU");
    return 0;
}

/*****************************************************************************
 * FUNCTION
 *    mau_dump_status
 * DESCRIPTION
 *    1. Dump register in MAU related register.
 *    2. Show MAU status 
 * PARAMETERS
 *	param1 : [IN] MTK_MAU_CONFIG* pMauConf
 *				MAU setting configuration.
 * RETURNS
 *    None.
 ****************************************************************************/
void mau_dump_status(const int larb)
{
    unsigned int larb_base;
    unsigned int regval;
    int i;
    
    larb_clock_on(larb, "MAU");
        
    larb_base = gLarbBaseAddr[larb];

    //dump interrupt debug infomation
    for(i=0; i<MAU_ENTRY_NR; i++)
    {
        regval = M4U_ReadReg32(larb_base, SMI_MAU_ENTR_GID(i));
        if(regval!=0)
        {
            SMIMSG("larb(%d), entry(%d)=========>\n", larb, i);
            SMIMSG("port mask = 0x%x\n", regval);
            regval = M4U_ReadReg32(larb_base, SMI_MAU_ENTR_START(i));
            SMIMSG("start_addr=0x%x, read_en=%d, write_en=%d\n", 
                F_MAU_START_ADDR_VAL(regval), 
                F_MAU_START_IS_RD(regval), F_MAU_START_IS_WR(regval));
            regval = M4U_ReadReg32(larb_base, SMI_MAU_ENTR_END(i));
            SMIMSG("end_addr=0x%x, virtual=%d\n", F_MAU_END_ADDR_VAL(regval), 
                F_MAU_END_IS_VIR(regval));
        }
        else
        {
            SMIMSG("larb(%d), entry(%d) is free\n", larb, i);
        }


        //dump interrupt debug infomation
        regval = M4U_ReadReg32(larb_base, SMI_MAU_ENTR_STAT(i));
        if(F_MAU_STAT_ASSERT(regval))
        {
            //violation happens in this entry
            int port =  F_MAU_STAT_ID(regval);
            SMIMSG("[MAU] larb=%d, entry=%d, port=%d\n",larb,i,port);
            SMIMSG("violation by %s\n",smi_port_name[port]);
        }
        else
        {
            SMIMSG("no violation of entry %d\n", i);
        }
#if 0
        //clear interrupt status
        regval = M4U_ReadReg32(larb_base, SMI_MAU_ENTR_STAT(i));
        M4U_WriteReg32(larb_base, SMI_MAU_ENTR_STAT(i), regval);
#endif        
    }


    larb_clock_off(larb, "MAU");

}

/*****************************************************************************
 * FUNCTION
 *    mau_init
 * DESCRIPTION
 *    1. Setup IRQ
 *    2. Call mau_enable_interrupt to enable MAU interrupt.
 * PARAMETERS
 *    None.
 * RETURNS
 *    Type: Integer.  zero mean success and others mean fail.
 ****************************************************************************/
int mau_init(void)
{
    int i;

    if(request_irq(MT_SMI_LARB0_IRQ_ID , (irq_handler_t)mau_isr, IRQF_TRIGGER_LOW, "MAU0" , NULL))
    {
        SMIERR("request MAU0 IRQ line failed");
        return -ENODEV;
    }

    for(i=0; i<SMI_LARB_NR; i++)
    {
        larb_clock_on(i, "MAU");
        mau_enable_interrupt(i);
        larb_clock_off(i, "MAU");
    }

    return 0;
}





