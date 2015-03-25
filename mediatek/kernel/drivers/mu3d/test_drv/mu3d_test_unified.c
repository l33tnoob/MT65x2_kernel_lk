
#include <linux/mu3d/hal/mu3d_hal_osal.h>
#include <linux/mu3d/test_drv/mu3d_test_usb_drv.h>
#include <linux/mu3d/hal/mu3d_hal_qmu_drv.h>
#include <linux/mu3d/hal/mu3d_hal_hw.h>
#include <linux/mu3d/hal/mu3d_hal_usb_drv.h>
#define _MTK_PHY_EXT_
#include <linux/mu3d/hal/mu3d_hal_phy.h>
#undef _MTK_PHY_EXT_
#include <linux/mu3d/test_drv/mu3d_test_qmu_drv.h>
#define _USB_UNIFIED_H_
#include <linux/mu3d/test_drv/mu3d_test_unified.h>
#undef _USB_UNIFIED_H_
#include <linux/mu3d/hal/ssusb_sifslv_ippc_c_header.h>
#include <linux/kthread.h>


int main_thread(void *data);
int otg_hnp_req_role_b_thread(void *data);
int otg_hnp_back_role_b_thread(void *data);
int otg_hnp_req_role_a_thread(void *data);
int otg_hnp_back_role_a_thread(void *data);
int otg_srp_thread(void *data);
DEV_INT32 otg_top(int argc, char** argv);

int debug_level = 8;

/**
 * u3w - i2c write function
 *@args - arg1: addr, arg2: value
 */
DEV_INT32 u3w(DEV_INT32 argc, DEV_INT8**argv){
	#ifdef CONFIG_U3_PHY_GPIO_SUPPORT
	DEV_UINT32 u4TimingValue;
	DEV_UINT8 u1TimingValue;
	DEV_UINT32 u4TimingAddress;

	if (argc<3)
    {
        os_printk(K_EMERG,"Arg: address value\n");
        return RET_FAIL;
    }

	u4TimingAddress = (DEV_UINT32)simple_strtol(argv[1], &argv[1], 16);
	u4TimingValue = (DEV_UINT32)simple_strtol(argv[2], &argv[2], 16);
	u1TimingValue = u4TimingValue & 0xff;

	_U3Write_Reg(u4TimingAddress,u1TimingValue);
	#endif

	return RET_SUCCESS;
}

/**
 * u3r - i2c read function
 *@args - arg1: addr
 */
DEV_INT32 u3r(DEV_INT32 argc, DEV_INT8**argv){
	#ifdef CONFIG_U3_PHY_GPIO_SUPPORT
	DEV_UINT8 u1ReadTimingValue;
	DEV_UINT32 u4TimingAddress;

	if (argc<2)
    {
        os_printk(K_EMERG, "Arg: address\n");
        return 0;
    }
	u4TimingAddress = (DEV_UINT32)simple_strtol(argv[1], &argv[1], 16);
	u1ReadTimingValue = _U3Read_Reg(u4TimingAddress);
	printk("Value = 0x%x\n", u1ReadTimingValue);
	#endif

	return 0;
}

/**
 * u3init - u3 phy pipe initial settings
 *
 */
DEV_INT32 u3init(DEV_INT32 argc, DEV_INT8**argv){
	if (!u3phy_ops)
		u3phy_init();

	u3phy_ops->init(u3phy);

	return 0;
}

/**
 * u3d_linkup - u3d link up
 *
 */
DEV_INT32 u3d_linkup(DEV_INT32 argc, DEV_INT8 **argv){
	DEV_INT32 latch,count,status;

	latch = 0;
	if(argc > 1){
		latch = (DEV_UINT32)simple_strtol(argv[1], &argv[1], 16);
	}

	mu3d_hal_link_up(latch);
	os_ms_delay(500);
	count = 10;
	status = RET_SUCCESS;
	do{
		if((os_readl(U3D_LINK_STATE_MACHINE)&LTSSM)!=STATE_U0_STATE){
			status = RET_FAIL;
			break;
		}
		os_ms_delay(50);
	}while(count--);

	if(status != RET_SUCCESS){
		printk("&&&&&& LINK UP FAIL !!&&&&&&\n");
	}else{
		printk("&&&&&& LINK UP PASS !!&&&&&&\n");
	}
	return 0;
}

/**
 * U3D_Phy_Cfg_Cmd - u3 phy clock phase scan
 *
 */
DEV_INT32 U3D_Phy_Cfg_Cmd(DEV_INT32 argc, DEV_INT8 **argv){

	DEV_INT32 latch_val;

	latch_val = 0;
	if(argc > 1){
		latch_val = (DEV_UINT32)simple_strtol(argv[1], &argv[1], 16);
	}

	mu3d_hal_phy_scan(latch_val, 2);

	return 0;
}


DEV_INT32 dbg_phy_eyeinit(int argc, char** argv){
	u3phy_ops->eyescan_init(u3phy);

	return RET_SUCCESS;
}

DEV_INT32 dbg_phy_eyescan(int argc, char** argv){
	if (argc > 10)
	{
		u3phy_ops->eyescan(
			u3phy,
			simple_strtol(argv[1], &argv[1], 10), //x_t1
			simple_strtol(argv[2], &argv[2], 10), //y_t1
			simple_strtol(argv[3], &argv[3], 10), //x_br
			simple_strtol(argv[4], &argv[4], 10), //y_br
			simple_strtol(argv[5], &argv[5], 10), //delta_x
			simple_strtol(argv[6], &argv[6], 10), //delta_y
			simple_strtol(argv[7], &argv[7], 10), //eye_cnt
			simple_strtol(argv[8], &argv[8], 10), //num_cnt
			simple_strtol(argv[9], &argv[9], 10), //PI_cal_en
			simple_strtol(argv[10], &argv[10], 10) //num_ignore_cnt
			);

		return RET_SUCCESS;
	}
	else
	{
		return RET_FAIL;
	}
};


#define EP0_SRAM_SIZE 512
#define EPN_TX_SRAM_SIZE 6144
#define EPN_RX_SRAM_SIZE 6144

