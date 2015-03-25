#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "zlib.h"
#include "aee.h"
#include "kdump.h"

#define KZIP_DEBUG(x...)

#define KDUMP_SIZE_REPORT 0x400000
#define KDUMP_TICK_WDT 0x40000

static void putvalue(void* dest, uint64_t x, int nbByte)
{
    uint8_t *buf = (uint8_t *)dest;
    int n;
    for (n = 0; n < nbByte; n++) {
        buf[n] = (unsigned char)(x & 0xff);
        x >>= 8;
    }
#if 0
    if (x != 0)
    {     /* data overflow - hack for ZIP64 */
       for (n = 0; n < nbByte; n++)
       {
          buf[n] = 0xff;
       }
    }
#endif
}

static int put_localheader(uint8_t *buf, const char *filename, int level)
{
    uint8_t *oldbuf = buf;
    putvalue(buf, LOCALHEADERMAGIC, 4);
    buf += 4;
    putvalue(buf, 20UL, 2);
    buf += 2;

    uint16_t flag = 0;
    if ((level==8) || (level == 9))
        flag |= 2;
    if ((level == 2))
        flag |= 4;
    if ((level == 1))
        flag |= 6;
#if 0
    if (password != NULL)
        flag |= 1;
#endif

    putvalue(buf, flag | 0x8, 2);
    buf += 2;

    putvalue(buf, Z_DEFLATED, 2);
    buf += 2;

    putvalue(buf,0UL, 4);
    buf += 4;

  // CRC / Compressed size / Uncompressed size will be filled in later and rewritten later
    putvalue(buf, 0UL, 4); /* crc 32, unknown */
    buf += 4;
    putvalue(buf, 0UL, 4); /* compressed size, unknown */
    buf += 4;
    putvalue(buf, 0UL, 4); /* uncompressed size, unknown */
    buf += 4;
    putvalue(buf, strlen(filename), 2); /* size of filename */
    buf += 2;
    putvalue(buf, 12UL, 2); /* size of extra field */
    buf += 2;

    memcpy(buf, filename, strlen(filename));
    buf += strlen(filename);

    /* Reserve local header room for crypt data */
    memcpy(buf, "---+---+---+", 12);
    buf += 12;

    return buf - oldbuf;
}

static int put_centralheader(uint8_t *buf, const char *filename, int level, uint64_t zip_localheader_offset, uint32_t size, uint32_t uncomp_size, uint32_t crc32_value)
{
    uint8_t *oldbuf = buf;

    putvalue(buf, CENTRALHEADERMAGIC, 4);
    buf += 4;
    putvalue(buf, 20UL, 2);
    buf += 2;
    putvalue(buf, 20UL, 2);
    buf += 2;

    uint16_t flag = 0;
    if ((level==8) || (level == 9))
        flag |= 2;
    if ((level == 2))
        flag |= 4;
    if ((level == 1))
        flag |= 6;
#if 0
    if (password != NULL)
        flag |= 1;
#endif

    putvalue(buf, flag | 0x8, 2);
    buf += 2;

    putvalue(buf, Z_DEFLATED, 2);
    buf += 2;

    putvalue(buf,0UL, 4);
    buf += 4;

    // CRC / Compressed size / Uncompressed size will be filled in later and rewritten later
    putvalue(buf, crc32_value, 4); /* crc 32, unknown */
    buf += 4;
    putvalue(buf, size, 4); /* compressed size, unknown */
    buf += 4;
    putvalue(buf, uncomp_size, 4); /* uncompressed size, unknown */
    buf += 4;
    putvalue(buf, strlen(filename), 2);
    buf += 2;
    putvalue(buf, 0UL, 2); /* size of extra field */
    buf += 2;
    putvalue(buf, 0UL, 2); /* size of comment field */
    buf += 2;
    putvalue(buf, 0UL, 2); /* disk number */
    buf += 2;
    putvalue(buf, 0UL, 2); /* internal attributes */
    buf += 2;
    putvalue(buf, 0UL, 4); /* external file attributes */
    buf += 4;
    putvalue(buf, zip_localheader_offset, 4); /* Relative offset */
    buf += 4;
    memcpy(buf, filename, strlen(filename));
    buf += strlen(filename);

    return buf - oldbuf;
}

static int put_eoc_directory_record(uint8_t *buf, int record_num, uint64_t zip_centralheader_offset, int zip_centralheader_size)
{
    uint8_t *oldbuf = buf;
    putvalue(buf, ENDOFCENTRALDIRMAGIC, 4);
    buf += 4;
    putvalue(buf, 0, 2); /* Number of this disk */
    buf += 2;
    putvalue(buf, 0, 2); /* Disk where central directory starts */
    buf += 2;
    putvalue(buf, record_num, 2); /* Number of central directory records on this disk */
    buf += 2;
    putvalue(buf, record_num, 2); /* Total number of central directory records */
    buf += 2;
    putvalue(buf, zip_centralheader_size, 4);
    buf += 4;
    putvalue(buf, zip_centralheader_offset, 4);
    buf += 4;
    putvalue(buf, 0, 2); /* Comment length (n) */
    buf += 2;
    return buf - oldbuf;
}

