#include <linux/pm.h>
#include <linux/bug.h>
#if defined(CONFIG_TRUSTONIC_TEE_SUPPORT)
#include <linux/memblock.h>
#include <linux/bootmem.h>
#include <mach/mtk_boot_share_page.h>
#endif
#include <asm/mach/arch.h>
#include <asm/mach/time.h>
#include <asm/mach/map.h>
#include <asm/mach-types.h>
#include <asm/hardware/cache-l2x0.h>
#include <asm/smp_scu.h>
#include <asm/page.h>
#include <mach/mt_reg_base.h>
#include <mach/irqs.h>

extern void arm_machine_restart(char mode, const char *cmd);
extern struct sys_timer mt6582_timer;
extern void mt_fixup(struct tag *tags, char **cmdline, struct meminfo *mi);
extern void mt_reserve(void);

void __init mt_init(void)
{
    /* enable bus out of order command queue to enhance boot time */
#if 1
    volatile unsigned int opt;
    opt = readl(IOMEM(MCU_BIU_BASE));
    opt |= 0x1;
    writel(opt, IOMEM(MCU_BIU_BASE));
    dsb();
#endif
}

#if defined(CONFIG_TRUSTONIC_TEE_SUPPORT)
void __init mt_init_early(void)
{
    int ret;

    ret = reserve_bootmem(__pa(BOOT_SHARE_BASE), 0x1000, BOOTMEM_EXCLUSIVE);
    if (ret < 0)
    {
        printk(KERN_WARNING "reserve_bootmem BOOT_SHARE_BASE failed %d\n", ret);
    }    
}
#endif

static struct map_desc mt_io_desc[] __initdata = 
{
#if !defined(CONFIG_MT6582_FPGA)
    {
        .virtual = INFRA_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(INFRA_BASE)),
        .length = (SZ_1M - SZ_4K),
        .type = MT_DEVICE
    },
    /* Skip the mapping of 0xF0130000~0xF013FFFF to protect access from APMCU */
    {
        .virtual = (DEBUGTOP_BASE - SZ_4K),
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS((DEBUGTOP_BASE - SZ_4K))),
        .length = (0x30000 + SZ_4K),
        .type = MT_DEVICE
    },        
    {
        .virtual = (DEBUGTOP_BASE + 0x40000),
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(DEBUGTOP_BASE + 0x40000)),
        .length = 0xC0000,
        .type = MT_DEVICE
    },            
    {
        .virtual = MCUSYS_CFGREG_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(MCUSYS_CFGREG_BASE)),
        .length = SZ_2M,
        .type = MT_DEVICE
    },
    /* //// */
    {
        .virtual = AP_DMA_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(AP_DMA_BASE)),
        .length = SZ_2M + SZ_1M,
        .type = MT_DEVICE
    },
    {
        /* virtual 0xF2000000, physical 0x00200000 */
        .virtual = SYSRAM_BASE,
        .pfn = __phys_to_pfn(0x00200000),
        .length = SZ_128K,
        .type = MT_MEMORY_NONCACHED
    },
    {
        .virtual = G3D_CONFIG_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(G3D_CONFIG_BASE)),
        .length = SZ_128K,
        .type = MT_DEVICE
    },
    {
        .virtual = DISPSYS_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(DISPSYS_BASE)),
        .length = SZ_16M,
        .type = MT_DEVICE
    },
    {
        .virtual = IMGSYS_CONFG_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(IMGSYS_CONFG_BASE)),
        .length = SZ_16M,
        .type = MT_DEVICE
    },
    {
        .virtual = VDEC_GCON_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(VDEC_GCON_BASE)),
        .length = SZ_16M,
        .type = MT_DEVICE
    },
    {
        /* virtual 0xF7000000, physical 0x08000000 */
        .virtual = DEVINFO_BASE,
        .pfn = __phys_to_pfn(0x08000000),
        .length = SZ_64K,
        .type = MT_DEVICE
    },
    {
        .virtual = CONN_BTSYS_PKV_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(CONN_BTSYS_PKV_BASE)),
        .length = SZ_1M,
        .type = MT_DEVICE
    },
    {
        /* virtual 0xF9000000, physical 0x00100000 */
        .virtual = INTER_SRAM,
        .pfn = __phys_to_pfn(0x00100000),
        .length = SZ_64K,
        .type = MT_MEMORY_NONCACHED
    },
#else
    {
        .virtual = INFRA_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(INFRA_BASE)),
        .length = SZ_4M,
        .type = MT_DEVICE
    },
    {
        .virtual = AP_DMA_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(AP_DMA_BASE)),
        .length = SZ_2M + SZ_1M,
        .type = MT_DEVICE
    },
    #if 0
    {
        .virtual = MMSYS1_CONFIG_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(MMSYS1_CONFIG_BASE)),
        .length = SZ_16M,
        .type = MT_DEVICE
    },
    #endif
    {
        /* From: 0xF2000000 to 0xF2020000*/
        .virtual = SYSRAM_BASE,
        .pfn = __phys_to_pfn(0x00200000),
        .length = SZ_128K,
        .type = MT_MEMORY_NONCACHED
    },    
    {
        .virtual = DISPSYS_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(DISPSYS_BASE)),
        .length = SZ_16M,
        .type = MT_DEVICE
    },
    {
        .virtual = IMGSYS_CONFG_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(IMGSYS_CONFG_BASE)),
        .length = SZ_16M,
        .type = MT_DEVICE
    },
    /* G3DSYS */
    {
        .virtual = G3D_CONFIG_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(G3D_CONFIG_BASE)),
        .length = SZ_4K,
        .type = MT_DEVICE
    },
    {
        .virtual = DEVINFO_BASE,
        .pfn = __phys_to_pfn(0x08000000),
        .length = SZ_64K,
        .type = MT_DEVICE
    },
    {
        .virtual = MALI_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(MALI_BASE)),
        .length = SZ_64K,
        .type = MT_DEVICE
    },
    {
        .virtual = INTER_SRAM,
        .pfn = __phys_to_pfn(0x00100000),
        .length = SZ_64K,
        .type = MT_MEMORY_NONCACHED
    },
#endif
};

void __init mt_map_io(void)
{
    iotable_init(mt_io_desc, ARRAY_SIZE(mt_io_desc));
}

#ifdef MTK_TABLET_PLATFORM
MACHINE_START(MT6582, MTK_TABLET_PLATFORM)
#else
MACHINE_START(MT6582, "MT6582")
#endif
    .atag_offset    = 0x00000100,
    .map_io         = mt_map_io,
    .init_irq       = mt_init_irq,
    .timer          = &mt6582_timer,
#if defined(CONFIG_TRUSTONIC_TEE_SUPPORT)
    .init_early     = mt_init_early,
#endif
    .init_machine   = mt_init,
    .fixup          = mt_fixup,
    .restart        = arm_machine_restart,
    .reserve        = mt_reserve,
MACHINE_END
