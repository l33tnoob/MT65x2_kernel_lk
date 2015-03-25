#include <string.h>
#include <malloc.h>
#include <mt_partition.h>
#include <stdint.h>
#include <string.h>
#include <video.h>
#include <platform/mtk_key.h>
#include <platform/mtk_wdt.h>

#include "aee.h"
#include "kdump.h"
#include "kdump_elf.h"

static int null_write_cb(void *handle, void *buf, int size)
{
    return size;
}

int kdump_null_output(const struct mrdump_control_block *mrdump_cb, uint32_t total_dump_size)
{
    const struct mrdump_machdesc *kparams = &mrdump_cb->machdesc;
  
    voprintf_info("null dumping(address %x, size:%dM)\n", kparams->phys_offset, total_dump_size / 0x100000UL);
    mtk_wdt_restart();

    bool ok = true;
    void *bufp = kdump_core_header_init(mrdump_cb, (uint32_t)kparams->phys_offset, total_dump_size);
    if (bufp != NULL) {
        mtk_wdt_restart();
        struct kzip_file *zf = kzip_open(NULL, null_write_cb);
        if (zf != NULL) {
            struct kzip_memlist memlist[3];
            memlist[0].address = bufp;
            memlist[0].size = KDUMP_CORE_SIZE;
            memlist[1].address = kparams->phys_offset;
            memlist[1].size = total_dump_size;
            memlist[2].address = NULL;
            memlist[2].size = 0;
            kzip_add_file(zf, memlist, "SYS_COREDUMP");
            kzip_close(zf);
            zf = NULL;
            }
        else {
            ok = false;
        }
        free(bufp);
    }
    
    mtk_wdt_restart();
    if (ok) {
        voprintf_info("%s: dump finished, dumped.\n", __func__);
        mrdump_status_ok("NULL-OUTPUT\n");
    }
    return 0;
}
