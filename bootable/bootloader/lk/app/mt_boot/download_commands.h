#ifndef __DOWNLOAD_COMMANDS_H
#define __DOWNLOAD_COMMANDS_H
#include <platform/mt_typedefs.h>

#define __le64 u64
#define __le32 u32
#define __le16 u16

#define __u64 u64
#define __u32 u32
#define __u16 u16
#define __u8 u8

typedef __u16  __le16;
typedef __u16  __be16;
typedef __u32  __le32;
typedef __u32  __be32;
typedef __u64  __le64;
typedef __u64  __be64;

#define swab16(x) \
        ((__u16)( \
                (((__u16)(x) & (__u16)0x00ffU) << 8) | \
                (((__u16)(x) & (__u16)0xff00U) >> 8) ))
#define swab32(x) \
        ((__u32)( \
                (((__u32)(x) & (__u32)0x000000ffUL) << 24) | \
                (((__u32)(x) & (__u32)0x0000ff00UL) <<  8) | \
                (((__u32)(x) & (__u32)0x00ff0000UL) >>  8) | \
                (((__u32)(x) & (__u32)0xff000000UL) >> 24) ))

#define swab64(x) \
		((__u64)( \
				(((__u64)(x) & (__u64)0x00000000000000ffULL) << 56) | \
				(((__u64)(x) & (__u64)0x000000000000ff00ULL) << 40) | \
				(((__u64)(x) & (__u64)0x0000000000ff0000ULL) << 24) | \
				(((__u64)(x) & (__u64)0x00000000ff000000ULL) << 8) | \
				(((__u64)(x) & (__u64)0x000000ff00000000ULL) >> 8) | \
				(((__u64)(x) & (__u64)0x0000ff0000000000ULL) >> 24) | \
				(((__u64)(x) & (__u64)0x00ff000000000000ULL) >> 40) | \
				(((__u64)(x) & (__u64)0xff00000000000000ULL) >> 56) ))



#define cpu_to_be16(x) ({ __u16 _x = x; swab16(_x); })
#define cpu_to_be32(x) ({ __u32 _x = x; swab32(_x); })
#define cpu_to_be64(x) ({ __u64 _x = x; swab64(_x); })

#define be16_to_cpu(x) cpu_to_be16(x)
#define be32_to_cpu(x) cpu_to_be32(x)
#define be64_to_cpu(x) cpu_to_be64(x)
void cmd_install_sig(const char *arg, void *data, unsigned sz);
BOOL cmd_flash_emmc_img(const char *arg, void *data, unsigned sz);
BOOL cmd_flash_emmc_sparse_img(const char *arg, void *data, unsigned sz);
void cmd_flash_emmc(const char *arg, void *data, unsigned sz);
void cmd_erase_emmc(const char *arg, void *data, unsigned sz);
BOOL cmd_flash_nand_img(const char *arg, void *data, unsigned sz);
void cmd_flash_nand(const char *arg, void *data, unsigned sz);
void cmd_erase_nand(const char *arg, void *data, unsigned sz);

#ifdef MTK_EMMC_SUPPORT
#define CHECK_SIZE 4*1024
#else
#include <platform/mtk_nand.h>
#define CHECK_SIZE (12*1024) //support ubi img, more than 3 pages
//#define CHECK_SIZE (4*1024+128) 
#define PARTITION_INFO_OFFSET 24
#define YAFFS_TAG_OFFSET 8

//ubi struct

/* Erase counter header magic number (ASCII "UBI#") */
#define UBI_EC_HDR_MAGIC  0x55424923
/* Volume identifier header magic number (ASCII "UBI!") */
#define UBI_VID_HDR_MAGIC 0x55424921

#define UBI_INTERNAL_VOL_START (0x7FFFFFFF - 4096)

#define UBI_LAYOUT_VOLUME_ID     UBI_INTERNAL_VOL_START

#define UBI_VOL_NAME_MAX 127

struct ubi_ec_hdr {
	__be32  magic;
	__u8    version;
	__u8    padding1[3];
	__be64  ec; /* Warning: the current limit is 31-bit anyway! */
	__be32  vid_hdr_offset;
	__be32  data_offset;
	__be32  image_seq;
	__u8    padding2[32];
	__be32  hdr_crc;
};

struct ubi_vid_hdr {
	__be32  magic;
	__u8    version;
	__u8    vol_type;
	__u8    copy_flag;
	__u8    compat;
	__be32  vol_id;
	__be32  lnum;
	__be32  leb_ver;
	__be32  data_size;
	__be32  used_ebs;
	__be32  data_pad;
	__be32  data_crc;
	__u8    padding2[4];
	__be64  sqnum;
	__u8    padding3[12];
	__be32  hdr_crc;
};

struct ubi_vtbl_record {
	__be32  reserved_pebs;
	__be32  alignment;
	__be32  data_pad;
	__u8    vol_type;
	__u8    upd_marker;
	__be16  name_len;
	__u8    name[UBI_VOL_NAME_MAX+1];
	__u8    flags;
	__u8    padding[23];
	__be32  crc;
};
#endif

#define EXT4_SUPER_MAGIC 0xEF53

struct ext4_super_block {
  __le32 s_inodes_count;
 __le32 s_blocks_count_lo;
 __le32 s_r_blocks_count_lo;
 __le32 s_free_blocks_count_lo;
  __le32 s_free_inodes_count;
 __le32 s_first_data_block;
 __le32 s_log_block_size;
 __le32 s_obso_log_frag_size;
  __le32 s_blocks_per_group;
 __le32 s_obso_frags_per_group;
 __le32 s_inodes_per_group;
 __le32 s_mtime;
  __le32 s_wtime;
 __le16 s_mnt_count;
 __le16 s_max_mnt_count;
 __le16 s_magic;
 __le16 s_state;
 __le16 s_errors;
 __le16 s_minor_rev_level;
  __le32 s_lastcheck;
 __le32 s_checkinterval;
 __le32 s_creator_os;
 __le32 s_rev_level;
  __le16 s_def_resuid;
 __le16 s_def_resgid;

 __le32 s_first_ino;
 __le16 s_inode_size;
 __le16 s_block_group_nr;
 __le32 s_feature_compat;
  __le32 s_feature_incompat;
 __le32 s_feature_ro_compat;
  __u8 s_uuid[16];
  char s_volume_name[16];
  char s_last_mounted[64];
  __le32 s_algorithm_usage_bitmap;

 __u8 s_prealloc_blocks;
 __u8 s_prealloc_dir_blocks;
 __le16 s_reserved_gdt_blocks;

  __u8 s_journal_uuid[16];
  __le32 s_journal_inum;
 __le32 s_journal_dev;
 __le32 s_last_orphan;
 __le32 s_hash_seed[4];
 __u8 s_def_hash_version;
 __u8 s_reserved_char_pad;
 __le16 s_desc_size;
  __le32 s_default_mount_opts;
 __le32 s_first_meta_bg;
 __le32 s_mkfs_time;
 __le32 s_jnl_blocks[17];

  __le32 s_blocks_count_hi;
 __le32 s_r_blocks_count_hi;
 __le32 s_free_blocks_count_hi;
 __le16 s_min_extra_isize;
 __le16 s_want_extra_isize;
 __le32 s_flags;
 __le16 s_raid_stride;
 __le16 s_mmp_interval;
 __le64 s_mmp_block;
 __le32 s_raid_stripe_width;
 __u8 s_log_groups_per_flex;
 __u8 s_reserved_char_pad2;
 __le16 s_reserved_pad;
 __le64 s_kbytes_written;
 __u32 s_reserved[160];
};

#endif
