#include <malloc.h>
#include <string.h>
#include <printf.h>
#include "kdump.h"

uint32_t roundup(uint32_t x, uint32_t y)
{
    return ((x + y - 1) / y) * y;
}

/* An ELF note in memory */
struct memelfnote
{
	const char *name;
	int type;
	unsigned int datasz;
	void *data;
};

static int notesize(struct memelfnote *en)
{
	int sz;

	sz = sizeof(struct elf_note);
	sz += roundup((strlen(en->name) + 1), 4);
	sz += roundup(en->datasz, 4);

	return sz;
}

static uint8_t *storenote(struct memelfnote *men, uint8_t *bufp)
{
    struct elf_note en;
    en.n_namesz = strlen(men->name) + 1;
    en.n_descsz = men->datasz;
    en.n_type = men->type;
    
    memcpy(bufp, &en, sizeof(en));
    bufp += sizeof(en);
    
    memcpy(bufp, men->name, en.n_namesz);
    bufp += en.n_namesz;
    
    bufp = (uint8_t*) roundup((unsigned long)bufp, 4);
    memcpy(bufp, men->data, men->datasz);
    bufp += men->datasz;

    bufp = (uint8_t*) roundup((unsigned long)bufp, 4);
    return bufp;
}

static uint8_t *kdump_core_write_cpu_note(const struct mrdump_control_block *mrdump_cb, int cpu, struct elf_phdr *nhdr, uint8_t *bufp)
{
    struct memelfnote notes;
    struct elf_prstatus prstatus;
    char cpustr[16];

    memset(&prstatus, 0, sizeof(struct elf_prstatus));

    snprintf(cpustr, sizeof(cpustr), "CPU%d", cpu);
    /* set up the process status */
    notes.name = cpustr;
    notes.type = NT_PRSTATUS;
    notes.datasz = sizeof(struct elf_prstatus);
    notes.data = &prstatus;
    
    prstatus.pr_pid = cpu + 1;
    memcpy(&prstatus.pr_reg, (unsigned long*)&mrdump_cb->crash_record.cpu_regs[cpu], sizeof(elf_gregset_t));
                
    nhdr->p_filesz += notesize(&notes);
    return storenote(&notes, bufp);
}