static int kzip_write_current(struct kzip_file *zfile, void *buf, int len)
{
    KZIP_DEBUG("%s: write_cb %p len %d\n", __func__, zfile->write_cb, len);
    int retval = zfile->write_cb(zfile->handle, buf, len);
    if (retval > 0) {
	zfile->current_size += retval;
    }
    if ((zfile->current_size - zfile->reported_size) >= KDUMP_SIZE_REPORT) {
        voprintf_info(".. Written %dM\n", zfile->current_size / 0x100000);
        zfile->reported_size = zfile->current_size;
    }
    if ((zfile->current_size - zfile->wdk_kick_size) >= KDUMP_TICK_WDT) {
        mtk_wdt_restart();
        zfile->wdk_kick_size = zfile->current_size;
    }
    return retval;
}

struct kzip_file *kzip_open(void *handle, int (*write_cb)(void *handle, void *p, int size))
{
    struct kzip_file *zf = malloc(sizeof(struct kzip_file));
    memset(zf, 0, sizeof(struct kzip_file));
    zf->handle = handle;
    zf->write_cb = write_cb;
    
    KZIP_DEBUG("%s: handle zf %p %p write_cb %p\n", __func__, zf, zf->handle, zf->write_cb);
    return zf;
}

bool kzip_close(struct kzip_file *zf)
{
    int32_t current_size = zf->current_size;
    struct kzip_entry *zentries = zf->zentries;
    int num = zf->entries_num;

    int i, hsize = 0;
    uint8_t databuf[128];
    for (i = 0; i < num; i++) {
	int local_hsize = put_centralheader(databuf, zentries[i].filename, zentries[i].level, 
					    zentries[i].localheader_offset, 
					    zentries[i].comp_size, 
					    zentries[i].uncomp_size, 
					    zentries[i].crc32);
	if (kzip_write_current(zf, databuf, local_hsize) != local_hsize) {
	    return false;
	}
	hsize += local_hsize;
    }
    hsize = put_eoc_directory_record(databuf, num, current_size, hsize);
    if (kzip_write_current(zf, databuf, hsize) != hsize) {
        return false;
    }
    free(zf);
    return true;
}

#define CHUNK 131072

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */

bool kzip_add_file(struct kzip_file *zfile, const struct kzip_memlist *memlist, const char *zfilename)
{
    int ret, flush;
    z_stream strm;
    struct aee_timer zip_time;

    if (zfile->entries_num >= KZIP_ENTRY_MAX) {
	voprintf_error("Too manry zip entry %d\n", zfile->entries_num);
	return false;
    }

    voprintf_debug("%s: zf %p(%p) %s\n", __func__, zfile, zfile->write_cb, zfilename);
    struct kzip_entry *zentry = &zfile->zentries[zfile->entries_num++];
    zentry->filename = strdup(zfilename);
    zentry->localheader_offset = zfile->current_size;
    zentry->level = DEF_MEM_LEVEL;

    KZIP_DEBUG("%s: write local header\n", __func__);
    uint8_t zip_localheader[128];
    int hsize = put_localheader(zip_localheader, zfilename, DEF_MEM_LEVEL);
    if (kzip_write_current(zfile, zip_localheader, hsize) != hsize) {
        return false;
    }

    KZIP_DEBUG("%s: init compressor\n", __func__);
    /* allocate deflate state */
    strm.workspace = malloc(zlib_deflate_workspacesize(-MAX_WBITS, DEF_MEM_LEVEL));
    ret = zlib_deflateInit2(&strm, zentry->level, Z_DEFLATED, -MAX_WBITS,
			    DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY);
    if (ret != Z_OK) {
	voprintf_error("zlib compress init failed\n");
	free(strm.workspace);
        return false;
    }

    uint8_t *out = malloc(CHUNK);
 
    aee_timer_init(&zip_time);
    aee_timer_start(&zip_time);
    /* compress until end of file */
    do {
        flush = (memlist->size == 0) ? Z_FINISH : Z_NO_FLUSH;
 	
	KZIP_DEBUG("-- Compress memory %x, size %d\n", memlist->address, memlist->size);
	strm.avail_in = memlist->size;
        strm.next_in = memlist->address;
        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = zlib_deflate(&strm, flush);    /* no bad return value */
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            int have = CHUNK - strm.avail_out;
	    if (have > 0) {
                aee_timer_stop(&zip_time);
		if (kzip_write_current(zfile, out, have) != have) {
                    goto error;
		}
                aee_timer_start(&zip_time);
	    }
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);     /* all input will be used */

	memlist++;
        /* done when last data in file processed */
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (void)zlib_deflateEnd(&strm);
    free(strm.workspace);
    free(out);
    aee_timer_stop(&zip_time);
    voprintf_info("Zip time: %d sec\n", zip_time.acc_ms / 1000);

    zentry->comp_size = strm.total_out;
    zentry->uncomp_size = strm.total_in;
    zentry->crc32 = strm.crc32 ^ 0xffffffffUL;

    return true;

 error:
    free(strm.workspace);
    free(out);
    (void)zlib_deflateEnd(&strm);
    return false;
}
