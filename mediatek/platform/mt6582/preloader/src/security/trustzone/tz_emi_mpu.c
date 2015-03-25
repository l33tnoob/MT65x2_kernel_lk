#include "trustzone.h"
#include "typedefs.h"
#include "tz_emi_reg.h"
#include "tz_emi_mpu.h"
#include "tz_utils.h"

/*
 * emi_mpu_set_region_protection: protect a region.
 * @start: start address of the region
 * @end: end address of the region
 * @region: EMI MPU region id
 * @access_permission: EMI MPU access permission
 * Return 0 for success, otherwise negative status code.
 */
int emi_mpu_set_region_protection(unsigned int start, unsigned int end, int region, unsigned int access_permission)
{
    int ret = 0;
    unsigned int tmp;
        
    if((end != 0) || (start !=0)) 
    {
        /*Address 64KB alignment*/
        start -= DRAM_PHY_ADDR;
        end -= DRAM_PHY_ADDR;
        start = start >> 16;
        end = end >> 16;

        if (end <= start) 
        {
            return -1;
        }
    }
	
    switch (region) 
    {
    case 0:
        writel((start << 16) | end, EMI_MPUA); 
        tmp = readl(EMI_MPUI) & 0xFFFF0000;
        writel(tmp | access_permission, EMI_MPUI);
        break; 

    case 1:
        writel((start << 16) | end, EMI_MPUB);
        tmp = readl(EMI_MPUI) & 0x0000FFFF;
        writel(tmp | (access_permission << 16), EMI_MPUI);
        break;

    case 2:
        writel((start << 16) | end, EMI_MPUC);
        tmp = readl(EMI_MPUJ) & 0xFFFF0000;
        writel(tmp | access_permission, EMI_MPUJ);
        break;

    case 3:
        writel((start << 16) | end, EMI_MPUD);
        tmp = readl(EMI_MPUJ) & 0x0000FFFF;
        writel(tmp | (access_permission << 16), EMI_MPUJ);
        break;        

    case 4:
        writel((start << 16) | end, EMI_MPUE);
        tmp = readl(EMI_MPUK) & 0xFFFF0000;
        writel(tmp | access_permission, EMI_MPUK);
        break;  

    case 5:
        writel((start << 16) | end, EMI_MPUF);
        tmp = readl(EMI_MPUK) & 0x0000FFFF;
        writel(tmp | (access_permission << 16), EMI_MPUK);
        break;

    case 6:
        writel((start << 16) | end, EMI_MPUG);
        tmp = readl(EMI_MPUL) & 0xFFFF0000;
        writel(tmp | access_permission, EMI_MPUL);
        break;

    case 7:
        writel((start << 16) | end, EMI_MPUH);
        tmp = readl(EMI_MPUL) & 0x0000FFFF;
        writel(tmp | (access_permission << 16), EMI_MPUL);
        break;

    default:
        ret = -1;
        break;
    }

    return ret;
}

void tz_emi_mpu_init(u32 start_add, u32 end_addr)
{
    int ret = 0;
    unsigned int sec_mem_mpu_attr;
    unsigned int sec_mem_phy_start, sec_mem_phy_end;

    /* Caculate start/end address */
    sec_mem_phy_start = start_add;
    sec_mem_phy_end = end_addr;

    // For MT6589
	//===================================================================
	//            | Region |  D0(AP)  |  D1(MD0)  |  D2(MD1)  |  D3(MM)
	//------------+------------------------------------------------------
	// Secure OS  |    0   |RW(S)     |Forbidden  |Forbidden  |Forbidden
	//------------+------------------------------------------------------
	// MD0 ROM    |    1   |RO(S/NS)  |RO(S/NS)   |Forbidden  |Forbidden
	//------------+------------------------------------------------------
	// MD0 R/W+   |    2   |Forbidden |No protect |Forbidden  |Forbidden
	//------------+------------------------------------------------------
	// MD1 ROM    |    3   |RO(S/NS)  |Forbidden  |RO(S/NS)   |Forbidden
	//------------+------------------------------------------------------
	// MD1 R/W+   |    4   |Forbidden |Forbidden  |No protect |Forbidden
	//------------+------------------------------------------------------
	// MD0 Share  |    5   |No protect|No protect |Forbidden  |Forbidden
	//------------+------------------------------------------------------
	// MD1 Share  |    6   |No protect|Forbidden  |No protect |Forbidden
	//------------+------------------------------------------------------
	// AP         |    7   |No protect|Forbidden  |Forbidden  |No protect
	//===================================================================

    sec_mem_mpu_attr = SET_ACCESS_PERMISSON(SEC_RW, FORBIDDEN, FORBIDDEN, SEC_RW);

    print("%s tz_emi_mpu_init start (0x%x)!\n", MOD, sec_mem_phy_start);
    print("%s tz_emi_mpu_init end (0x%x)!\n", MOD, sec_mem_phy_end);

    ret = emi_mpu_set_region_protection(sec_mem_phy_start,	  /*START_ADDR*/									
                                        sec_mem_phy_end,      /*END_ADDR*/                                  
                                        sec_mem_mpu_id,       /*region*/                                   
                                        sec_mem_mpu_attr);

    if(ret)
    {
        print("%s tz_emi_mpu_init error!!\n", MOD, sec_mem_phy_end);
    }    
}
