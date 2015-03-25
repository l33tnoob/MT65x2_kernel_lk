#include <sys/types.h>
#include <stdint.h>


#include <mt_partition.h>
#include <platform/mt_typedefs.h>
#include <platform/boot_mode.h>
#include <platform/mt_reg_base.h>
#include <platform/bootimg.h>
#include <platform/errno.h>
#include <printf.h>
#include <string.h>
#include <malloc.h>
#include <platform/mt_gpt.h>
#define MODULE_NAME "LK_BOOT"

// ************************************************************************


//*********
//* Notice : it's kernel start addr (and not include any debug header)
unsigned int g_kmem_off = 0;

//*********
//* Notice : it's rootfs start addr (and not include any debug header)
unsigned int g_rmem_off = 0;


unsigned int g_bimg_sz = 0;
unsigned int g_rcimg_sz = 0;
unsigned int g_fcimg_sz = 0;
int g_kimg_sz = 0;
int g_rimg_sz = 0;

#if 1

static int mboot_common_load_part_info(part_dev_t *dev, char *part_name, part_hdr_t *part_hdr)
{
    long len;
 #ifdef MTK_EMMC_SUPPORT
    u64 addr;
#else
	ulong addr;
#endif
    part_t *part;    

    part = mt_part_get_partition(part_name);   
    
#ifdef MTK_EMMC_SUPPORT
	addr = (u64)part->startblk * BLK_SIZE;
#else
	addr = part->startblk * BLK_SIZE;
#endif
    
    //***************
    //* read partition header
    //*
    len = dev->read(dev, addr, (uchar*)part_hdr, sizeof(part_hdr_t));
    if (len < 0) {
        printf("[%s] %s partition read error. LINE: %d\n", MODULE_NAME, part_name, __LINE__);
        return -1;
    }

	printf("\n=========================================\n");
    printf("[%s] %s magic number : 0x%x\n",MODULE_NAME,part_name,part_hdr->info.magic);
    part_hdr->info.name[31]='\0'; //append end char
    printf("[%s] %s name         : %s\n",MODULE_NAME,part_name,part_hdr->info.name);
    printf("[%s] %s size         : %d\n",MODULE_NAME,part_name,part_hdr->info.dsize);
	printf("=========================================\n");

	//***************
    //* check partition magic
	//*
    if (part_hdr->info.magic != PART_MAGIC) {
        printf("[%s] %s partition magic error\n", MODULE_NAME, part_name);
        return -1;
    }

	//***************
    //* check partition name
    //*
    if (strncmp(part_hdr->info.name, part_name, sizeof(part_hdr->info.name))) {
        printf("[%s] %s partition name error\n", MODULE_NAME, part_name);
        return -1;
    }

	//***************
    //* check partition data size
    //*
    if (part_hdr->info.dsize > part->blknum * BLK_SIZE) {
        printf("[%s] %s partition size error\n", MODULE_NAME, part_name);
        return -1;
    }
    
    return 0;
}


/**********************************************************
 * Routine: mboot_common_load_part
 *
 * Description: common function for loading image from nand flash
 *				this function is called by
 *					(1) 'mboot_common_load_logo' to display logo
 *
 **********************************************************/
int mboot_common_load_part(char *part_name, unsigned long addr)
{
    long len;
#ifdef MTK_EMMC_SUPPORT
	  unsigned long long start_addr;
#else
	  unsigned long start_addr;	
#endif
    part_t *part;	
    part_dev_t *dev;
    part_hdr_t *part_hdr;

    dev = mt_part_get_device();
    if (!dev)
    {	return -ENODEV;
    }

    part = mt_part_get_partition(part_name);
    if (!part)
    {	return -ENOENT;
    }

#ifdef MTK_EMMC_SUPPORT
	start_addr = (u64)part->startblk * BLK_SIZE; 
#else
    start_addr = part->startblk * BLK_SIZE;    
#endif  

    part_hdr = (part_hdr_t*)malloc(sizeof(part_hdr_t));

    
    if (!part_hdr)
    {	return -ENOMEM;
    }

    len = mboot_common_load_part_info(dev, part_name, part_hdr);
    if (len < 0) {
        len = -EINVAL;        
        goto exit;
    }


	//****************
    //* read image data
	//*
	printf("read the data of %s\n", part_name);

	
    len = dev->read(dev, start_addr + sizeof(part_hdr_t), (uchar*)addr, part_hdr->info.dsize);    
    if (len < 0) {
        len = -EIO;
        goto exit;
    }


exit:
    if (part_hdr) 
        free(part_hdr);

    return len;
}

