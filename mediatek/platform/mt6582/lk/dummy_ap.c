// Dummy AP
#include <platform/boot_mode.h>
#include <debug.h>
#include <dev/uart.h>
#include <platform/mtk_key.h>
#include <target/cust_key.h>
#include <platform/mt_gpio.h>

#define MAX_MD_NUM			(1)
#define MAX_IMG_NUM			(8)
#define PART_HEADER_MAGIC	(0x58881688)
#define BOOT_ARGS_ADDR		(0x87F00000)
#define IMG_HEADER_ADDR		(0x87F00000+1024)

typedef enum{
	DUMMY_AP_IMG = 0,
	MD1_IMG,
	MD1_RAM_DISK,
	MD2_IMG,
	MD2_RAM_DISK
}img_idx_t;

typedef struct _map
{
	char		name[32];
	img_idx_t	idx;
}map_t;

//typedef union
//{
//    struct
//    {
//        unsigned int magic;     /* partition magic */
//        unsigned int dsize;     /* partition data size */
//        char name[32];          /* partition name */
//        unsigned int maddr;     /* partition memory address */
//    } info;
//    unsigned char data[512];
//} part_hdr_t;


// Notice for MT6582
// Update LK BOOT_ARGUMENT structure
/*
typedef struct {
    unsigned int  magic_number;
    BOOTMODE      boot_mode;
    unsigned int  e_flag;
    unsigned int  log_port;
    unsigned int  log_baudrate;
    unsigned char log_enable;
    unsigned char part_num; //<<<-------
    unsigned char reserved[2]; //<<<-------
    unsigned int  dram_rank_num;
    unsigned int  dram_rank_size[4];
    unsigned int  boot_reason;
    unsigned int  meta_com_type;
    unsigned int  meta_com_id;
    unsigned int  boot_time;
    da_info_t     da_info;
    SEC_LIMIT     sec_limit;	
    part_hdr_t    *part_info; //<<<------
} BOOT_ARGUMENT;
*/
extern BOOT_ARGUMENT    *g_boot_arg; //<<<-----
//static BOOT_ARGUMENT	*boot_args=BOOT_ARGS_ADDR;
//static unsigned int	*img_header_array = (unsigned int*)IMG_HEADER_ADDR;
static unsigned int		img_load_flag = 0;
static part_hdr_t		*img_info_start = NULL;
static unsigned int		img_addr_tbl[MAX_IMG_NUM];
static unsigned int		img_size_tbl[MAX_IMG_NUM];
static map_t			map_tbl[] = 
						{
							{"DUMMY_AP",		DUMMY_AP_IMG},
							{"MD_IMG",			MD1_IMG},
							{"MD_RAM_DISK", 	MD1_RAM_DISK},
							//{"MD2IMG",			MD2_IMG},
							//{"MD2_RAM_DISK", 	MD2_RAM_DISK},
						};

extern int mt_set_gpio_mode_chip(unsigned int pin, unsigned int mode);

int parse_img_header(unsigned int *start_addr, unsigned int img_num) //<<<------
{
	int i, j;
	int idx;

	if(start_addr == NULL) {
		printf("parse_img_header get invalid parameters!\n");
		return -1;
	}
	img_info_start = (part_hdr_t*)start_addr;
	for(i=0; i<img_num; i++) //<<<------
	{
		if(img_info_start[i].info.magic != PART_HEADER_MAGIC)
			continue;

		for(j=0; j<(sizeof(map_tbl)/sizeof(map_t)); j++)
		{
			if(strcmp(img_info_start[i].info.name, map_tbl[j].name) == 0) {
				idx = map_tbl[j].idx;
				img_addr_tbl[idx] = img_info_start[i].info.maddr;
				img_size_tbl[idx] = img_info_start[i].info.dsize;
				img_load_flag |= (1<<idx);
				printf("[%s] idx:%d, addr:0x%x, size:0x%x\n", map_tbl[j].name, idx, img_addr_tbl[idx], img_size_tbl[idx]);
			}
		}
	}
	return 0;
}

