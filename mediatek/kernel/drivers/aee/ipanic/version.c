#include <linux/version.h>
#include <linux/module.h>
#include <linux/mtd/mtd.h>
#include "ipanic.h"

struct ipanic_log_index ipanic_detail_start, ipanic_detail_end;

void ipanic_oops_start(void) 
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 5, 0)
    ipanic_detail_start.value = log_end;
#else
    ipanic_detail_start.idx = log_next_idx;
    ipanic_detail_start.seq = log_next_seq;
#endif
}

void ipanic_oops_end(void) 
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 5, 0)
    ipanic_detail_end.value = log_end;
#else
    ipanic_detail_end.idx = log_next_idx;
    ipanic_detail_end.seq = log_next_seq;
#endif
}

struct ipanic_log_index ipanic_get_log_start(void) 
{
    struct ipanic_log_index log_idx;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 5, 0)
#define __LOG_BUF_LEN (1 << CONFIG_LOG_BUF_SHIFT)
   if (log_end > __LOG_BUF_LEN)
       log_idx.value = log_end - __LOG_BUF_LEN;
   else
       log_idx.value = 0;
#else 
   log_idx.idx = log_first_idx;
   log_idx.seq = log_first_seq;
#endif
   return log_idx;
}

struct ipanic_log_index ipanic_get_log_end(void)
{
    struct ipanic_log_index log_idx;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 5, 0)
    log_idx.value = log_end;
#else
    log_idx.idx = log_next_idx;
    log_idx.seq = log_next_seq;
#endif
    return log_idx;
}

u8 *log_temp;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 5, 0)
/*
 * The function is used to dump kernel log after *Linux 3.5*
 */
size_t log_rest;
u8 *log_idx;
int ipanic_kmsg_dump3(struct kmsg_dumper *dumper, char *buf, size_t len)
{
    size_t rc = 0;
    size_t sum = 0;
    // if log_rest is no 0, it means that there some bytes left by previous log entry need to write to buf
    if (log_rest != 0) {
        memcpy(buf, log_idx, log_rest);
        sum += log_rest;
        log_rest = 0;
    }
    while (kmsg_dump_get_line_nolock(dumper, false, log_temp, len, &rc) && dumper->cur_seq <= dumper->next_seq) {
        if (sum + rc >= len) {
            memcpy(buf + sum, log_temp, len - sum);
            log_rest = rc - (len - sum);
            log_idx = log_temp + (len - sum);
            sum += (len - sum);
            return len;
        }
        memcpy(buf + sum, log_temp, rc);
        sum += rc;
    }
    return sum;
}
EXPORT_SYMBOL(ipanic_kmsg_dump3);
#else
extern int log_buf_copy2(char *dest, int dest_len, int log_copy_start, int log_copy_end);
#endif

#ifdef MTK_EMMC_SUPPORT
extern u8 *emmc_bounce;
extern int emmc_ipanic_write(void *buf, int off, int len);
int ipanic_write_log_buf(unsigned int off, struct ipanic_log_index start, struct ipanic_log_index end)
{
	int saved_oip;
	int rc, rc2;
	unsigned int last_chunk = 0, copy_count = 0;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 5, 0)
    int log_copy_start = start.value;
    int log_copy_end = end.value;
#else
    struct kmsg_dumper dumper;
    dumper.active = true;
    dumper.cur_idx = start.idx;
    dumper.cur_seq = start.seq;
    dumper.next_idx = end.idx;
    dumper.next_seq = end.seq;
#endif
	while (!last_chunk) {
		saved_oip = oops_in_progress;
		oops_in_progress = 1;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 5, 0)
        rc = log_buf_copy2(emmc_bounce, PAGE_SIZE, log_copy_start, log_copy_end);
		BUG_ON(rc < 0);
  		log_copy_start += rc;
#else
        rc = ipanic_kmsg_dump3(&dumper, emmc_bounce, PAGE_SIZE);
		BUG_ON(rc < 0);
#endif
		copy_count += rc;
		if (rc != PAGE_SIZE)
			last_chunk = rc;

		oops_in_progress = saved_oip;
		if (rc <= 0)
			break;

		rc2 = emmc_ipanic_write(emmc_bounce, off, rc);
		if (rc2 <= 0) {
			xlog_printk(ANDROID_LOG_ERROR, IPANIC_LOG_TAG,
			       "aee-ipanic: Flash write failed (%d)\n", rc2);
			return rc2;
		}
		off += rc2;
	}
	return copy_count;
}

#else
/*
 * Writes the contents of the console to the specified offset in flash.
 * Returns number of bytes written
 */
extern struct mtd_ipanic_data mtd_drv_ctx;
extern int mtd_ipanic_block_write(struct mtd_ipanic_data *ctx, loff_t to, int bounce_len);
int ipanic_write_log_buf(struct mtd_info *mtd, unsigned int off, struct ipanic_log_index start, struct ipanic_log_index end)  
{
	struct mtd_ipanic_data *ctx = &mtd_drv_ctx;
	int saved_oip;
	int rc, rc2;
	unsigned int last_chunk = 0, copy_count = 0;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 5, 0)
    int log_copy_start = start.value;
    int log_copy_end = end.value;
    if (log_copy_start == log_copy_end) {
      int rc3 = mtd_ipanic_block_write(ctx, off, 5);
      return rc3;
    }
#else
    struct kmsg_dumper dumper;
    dumper.active = true;
    dumper.cur_idx = start.idx;
    dumper.cur_seq = start.seq;
    dumper.next_idx = end.idx;
    dumper.next_seq = end.seq;
    if (dumper.cur_idx == dumper.next_idx) {
        int rc3 = mtd_ipanic_block_write(ctx, off, 5);
        return rc3; 
    } 
#endif
    
	while (!last_chunk) {
		saved_oip = oops_in_progress;
		oops_in_progress = 1;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 5, 0)		
		rc = log_buf_copy2(ctx->bounce, mtd->writesize, log_copy_start, log_copy_end);
		BUG_ON(rc < 0);
		log_copy_start += rc;
#else		
		rc = ipanic_kmsg_dump3(&dumper, ctx->bounce, PAGE_SIZE);
		BUG_ON(rc < 0);
#endif
		copy_count += rc;
		if (rc != mtd->writesize)
			last_chunk = rc;

		oops_in_progress = saved_oip;
		if (rc <= 0)
			break;

		rc2 = mtd_ipanic_block_write(ctx, off, rc);
		if (rc2 <= 0) {
			xlog_printk(ANDROID_LOG_ERROR, IPANIC_LOG_TAG,
			       "aee-ipanic: Flash write failed (%d)\n", rc2);
			return rc2;
		}
		off += rc2;
	}
	return copy_count;
}

#endif //MTK_EMMC_SUPPORT