/**********************************************************
 * Routine: mboot_common_load_logo
 *
 * Description: function to load logo to display
 *
 **********************************************************/
int mboot_common_load_logo(unsigned long logo_addr, char* filename)
{
    int ret;
#if (CONFIG_COMMANDS & CFG_CMD_FAT) 
   long len;
#endif

#if (CONFIG_COMMANDS & CFG_CMD_FAT)
    len = file_fat_read(filename, (unsigned char *)logo_addr, 0);    

    if (len > 0) 
        return (int)len;
#endif

    ret = mboot_common_load_part(PART_LOGO, logo_addr);

    return ret;
}

/**********************************************************
 * Routine: mboot_android_check_img_info
 *
 * Description: this function is called to 
 *				(1) check the header of kernel / rootfs
 *
 * Notice : this function will be called by 'mboot_android_check_bootimg_hdr'
 *
 **********************************************************/
int mboot_android_check_img_info(char *part_name, part_hdr_t *part_hdr)
{
	printf("\n=========================================\n");
    printf("[%s] %s magic number : 0x%x\n",MODULE_NAME,part_name,part_hdr->info.magic);
    part_hdr->info.name[31]='\0'; 
    printf("[%s] %s name         : %s\n",MODULE_NAME,part_name,part_hdr->info.name);
    printf("[%s] %s size         : 0x%x\n",MODULE_NAME,part_name,part_hdr->info.dsize);
	printf("=========================================\n");

	//***************
    //* check partition magic
    //*
    if (part_hdr->info.magic != PART_MAGIC) {
        printf("[%s] %s partition magic error\n", MODULE_NAME, part_name);
        return -1;
    }

	//***************
    //* check partition name
    //*
    if (strncmp(part_hdr->info.name, part_name, sizeof(part_hdr->info.name))) {
        printf("[%s] %s partition name error\n", MODULE_NAME, part_name);
        return -1;
    }

	//***************
    //* return the image size
    //*    
    return part_hdr->info.dsize;
}

/**********************************************************
 * Routine: mboot_android_check_bootimg_hdr
 *
 * Description: this function is called to 
 *				(1) 'read' the header of boot image from nand flash
 *				(2) 'parse' the header of boot image to obtain
 *					- (a) kernel image size
 *					- (b) rootfs image size
 *					- (c) rootfs offset
 *
 * Notice : this function must be read first when doing nand / msdc boot
 *
 **********************************************************/
