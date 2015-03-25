
#ifndef __MTEE_SYS_MMU_H__
#define __MTEE_SYS_MMU_H__

        // Share, Read/write at PL1, Normal Memory, Inner/outer cachable, write-back/write-allocate
#define PROT_SECURE_MEMORY       ((1<<16)|(1<<10)|(1<<12)|(3<<2))
        // Share, Read-only at PL1, Normal Memory, Inner/outer cachable, write-back/write-allocate
#define PROT_SECURE_MEMORY_RO    ((1<<16)|(1<<15)|(1<<10)|(1<<12)|(3<<2))
        // +NS/Client/XN
#define PROT_NORMAL_MEMORY       ((PROT_SECURE_MEMORY)|(8<<5)|(1<<4)|(1<<19))
        // +NS/Client/XN/RO
#define PROT_NORMAL_MEMORY_RO    ((PROT_SECURE_MEMORY_RO)|(8<<5)|(1<<4)|(1<<19))
        // Share, Read/write at PL1, Device, non-cachable.
#define PROT_SECURE_DEVICE       ((1<<16)|(1<<10)|(0<<12)|(1<<2))
        // +NS/Client/XN
#define PROT_NORMAL_DEVICE       ((PROT_SECURE_DEVICE)|(8<<5)|(1<<4)|(1<<19))
        // Clear virtual memory mapping
#define PROT_NO_ACCESS           0


#define SZ_1K              (0x400)
#define SZ_4K              (0x1000)
#define SZ_64K             (0x10000)
#define SZ_128K            (0x20000)
#define SZ_256K            (0x40000)
#define SZ_1M              (0x100000)
#define SZ_2M              (0x200000)
#define SZ_4M              (0x400000)
#define SZ_16M             (0x1000000)


struct MTEE_MMUMaps {
    uint32_t virt_addr;
    uint32_t phys_addr;
    int size;
    uint32_t prot;
};

int MTEE_SetMMUTable(uint32_t virt_addr, uint32_t phys_addr, int size, uint32_t prot);
int MTEE_SetMMUMaps(const struct MTEE_MMUMaps *maps, int num);

#endif /* __MTEE_SYS_MMU_H__ */