static int meta_detection(void)
{
	int boot_mode = 0;
	// Put check bootmode code here
	if(g_boot_arg->boot_mode != NORMAL_BOOT)
		boot_mode = 1;
	//else if(mtk_detect_key(MT65XX_BOOT_MENU_KEY))
	//	boot_mode = 1;
	//boot_mode=0;//always enter normal mode
	//boot_mode=1;//always enter meta mode
	printf("Meta mode: %d, boot_mode: %d-v1)\n", boot_mode, g_boot_arg->boot_mode);
	return boot_mode;
}

static void md_gpio_config(unsigned int boot_md_id)
{
	unsigned int tmp;
	volatile unsigned int loop = 10000;

	switch(boot_md_id)
	{
		case 0:			
			printf("configure SIM GPIO as SIM mode\n");
			//set GPIO as sim mode: GPIO0=SIM2_SCLK, GPIO1=SIM2_SIO, GPIO2=SIM1_SCLK, GPIO3=SIM1_SIO
			mt_set_gpio_mode(GPIO0, 1); 	//SIM2_SCLK
			mt_set_gpio_mode(GPIO1, 1); 	//SIM2_SIO
			mt_set_gpio_mode(GPIO2, 1);		//SIM1_SCLK	
			mt_set_gpio_mode(GPIO3, 1); 	//SIM1_SIO

			//set GPIO dir: SCLK=output, SIO=input
			mt_set_gpio_dir(GPIO0, GPIO_DIR_OUT);	//GPIO0->SIM2_CLK, out
			mt_set_gpio_dir(GPIO1, GPIO_DIR_IN);	//GPIO1->SIM2_SIO, in
			mt_set_gpio_dir(GPIO2, GPIO_DIR_OUT);	//GPIO2->SIM1_CLK, out
			mt_set_gpio_dir(GPIO3, GPIO_DIR_IN);	//GPIO3->SIM1_SIO, in

			#if 0
			tmp = *((volatile unsigned int *)0x10005300); //GPIO_MODE0
			//set GPIO as sim mode: GPIO0=SIM2_SCLK, GPIO1=SIM2_SIO, GPIO2=SIM1_SCLK, GPIO3=SIM1_SIO
			*((volatile unsigned int *)0x10005300) = tmp|(1<<0)|(1<<4)|(1<<8)|(1<<12);

			//set GPIO dir: SCLK=output, SIO=input
			*((volatile unsigned int *)0x10005004) = (1<<0)|(1<<2); //GPIO_DIR0_SET: GPIO0 & GPIO2
			*((volatile unsigned int *)0x10005008) = (1<<1)|(1<<3); //GPIO_DIR0_CLR: GPIO1 & GPIO3
			#endif

			#if 0
			// MD uart gpio
			mt_set_gpio_mode_chip(77, 4);
			mt_set_gpio_mode_chip(78, 4);
			mt_set_gpio_mode_chip(44, 1);
			mt_set_gpio_mode_chip(45, 1);
			mt_set_gpio_mode_chip(46, 1);
			mt_set_gpio_mode_chip(47, 1);
			mt_set_gpio_mode_chip(48, 1);
			mt_set_gpio_mode_chip(49, 1);
			#endif

			break;
			
		case 1:
			// Put MD2 gpio configure code here
			break;
	}

	return;
}

static void md_emi_remapping(unsigned int boot_md_id)
{
	unsigned int md_img_start_addr = 0;
	unsigned int md_emi_remapping_addr = 0;

	switch(boot_md_id)
	{
		case 0: // MD1
			md_img_start_addr = img_addr_tbl[MD1_IMG] - 0x80000000;
			md_emi_remapping_addr = 0x10001300; // MD1 BANK0_MAP0
			break;
			
		case 1: // MD2
			md_img_start_addr = img_addr_tbl[MD2_IMG] - 0x80000000;
			md_emi_remapping_addr = 0x10001310; // MD2 BANK0_MAP0
			break;
			
		default:
			break;
	}

	printf("  ---> Map 0x00000000 to %x for MD%d\n", md_img_start_addr+0x80000000, boot_md_id+1); 
	*((volatile unsigned int*)md_emi_remapping_addr) = ((md_img_start_addr >> 24) | 1) & 0xFF; // For MD1 BANK0_MAP0
}