static int mboot_android_check_bootimg_hdr(part_dev_t *dev, char *part_name, boot_img_hdr *boot_hdr)
{
    long len;
#ifdef MTK_EMMC_SUPPORT
    u64 addr;
#else
    ulong addr;
#endif
    part_t *part;


    //**********************************
	// TODO : fix pg_sz assignment
    //**********************************	
    unsigned int pg_sz = 2*KB ;    
   
    part = mt_part_get_partition(part_name);   
#ifdef MTK_EMMC_SUPPORT   
	addr = (u64)part->startblk * BLK_SIZE;
#else
	addr = part->startblk * BLK_SIZE;
#endif

	//***************    
    //* read partition header
    //*

    printf("part page addr is 0x%x\n", addr);
    
    len = dev->read(dev, addr, (uchar*) boot_hdr, sizeof(boot_img_hdr));
    if (len < 0) {
        printf("[%s] %s boot image header read error. LINE: %d\n", MODULE_NAME, part_name, __LINE__);
        return -1;
    }    

	printf("\n============================================================\n");
	boot_hdr->magic[7] = '\0';
    printf("[%s] Android Partition Name     	        : %s\n",MODULE_NAME,part_name);
    printf("[%s] Android Boot IMG Hdr - Magic 	        : %s\n",MODULE_NAME,boot_hdr->magic);
    printf("[%s] Android Boot IMG Hdr - Kernel Size 	: 0x%x\n",MODULE_NAME,boot_hdr->kernel_size);
    printf("[%s] Android Boot IMG Hdr - Rootfs Size 	: 0x%x\n",MODULE_NAME,boot_hdr->ramdisk_size);
    printf("[%s] Android Boot IMG Hdr - Page Size    	: 0x%x\n",MODULE_NAME,boot_hdr->page_size);    
	printf("============================================================\n");

	//***************
    //* check partition magic
	//*
    if (strncmp((char const *)boot_hdr->magic,BOOT_MAGIC, sizeof(BOOT_MAGIC))!=0) {
        printf("[%s] boot image header magic error\n", MODULE_NAME);
        return -1;
    }

	//***************
	//* follow bootimg.h to calculate the location of rootfs
	//*
	if(len != -1)
	{
		unsigned int k_pg_cnt = 0;
		unsigned int r_pg_cnt = 0;
//		unsigned int size_b = 0;
	
		g_kmem_off =  CFG_BOOTIMG_LOAD_ADDR;

		if(boot_hdr->kernel_size % pg_sz == 0)
		{	
		  k_pg_cnt = boot_hdr->kernel_size / pg_sz;
		}
		else
		{	
		  k_pg_cnt = (boot_hdr->kernel_size / pg_sz) + 1;
		}
		
		if(boot_hdr->ramdisk_size % pg_sz == 0)
		{	
		  r_pg_cnt = boot_hdr->ramdisk_size / pg_sz;
		}
		else
		{	
		  r_pg_cnt = (boot_hdr->ramdisk_size / pg_sz) + 1;
		}

		printf(" > page count of kernel image = %d\n",k_pg_cnt);
		g_rmem_off = g_kmem_off + k_pg_cnt * pg_sz;

		printf(" > kernel mem offset = 0x%x\n",g_kmem_off);
		printf(" > rootfs mem offset = 0x%x\n",g_rmem_off);		


		//***************
		//* specify boot image size
		//*
		//g_bimg_sz = PART_BLKS_BOOTIMIG * BLK_SIZE;
		g_bimg_sz = (k_pg_cnt + r_pg_cnt + 2)* pg_sz;

		printf(" > boot image size = 0x%x\n",g_bimg_sz);
	}
    
    return 0;
}

/**********************************************************
 * Routine: mboot_android_check_recoveryimg_hdr
 *
 * Description: this function is called to 
 *				(1) 'read' the header of boot image from nand flash
 *				(2) 'parse' the header of boot image to obtain
 *					- (a) kernel image size
 *					- (b) rootfs image size
 *					- (c) rootfs offset
 *
 * Notice : this function must be read first when doing nand / msdc boot
 *
 **********************************************************/