void sram_write(DEV_UINT32 mode, DEV_UINT32 addr, DEV_UINT32 data)
{
	DEV_UINT8 index = 0;
	#ifdef SUPPORT_U3
	DEV_UINT32 port[] = {EP0_SRAM_DEBUG_MODE, EPNTX_SRAM_DEBUG_MODE, EPNRX_SRAM_DEBUG_MODE};
	#else
	DEV_UINT32 port[] = {EP0_SRAM_DEBUG_MODE, EP0_SRAM_DEBUG_MODE, EP0_SRAM_DEBUG_MODE};
	#endif
	DEV_UINT32 fifo[] = {U3D_FIFO0, U3D_FIFO1, U3D_FIFO1};

	//set debug mode
	os_writel(U3D_SRAM_DBG_CTRL, port[index]);

	if (mode == EP0_SRAM_DEBUG_MODE)
		index = 0;
	else if (mode == EPNTX_SRAM_DEBUG_MODE)
		index = 1;
	else if (mode == EPNRX_SRAM_DEBUG_MODE)		index = 2;
	else
		BUG_ON(1);

	//set increment to 1024
	os_writelmsk(U3D_SRAM_DBG_CTRL_1, 10<<SRAM_DEBUG_FIFOSEGSIZE_OFST, SRAM_DEBUG_FIFOSEGSIZE);
	os_writelmsk(U3D_SRAM_DBG_CTRL_1, (addr/1024)<<SRAM_DEBUG_SLOT_OFST, SRAM_DEBUG_SLOT);
	os_writelmsk(U3D_SRAM_DBG_CTRL_1, (addr%1024)<<SRAM_DEBUG_DP_COUNT_OFST, SRAM_DEBUG_DP_COUNT);

	os_writel(fifo[index], data);

	//disable debug mode
	os_writel(U3D_SRAM_DBG_CTRL, 0);
}

DEV_UINT32 sram_read(DEV_UINT32 mode, DEV_UINT32 addr)
{
	DEV_UINT8 index;
	DEV_UINT32 temp;
	#ifdef SUPPORT_U3
	DEV_UINT32 port[] = {EP0_SRAM_DEBUG_MODE, EPNTX_SRAM_DEBUG_MODE, EPNRX_SRAM_DEBUG_MODE};
	#else
	DEV_UINT32 port[] = {EP0_SRAM_DEBUG_MODE, EP0_SRAM_DEBUG_MODE, EP0_SRAM_DEBUG_MODE};
	#endif
	DEV_UINT32 fifo[] = {U3D_FIFO0, U3D_FIFO1, U3D_FIFO1};

	if (mode == EP0_SRAM_DEBUG_MODE)
		index = 0;
	else if (mode == EPNTX_SRAM_DEBUG_MODE)
		index = 1;
	else if (mode == EPNRX_SRAM_DEBUG_MODE)
		index = 2;
	else
		BUG_ON(1);

	//set debug mode
	os_writel(U3D_SRAM_DBG_CTRL, port[index]);

	//set increment to 1024
	os_writelmsk(U3D_SRAM_DBG_CTRL_1, 10<<SRAM_DEBUG_FIFOSEGSIZE_OFST, SRAM_DEBUG_FIFOSEGSIZE);
	os_writelmsk(U3D_SRAM_DBG_CTRL_1, (addr/1024)<<SRAM_DEBUG_SLOT_OFST, SRAM_DEBUG_SLOT);
	os_writelmsk(U3D_SRAM_DBG_CTRL_1, (addr%1024)<<SRAM_DEBUG_DP_COUNT_OFST, SRAM_DEBUG_DP_COUNT);
	os_printk(K_ERR, "U3D_SRAM_DBG_CTRL: %x\n", os_readl(U3D_SRAM_DBG_CTRL));
	os_printk(K_ERR, "U3D_SRAM_DBG_CTRL_1: %x\n", os_readl(U3D_SRAM_DBG_CTRL_1));

	temp = os_readl(fifo[index]);

	//disable debug mode
	os_writel(U3D_SRAM_DBG_CTRL, 0);

	return temp;
}