static void md_power_up_mtcmos(unsigned int boot_md_id)
{

	volatile unsigned int loop = 10000;

	loop =10000;
	while(loop-->0);

	switch(boot_md_id)
	{
		case 0://MD 1
			//MD MTCMOS power on sequence
			*((volatile unsigned int *)0x10006284) |= 0x00000004; //SPM_MD1_PWR_CON |= PWR_ON_BIT
			loop = 10000; //delay 1us
			while(loop-->0);
			
			*((volatile unsigned int *)0x10006284) |= 0x00000008; //SPM_MD1_PWR_CON |= PWR_ON_S_BIT
			loop = 30000; //delay 3us
			while(loop-->0);

			//!(SPM_PWR_STATUS & 0x1) || !(SPM_PWR_STATUS_S & 0x1)
			while(!(*((volatile unsigned int *)0x1000660c)&0x1)|| !(*((volatile unsigned int *)0x10006610)&0x1));

			//SRAM PDN
			*((volatile unsigned int *)0x10006284) &= ~(1<<8); //SPM_MD1_PWR_CON &= ~SRAM_PDN_BIT
			*((volatile unsigned int *)0x10006284) &= ~(1<<4); //SPM_MD1_PWR_CON &= ~PWR_CLK_DIS_BIT
			*((volatile unsigned int *)0x10006284) &= ~(1<<1); //SPM_MD1_PWR_CON &= ~PWR_ISO_BIT
			*((volatile unsigned int *)0x10006284) |= (1<<4); //SPM_MD1_PWR_CON |= PWR_CLK_DIS_BIT
			*((volatile unsigned int *)0x10006284) |= (1<<0); //SPM_MD1_PWR_CON |= PWR_RST_B_BIT
			*((volatile unsigned int *)0x10006284) &= ~(1<<4);//SPM_MD1_PWR_CON &= ~PWR_CLK_DIS_BIT
			
			//release bus protection
			*((volatile unsigned int *)0x10001220) &= ~((1<<10)|(1<<9)|(1<<8)|(1<<7)); //TOPAXI_PORT_STA1&=(~0x780)
			while(*((volatile unsigned int *)0x10001220)&((1<<10)|(1<<9)|(1<<8)|(1<<7)));
			
			break;
			
		case 1:// MD2
			break;
		
	}
}

static void md_common_setting(int boot_md_id)
{
	switch(boot_md_id)
	{
		case 0:
			// Put special setting here if needed
			//; ## Disable WDT
			//print "Disable MD1 WDT"
			printf("Disable MD1 WDT\n");
			*((volatile unsigned int*)0x20050000) = 0x2200;
			
			//printf("setting md BPI GPIO-v1\n");
			//*((volatile unsigned int*)0x10005410) &= 0x00111111;
			//*((volatile unsigned int*)0x10005410) |= 0x11000000;
			//*((volatile unsigned int*)0x10005400) &= 0x00000011;
			//*((volatile unsigned int*)0x10005400) |= 0x11111100;
			break;
		}
}

static void md_boot_up(unsigned int boot_md_id, unsigned int is_meta_mode)
{
	switch(boot_md_id){
		case 0:// For MD1
			if(is_meta_mode)
				// Put META Register setting here
				*((volatile unsigned int*)0x20000010) |= 0x1; // Bit0, Meta mode flag, this need sync with MD init owner

			// Set boot slave to let MD to run
			*((volatile unsigned int*)0x2019379C) = 0x3567C766; // Key Register
			*((volatile unsigned int*)0x20190000) = 0x0; 		// Vector Register
			*((volatile unsigned int*)0x20195488) = 0xA3B66175; // Slave En Register
			break;

		case 1:// For MD2
			break;
			
		default:
			break;
	}

}

int md_jtag_config(int boot_md_id)
{
	// Add Jtag setting here
	return 0;
}

int get_input(void)
{
	return 0;
}

void apply_env_setting(int case_id)
{
	printf("Apply case:%d setting for dummy AP!\n", case_id);
}