static int mboot_android_check_recoveryimg_hdr(part_dev_t *dev, char *part_name, boot_img_hdr *boot_hdr)
{
    long len;
#ifdef MTK_EMMC_SUPPORT
	u64 addr;
#else
    ulong addr;
#endif
    part_t *part;    

    //**********************************
	// TODO : fix pg_sz assignment
    //**********************************	
    unsigned int pg_sz = 2*KB ;    


    part = mt_part_get_partition(part_name);   
#ifdef MTK_EMMC_SUPPORT  
	addr = (u64)part->startblk * BLK_SIZE;
#else
		addr = part->startblk * BLK_SIZE;
#endif

	//***************    
    //* read partition header
    //*
    len = dev->read(dev, addr, (uchar*) boot_hdr, sizeof(boot_img_hdr));
    if (len < 0) {
        printf("[%s] %s Recovery image header read error. LINE: %d\n", MODULE_NAME, part_name, __LINE__);
        return -1;
    }    

	printf("\n============================================================\n");
	boot_hdr->magic[7] = '\0';
    printf("[%s] Android Recovery IMG Hdr - Magic 	        : %s\n",MODULE_NAME,boot_hdr->magic);
    printf("[%s] Android Recovery IMG Hdr - Kernel Size 	: 0x%x\n",MODULE_NAME,boot_hdr->kernel_size);
    printf("[%s] Android Recovery IMG Hdr - Rootfs Size 	: 0x%x\n",MODULE_NAME,boot_hdr->ramdisk_size);
    printf("[%s] Android Recovery IMG Hdr - Page Size    	: 0x%x\n",MODULE_NAME,boot_hdr->page_size);    
	printf("============================================================\n");

	//***************
    //* check partition magic
	//*
    if (strncmp((char const *)boot_hdr->magic,BOOT_MAGIC, sizeof(BOOT_MAGIC))!=0) {
        printf("[%s] Recovery image header magic error\n", MODULE_NAME);
        return -1;
    }

	//***************
	//* follow bootimg.h to calculate the location of rootfs
	//*
	if(len != -1)
	{
		unsigned int k_pg_cnt = 0;
	
		g_kmem_off =  CFG_BOOTIMG_LOAD_ADDR;

		if(boot_hdr->kernel_size % pg_sz == 0)
		{	k_pg_cnt = boot_hdr->kernel_size / pg_sz;
		}
		else
		{	k_pg_cnt = (boot_hdr->kernel_size / pg_sz) + 1;
		}

		printf(" > page count of kernel image = %d\n",k_pg_cnt);
		g_rmem_off = g_kmem_off + k_pg_cnt * pg_sz;

		printf(" > kernel mem offset = 0x%x\n",g_kmem_off);
		printf(" > rootfs mem offset = 0x%x\n",g_rmem_off);		


		//***************
		//* specify boot image size
		//*
		g_rcimg_sz = PART_BLKS_RECOVERY * BLK_SIZE;

		printf(" > Recovery image size = 0x%x\n", g_rcimg_sz);
	}
    
    return 0;
}


/**********************************************************
 * Routine: mboot_android_check_factoryimg_hdr
 *
 * Description: this function is called to 
 *				(1) 'read' the header of boot image from nand flash
 *				(2) 'parse' the header of boot image to obtain
 *					- (a) kernel image size
 *					- (b) rootfs image size
 *					- (c) rootfs offset
 *
 * Notice : this function must be read first when doing nand / msdc boot
 *
 **********************************************************/
static int mboot_android_check_factoryimg_hdr(char *part_name, boot_img_hdr *boot_hdr)
{
    int len=0;
 //   ulong addr;

    //**********************************
	// TODO : fix pg_sz assignment
    //**********************************	
    unsigned int pg_sz = 2*KB ;     

	//***************    
    //* read partition header
    //*
    
#if (CONFIG_COMMANDS & CFG_CMD_FAT)
    len = file_fat_read(part_name, (uchar*) boot_hdr, sizeof(boot_img_hdr));
    
    if (len < 0) {
        printf("[%s] %s Factory image header read error. LINE: %d\n", MODULE_NAME, part_name, __LINE__);
        return -1;
    }    
#endif

	printf("\n============================================================\n");
	boot_hdr->magic[7] = '\0';
    printf("[%s] Android Factory IMG Hdr - Magic 	        : %s\n",MODULE_NAME,boot_hdr->magic);
    printf("[%s] Android Factory IMG Hdr - Kernel Size 	: 0x%x\n",MODULE_NAME,boot_hdr->kernel_size);
    printf("[%s] Android Factory IMG Hdr - Rootfs Size 	: 0x%x\n",MODULE_NAME,boot_hdr->ramdisk_size);
    printf("[%s] Android Factory IMG Hdr - Page Size    	: 0x%x\n",MODULE_NAME,boot_hdr->page_size);    
	printf("============================================================\n");

	//***************
    //* check partition magic
	//*
    if (strncmp((char const *)boot_hdr->magic,BOOT_MAGIC, sizeof(BOOT_MAGIC))!=0) {
        printf("[%s] Factory image header magic error\n", MODULE_NAME);
        return -1;
    }

	//***************
	//* follow bootimg.h to calculate the location of rootfs
	//*
	if(len != -1)
	{
		unsigned int k_pg_cnt = 0;
	
		g_kmem_off =  CFG_BOOTIMG_LOAD_ADDR;

		if(boot_hdr->kernel_size % pg_sz == 0)
		{	k_pg_cnt = boot_hdr->kernel_size / pg_sz;
		}
		else
		{	k_pg_cnt = (boot_hdr->kernel_size / pg_sz) + 1;
		}

		printf(" > page count of kernel image = %d\n",k_pg_cnt);
		g_rmem_off = g_kmem_off + k_pg_cnt * pg_sz;

		printf(" > kernel mem offset = 0x%x\n",g_kmem_off);
		printf(" > rootfs mem offset = 0x%x\n",g_rmem_off);		


		//***************
		//* specify boot image size
		//*
		g_fcimg_sz = PART_BLKS_RECOVERY * BLK_SIZE;

		printf(" > Factory image size = 0x%x\n", g_rcimg_sz);
	}
    
    return 0;
}