static uint8_t *kdump_core_write_machdesc(const struct mrdump_control_block *mrdump_cb, struct elf_phdr *nhdr, uint8_t *bufp)
{
    struct memelfnote notes;
    struct elf_mrdump_machdesc machdesc;
    const struct mrdump_machdesc *kparams = &mrdump_cb->machdesc;

    memset(&machdesc, 0, sizeof(struct elf_mrdump_machdesc));

    notes.name = "MACHDESC";
    notes.type = NT_MRDUMP_MACHDESC;
    notes.datasz = sizeof(struct elf_mrdump_machdesc);
    notes.data = &machdesc;

    machdesc.flags = MRDUMP_TYPE_FULL_MEMORY;
    machdesc.phys_offset = (uint32_t)kparams->phys_offset;
    machdesc.page_offset = (uint32_t)kparams->page_offset;
    machdesc.high_memory = (uint32_t)kparams->high_memory;
    machdesc.modules_start = (uint32_t)kparams->modules_start;
    machdesc.modules_end = (uint32_t)kparams->modules_end;
    machdesc.vmalloc_start = (uint32_t)kparams->vmalloc_start;
    machdesc.vmalloc_end = (uint32_t)kparams->vmalloc_end;

    nhdr->p_filesz += notesize(&notes);
    return storenote(&notes, bufp);
}
extern void voprintf_debug(const char *msg, ...);
void *kdump_core_header_init(const struct mrdump_control_block *mrdump_cb, uint32_t kmem_address, uint32_t kmem_size)
{
	struct elf_phdr *nhdr, *phdr;
	struct elfhdr *elf;
	off_t offset = 0;
	const struct mrdump_machdesc *kparams = &mrdump_cb->machdesc;

	uint8_t *oldbufp = malloc(KDUMP_CORE_SIZE);
	uint8_t *bufp = oldbufp;

	/* setup ELF header */
	elf = (struct elfhdr *) bufp;
	bufp += sizeof(struct elfhdr);
	offset += sizeof(struct elfhdr);
	memcpy(elf->e_ident, ELFMAG, SELFMAG);
	elf->e_ident[EI_CLASS]	= ELFCLASS32;
	elf->e_ident[EI_DATA]	= ELFDATA2LSB;
	elf->e_ident[EI_VERSION]= EV_CURRENT;
	elf->e_ident[EI_OSABI] = ELFOSABI_NONE;
	memset(elf->e_ident+EI_PAD, 0, EI_NIDENT-EI_PAD);
	elf->e_type	= ET_CORE;
	elf->e_machine	= EM_ARM;
	elf->e_version	= EV_CURRENT;
	elf->e_entry	= 0;
	elf->e_phoff	= sizeof(struct elfhdr);
	elf->e_shoff	= 0;
	elf->e_flags	= ELF_CORE_EFLAGS;
	elf->e_ehsize	= sizeof(struct elfhdr);
	elf->e_phentsize= sizeof(struct elf_phdr);
	elf->e_phnum	= 2;
	elf->e_shentsize= 0;
	elf->e_shnum	= 0;
	elf->e_shstrndx	= 0;

	nhdr = (struct elf_phdr *) bufp;
	bufp += sizeof(struct elf_phdr);
	offset += sizeof(struct elf_phdr);
	memset(nhdr, 0, sizeof(struct elf_phdr));
	nhdr->p_type = PT_NOTE;

	phdr = (struct elf_phdr *) bufp;
	bufp += sizeof(struct elf_phdr);
	offset += sizeof(struct elf_phdr);

        uint32_t low_memory_size = kparams->high_memory - kparams->page_offset;
        if (low_memory_size > kmem_size) {
            low_memory_size = kmem_size;
        }
	phdr->p_type	= PT_LOAD;
	phdr->p_flags	= PF_R|PF_W|PF_X;
	phdr->p_offset	= KDUMP_CORE_SIZE;
	phdr->p_vaddr	= (size_t) kparams->page_offset;
	phdr->p_paddr	= kmem_address;
	phdr->p_filesz	= kmem_size;
	phdr->p_memsz	= low_memory_size;
	phdr->p_align	= KDUMP_CORE_SIZE;

	nhdr->p_offset = offset;

	struct elf_prpsinfo prpsinfo;	/* NT_PRPSINFO */
	struct memelfnote notes;
	/* set up the process info */
	notes.name = CORE_STR;
	notes.type = NT_PRPSINFO;
	notes.datasz = sizeof(struct elf_prpsinfo);
	notes.data = &prpsinfo;

	memset(&prpsinfo, 0, sizeof(struct elf_prpsinfo));
	prpsinfo.pr_state = 0;
	prpsinfo.pr_sname = 'R';
	prpsinfo.pr_zomb = 0;
	prpsinfo.pr_gid = prpsinfo.pr_uid = mrdump_cb->crash_record.fault_cpu + 1;
	strlcpy(prpsinfo.pr_fname, "vmlinux", sizeof(prpsinfo.pr_fname));
	strlcpy(prpsinfo.pr_psargs, "vmlinux", ELF_PRARGSZ);

	nhdr->p_filesz += notesize(&notes);
	bufp = storenote(&notes, bufp);

	bufp = kdump_core_write_machdesc(mrdump_cb, nhdr, bufp);

        /* Store pre-cpu backtrace */
        bufp = kdump_core_write_cpu_note(mrdump_cb, mrdump_cb->crash_record.fault_cpu, nhdr, bufp);
	for (unsigned int cpu = 0; cpu < kparams->nr_cpus; cpu++) {
            if (cpu != mrdump_cb->crash_record.fault_cpu) {
                bufp = kdump_core_write_cpu_note(mrdump_cb, cpu, nhdr, bufp);
            }
        }
	voprintf_debug("%s cpu %d header size %d\n", __FUNCTION__, kparams->nr_cpus, bufp - oldbufp);
    
	return oldbufp;
}