void sram_dbg(void)
{
	DEV_UINT8 *ptr;
	DEV_UINT16 i, j, addr, data;
	DEV_UINT32 value[] = {0xaaaaaaaa, 0x55555555, 0x5a5a5a5a, 0xa5a5a5a5, 0x0, 0xffffffff, 0x12345678, 0x87654321};
	DEV_UINT32 size[] = {EP0_SRAM_SIZE, EPN_TX_SRAM_SIZE, EPN_RX_SRAM_SIZE};
	DEV_UINT32 en[] = {EP0_SRAM_DEBUG_MODE, EPNTX_SRAM_DEBUG_MODE, EPNRX_SRAM_DEBUG_MODE};
	DEV_UINT32 fifo[] = {U3D_FIFO0, U3D_FIFO1, U3D_FIFO1};


	//set increment to 1024
	os_writelmsk(U3D_SRAM_DBG_CTRL_1, 10<<SRAM_DEBUG_FIFOSEGSIZE_OFST, SRAM_DEBUG_FIFOSEGSIZE);

	for (j = 0; j < sizeof(en)/sizeof(en[0]); j++)
	{
		os_writel(U3D_SRAM_DBG_CTRL, en[j]);
		os_printk(K_ERR, "test mode %d\n", j);

		for (addr = 0; addr < size[j]; addr = addr + 4)
		//addr = 0x100;
		{
			//addr = dp_count + slot * 2^10
			os_writel(U3D_RISC_SIZE, RISC_SIZE_4B);
			os_writelmsk(U3D_SRAM_DBG_CTRL_1, (addr/1024)<<SRAM_DEBUG_SLOT_OFST, SRAM_DEBUG_SLOT);
			os_writelmsk(U3D_SRAM_DBG_CTRL_1, (addr%1024)<<SRAM_DEBUG_DP_COUNT_OFST, SRAM_DEBUG_DP_COUNT);

			for (i = 0; i < sizeof(value)/sizeof(value[0]); i++)
			{
				//test 4-byte access
				os_writel(U3D_RISC_SIZE, RISC_SIZE_4B);
				os_writel(fifo[j], value[i]);
				if (os_readl(fifo[j]) != value[i])
				{
					os_printk(K_ERR, "[4-byte access]Write addr %x with value %x fail, got %x\n", addr, value[i], os_readl(fifo[j]));
				}


				//test 2-byte write access
				ptr = (DEV_UINT8 *)&value[i];
				//WORD only access for the following code
				os_writel(U3D_RISC_SIZE, RISC_SIZE_2B);
				os_writel(fifo[j], (*ptr | *(ptr+1)<<8));
				os_writelmsk(U3D_SRAM_DBG_CTRL_1, ((addr+2)%1024)<<SRAM_DEBUG_DP_COUNT_OFST, SRAM_DEBUG_DP_COUNT);
				os_writel(fifo[j], (*(ptr+2) | *(ptr+3)<<8));
				//restore to DWORD access
				os_writel(U3D_RISC_SIZE, RISC_SIZE_4B);
				os_writelmsk(U3D_SRAM_DBG_CTRL_1, (addr%1024)<<SRAM_DEBUG_DP_COUNT_OFST, SRAM_DEBUG_DP_COUNT);
				if (value[i] != os_readl(fifo[j]))
				{
					os_printk(K_ERR, "[2-byte access]Write addr %x with value %x fail, got %x\n", addr, value[i], os_readl(fifo[j]));
				}

				//test 1-byte access
				ptr = (DEV_UINT8 *)&value[i];
				//BYTE only access for the following code
				os_writel(U3D_RISC_SIZE, RISC_SIZE_1B);
				os_writel(fifo[j], *ptr);
				os_writelmsk(U3D_SRAM_DBG_CTRL_1, ((addr+1)%1024)<<SRAM_DEBUG_DP_COUNT_OFST, SRAM_DEBUG_DP_COUNT);
				os_writel(fifo[j], *(ptr+1));
				os_writelmsk(U3D_SRAM_DBG_CTRL_1, ((addr+2)%1024)<<SRAM_DEBUG_DP_COUNT_OFST, SRAM_DEBUG_DP_COUNT);
				os_writel(fifo[j], *(ptr+2));
				os_writelmsk(U3D_SRAM_DBG_CTRL_1, ((addr+3)%1024)<<SRAM_DEBUG_DP_COUNT_OFST, SRAM_DEBUG_DP_COUNT);
				os_writel(fifo[j], *(ptr+3));
				//restore to DWORD access
				os_writel(U3D_RISC_SIZE, RISC_SIZE_4B);
				os_writelmsk(U3D_SRAM_DBG_CTRL_1, (addr%1024)<<SRAM_DEBUG_DP_COUNT_OFST, SRAM_DEBUG_DP_COUNT);
				if (value[i] != os_readl(fifo[j]))
				{
					os_printk(K_ERR, "[1-byte access]Write addr %x with value %x fail, got %x\n", addr, value[i], os_readl(fifo[j]));
					while(1);
				}
			}
		}


		//write serial to every 4-byte, and then read back for comparison
		os_writel(U3D_RISC_SIZE, RISC_SIZE_4B);
		for (addr = 0; addr < size[j]; addr = addr + 4)
		{
			data = addr+0x100;

			//addr = dp_count + slot * 2^10
			os_writelmsk(U3D_SRAM_DBG_CTRL_1, (addr/1024)<<SRAM_DEBUG_SLOT_OFST, SRAM_DEBUG_SLOT);
			os_writelmsk(U3D_SRAM_DBG_CTRL_1, (addr%1024)<<SRAM_DEBUG_DP_COUNT_OFST, SRAM_DEBUG_DP_COUNT);

			os_writel(fifo[j], data);
		}

		for (addr = 0; addr < size[j]; addr = addr + 4)
		{
			data = addr+0x100;

			//addr = dp_count + slot * 2^10
			os_writelmsk(U3D_SRAM_DBG_CTRL_1, (addr/1024)<<SRAM_DEBUG_SLOT_OFST, SRAM_DEBUG_SLOT);
			os_writelmsk(U3D_SRAM_DBG_CTRL_1, (addr%1024)<<SRAM_DEBUG_DP_COUNT_OFST, SRAM_DEBUG_DP_COUNT);

			if (os_readl(fifo[j]) != data)
			{
				os_printk(K_ERR, "addr %x data has been modified, should be %x, got %x\n", addr, data, os_readl(fifo[j]));
			}
		}
	}
}

DEV_UINT8 g_first_init = 1;
#ifdef SUPPORT_OTG
DEV_INT32 TS_AUTO_TEST(DEV_INT32 argc, DEV_INT8** argv){
	if (g_run)
		os_printk(K_ERR, "main_thread is already running\n");

	kthread_run(main_thread, NULL, "main_thread\n");

	return RET_SUCCESS;
}