/**********************************************************
 * Routine: mboot_android_load_bootimg_hdr
 *
 * Description: this is the entry function to handle boot image header
 *
 **********************************************************/
int mboot_android_load_bootimg_hdr(char *part_name, unsigned long addr)
{
    long len;
//	unsigned long begin;
//	unsigned long start_addr;	
    part_t *part;	
    part_dev_t *dev;
    boot_img_hdr *boot_hdr;
	
    dev = mt_part_get_device();
    if (!dev)
    {    
		printf("mboot_android_load_bootimg_hdr, dev = NULL\n");
        return -ENODEV;
    }
	
    part = mt_part_get_partition(part_name);
    if (!part)
    {
		printf("mboot_android_load_bootimg_hdr (%s), part = NULL\n",part_name);
    	return -ENOENT;
    }

//    start_addr = part->startblk * BLK_SIZE;    

    boot_hdr = (boot_img_hdr*)malloc(sizeof(boot_img_hdr));
    if (!boot_hdr)
	{
    	printf("mboot_android_load_bootimg_hdr, boot_hdr = NULL\n");
    	return -ENOMEM;
    }
	
    len = mboot_android_check_bootimg_hdr(dev, part_name, boot_hdr);

    return len;
}

/**********************************************************
 * Routine: mboot_android_load_recoveryimg_hdr
 *
 * Description: this is the entry function to handle Recovery image header
 *
 **********************************************************/
int mboot_android_load_recoveryimg_hdr(char *part_name, unsigned long addr)
{
    long len;
//	unsigned long begin;
//	unsigned long start_addr;	
    part_t *part;	
    part_dev_t *dev;
    boot_img_hdr *boot_hdr;
	
    dev = mt_part_get_device();
    if (!dev)
    {    
		printf("mboot_android_load_recoveryimg_hdr, dev = NULL\n");
        return -ENODEV;
    }
	
    part = mt_part_get_partition(part_name);
    if (!part)
    {
		printf("mboot_android_load_recoveryimg_hdr (%s), part = NULL\n",part_name);
    	return -ENOENT;
    }

//    start_addr = part->startblk * BLK_SIZE;    

    boot_hdr = (boot_img_hdr*)malloc(sizeof(boot_img_hdr));
    if (!boot_hdr)
	{
    	printf("mboot_android_load_bootimg_hdr, boot_hdr = NULL\n");
    	return -ENOMEM;
    }
	
    len = mboot_android_check_recoveryimg_hdr(dev, part_name, boot_hdr);

    return len;
}


/**********************************************************
 * Routine: mboot_android_load_factoryimg_hdr
 *
 * Description: this is the entry function to handle Factory image header
 *
 **********************************************************/
int mboot_android_load_factoryimg_hdr(char *part_name, unsigned long addr)
{
    long len;

    boot_img_hdr *boot_hdr; 

    boot_hdr = (boot_img_hdr*)malloc(sizeof(boot_img_hdr));
    
    if (!boot_hdr)
	{
    	printf("mboot_android_load_factoryimg_hdr, boot_hdr = NULL\n");
    	return -ENOMEM;
    }
	
    len = mboot_android_check_factoryimg_hdr(part_name, boot_hdr);

    return len;
}


