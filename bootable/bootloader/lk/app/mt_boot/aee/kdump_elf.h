/* 
 * (C) Copyright 2010
 * MediaTek <www.MediaTek.com>
 *
 * Android Exception Device
 *
 */

#if !defined(__KDUMP_ELF_H__)
#define __KDUMP_ELF_H__

#include <stdint.h>

#define CORE_STR "CORE"

#ifndef ELF_CORE_EFLAGS
#define ELF_CORE_EFLAGS	0
#endif

#define ELF_NGREGS 18

typedef unsigned long elf_greg_t;
typedef elf_greg_t elf_gregset_t[ELF_NGREGS];

#define ELF_NGREGS 18

typedef unsigned long elf_greg_t;
typedef elf_greg_t elf_gregset_t[ELF_NGREGS];

#define KDUMP_ARM_PAGE_SIZE 4096
#define KDUMP_CORE_SIZE (2 * KDUMP_ARM_PAGE_SIZE)

#define	ELFMAG		"\177ELF"
#define	SELFMAG		4

#define EI_NIDENT	16

#define	EI_CLASS	4
#define	EI_DATA		5
#define	EI_VERSION	6
#define	EI_OSABI	7
#define	EI_PAD		8

#define EM_ARM	40

#define ET_CORE   4

#define PT_LOAD    1
#define PT_NOTE    4

#define	ELFCLASS32	1

#define NT_PRSTATUS	1
#define NT_PRFPREG	2
#define NT_PRPSINFO	3

#define NT_MRDUMP_MACHDESC 0xAEE0

#define PF_R            0x4
#define PF_W            0x2
#define PF_X            0x1

#define ELFOSABI_NONE	0

#define EV_CURRENT	1

#define ELFDATA2LSB	1

typedef uint32_t Elf32_Addr;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef int32_t Elf32_Sword;
typedef uint32_t Elf32_Word;

typedef struct elfhdr {
  unsigned char	e_ident[EI_NIDENT];
  Elf32_Half	e_type;
  Elf32_Half	e_machine;
  Elf32_Word	e_version;
  Elf32_Addr	e_entry;  /* Entry point */
  Elf32_Off	e_phoff;
  Elf32_Off	e_shoff;
  Elf32_Word	e_flags;
  Elf32_Half	e_ehsize;
  Elf32_Half	e_phentsize;
  Elf32_Half	e_phnum;
  Elf32_Half	e_shentsize;
  Elf32_Half	e_shnum;
  Elf32_Half	e_shstrndx;
} Elf_Ehdr;

typedef struct elf_phdr {
  Elf32_Word	p_type;
  Elf32_Off	p_offset;
  Elf32_Addr	p_vaddr;
  Elf32_Addr	p_paddr;
  Elf32_Word	p_filesz;
  Elf32_Word	p_memsz;
  Elf32_Word	p_flags;
  Elf32_Word	p_align;
} Elf_Phdr;

typedef struct elf_note {
  Elf32_Word	n_namesz;	/* Name size */
  Elf32_Word	n_descsz;	/* Content size */
  Elf32_Word	n_type;		/* Content type */
} Elf_Nhdr;

#define ELF_PRARGSZ	(80)	/* Number of chars for args */

struct elf_siginfo
{
	int	si_signo;			/* signal number */
	int	si_code;			/* extra code */
	int	si_errno;			/* errno */
};

struct pt_regs 
{
    long uregs[18];
};

typedef unsigned long elf_greg_t;
typedef unsigned long elf_freg_t[3];

#define ELF_NGREG (sizeof (struct pt_regs) / sizeof(elf_greg_t))
typedef elf_greg_t elf_gregset_t[ELF_NGREG];

typedef struct user_fp elf_fpregset_t;

struct elf_timeval {
	int32_t	tv_sec;
	int32_t tv_usec;
};

struct elf_prstatus
{
    struct elf_siginfo pr_info;
    short pr_cursig;
    unsigned long pr_sigpend;
    unsigned long pr_sighold;

    int32_t pr_pid;
    int32_t pr_ppid;
    int32_t pr_pgrp;

    int32_t pr_sid;
    struct elf_timeval pr_utime;
    struct elf_timeval pr_stime;
    struct elf_timeval pr_cutime;
    struct elf_timeval pr_cstime;

    elf_gregset_t pr_reg;

    int pr_fpvalid;
};

struct elf_prpsinfo
{
    char pr_state;
    char pr_sname;
    char pr_zomb;
    char pr_nice;
    unsigned long pr_flag;

    uint16_t pr_uid;
    uint16_t pr_gid;

    int32_t pr_pid;
    int32_t pr_ppid;
    int32_t pr_pgrp;
    int32_t pr_sid;

    char pr_fname[16];
    char pr_psargs[ELF_PRARGSZ];
};

#define MRDUMP_TYPE_FULL_MEMORY 0
#define MRDUMP_TYPE_KERNEL_1 1
#define MRDUMP_TYPE_KERNEL_2 2

#define MRDUMP_TYPE_MASK 0x3

struct elf_mrdump_machdesc {
    uint32_t flags;

    uint32_t phys_offset;
    uint32_t total_memory;

    uint32_t page_offset;
    uint32_t high_memory;

    uint32_t modules_start;
    uint32_t modules_end;

    uint32_t vmalloc_start;
    uint32_t vmalloc_end;

};

#endif /* __KDUMP_ELF_H__ */