int main_thread(void *data){
#else
DEV_INT32 TS_AUTO_TEST(DEV_INT32 argc, DEV_INT8** argv){
#endif
	EP_INFO *ep_info;
	REST_INFO *reset;
	LOOPBACK_INFO *lb_info;
	STRESS_INFO *st_info;
	RANDOM_STOP_INFO *rs_info;
	STOP_QMU_INFO *sq_info;
	RX_ZLP_INFO *cz_info;
	DEV_NOTIF_INFO *notif_info;
	SINGLE_INFO *sg_info;
	POWER_INFO *pw_info;
	U1U2_INFO *u1u2_info;
	LPM_INFO *lpm_info;
	STALL_INFO *stall_info;
	REMOTE_WAKE_INFO *remote_wake_info;
	CTRL_MODE_INFO *ctrl_mode_info;
	OTG_MODE_INFO *otg_mode_info;
	unsigned long flags;
    USB_SPEED speed;
	DEV_UINT32 status = RET_SUCCESS,i,stop_count_1,stop_count_2;
	DEV_UINT8 EP_TX[16],EP_RX[16],st_num,delay,run_stop,run_test,ran_num,method;
	static DEV_INT32 tx_num,rx_num;
	DEV_UINT8 dir_1,dir_2,ep_num,ep_num1,ep_num2,dir,ep_n[15];


	g_run = 1;

	//dump HW/SW version
	os_printk(K_EMERG, "HW VERSION: %x\n", os_readl(U3D_SSUSB_HW_ID));
	os_printk(K_EMERG, "HW SUB VERSION: %x\n", os_readl(U3D_SSUSB_HW_SUB_ID));
	os_printk(K_EMERG, "SW VERSION: %s\n", SW_VERSION);

	//dump HW capability
	os_printk(K_EMERG, "TX EP: %x, RX EP: %x\n",
		os_readl(U3D_CAP_EPINFO) & CAP_TX_EP_NUM,
		(os_readl(U3D_CAP_EPINFO) & CAP_RX_EP_NUM) >> CAP_RX_EP_NUM_OFST);
	os_printk(K_EMERG, "EP0 FIFO: 0x%x\n", os_readl(U3D_CAP_EP0FFSZ));
	os_printk(K_EMERG, "TX FIFO: 0x%x\n", os_readl(U3D_CAP_EPNTXFFSZ));
	os_printk(K_EMERG, "RX FIFO: 0x%x\n", os_readl(U3D_CAP_EPNRXFFSZ));


	if (g_first_init)
	{
		//apply probe setting to detect register R/W hang problem
		//os_writel(U3D_SSUSB_PRB_CTRL1, 0x00160015);
		//os_writel(U3D_SSUSB_PRB_CTRL2, 0x00000000);
		//os_writel(U3D_SSUSB_PRB_CTRL3, 0x00000404);

		//IDDIG function does not work on FPGA. Need to set B role manually
		#ifdef SUPPORT_OTG
		os_setmsk(U3D_SSUSB_OTG_STS, SSUSB_CHG_B_ROLE_B);
		#endif


		g_dma_buffer_size=STRESS_DATA_LENGTH*MAX_GPD_NUM;

		g_loopback_buffer[0] = (DEV_UINT8 *)os_mem_alloc(g_dma_buffer_size);
		g_loopback_buffer[1] = (DEV_UINT8 *)os_mem_alloc(g_dma_buffer_size);
		g_loopback_buffer[2] = (DEV_UINT8 *)os_mem_alloc(g_dma_buffer_size);
		g_loopback_buffer[3] = (DEV_UINT8 *)os_mem_alloc(g_dma_buffer_size);

		os_printk(K_CRIT,"STRESS_GPD_TH : %d\n",STRESS_GPD_TH);
		os_printk(K_CRIT,"g_dma_buffer_size : %d\n",g_dma_buffer_size);
		os_printk(K_CRIT,"g_loopback_buffer[0] : %x\n",(DEV_UINT32)g_loopback_buffer[0]);
		os_printk(K_CRIT,"g_loopback_buffer[1] : %x\n",(DEV_UINT32)g_loopback_buffer[1]);
		os_printk(K_CRIT,"g_loopback_buffer[2] : %x\n",(DEV_UINT32)g_loopback_buffer[2]);
		os_printk(K_CRIT,"g_loopback_buffer[3] : %x\n",(DEV_UINT32)g_loopback_buffer[3]);

		ep_info=(EP_INFO *)os_mem_alloc(sizeof(EP_INFO));

		#if (EP0_BUS_MODE==DMA_MODE)
		g_ep0_mode = DMA_MODE;
		#else
		g_ep0_mode = PIO_MODE;
		#endif

		/* initialize PHY related data structure */
		u3phy_init();

		u3phy_ops->init(u3phy);

		/* USB 2.0 slew rate calibration */
		u3phy_ops->u2_slew_rate_calibration(u3phy);

		/* initialize U3D BMU & QMU module. */
		u3d_init();

		g_first_init = 0;
	}


	os_memset(EP_RX, 0 , 16);
	os_memset(EP_TX, 0 , 16);
	rx_num=0;
	tx_num=0;


	u3d_rst_request();
	while(g_run){
		if(u3d_req_valid()){
			os_printk(K_INFO, "[START STATE %d]\n", u3d_command());
			switch(u3d_command()){

				case RESET_STATE:
					//TODO: make sure all DMA actions are stopped
					os_printk(K_INFO,"RESET_STATE\n");
					#if (BUS_MODE==QMU_MODE)
					for(i=0;i<MAX_EP_NUM;i++){
						if(EP_RX[i]!=0){
							mu3d_hal_flush_qmu(EP_RX[i],USB_RX);
						}
						if(EP_TX[i]!=0){
							mu3d_hal_flush_qmu(EP_TX[i],USB_TX);
						}
					}
					#endif
					reset=(REST_INFO *)u3d_req_buffer();
					os_printk(K_DEBUG,"reset->speed : 0x%08X\n",reset->speed);

					//device speed is specified after reset
					reset_dev(reset->speed & 0xf, 1,
						(reset->speed & 0x10) ? 0 : 1);
					tx_num=0;
					rx_num=0;
					speed = reset->speed;
					break;


				case CONFIG_EP_STATE:
					os_printk(K_INFO,"CONFIG_EP_STATE\n");

					ep_info=(EP_INFO *)u3d_req_buffer();
					os_printk(K_ERR,"ep_info->ep_num :%x\r\n",ep_info->ep_num);
					os_printk(K_ERR,"ep_info->dir :%x\r\n",ep_info->dir);
					os_printk(K_ERR,"ep_info->type :%x\r\n",ep_info->type);
					os_printk(K_ERR,"ep_info->ep_size :%d\r\n",ep_info->ep_size);
					os_printk(K_ERR,"ep_info->interval :%x\r\n",ep_info->interval);
					os_printk(K_ERR,"ep_info->slot :%d\r\n",ep_info->slot);
					os_printk(K_ERR,"ep_info->burst :%d\r\n",ep_info->burst);
					os_printk(K_ERR,"ep_info->mult :%d\r\n",ep_info->mult);

					mu3d_hal_ep_enable(ep_info->ep_num, ep_info->dir, ep_info->type, ep_info->ep_size, ep_info->interval,ep_info->slot,ep_info->burst,ep_info->mult);

					#if (BUS_MODE==QMU_MODE)
					mu3d_hal_start_qmu(ep_info->ep_num, ep_info->dir);
					#endif
					if(ep_info->dir==USB_RX){
						EP_RX[rx_num]=ep_info->ep_num;
						rx_num++;
					}else{
						EP_TX[tx_num]=ep_info->ep_num;
						tx_num++;
					}
					if(speed == SSUSB_SPEED_SUPER){
						dev_power_mode(0, 0, 0,1, 1);
					}
					os_printk(K_ERR,"EP_RX :%x\r\n",EP_RX[0]);
					os_printk(K_ERR,"EP_TX :%x\r\n",EP_TX[0]);
					break;


				case LOOPBACK_STATE:
					os_printk(K_INFO,"LOOPBACK_STATE\n");
					lb_info=(LOOPBACK_INFO *)u3d_req_buffer();
					TransferLength=lb_info->transfer_length;
					is_bdp=lb_info->bdp;
					gpd_buf_size=lb_info->gpd_buf_size;
					bd_buf_size=lb_info->bd_buf_size;
					bDramOffset=lb_info->dram_offset;
					bBD_Extension=lb_info->extension;
					bGPD_Extension=lb_info->extension;
					bdma_burst=lb_info->dma_burst;
					bdma_limiter=lb_info->dma_limiter;

					#if (BUS_MODE==QMU_MODE)
					os_printk(K_INFO,"TransferLength :%d\r\n",TransferLength);
					os_printk(K_INFO,"gpd_buf_size :%x\r\n",gpd_buf_size);
					os_printk(K_INFO,"bd_buf_size :%x\r\n",bd_buf_size);
					os_printk(K_INFO,"is_bdp :%x\r\n",is_bdp);
					os_printk(K_INFO,"bBD_Extension :%x\r\n",bBD_Extension);
					os_printk(K_INFO,"bGPD_Extension :%x\r\n",bGPD_Extension);
					os_printk(K_INFO,"bDramOffset :%x\r\n",bDramOffset);
					//dev_set_dma_busrt_limiter(bdma_burst,bdma_limiter);
					dev_qmu_loopback(EP_RX[0],EP_TX[0]);
					#else
					os_printk(K_INFO,"TransferLength :%d\r\n",TransferLength);
					u3d_dev_loopback(EP_RX[0],EP_TX[0]);
					#endif
					break;


				case LOOPBACK_EXT_STATE:
					os_printk(K_INFO,"LOOPBACK_EXT_STATE\n");

					lb_info=(LOOPBACK_INFO *)u3d_req_buffer();
					TransferLength=lb_info->transfer_length;
					is_bdp=lb_info->bdp;
					gpd_buf_size=lb_info->gpd_buf_size;
					bd_buf_size=lb_info->bd_buf_size;
					bDramOffset=lb_info->dram_offset;
					bBD_Extension=lb_info->extension;
					bGPD_Extension=lb_info->extension;

					os_printk(K_INFO,"TransferLength :%d\r\n",TransferLength);
					os_printk(K_INFO,"gpd_buf_size :%x\r\n",gpd_buf_size);
					os_printk(K_INFO,"bd_buf_size :%x\r\n",bd_buf_size);
					os_printk(K_INFO,"is_bdp :%x\r\n",is_bdp);
					os_printk(K_INFO,"bBD_Extension :%x\r\n",bBD_Extension);
					os_printk(K_INFO,"bGPD_Extension :%x\r\n",bGPD_Extension);
					os_printk(K_INFO,"bDramOffset :%x\r\n",bDramOffset);

					dev_qmu_loopback_ext(EP_RX[0],EP_TX[0]);
					break;


				case REMOTE_WAKEUP:
					remote_wake_info = (REMOTE_WAKE_INFO *)u3d_req_buffer();
					os_printk(K_INFO, "REMOTE_WAKEUP\n");
					printk("remote wakeup suspend=%d\n",g_usb_status.suspend);
					while(!u3d_dev_suspend());
					printk("remote wakeup suspend=%d\n",g_usb_status.suspend);
					os_ms_delay(10);
					os_us_delay(remote_wake_info->delay);
					mu3d_hal_resume();
					if(g_u3d_setting.speed == SSUSB_SPEED_SUPER)
						dev_notification(1, 0, 0); //send FUNCTION WAKE UP

					os_printk(K_INFO,"resume+\n");
					while(u3d_dev_suspend());
					os_printk(K_INFO,"resume-\n");
					break;


				case STRESS:
					os_printk(K_CRIT,"STRESS\n");
					st_info=(STRESS_INFO *)u3d_req_buffer();
					TransferLength=st_info->transfer_length;
					is_bdp=st_info->bdp;
					gpd_buf_size=st_info->gpd_buf_size;
					bd_buf_size=st_info->bd_buf_size;
					st_num=st_info->num;

					bDramOffset=0;
					bBD_Extension=0;
					bGPD_Extension=0;


					g_dma_buffer_size=TransferLength*MAX_GPD_NUM;

					for(i=0;i<st_num;i++){
						g_loopback_buffer[i] = (DEV_UINT8 *)os_mem_alloc(g_dma_buffer_size);
					}

					g_run_stress = true;
					g_insert_hwo = true;

					for(i=0;i<st_num;i++){
					 	dev_prepare_stress_gpd(MAX_GPD_NUM,USB_RX,EP_RX[i],g_loopback_buffer[i]);
					 	dev_prepare_stress_gpd(MAX_GPD_NUM,USB_TX,EP_TX[i],g_loopback_buffer[i]);
					}

					for(i=0;i<st_num;i++){
						dev_insert_stress_gpd_hwo(USB_RX, EP_RX[i]);
						dev_start_stress(USB_RX,EP_RX[i]);
					}
					break;


				case WARM_RESET: //fall down on purpose
					os_printk(K_INFO,"WARM_RESET\n");


				case STALL:
					os_printk(K_INFO,"STALL\n");
					stall_info=(STALL_INFO *)u3d_req_buffer();
					TransferLength=stall_info->transfer_length;
					gpd_buf_size=stall_info->gpd_buf_size;
					bd_buf_size=stall_info->bd_buf_size;
					is_bdp=stall_info->bdp;

				 	dev_tx_rx(EP_RX[0],EP_TX[0]);
				 	g_u3d_status = READY;
					break;


				case EP_RESET_STATE:
					os_printk(K_INFO,"EP_RESET_STATE\n");
				 	g_u3d_status = dev_ep_reset();
					break;


				case SINGLE:
					os_printk(K_INFO,"SINGLE\n");
					sg_info=(SINGLE_INFO *)u3d_req_buffer();


					os_printk(K_INFO,"sg_info->transfer_length = %d\n",sg_info->transfer_length);
					os_printk(K_INFO,"sg_info->gpd_buf_size = %d\n",sg_info->gpd_buf_size);
					os_printk(K_INFO,"sg_info->bd_buf_size = %d\n",sg_info->bd_buf_size);
					os_printk(K_INFO,"sg_info->dir = %d\n",sg_info->dir);


					TransferLength=sg_info->transfer_length;
					gpd_buf_size=sg_info->gpd_buf_size;
					bd_buf_size=sg_info->bd_buf_size;
					g_run_stress = true;
					dir=sg_info->dir;
					st_num=sg_info->num;
					if(bd_buf_size){
						is_bdp=1;
					}
					for(i=0;i<st_num;i++){
						if(dir==USB_RX){
							ep_n[i] = EP_RX[i];
						}
						else{
							ep_n[i] = EP_TX[i];
						}
					}
					os_printk(K_INFO,"SINGLE 01\n");

					for(i=0;i<st_num;i++){
						dev_prepare_gpd(STRESS_GPD_TH,dir,ep_n[i],g_loopback_buffer[0]);
					}

					for(i=0;i<st_num;i++){
						dev_start_stress(dir,ep_n[i]);
					}
					break;


				case POWER_STATE:
					pw_info=(POWER_INFO *)u3d_req_buffer();
					if(pw_info->mode == 5){
						dev_send_one_packet(EP_TX[0]);
					}
					dev_power_mode(pw_info->mode, pw_info->u1_value, pw_info->u2_value,pw_info->en_u1, pw_info->en_u2);
					break;


				case U1U2_STATE:
					u1u2_info=(U1U2_INFO *)u3d_req_buffer();
					dev_u1u2_en_cond(u1u2_info->opt,u1u2_info->cond,EP_RX[0] , EP_TX[0]);
					dev_u1u2_en_ctrl(u1u2_info->type,u1u2_info->u_num,u1u2_info->opt,u1u2_info->cond,u1u2_info->u1_value,u1u2_info->u2_value);
					dev_send_erdy(u1u2_info->opt ,EP_RX[0] , EP_TX[0]);
					dev_receive_ep0_test_packet(u1u2_info->opt);
					break;


				case LPM_STATE:
					lpm_info=(LPM_INFO *)u3d_req_buffer();
					#if !LPM_STRESS
					dev_u1u2_en_cond(lpm_info->cond+1, lpm_info->cond_en, EP_RX[0], EP_TX[0]);
					#endif
					dev_lpm_config_dev(lpm_info);

					g_u3d_status = READY;
					break;


				case RANDOM_STOP_STATE:
					os_printk(K_INFO,"RANDOM_STOP_STATE\n");
					rs_info=(RANDOM_STOP_INFO *)u3d_req_buffer();
					TransferLength=rs_info->transfer_length;
					gpd_buf_size=rs_info->gpd_buf_size;
					bd_buf_size=rs_info->bd_buf_size;
					dir_1=rs_info->dir_1;
					dir_2=rs_info->dir_2;
					stop_count_1=rs_info->stop_count_1;
					stop_count_2=rs_info->stop_count_2;
					is_bdp = (!bd_buf_size) ? false : true;
					bDramOffset=0;
					bBD_Extension=0;
					bGPD_Extension=0;

					os_printk(K_CRIT,"TransferLength :%d\r\n",TransferLength);
					os_printk(K_CRIT,"gpd_buf_size :%x\r\n",gpd_buf_size);
					os_printk(K_CRIT,"bd_buf_size :%x\r\n",bd_buf_size);
					os_printk(K_CRIT,"is_bdp :%x\r\n",is_bdp);
					os_printk(K_CRIT,"dir_1 :%x\r\n",dir_1);
					os_printk(K_CRIT,"dir_2 :%x\r\n",dir_2);
					os_printk(K_CRIT,"stop_count_1 :%x\r\n",stop_count_1);
					os_printk(K_CRIT,"stop_count_2 :%x\r\n",stop_count_2);
					os_printk(K_CRIT,"EP_RX :%x  EP_TX :%x\r\n",EP_RX[0],EP_TX[0]);

					g_run_stress = true;
					g_insert_hwo = false;

					if(dir_1==USB_RX){
						ep_num1 = EP_RX[0];
						dev_prepare_gpd(STRESS_GPD_TH,USB_RX,ep_num1,g_loopback_buffer[0]);
					}
					else{
						ep_num1 = EP_TX[0];
						dev_prepare_gpd(STRESS_GPD_TH,USB_TX,ep_num1,g_loopback_buffer[0]);
					}
					if(dir_2==USB_RX){
						ep_num2 = (dir_1==USB_RX) ? EP_RX[1] : EP_RX[0];
						dev_prepare_gpd(STRESS_GPD_TH,USB_RX,ep_num2,g_loopback_buffer[0]);
					}
					else{
						ep_num2 = (dir_1==USB_TX) ? EP_TX[1] : EP_TX[0];
						dev_prepare_gpd(STRESS_GPD_TH,USB_TX,ep_num2,g_loopback_buffer[0]);
					}


					dev_start_stress(dir_1,ep_num1);
					dev_start_stress(dir_2,ep_num2);
					run_test = true;
					while(run_test)
					{

						os_get_random_bytes(&ran_num,1);
						if((ran_num%3)==1){
							if(stop_count_1){
								ep_num = ep_num1;
								stop_count_1--;
								dir = dir_1;
								run_stop = true;
							}
						}
						if((ran_num%3)==2){

							if(stop_count_2){
								ep_num = ep_num2;
								stop_count_2--;
								dir = dir_2;
								run_stop = true;
							}
						}
						if(!(ran_num%3)){
							os_get_random_bytes(&delay,1);
							os_ms_delay(delay);
						}
						if(run_stop){
							g_u3d_status = BUSY;
							os_printk(K_CRIT,"ep_num :%x\r\n",ep_num);
							os_printk(K_CRIT,"dir :%x\r\n",dir);
							os_get_random_bytes(&delay,1);
							os_ms_delay(delay);
							mu3d_hal_send_stall(ep_num,dir);
							mu3d_hal_flush_qmu(ep_num,dir);

							//os_ms_delay(100);
							mu3d_hal_restart_qmu(ep_num,dir);
							dev_prepare_gpd(STRESS_GPD_TH,dir,ep_num,g_loopback_buffer[0]);
							dev_start_stress(dir,ep_num);
							g_u3d_status = READY;
							run_stop = false;
							os_printk(K_CRIT,"stop_count_1 :%x\r\n",stop_count_1);
							os_printk(K_CRIT,"stop_count_2 :%x\r\n",stop_count_2);

							for(i=0;i<RANDOM_STOP_DELAY;i++){
								os_ms_delay(100);
							}
						}
						if((stop_count_1==0)&&(stop_count_2==0)){
							run_test = false;
							os_printk(K_CRIT,"End !!\r\n");
						}

					}
					break;


				case STOP_QMU_STATE:
					os_printk(K_INFO,"STOP_QMU_STATE\n");
					sq_info=(STOP_QMU_INFO *)u3d_req_buffer();
					TransferLength=sq_info->transfer_length;
					gpd_buf_size=sq_info->gpd_buf_size;
					bd_buf_size=sq_info->bd_buf_size;
					method=sq_info->tx_method;

					is_bdp = (!bd_buf_size) ? false : true;
					bDramOffset=0;
					bBD_Extension=0;
					bGPD_Extension=0;
					is_bdp = 1;
					bd_buf_size = 1000;
					os_printk(K_CRIT,"TransferLength :%d\r\n",TransferLength);
					os_printk(K_CRIT,"gpd_buf_size :%x\r\n",gpd_buf_size);
					os_printk(K_CRIT,"bd_buf_size :%x\r\n",bd_buf_size);
					os_printk(K_CRIT,"is_bdp :%x\r\n",is_bdp);
					os_printk(K_CRIT,"dir_1 :%x\r\n",dir_1);
					os_printk(K_CRIT,"dir_2 :%x\r\n",dir_2);
					os_printk(K_CRIT,"EP_RX :%x  EP_TX :%x\r\n",EP_RX[0],EP_TX[0]);

					os_printk(K_CRIT,"method :%x\r\n",method);

					g_run_stress = true;

					dev_prepare_gpd(STRESS_GPD_TH,USB_RX,EP_RX[0],g_loopback_buffer[0]);
					dev_prepare_gpd(STRESS_GPD_TH,USB_TX,EP_TX[0],g_loopback_buffer[0]);
					dev_prepare_gpd(STRESS_GPD_TH,USB_TX,EP_TX[1],g_loopback_buffer[0]);
					dev_start_stress(USB_RX,EP_RX[0]);
					dev_start_stress(USB_TX,EP_TX[0]);
					dev_start_stress(USB_TX,EP_TX[1]);


					//for(stop_count_1=0;stop_count_1<1000;stop_count_1++){
					while(1){
						/*1.Random delay*/
						os_get_random_bytes(&delay,1);
						os_ms_delay((delay));

						/*2.Stop QMU*/
						mu3d_hal_restart_qmu_no_flush(EP_RX[0], USB_RX,0);

						/*3.Random delay*/
						os_get_random_bytes(&delay,1);
						os_ms_delay((delay));
						spin_lock_irqsave(&_lock, flags);

						/*4.Insert GPD*/
						dev_prepare_gpd(STRESS_GPD_TH,USB_RX,EP_RX[0],g_loopback_buffer[0]);

						/*5.Resume QMU*/
						dev_start_stress(USB_RX,EP_RX[0]);
						spin_unlock_irqrestore(&_lock, flags);
						os_ms_delay(20);

						/*1.Random delay*/
						os_get_random_bytes(&delay,1);
						os_ms_delay(delay);

						/*2.Stop QMU*/
						mu3d_hal_restart_qmu_no_flush(EP_TX[0], USB_TX,method);

						/*3.Random delay*/
						os_get_random_bytes(&delay,1);
						os_ms_delay(delay);

						spin_lock_irqsave(&_lock, flags);

						/*4.Insert GPD*/
						if(method==1){
							dev_prepare_gpd(STRESS_GPD_TH,USB_TX,EP_TX[0],g_loopback_buffer[0]);
						}
						if(method==2){
							//dev_prepare_gpd((STRESS_GPD_TH-1),USB_TX,EP_TX[0],g_loopback_buffer[0]);
							dev_prepare_gpd_short(STRESS_GPD_TH,USB_TX,EP_TX[0],g_loopback_buffer[0]);
							//dev_prepare_gpd(STRESS_GPD_TH,USB_TX,EP_TX[0],g_loopback_buffer[0]);
						}

						/*5.Resume QMU*/
						dev_start_stress(USB_TX,EP_TX[0]);
						spin_unlock_irqrestore(&_lock, flags);
						os_ms_delay(20);
					}
					break;


				case RX_ZLP_STATE:
					os_printk(K_ERR,"RX_ZLP_STATE\n");
					cz_info=(RX_ZLP_INFO *)u3d_req_buffer();
					TransferLength=cz_info->transfer_length;
					gpd_buf_size=cz_info->gpd_buf_size;
					bd_buf_size=cz_info->bd_buf_size;
					is_bdp = (!bd_buf_size) ? false : true;
					bDramOffset=0;
					bBD_Extension=0;
					bGPD_Extension=0;
				 	cfg_rx_zlp_en = cz_info->zlp_en;
 					cfg_rx_coz_en = cz_info->coz_en;
					rx_done_count = 0;
					rx_IOC_count = 0;
					if(!TransferLength){
						TransferLength++;
					}


					dev_qmu_rx(EP_RX[0]);
					os_printk(K_CRIT,"rx_zlp:[%d], rx_coz:[%d], rx_done_count:[%d], rx_IOC_count:[%d]\r\n",cfg_rx_zlp_en,cfg_rx_coz_en,rx_done_count,rx_IOC_count);
					os_printk(K_CRIT,"done\r\n");
					break;


				 case DEV_NOTIFICATION_STATE:
					os_printk(K_ERR,"DEV_NOTIFICATION_STATE\n");
					notif_info=(DEV_NOTIF_INFO *)u3d_req_buffer();
					dev_notification(notif_info->type,notif_info->valuel,notif_info->valueh);
					break;


				 case STOP_DEV_STATE:
					os_printk(K_ERR,"STOP_DEV_STATE\n");

					reset_dev(U3D_DFT_SPEED, 0, 1);

					//wait for the current control xfer to finish
					os_ms_delay(1000);

					return RET_SUCCESS;

				 case CTRL_MODE_STATE:
					os_printk(K_ERR,"CTRL_MODE_STATE\n");
					ctrl_mode_info=(CTRL_MODE_INFO *)u3d_req_buffer();

					//g_ep0_mode = ctrl_mode_info->mode;
					//os_printk(K_ERR,"EP0 mode = %s\n", (ctrl_mode_info->mode==DMA_MODE?"DMA":"PIO"));
					debug_level = ctrl_mode_info->mode==DMA_MODE?4:8;
					os_printk(K_ERR,"Debug Log level= %d\n", debug_level);
				 	break;


				case OTG_MODE_STATE:
					os_printk(K_ERR,"OTG_MODE_STATE\n");
					otg_mode_info=(OTG_MODE_INFO *)u3d_req_buffer();
					dev_otg(otg_mode_info->mode);
					break;
			}
			os_printk(K_INFO, "[END STATE %d]\n", u3d_command());


			if(u3d_command() == RESET_STATE) {
				u3d_rst_request();
				u3d_irq_en();
			} else {
				u3d_rst_request();
			}

			//u3d_rst_request();
		}
	}
    status=RET_SUCCESS;
	return status;
}

#ifdef SUPPORT_OTG
DEV_INT32 TS_AUTO_TEST_STOP(DEV_INT32 argc, DEV_INT8** argv){

	g_run = 0;

	return RET_SUCCESS;
}

DEV_INT32 otg_top(int argc, char** argv)
{
	DEV_UINT8 speed;

	#ifdef SUPPORT_OTG
	if (argc >= 2)
	{
		g_otg_mode = (DEV_UINT32)simple_strtol(argv[1], &argv[1], 16);
		os_printk(K_ERR, "OTG_MODE: %x\n", g_otg_mode);
	}
	if (argc >= 3)
	{
		speed = (DEV_UINT32)simple_strtol(argv[2], &argv[2], 16);
		os_printk(K_ERR, "SPEED: %x\n", speed);
	}

	if (g_otg_mode)
	{
		g_otg_suspend = 0;
		g_otg_reset = 0;
		g_otg_vbus_chg = 0;
		g_otg_chg_a_role_b = 0;
		g_otg_chg_b_role_b = 0;

		if (g_otg_mode == 0)
		{
			os_printk(K_ERR, "1: iddig_b\n");
			os_printk(K_ERR, "2: org_srp_thread_role_b\n");
			os_printk(K_ERR, "3: otg_hnp_req_thread_role_b\n");
			os_printk(K_ERR, "4: otg_hnp_back_thread_role_b\n");
			os_printk(K_ERR, "5: otg_hnp_req_thread_role_a\n");
			os_printk(K_ERR, "6: otg_hnp_back_thread_role_a\n");
			os_printk(K_ERR, "6: otg_srp_thread\n");
		}
		if (g_otg_mode == 1)
		{
			os_setmsk(U3D_SSUSB_OTG_STS, SSUSB_ATTACH_B_ROLE);
		}
		else if (g_otg_mode == 2)
		{
			reset_dev((speed == 0) ? SSUSB_SPEED_FULL : SSUSB_SPEED_HIGH, 0, 1);
		}
		else if (g_otg_mode == 3) //HNP request role B
		{
			kthread_run(otg_hnp_req_role_b_thread, NULL, "otg_hnp_req_thread_role_b\n");
		}
		else if (g_otg_mode == 4) //HNP back role B
		{
			kthread_run(otg_hnp_back_role_b_thread, NULL, "otg_hnp_back_thread_role_b\n");
		}
		else if (g_otg_mode == 5) //HNP request role A
		{
			kthread_run(otg_hnp_req_role_a_thread, NULL, "otg_hnp_req_thread_role_a\n");
		}
		else if (g_otg_mode == 6) //HNP back role A
		{
			kthread_run(otg_hnp_back_role_a_thread, NULL, "otg_hnp_back_thread_role_a\n");
		}
		else if (g_otg_mode == 7) //SRP
		{
			kthread_run(otg_srp_thread, NULL, "otg_srp_thread\n");
		}

		g_otg_mode = 0;
	}
	#endif

    return RET_SUCCESS;
}

int otg_hnp_req_role_b_thread(void *data){
	os_printk(K_ERR, "%s\n", __func__);

	os_printk(K_ERR, "Enable host request\n");
	os_setmsk(U3D_DEVICE_CONTROL, HOSTREQ);

	os_printk(K_ERR, "Wait for suspend\n");
	while (!g_otg_suspend)
	{
		msleep(1);
	};

	os_printk(K_ERR, "Request HNP\n");
	//os_clrmsk(U3D_SSUSB_U2_CTRL_0P, SSUSB_U2_PORT_PDN);

	os_printk(K_ERR, "Wait to become host (CHG_A_ROLE_B)\n");
	while (!g_otg_chg_a_role_b)
	{
		msleep(1);
	}

	os_printk(K_ERR, "Check if host role is reported\n");
	while (!(os_readl(U3D_SSUSB_OTG_STS) & SSUSB_HOST_DEV_MODE))
	{
		msleep(1);
	}

	os_printk(K_ERR, "Set CHG_A_ROLE_A\n");
	os_setmsk(U3D_SSUSB_OTG_STS, SSUSB_CHG_A_ROLE_A);

	os_printk(K_ERR, "Done\n");
}

int otg_hnp_back_role_b_thread(void *data){
	os_printk(K_ERR, "%s\n", __func__);

	os_printk(K_ERR, "Wait to become device (CHG_B_ROLE_B)\n");
	while (!g_otg_chg_b_role_b)
	{
		msleep(1);
	}

	os_printk(K_ERR, "Check if device role is reported\n");
	while ((os_readl(U3D_SSUSB_OTG_STS) & SSUSB_HOST_DEV_MODE))
	{
		msleep(1);
	}

	os_printk(K_ERR, "Waiting for reset\n");
	while (!g_otg_reset)
	{
		msleep(1);
	}

	os_printk(K_ERR, "Done\n");
}

int otg_hnp_req_role_a_thread(void *data){
	os_printk(K_ERR, "%s\n", __func__);

	os_printk(K_ERR, "Wait CHG_B_ROLE_B\n");
	while (!g_otg_chg_b_role_b)
	{
		msleep(1);
	}

	#if 0
	os_printk(K_ERR, "Clear CHG_B_ROLE_B\n");
	os_setmsk(U3D_SSUSB_OTG_STS_CLR, SSUSB_CHG_B_ROLE_B_CLR);
	#endif

	os_printk(K_ERR, "Waiting for reset\n");
	while (!g_otg_reset)
	{
		msleep(1);
	}

	os_printk(K_ERR, "Wait to become device\n");
	while ((os_readl(U3D_SSUSB_OTG_STS) & SSUSB_HOST_DEV_MODE))
	{
		msleep(1);
	}

	os_printk(K_ERR, "Done\n");
}

int otg_hnp_back_role_a_thread(void *data){
	os_printk(K_ERR, "%s\n", __func__);

	os_printk(K_ERR, "Wait to become host (CHG_A_ROLE_B)\n");
	while (!g_otg_chg_a_role_b)
	{
		msleep(1);
	}

	os_printk(K_ERR, "Check if host role is reported\n");
	while (!(os_readl(U3D_SSUSB_OTG_STS) & SSUSB_HOST_DEV_MODE))
	{
		msleep(1);
	}

	os_printk(K_ERR, "Set CHG_A_ROLE_A\n");
	os_setmsk(U3D_SSUSB_OTG_STS, SSUSB_CHG_A_ROLE_A);

	os_printk(K_ERR, "Done\n");
}

int otg_srp_thread(void *data){
	os_printk(K_ERR, "%s\n", __func__);

	os_printk(K_ERR, "Wait for A-valid to drop\n");
	while (!g_otg_vbus_chg)
	{
		msleep(1);
	}
	while (os_readl(U3D_SSUSB_OTG_STS) & SSUSB_AVALID_STS)
	{
		msleep(1);
	}

	os_printk(K_ERR, "Delay 1s\n");
	os_ms_delay(1000);

	os_printk(K_ERR, "Wait for session to drop\n");
	while(os_readl(U3D_DEVICE_CONTROL) & SESSION);

	os_printk(K_ERR, "Set session\n");
	os_setmsk(U3D_DEVICE_CONTROL, SESSION);

	os_printk(K_ERR, "Wait for reset\n");
	while (!g_otg_reset);

	os_printk(K_ERR, "Done\n");
}
#endif