void md_wdt_init(void);
void dummy_ap_entry(void)
{
	unsigned int	is_meta_mode = 0;
	int				md_check_tbl[] = {1<<MD1_IMG, 1<<MD2_IMG};
	int				i=0;
	int				get_val;

	volatile unsigned int	count;
	volatile unsigned int	count1;

	// Disable WDT
	*(volatile unsigned int *)(0x10007000) = 0x22000000; 

	printf("Welcome to use dummy AP!\n");
	get_val = get_input();

	apply_env_setting(get_val);

	// 0, Parse header info
	printf("Parsing image info!\n");
	//parse_img_header(img_header_array); //<<<------
	parse_img_header((unsigned int*)g_boot_arg->part_info, (unsigned int)g_boot_arg->part_num);

	printf("Begin to configure MD run env!\n");
	for(i=0; i<MAX_MD_NUM; i++) {
		if(img_load_flag & md_check_tbl[i]) {
			printf("MD%d Enabled\n", i+1);

			// 1, Setup special GPIO request (RF/SIM/UART ... etc)
			//printf("Step 1: Configure special GPIO request!\n");
			//md_gpio_config(i);

			// 2, Configure EMI remapping setting
			printf("Step 2: Configure EMI remapping...\n");
			md_emi_remapping(i);
	 
			// 3, Power up MD MTCMOS
			//printf("Step 3: Power up MD!\n");
			//md_power_up_mtcmos(i);

			// 4, Configure DAP for ICE to connect to MD
			printf("Step 4: Configure DAP for ICE to connect to MD!\n");
			md_jtag_config(i);

			// 5, Check boot Mode
			is_meta_mode = meta_detection();
			printf("Step 5: Notify MD enter %s mode!\n", is_meta_mode ? "META" : "NORMAL");

			// 6, MD register setting
			printf("Step 6: MD Common setting!\n");
			md_common_setting(i);

			// 7, Boot up MD
			printf("Step 7: MD%d boot up with meta(%d)!\n", i+1, is_meta_mode);
			md_boot_up(i, is_meta_mode);

			printf("\nmd%d boot up done!!\n", i + 1);
		}
	}

	printf("All dummy AP config done, enter while(1), Yeah!!\n");
	md_wdt_init();
	count = 1;
	while(count--) {
		count1 = 0x80000000;
		while(count1--);
	}
	printf("Write MD WDT SWRST\n");
	*((volatile unsigned int *)0x2005001C) = 0x1209; 
	count = 1;
	while(count--) {
		count1 = 0x08000000;
		while(count1--);
	}
	printf("Read back STA:%x!!\n", *((volatile unsigned int*)0x2005000C));
	while(1);
}

// EXT functions
#include <sys/types.h>
#include <debug.h>
#include <err.h>
#include <reg.h>
#include <platform/mt_typedefs.h>
#include <platform/mt_reg_base.h>
#include <platform/mt_irq.h>
#include <sys/types.h>

#define GIC_PRIVATE_SIGNALS	(32)
#define MT_MD_WDT1_IRQ_ID	(GIC_PRIVATE_SIGNALS + 179)


void md_wdt_irq_handler(unsigned int irq)
{
	printf("Get MD WDT irq, STA:%x!!\n", *((volatile unsigned int*)0x2005000C));
}

void dummy_ap_irq_handler(unsigned int irq)
{
	switch(irq){
	case MT_MD_WDT1_IRQ_ID:
		md_wdt_irq_handler(MT_MD_WDT1_IRQ_ID);
		mt_irq_ack(MT_MD_WDT1_IRQ_ID);
		mt_irq_unmask(MT_MD_WDT1_IRQ_ID);
		break;

	default:
		break;
	}
}

void md_wdt_init(void)
{
	mt_irq_set_sens(MT_MD_WDT1_IRQ_ID, MT65xx_EDGE_SENSITIVE);
	mt_irq_set_polarity(MT_MD_WDT1_IRQ_ID, MT65xx_POLARITY_LOW);
	mt_irq_unmask(MT_MD_WDT1_IRQ_ID);
}

