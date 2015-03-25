#ifndef ADDR_TRANS_H
#define ADDR_TRANS_H

//#include <platform/mt_typedefs.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct addr_trans_info {
    unsigned int id;
    unsigned long long len;
} addr_trans_info_t;

typedef struct addr_trans_tbl {
    unsigned int num;
    addr_trans_info_t *info;
} addr_trans_tbl_t;

typedef struct virt_addr {
    unsigned long long addr;
} virt_addr_t_addtrans;

typedef struct phys_addr {
    unsigned int id;
    unsigned long long addr;
} phys_addr_t_addtrans;

extern int virt_to_phys_addr(addr_trans_tbl_t *tbl, virt_addr_t_addtrans *virt, phys_addr_t_addtrans *phys);
extern int phys_to_virt_addr(addr_trans_tbl_t *tbl, phys_addr_t_addtrans *phys, virt_addr_t_addtrans *virt);

#ifdef __cplusplus
}
#endif

#endif /* ADDR_TRANS_H */