/**********************************************************
 * Routine: mboot_android_load_bootimg
 *
 * Description: main function to load Android Boot Image
 *
 **********************************************************/
int mboot_android_load_bootimg(char *part_name, unsigned long addr)
{
    long len;
//	unsigned long begin;
#ifdef MTK_EMMC_SUPPORT
	unsigned long long start_addr;	
#else
		unsigned long start_addr;	
#endif
    part_t *part;	
    part_dev_t *dev;
    
    dev = mt_part_get_device();
    if (!dev)
    {	printf("mboot_android_load_bootimg , dev = NULL\n");
    	return -ENODEV;
    }

    part = mt_part_get_partition(part_name);
    if (!part)
    {	printf("mboot_android_load_bootimg , part = NULL\n");
    	return -ENOENT;
    }

	//***************
	//* not to include unused header
	//*
#ifdef MTK_EMMC_SUPPORT
	start_addr =(u64)part->startblk * BLK_SIZE + BIMG_HEADER_SZ;
#else
	start_addr = part->startblk * BLK_SIZE + BIMG_HEADER_SZ;
#endif
	addr  = addr - MKIMG_HEADER_SZ;

	//***************    
    //* read image data
	//*    
	printf("\nread the data of %s (size = 0x%x)\n", part_name, g_bimg_sz);

#ifdef MTK_EMMC_SUPPORT
	printf(" > from - 0x%016llx (skip boot img hdr)\n",start_addr);
#else
	printf(" > from - 0x%x (skip boot img hdr)\n",start_addr);
#endif
	printf(" > to   - 0x%x (starts with kernel img hdr)\n",addr);

    len = dev->read(dev, start_addr, (uchar*)addr, g_bimg_sz );    


	//***************
	//* check kernel header	
	//*
	g_kimg_sz = mboot_android_check_img_info(PART_KERNEL,(part_hdr_t *)(g_kmem_off - MKIMG_HEADER_SZ));
	if(g_kimg_sz == -1) {
        len = -EIO;
        goto exit;
}

	//***************
	//* check rootfs header
	//*
	g_rimg_sz = mboot_android_check_img_info(PART_ROOTFS,(part_hdr_t *)(g_rmem_off - MKIMG_HEADER_SZ));
	if(g_rimg_sz == -1) {
        len = -EIO;
        goto exit;
    }

    if (len < 0) {
        len = -EIO;
        goto exit;
    }


exit:

    return len;
}

/**********************************************************
 * Routine: mboot_android_load_recoveryimg
 *
 * Description: main function to load Android Recovery Image
 *
 **********************************************************/
int mboot_android_load_recoveryimg(char *part_name, unsigned long addr)
{
    long len;
//	unsigned long begin;
#ifdef MTK_EMMC_SUPPORT
	unsigned long long start_addr;
#else
	unsigned long start_addr;

#endif
    part_t *part;	
    part_dev_t *dev;
    
    dev = mt_part_get_device();
    if (!dev)
    {	printf("mboot_android_load_bootimg , dev = NULL\n");
    	return -ENODEV;
    }

    part = mt_part_get_partition(part_name);
    if (!part)
    {	printf("mboot_android_load_bootimg , part = NULL\n");
    	return -ENOENT;
    }


	//***************
	//* not to include unused header
	//*
#ifdef MTK_EMMC_SUPPORT
	start_addr = (u64)part->startblk * BLK_SIZE + BIMG_HEADER_SZ;
#else
	start_addr = part->startblk * BLK_SIZE + BIMG_HEADER_SZ;

#endif
	addr  = addr - MKIMG_HEADER_SZ;

	//***************    
    //* read image data
	//*    
	printf("\nread the data of %s (size = 0x%x)\n", part_name, g_rcimg_sz);
#ifdef MTK_EMMC_SUPPORT	
	printf(" > from - 0x%016llx (skip recovery img hdr)\n",start_addr);
#else
	printf(" > from - 0x%x (skip recovery img hdr)\n",start_addr);
#endif
	printf(" > to   - 0x%x (starts with kernel img hdr)\n",addr);

    len = dev->read(dev, start_addr, (uchar*)addr, g_rcimg_sz );    


	//***************
	//* check kernel header	
	//*
	g_kimg_sz = mboot_android_check_img_info(PART_KERNEL,(part_hdr_t *)(g_kmem_off - MKIMG_HEADER_SZ));
	if(g_kimg_sz == -1) {
        len = -EIO;
        goto exit;
}

	//***************
	//* check rootfs header
	//*
	g_rimg_sz = mboot_android_check_img_info(PART_RECOVERY,(part_hdr_t *)(g_rmem_off - MKIMG_HEADER_SZ));
	if(g_rimg_sz == -1) {
        len = -EIO;
        goto exit;
    }

    if (len < 0) {
        len = -EIO;
        goto exit;
    }


exit:

    return len;
}


/**********************************************************
 * Routine: mboot_android_load_factoryimg
 *
 * Description: main function to load Android Factory Image
 *
 **********************************************************/
int mboot_android_load_factoryimg(char *part_name, unsigned long addr)
{
    int len = 0;
//	unsigned long start_addr;	

	//***************
	//* not to include unused header
	//*
	addr  = addr - MKIMG_HEADER_SZ - BIMG_HEADER_SZ;


#if (CONFIG_COMMANDS & CFG_CMD_FAT)
    len = file_fat_read(part_name, (uchar*)addr, 0);

    printf("len = %d, addr = 0x%x\n", len, addr);
    printf("part name = %s \n", part_name);
#endif

	//***************
	//* check kernel header	
	//*
	
	g_kimg_sz = mboot_android_check_img_info(PART_KERNEL,(part_hdr_t *)(g_kmem_off - MKIMG_HEADER_SZ));
	if(g_kimg_sz == -1) {
        len = -EIO;
        goto exit;
    }

	//***************
	//* check rootfs header
	//*
	g_rimg_sz = mboot_android_check_img_info(PART_ROOTFS,(part_hdr_t *)(g_rmem_off - MKIMG_HEADER_SZ));
	if(g_rimg_sz == -1) {
        len = -EIO;
        goto exit;
    }

    if (len < 0) {
        len = -EIO;
        goto exit;
    }


exit:

    return len;
}


/**********************************************************
 * Routine: mboot_recovery_load_raw_part
 *
 * Description: load raw data for recovery mode support
 *
 **********************************************************/
int mboot_recovery_load_raw_part(char *part_name, unsigned long *addr, unsigned int size)
{
    long len;
	unsigned long begin;

#ifdef MTK_EMMC_SUPPORT
	unsigned long long start_addr;	
#else
	unsigned long start_addr;	
#endif
    part_t *part;	
    part_dev_t *dev;

    dev = mt_part_get_device();
    if (!dev)
    {    
        return -ENODEV;
    }

    part = mt_part_get_partition(part_name);
    if (!part)
		{
    	return -ENOENT;
	    }
#ifdef MTK_EMMC_SUPPORT
    start_addr = (u64)part->startblk * BLK_SIZE;    
#else
	    start_addr = part->startblk * BLK_SIZE;    
#endif
	begin = get_timer(0);

    len = dev->read(dev, start_addr,(uchar*)addr, size);
    if (len < 0) 
		{   
        len = -EIO;
        goto exit;
		}

    printf("[%s] Load '%s' partition to 0x%08X (%d bytes in %ld ms)\n", MODULE_NAME, part->name, addr, size, get_timer(begin));
    
exit:
    return len;
}

/**********************************************************
 * Routine: mboot_recovery_load_misc
 *
 * Description: load recovery command
 *
 **********************************************************/
int mboot_recovery_load_misc(unsigned long *misc_addr, unsigned int size)
{
    int ret;

	printf("[mboot_recovery_load_misc]: size is %u\n", size);
	printf("[mboot_recovery_load_misc]: misc_addr is 0x%x\n", misc_addr);

    ret = mboot_recovery_load_raw_part(PART_MISC, misc_addr, size);

    if (ret < 0)
        return ret;

    return ret;
}

#endif
