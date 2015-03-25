/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <errno.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#if defined(HAVE_ANDROID_OS) && !defined(ARCH_X86) //wschen 2012-07-10
#include "include/linux/mmc/sd_misc.h"
#endif
//#include <fs_mgr.h>  //tonykuo 2013-12-05
#include "mtdutils/mtdutils.h"
#include "mtdutils/mounts.h"
#include "roots.h"
#include "common.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "make_ext4fs.h"
#ifdef __cplusplus
}
#endif

#include <time.h>
#include "libubi.h"
#include "ubiutils-common.h"
#include "util.h"

#define DEFAULT_CTRL_DEV "/dev/ubi_ctrl"

//static struct fstab *fstab = NULL;
static int num_volumes = 0;
static Volume* device_volumes = NULL;

extern struct selabel_handle *sehandle;

#if defined(CACHE_MERGE_SUPPORT)
#include <dirent.h>
#include "backup_restore.h"

static int need_clear_cache = 0;
static const char *DATA_CACHE_ROOT = "/data/.cache";

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
static int remove_dir(const char *dirname)
{
    DIR *dir;
    struct dirent *entry;
    char path[PATH_MAX];

    dir = opendir(dirname);
    if (dir == NULL) {
        LOGE("opendir %s failed\n", dirname);
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
            snprintf(path, (size_t) PATH_MAX, "%s/%s", dirname, entry->d_name);
            if (entry->d_type == DT_DIR) {
                remove_dir(path);
            }
            else {
                // delete file
                unlink(path);
            }
        }
    }
    closedir(dir);

    // now we can delete the empty dir
    rmdir(dirname);
    return 0;
}
#endif


static int parse_options(char* options, Volume* volume) {
    char* option;
    while ((option = strtok(options, ","))) {
        options = NULL;

        if (strncmp(option, "length=", 7) == 0) {
            volume->length = strtoll(option+7, NULL, 10);
        } else {
            LOGE("bad option \"%s\"\n", option);
            return -1;
        }
    }
    return 0;
}

#if defined(UBIFS_SUPPORT)
#define UBI_EC_HDR_MAGIC  0x55424923
int ubifs_exist(const char *part_name)
{
    const MtdPartition *partition;
    MtdReadContext *mtd_read;
    char buf[64] = {0};
    __u32 *magic;

    mtd_scan_partitions();
    partition = mtd_find_partition_by_name(part_name);
    if (partition == NULL) {
        fprintf(stderr,"1. failed to find \"%s\" partition\n", part_name);
        return 0;
    }
    mtd_read = mtd_read_partition(partition);
    if(mtd_read == NULL) {
        fprintf(stderr,"2. failed to open \"%s\" partition\n", part_name);
        return 0;
    }
    if(64 != mtd_read_data(mtd_read, buf, 64)) {
        fprintf(stderr,"3. failed to read \"%s\" partition\n", part_name);
        mtd_read_close(mtd_read);
        return 0;
    }
    mtd_read_close(mtd_read);
    magic = (__u32 *)buf;
    if(*magic ==  UBI_EC_HDR_MAGIC)
        return 1;

    return 0;
}
#endif


void load_volume_table() {
    int alloc = 2;
    device_volumes = (Volume*)malloc(alloc * sizeof(Volume));

    // Insert an entry for /tmp, which is the ramdisk and is always mounted.
    device_volumes[0].mount_point = "/tmp";
    device_volumes[0].fs_type = "ramdisk";
    device_volumes[0].device = NULL;
    device_volumes[0].device2 = NULL;
    device_volumes[0].length = 0;
    num_volumes = 1;

#if 1 //wschen 2011-12-23 auto workaround if recovery.fstab is wrong, phone is eMMC but recovery.fstab is NAND
#define NAND_TYPE    0
#define EMMC_TYPE    1
#define UNKNOWN_TYPE 2

#define CACHE_INDEX  0
#define DATA_INDEX   1
#define SYSTEM_INDEX 2
#define FAT_INDEX    3
#define CUSTOM_INDEX 4

    int setting_ok = 0;
    int phone_type = 0, fstab_type = UNKNOWN_TYPE;
    int has_fat = 0;
    FILE *fp_fstab, *fp_info;
    char buf[512];
    char p_name[32], p_size[32], p_addr[32], p_actname[64];
    char dev[5][64];
    unsigned int p_type;
    int j;
    int has_custom = 0;
    int new_fstab = 0;

    fp_info = fopen("/proc/dumchar_info", "r");
    if (fp_info) {
        //ignore the header line
        if (fgets(buf, sizeof(buf), fp_info) != NULL) {
            printf("Partition Information:\n");
            while (fgets(buf, sizeof(buf), fp_info)) {
                printf("%s", buf);
                if (sscanf(buf, "%s %s %s %d %s", p_name, p_size, p_addr, &p_type, p_actname) == 5) {
                    if (!strcmp(p_name, "bmtpool")) {
                        break;
                    }
                    if (!strcmp(p_name, "preloader")) {
                        if (p_type == 2) {
                            phone_type = EMMC_TYPE;
                        } else {
                            phone_type = NAND_TYPE;
                        }
                    } else if (!strcmp(p_name, "fat")) {
                        has_fat = 1;
                        snprintf(dev[FAT_INDEX], sizeof(dev[FAT_INDEX]), "%s", p_actname);
                    } else if (!strcmp(p_name, "cache")) {
                        snprintf(dev[CACHE_INDEX], sizeof(dev[CACHE_INDEX]), "%s", p_actname);
                    } else if (!strcmp(p_name, "usrdata")) {
                        snprintf(dev[DATA_INDEX], sizeof(dev[DATA_INDEX]), "%s", p_actname);
                    } else if (!strcmp(p_name, "android")) {
                        snprintf(dev[SYSTEM_INDEX], sizeof(dev[SYSTEM_INDEX]), "%s", p_actname);
                    } else if (!strcmp(p_name, "custom")) {
                        snprintf(dev[CUSTOM_INDEX], sizeof(dev[CUSTOM_INDEX]), "%s", p_actname);
                        has_custom = 1;
                    }
                }
            }
        }
        fclose(fp_info);
        printf("\n");

        fp_fstab = fopen("/etc/recovery.fstab", "r");
        if (fp_fstab) {
            while (fgets(buf, sizeof(buf), fp_fstab)) {
                for (j = 0; buf[j] && isspace(buf[j]); ++j) {
                }

                if (buf[j] == '\0' || buf[j] == '#') {
                    continue;
                }
                if (strstr(&buf[j], "/boot") == (&buf[j])) {
                    j += strlen("/boot");
                    for (; buf[j] && isspace(buf[j]); ++j) {
                    }
                    if (strstr(&buf[j], "emmc") == (&buf[j])) {
                        fstab_type = EMMC_TYPE;
                    } else if (strstr(&buf[j], "mtd") == (&buf[j])) {
                        fstab_type = NAND_TYPE;
                    }
                    break;
                }else if(strstr(&buf[j], "boot") == (&buf[j])){
                    //tonykuo 2013-11-28 
                    new_fstab =1;
                    j += strlen("boot");
                    for (; buf[j] && isspace(buf[j]); ++j) {
                    }
                    if (strstr(&buf[j], "/boot") == (&buf[j])) {
                        j += strlen("/boot");
                        for (; buf[j] && isspace(buf[j]); ++j) {
                        }
                        if (strstr(&buf[j], "emmc") == (&buf[j])) {
                            fstab_type = EMMC_TYPE;
                        } else if (strstr(&buf[j], "mtd") == (&buf[j])) {
                            fstab_type = NAND_TYPE;
                        }
                        break;
                    }
               }
            }

            fclose(fp_fstab);

            if (fstab_type != UNKNOWN_TYPE) {
                if (phone_type != fstab_type) {
                    printf("WARNING : fstab is wrong, phone=%s fstab=%s\n", phone_type == EMMC_TYPE ? "eMMC" : "NAND", fstab_type == EMMC_TYPE ? "eMMC" : "NAND");

#if defined(SUPPORT_SDCARD2) && !defined(MTK_SHARED_SDCARD) //wschen 2012-11-15
                    if (has_custom) {
                        device_volumes = (Volume*)realloc(device_volumes, 10 * sizeof(Volume));
                        num_volumes = 10;
                    } else {
                        device_volumes = (Volume*)realloc(device_volumes, 9 * sizeof(Volume));
                        num_volumes = 9;
                    }
#else
                    if (has_custom) {
                        device_volumes = (Volume*)realloc(device_volumes, 9 * sizeof(Volume));
                        num_volumes = 9;
                    } else {
                        device_volumes = (Volume*)realloc(device_volumes, 8 * sizeof(Volume));
                        num_volumes = 8;
                    }
#endif //SUPPORT_SDCARD2

                    if (phone_type == EMMC_TYPE) {
                        //boot
                        device_volumes[1].mount_point = strdup("/boot");
                        device_volumes[1].fs_type = strdup("emmc");
                        device_volumes[1].device = strdup("boot");
                        device_volumes[1].device2 = NULL;
                        device_volumes[1].length = 0;

                        //cache
#if defined(CACHE_MERGE_SUPPORT)
                        device_volumes[2].mount_point = strdup("/.cache");
#else
                        device_volumes[2].mount_point = strdup("/cache");
#endif
                        device_volumes[2].fs_type = strdup("ext4");
                        device_volumes[2].device = strdup(dev[CACHE_INDEX]);
                        device_volumes[2].device2 = NULL;
                        device_volumes[2].length = 0;

                        //data
                        device_volumes[3].mount_point = strdup("/data");
                        device_volumes[3].fs_type = strdup("ext4");
                        device_volumes[3].device = strdup(dev[DATA_INDEX]);
                        device_volumes[3].device2 = NULL;
                        device_volumes[3].length = 0;

                        //misc
                        device_volumes[4].mount_point = strdup("/misc");
                        device_volumes[4].fs_type = strdup("emmc");
                        device_volumes[4].device = strdup("misc");
                        device_volumes[4].device2 = NULL;
                        device_volumes[4].length = 0;

                        //recovery
                        device_volumes[5].mount_point = strdup("/recovery");
                        device_volumes[5].fs_type = strdup("emmc");
                        device_volumes[5].device = strdup("recovery");
                        device_volumes[5].device2 = NULL;
                        device_volumes[5].length = 0;

                        //sdcard
                        if (has_fat) {
                            device_volumes[6].mount_point = strdup("/sdcard");
                            device_volumes[6].fs_type = strdup("vfat");
#if defined(MTK_LCA_ROM_OPTIMIZE)
                            device_volumes[6].device = strdup("/dev/block/mmcblk1p1");
                            device_volumes[6].device2 = strdup("/dev/block/mmcblk1");
#else
#if defined(MTK_SHARED_SDCARD) || defined(MTK_2SDCARD_SWAP)
                            device_volumes[6].device = strdup("/dev/block/mmcblk1p1");
                            device_volumes[6].device2 = strdup(dev[FAT_INDEX]);
#else
                            device_volumes[6].device = strdup(dev[FAT_INDEX]);
                            device_volumes[6].device2 = strdup("/dev/block/mmcblk1p1");
#endif
#endif
                            device_volumes[6].length = 0;
                        } else {
                            device_volumes[6].mount_point = strdup("/sdcard");
                            device_volumes[6].fs_type = strdup("vfat");
                            device_volumes[6].device = strdup("/dev/block/mmcblk1p1");
                            device_volumes[6].device2 = strdup("/dev/block/mmcblk1");
                            device_volumes[6].length = 0;
                        }

                        //system
                        device_volumes[7].mount_point = strdup("/system");
                        device_volumes[7].fs_type = strdup("ext4");
                        device_volumes[7].device = strdup(dev[SYSTEM_INDEX]);
                        device_volumes[7].device2 = NULL;
                        device_volumes[7].length = 0;

                        if (has_custom) {
                            //custom
                            device_volumes[8].mount_point = strdup("/custom");
                            device_volumes[8].fs_type = strdup("ext4");
                            device_volumes[8].device = strdup(dev[CUSTOM_INDEX]);
                            device_volumes[8].device2 = NULL;
                            device_volumes[8].length = 0;
                        }

#if defined(SUPPORT_SDCARD2) && !defined(MTK_SHARED_SDCARD) //wschen 2012-11-15
                        if (has_custom) {
                            //sdcard2
                            if (has_fat) {
                                device_volumes[9].mount_point = strdup("/sdcard2");
                                device_volumes[9].fs_type = strdup("vfat");
#if defined(MTK_LCA_ROM_OPTIMIZE)
                                device_volumes[9].device = strdup("/dev/block/mmcblk1p1");
                                device_volumes[9].device2 = strdup("/dev/block/mmcblk1");
#else
#if defined(MTK_2SDCARD_SWAP)
                                device_volumes[9].device = strdup(dev[FAT_INDEX]);
                                device_volumes[9].device2 = NULL;
#else
                                device_volumes[9].device = strdup("/dev/block/mmcblk1p1");
                                device_volumes[9].device2 = strdup("/dev/block/mmcblk1");
#endif
#endif
                                device_volumes[9].length = 0;
                            } else {
                                //no 2nd SD
                                num_volumes--;
                            } // has_fat
                        } else {
                            //sdcard2
                            if (has_fat) {
                                device_volumes[8].mount_point = strdup("/sdcard2");
                                device_volumes[8].fs_type = strdup("vfat");
#if defined(MTK_LCA_ROM_OPTIMIZE)
                                device_volumes[8].device = strdup("/dev/block/mmcblk1p1");
                                device_volumes[8].device2 = strdup("/dev/block/mmcblk1");
#else
#if defined(MTK_2SDCARD_SWAP)
                                device_volumes[8].device = strdup(dev[FAT_INDEX]);
                                device_volumes[8].device2 = NULL;
#else
                                device_volumes[8].device = strdup("/dev/block/mmcblk1p1");
                                device_volumes[8].device2 = strdup("/dev/block/mmcblk1");
#endif
#endif
                                device_volumes[8].length = 0;
                            } else {
                                //no 2nd SD
                                num_volumes--;
                            } // has_fat

                        }
#endif //SUPPORT_SDCARD2

                    } else {
                        //boot
                        device_volumes[1].mount_point = strdup("/boot");
                        device_volumes[1].fs_type = strdup("mtd");
                        device_volumes[1].device = strdup("boot");
                        device_volumes[1].device2 = NULL;
                        device_volumes[1].length = 0;

                        //cache
#if defined(CACHE_MERGE_SUPPORT)
                        device_volumes[2].mount_point = strdup("/.cache");
#else
                        device_volumes[2].mount_point = strdup("/cache");
#endif

#if defined(UBIFS_SUPPORT)
                        if(ubifs_exist("cache"))
                            device_volumes[2].fs_type = strdup("ubifs");
                        else
                            device_volumes[2].fs_type = strdup("yaffs2");
#else
                            device_volumes[2].fs_type = strdup("yaffs2");
#endif

#if defined(UBIFS_SUPPORT)
                        if(!strcmp(device_volumes[2].fs_type, "yaffs2"))
                            device_volumes[2].device = strdup("cache");
                        else
                            device_volumes[2].device = strdup(dev[CACHE_INDEX]);
#else
                            device_volumes[2].device = strdup("cache");
#endif
                        device_volumes[2].device2 = NULL;
                        device_volumes[2].length = 0;

                        //data
                        device_volumes[3].mount_point = strdup("/data");

#if defined(UBIFS_SUPPORT)
                        device_volumes[3].fs_type = strdup("ubifs");
#else
                        device_volumes[3].fs_type = strdup("yaffs2");
#endif

#if defined(UBIFS_SUPPORT)
                        //device_volumes[3].device = strdup("/dev/mtd/mtd13");
                        device_volumes[3].device = strdup(dev[DATA_INDEX]);
#else
                        device_volumes[3].device = strdup("userdata");
#endif

                        device_volumes[3].device2 = NULL;
                        device_volumes[3].length = 0;

                        //misc
                        device_volumes[4].mount_point = strdup("/misc");
                        device_volumes[4].fs_type = strdup("mtd");
                        device_volumes[4].device = strdup("misc");
                        device_volumes[4].device2 = NULL;
                        device_volumes[4].length = 0;

                        //recovery
                        device_volumes[5].mount_point = strdup("/recovery");
                        device_volumes[5].fs_type = strdup("mtd");
                        device_volumes[5].device = strdup("recovery");
                        device_volumes[5].device2 = NULL;
                        device_volumes[5].length = 0;

                        //sdcard
                        device_volumes[6].mount_point = strdup("/sdcard");
                        device_volumes[6].fs_type = strdup("vfat");
                        device_volumes[6].device = strdup("/dev/block/mmcblk0p1");
                        device_volumes[6].device2 = strdup("/dev/block/mmcblk0");
                        device_volumes[6].length = 0;

                        //system
                        device_volumes[7].mount_point = strdup("/system");

#if defined(UBIFS_SUPPORT)
                        device_volumes[7].fs_type = strdup("ubifs");
#else
                        device_volumes[7].fs_type = strdup("yaffs2");
#endif

#if defined(UBIFS_SUPPORT)
                        //device_volumes[7].device = strdup("/dev/mtd/mtd11");
                        device_volumes[7].device = strdup(dev[SYSTEM_INDEX]);
#else
                        device_volumes[7].device = strdup("system");
#endif

                        device_volumes[7].device2 = NULL;
                        device_volumes[7].length = 0;

                        if (has_custom) {
                            //custom
                        device_volumes[8].mount_point = strdup("/custom");

#if defined(UBIFS_SUPPORT)
                        device_volumes[8].fs_type = strdup("ubifs");
#else
                        device_volumes[8].fs_type = strdup("yaffs2");
#endif

#if defined(UBIFS_SUPPORT)
                        device_volumes[8].device = strdup(dev[CUSTOM_INDEX]);
#else
                        device_volumes[8].device = strdup("custom");
#endif

                        device_volumes[8].device2 = NULL;
                        device_volumes[8].length = 0;
                        }

#if defined(SUPPORT_SDCARD2) && !defined(MTK_SHARED_SDCARD) //wschen 2012-11-15
                        //NAND no 2nd SD
                        num_volumes--;
#endif //SUPPORT_SDCARD2

                    }

                    printf("recovery filesystem table\n");
                    printf("=========================\n");
                    for (j = 0; j < num_volumes; ++j) {
                        Volume* v = &device_volumes[j];
                        printf("  %d %s %s %s %s %lld\n", j, v->mount_point, v->fs_type, v->device, v->device2, v->length);
                    }
                    printf("\n");
                    return;
                }
            } else {
                printf("fstab type setting is wrong\n");
            }
        }

    } else {
        printf("Fail to open /proc/dumchar_info\n");
    }
#endif

    FILE* fstab = fopen("/etc/recovery.fstab", "r");
    if (fstab == NULL) {
        LOGE("failed to open /etc/recovery.fstab (%s)\n", strerror(errno));
        return;
    }

    char buffer[1024];
    int i;
    int fstab_has_custom = 0;

    while (fgets(buffer, sizeof(buffer)-1, fstab)) {
        for (i = 0; buffer[i] && isspace(buffer[i]); ++i);
        if (buffer[i] == '\0' || buffer[i] == '#') continue;
        //tonykuo 2013-11-28 
        char* original;
        char* device;
        char* mount_point;
        char* fs_type;
        char* options;
        char* device2;

        //tonykuo 2013-11-28 
        if(new_fstab){
            original = strdup(buffer);

            device = strtok(buffer+i, " \t\n");
            mount_point = strtok(NULL, " \t\n");
            fs_type = strtok(NULL, " \t\n");
            char* defualt1 = strtok(NULL, " \t\n");
            char* defualt2 = strtok(NULL, " \t\n");
            // lines may optionally have a second device, to use if
            // mounting the first one fails.
            options = NULL;
            device2 = strtok(NULL, " \t\n");
            if (device2) {
                if (device2[0] == '/') {
                    options = strtok(NULL, " \t\n");
                } else {
                    options = device2;
                    device2 = NULL;
                }
            }
        }else{
            original = strdup(buffer);

            mount_point = strtok(buffer+i, " \t\n");
            fs_type = strtok(NULL, " \t\n");
            device = strtok(NULL, " \t\n");
            // lines may optionally have a second device, to use if
            // mounting the first one fails.
            options = NULL;
            device2 = strtok(NULL, " \t\n");
            if (device2) {
                if (device2[0] == '/') {
                    options = strtok(NULL, " \t\n");
                } else {
                    options = device2;
                    device2 = NULL;
                }
            }
        }


#if 1 //wschen 2013-01-31
        if (mount_point && !strcmp(mount_point, "/custom")) {
            fstab_has_custom = 1;
        }
#endif

        if (mount_point && fs_type && device) {
            while (num_volumes >= alloc) {
                alloc *= 2;
                device_volumes = (Volume*)realloc(device_volumes, alloc*sizeof(Volume));
            }

#if defined(CACHE_MERGE_SUPPORT)
            if (!strcmp(mount_point, "/cache")) {
                mount_point = "/.cache";
            }
#endif
            device_volumes[num_volumes].mount_point = strdup(mount_point);

#if defined(UBIFS_SUPPORT)
            if (!strcmp(mount_point, "/cache") || !strcmp(mount_point, "/.cache")) {
                if(ubifs_exist("cache")){
                    device_volumes[num_volumes].fs_type = strdup("ubifs");
                }else{
                    device_volumes[num_volumes].fs_type = strdup("yaffs2");
                }
            }else{
                device_volumes[num_volumes].fs_type = strdup(fs_type);
            }
#else
            device_volumes[num_volumes].fs_type = strdup(fs_type);
#endif

#if 0 //wschen 2012-05-15
            device_volumes[num_volumes].device = strdup(device);
            device_volumes[num_volumes].device2 =
                device2 ? strdup(device2) : NULL;
#else

            if (phone_type == EMMC_TYPE) {
#if defined(CACHE_MERGE_SUPPORT)
                if (!strcmp(mount_point, "/.cache") && strcmp(device, dev[CACHE_INDEX])) {
#else
                if (!strcmp(mount_point, "/cache") && strcmp(device, dev[CACHE_INDEX])) {
#endif
                    device_volumes[num_volumes].device = strdup(dev[CACHE_INDEX]);
                    device_volumes[num_volumes].device2 = device2 ? strdup(device2) : NULL;
                } else if (!strcmp(mount_point, "/data") && strcmp(device, dev[DATA_INDEX])) {
                    device_volumes[num_volumes].device = strdup(dev[DATA_INDEX]);
                    device_volumes[num_volumes].device2 = device2 ? strdup(device2) : NULL;
                } else if (!strcmp(mount_point, "/system") && strcmp(device, dev[SYSTEM_INDEX])) {
                    device_volumes[num_volumes].device = strdup(dev[SYSTEM_INDEX]);
                    device_volumes[num_volumes].device2 = device2 ? strdup(device2) : NULL;
                } else if (!strcmp(mount_point, "/custom") && strcmp(device, dev[CUSTOM_INDEX])) {
                    device_volumes[num_volumes].device = strdup(dev[CUSTOM_INDEX]);
                    device_volumes[num_volumes].device2 = device2 ? strdup(device2) : NULL;
#if defined(MTK_LCA_ROM_OPTIMIZE)
                 } else if (!strcmp(mount_point, "/sdcard")) {
                    device_volumes[num_volumes].device = strdup("/dev/block/mmcblk1p1");
                    device_volumes[num_volumes].device2 = strdup("/dev/block/mmcblk1");
#else
#if defined(MTK_SHARED_SDCARD) || defined(MTK_2SDCARD_SWAP)
                } else if (!strcmp(mount_point, "/sdcard")) {
                    device_volumes[num_volumes].device = strdup("/dev/block/mmcblk1p1");
                    if (has_fat) {
                        device_volumes[num_volumes].device2 = strdup(dev[FAT_INDEX]);
                    } else {
                    device_volumes[num_volumes].device2 = strdup("/dev/block/mmcblk1");
                    }
#else
                } else if (!strcmp(mount_point, "/sdcard") && has_fat && strcmp(device, dev[FAT_INDEX])) {
                    device_volumes[num_volumes].device = strdup(dev[FAT_INDEX]);
                    device_volumes[num_volumes].device2 = device2 ? strdup(device2) : NULL;
#endif
#endif
                } else {
                    device_volumes[num_volumes].device = strdup(device);
                    device_volumes[num_volumes].device2 = device2 ? strdup(device2) : NULL;
                }

            } else {
                    //device_volumes[num_volumes].device = strdup(device);
                    //device_volumes[num_volumes].device2 = device2 ? strdup(device2) : NULL;

//Tony
#if defined(CACHE_MERGE_SUPPORT)
                if (!strcmp(mount_point, "/.cache") && strcmp(device, dev[CACHE_INDEX])) {
#else
                if (!strcmp(mount_point, "/cache") && strcmp(device, dev[CACHE_INDEX])) {
#endif
#if defined(UBIFS_SUPPORT)
                    if(!strcmp(device_volumes[num_volumes].fs_type, "yaffs2"))
                       device_volumes[num_volumes].device = strdup("cache");
                    else
                       device_volumes[num_volumes].device = strdup(dev[CACHE_INDEX]);
#else
                    device_volumes[num_volumes].device = strdup("cache");
#endif
                    device_volumes[num_volumes].device2 = device2 ? strdup(device2) : NULL;
                } else if (!strcmp(mount_point, "/data") && strcmp(device, dev[DATA_INDEX])) {
#if defined(UBIFS_SUPPORT)
                    device_volumes[num_volumes].device = strdup(dev[DATA_INDEX]);
#else
                    device_volumes[num_volumes].device = strdup("userdata");
#endif
                    device_volumes[num_volumes].device2 = device2 ? strdup(device2) : NULL;
                } else if (!strcmp(mount_point, "/system") && strcmp(device, dev[SYSTEM_INDEX])) {
#if defined(UBIFS_SUPPORT)
                    device_volumes[num_volumes].device = strdup(dev[SYSTEM_INDEX]);
#else
                    device_volumes[num_volumes].device = strdup("system");
#endif
                    device_volumes[num_volumes].device2 = device2 ? strdup(device2) : NULL;
                }else{
                    device_volumes[num_volumes].device = strdup(device);
                    device_volumes[num_volumes].device2 = device2 ? strdup(device2) : NULL;
                }
            }
#endif

            device_volumes[num_volumes].length = 0;
            if (parse_options(options, device_volumes + num_volumes) != 0) {
                LOGE("skipping malformed recovery.fstab line: %s\n", original);
            } else {
                ++num_volumes;
            }
        } else {
            LOGE("skipping malformed recovery.fstab line: %s\n", original);
        }
        free(original);
    }

    fclose(fstab);

    if (has_custom && !fstab_has_custom) {
        while (num_volumes >= alloc) {
            alloc *= 2;
            device_volumes = (Volume*)realloc(device_volumes, alloc*sizeof(Volume));
        }
        device_volumes[num_volumes].mount_point = strdup("/custom");

        if (phone_type == EMMC_TYPE) {
            device_volumes[num_volumes].fs_type = strdup("ext4");
            device_volumes[num_volumes].device = strdup(dev[CUSTOM_INDEX]);
        } else {
            device_volumes[num_volumes].fs_type = strdup("yaffs2");
            device_volumes[num_volumes].device = strdup("custom");
        }

        device_volumes[num_volumes].device2 = NULL;
        device_volumes[num_volumes].length = 0;

        num_volumes++;
    }

#if defined(SUPPORT_SDCARD2) && !defined(MTK_SHARED_SDCARD) //wschen 2012-11-15

    if ((phone_type == EMMC_TYPE) && has_fat) {
        int match = 0;
        for (i = 0; i < num_volumes; i++) {
            Volume* v = &device_volumes[i];
            if (strcmp(v->mount_point, "/sdcard2") == 0) {
                match = 1;
                break;
            }
        }

        if (match == 0) {
            while (num_volumes >= alloc) {
                alloc *= 2;
                device_volumes = (Volume*)realloc(device_volumes, alloc*sizeof(Volume));
            }

            device_volumes[num_volumes].mount_point = strdup("/sdcard2");
            device_volumes[num_volumes].fs_type = strdup("vfat");
#if defined(MTK_LCA_ROM_OPTIMIZE)
             device_volumes[num_volumes].device = strdup("/dev/block/mmcblk1p1");
             device_volumes[num_volumes].device2 = strdup("/dev/block/mmcblk1");
#else
#ifdef MTK_2SDCARD_SWAP
            device_volumes[num_volumes].device = strdup(dev[FAT_INDEX]);
            device_volumes[num_volumes].device2 = NULL;
#else
            device_volumes[num_volumes].device = strdup("/dev/block/mmcblk1p1");
            device_volumes[num_volumes].device2 = strdup("/dev/block/mmcblk1");
#endif
#endif

            device_volumes[num_volumes].length = 0;
            num_volumes++;
        }
    }

#endif //SUPPORT_SDCARD2

    printf("recovery filesystem table\n");
    printf("=========================\n");
    for (i = 0; i < num_volumes; ++i) {
        Volume* v = &device_volumes[i];
        printf("  %d %s %s %s %s %lld\n", i, v->mount_point, v->fs_type,
               v->device, v->device2, v->length);
    }
    printf("\n");
}

Volume* volume_for_path(const char* path) {
    int i;
#if defined(CACHE_MERGE_SUPPORT)
    char *search_path;

    // replace /cache to DATA_CACHE_ROOT (return data volume)
    if (!strncmp(path, "/cache", strlen("/cache"))) {
        search_path = (char *)DATA_CACHE_ROOT;
    } else {
        search_path = (char *)path;
    }

    for (i = 0; i < num_volumes; ++i) {
        Volume* v = device_volumes+i;
        int len = strlen(v->mount_point);
        if (strncmp(search_path, v->mount_point, len) == 0 &&
            (search_path[len] == '\0' || search_path[len] == '/')) {
            return v;
        }
    }
#else
    for (i = 0; i < num_volumes; ++i) {
        Volume* v = device_volumes+i;
        int len = strlen(v->mount_point);
        if (strncmp(path, v->mount_point, len) == 0 &&
            (path[len] == '\0' || path[len] == '/')) {
            return v;
        }
    }
#endif
    return NULL;
}

void load_volume_table2() {
}

#if defined (UBIFS_SUPPORT)
#define UBI_CTRL_DEV "/dev/ubi_ctrl"
#define UBI_SYS_PATH "/sys/class/ubi"


time_t gettime(void)
{
    struct timespec ts;
    int ret;

    ret = clock_gettime(CLOCK_MONOTONIC, &ts);
    if (ret < 0) {
        //ERROR("clock_gettime(CLOCK_MONOTONIC) failed: %s\n", strerror(errno));
		LOGE("clock_gettime(CLOCK_MONOTONIC) failed");
        return 0;
    }

    return ts.tv_sec;
}


int wait_for_file(const char *filename, int timeout)
{
    struct stat info;
    time_t timeout_time = gettime() + timeout;
    int ret = -1;

    while (gettime() < timeout_time && ((ret = stat(filename, &info)) < 0))
        usleep(10000);

    return ret;
}

static int ubi_dev_read_int(int dev, const char *file, int def)
{
    int fd, val = def;
    char path[128], buf[64];

    sprintf(path, UBI_SYS_PATH "/ubi%d/%s", dev, file);
    wait_for_file(path, 5);
    fd = open(path, O_RDONLY);
    if (fd == -1) {
        return val;
    }

    if (read(fd, buf, 64) > 0) {
        val = atoi(buf);
    }

    close(fd);
    return val;
}

// Should include kernel header include/mtd/ubi-user.h

#include <linux/types.h>
#include <asm/ioctl.h>
/*
#define UBI_CTRL_IOC_MAGIC 'o'
#define UBI_IOC_MAGIC 'o'
#define UBI_VOL_NUM_AUTO (-1)
#define UBI_DEV_NUM_AUTO (-1)
#define UBI_IOCATT _IOW(UBI_CTRL_IOC_MAGIC, 64, struct ubi_attach_req)
#define UBI_IOCDET _IOW(UBI_CTRL_IOC_MAGIC, 65, __s32)
#define UBI_IOCMKVOL _IOW(UBI_IOC_MAGIC, 0, struct ubi_mkvol_req)
#define UBI_MAX_VOLUME_NAME 127
struct ubi_attach_req {
	__s32 ubi_num;
	__s32 mtd_num;
	__s32 vid_hdr_offset;
	__s8 padding[12];
};

struct ubi_mkvol_req {
	__s32 vol_id;
	__s32 alignment;
	__s64 bytes;
	__s8 vol_type;
	__s8 padding1;
	__s16 name_len;
	__s8 padding2[4];
	char name[UBI_MAX_VOLUME_NAME + 1];
} __packed;

enum {
	UBI_DYNAMIC_VOLUME = 3,
	UBI_STATIC_VOLUME  = 4,
};
*/


// Should include kernel header include/mtd/ubi-user.h
#define UBI_DEV_NUM_AUTO (-1)
#define UBI_VID_OFFSET_AUTO (0)

int ubi_attach_mtd_user(const char *mount_point)
{
    int ret;
    int vid_off;
    int ubi_ctrl, ubi_dev;
    int vols, avail_lebs, leb_size;
	int32_t ubi_num, mtd_num, ubi_check;
    char path[128];
	char name[128];
    struct ubi_attach_req attach_req;
    struct ubi_mkvol_req mkvol_req;
	const MtdPartition *partition;
	const char* partition_name;
	int ubi_attached = 0;

/*
    mtd_num = mtd_name_to_number(name);
    if (mtd_num == -1) {
        return -1;
    }
*/
	if (!(!strcmp(mount_point, "/system")||!strcmp(mount_point, "/data")||!strcmp(mount_point, "/cache")||!strcmp(mount_point, "/.cache")||!strcmp(mount_point, "/custom"))){
		LOGE("Invalid mount_point: %s\n", mount_point);
		return -1;
	}

	if (!strcmp(mount_point, "/system")){
		ubi_num = 0;
		partition_name = "system";
	}

	if (!strcmp(mount_point, "/data")){
		ubi_num = 1;
		partition_name = "userdata";
	}

	if (!strcmp(mount_point, "/cache")||!strcmp(mount_point, "/.cache")){
		ubi_num = 2;
		partition_name = "cache";
	}

	if (!strcmp(mount_point, "/custom")){
		//ubi_num = 3;
		//mtd_num = 14;
	}

	mtd_scan_partitions();
	partition = mtd_find_partition_by_name(partition_name);
	if (partition == NULL) {
  	LOGE("failed to find \"%s\" partition to mount at \"%s\"\n",
    		partition_name, mount_point);
                return -1;
	}

	mtd_num = mtd_part_to_number(partition);
	printf("mount point[%s], mtd_num:%d \n", mount_point, mtd_num);

	//Check if device already attached
    for (ubi_check = 0; ubi_check < 4; ubi_check++)
    {
      sprintf(path, "/sys/class/ubi/ubi%d/mtd_num", ubi_check);
      ubi_dev = open(path, O_RDONLY);
      if (ubi_dev != -1)
      {
        ret = read(ubi_dev, path, sizeof(path));
        close(ubi_dev);
        if (ret > 0 && mtd_num == atoi(path)){
          printf("ubi%d already attached\n", ubi_check);
		  ubi_attached =1;
          //return ubi_check;
        }
      }
    }

//If UBI device is already attached, skip it, just make UBI volume
if(!ubi_attached){
    ubi_ctrl = open(UBI_CTRL_DEV, O_RDONLY);
	printf("ubi_ctrl = %d\n", ubi_ctrl);

    if (ubi_ctrl == -1) {
		LOGE("failed to open UBI_CTRL_DEV\n");
        return -1;
    }

	//attach UBI device to MTD

	printf("ubi_num = %d\n",ubi_num);

    memset(&attach_req, 0, sizeof(struct ubi_attach_req));
    attach_req.ubi_num =  ubi_num;
    attach_req.mtd_num = mtd_num;
    attach_req.vid_hdr_offset = UBI_VID_OFFSET_AUTO;

    ret = ioctl(ubi_ctrl, UBI_IOCATT, &attach_req);
    if (ret == -1) {
        close(ubi_ctrl);
		LOGE("failed to UBI_IOCATT\n");
        return -1;
    }
}
    //ubi_num = attach_req.ubi_num;
    vid_off = attach_req.vid_hdr_offset;
    vols = ubi_dev_read_int(ubi_num, "volumes_count", -1);
    if (vols == 0) {
        sprintf(path, "/dev/ubi%d", ubi_num);
        ubi_dev = open(path, O_RDONLY);
        if (ubi_dev == -1) {
            close(ubi_ctrl);
			LOGE("failed to open attached UBI device\n");
            return ubi_num;
        }

        avail_lebs = ubi_dev_read_int(ubi_num, "avail_eraseblocks", 0);
        leb_size = ubi_dev_read_int(ubi_num, "eraseblock_size", 0);

	//Make UBI volume
        memset(&mkvol_req, 0, sizeof(struct ubi_mkvol_req));
        mkvol_req.vol_id = 0; //UBI_VOL_NUM_AUTO;
        mkvol_req.alignment = 1;
        mkvol_req.bytes = (long long)avail_lebs * leb_size;
        mkvol_req.vol_type = UBI_DYNAMIC_VOLUME;
        ret = snprintf(mkvol_req.name, UBI_MAX_VOLUME_NAME + 1, "%s", partition_name);
        mkvol_req.name_len = ret;
        ioctl(ubi_dev, UBI_IOCMKVOL, &mkvol_req);
        close(ubi_dev);
    }

    close(ubi_ctrl);
    return ubi_num;
}

int ubi_detach_dev(int dev)
{
	printf("It's in ubi_detach_dev!!\n");

    int ret, ubi_ctrl;
    ubi_ctrl = open(UBI_CTRL_DEV, O_RDONLY);
    if (ubi_ctrl == -1) {
        return -1;
    }

    ret = ioctl(ubi_ctrl, UBI_IOCDET, &dev);
    close(ubi_ctrl);
    return ret;
}


int ubi_mkvol_user(const char *mount_point)
{
	//int fd;
	int ret;
	int ubi_num, ubi_dev,vols;
	int avail_lebs, leb_size;
	char path[128];
	struct ubi_mkvol_req r;
	//size_t n;

	//memset(&r, 0, sizeof(struct ubi_mkvol_req));
	if (!(!strcmp(mount_point, "/system")||!strcmp(mount_point, "/data")||!strcmp(mount_point, "/cache")||!strcmp(mount_point, "/.cache")||!strcmp(mount_point, "/custom"))){
		LOGE("Invalid mount_point: %s\n", mount_point);
		return -1;
	}

	if (!strcmp(mount_point, "/system")){
		ubi_num = 0;
	}

	if (!strcmp(mount_point, "/data")){
		ubi_num = 1;
	}

	if (!strcmp(mount_point, "/cache")||!strcmp(mount_point, "/.cache")){
		ubi_num = 2;
	}

	if (!strcmp(mount_point, "/custom")){
		//ubi_num = 3;
	}

	vols = ubi_dev_read_int(ubi_num, "volumes_count", -1);
	if (vols == 0) {
		sprintf(path, "/dev/ubi%d", ubi_num);
		ubi_dev = open(path, O_RDONLY);
		if (ubi_dev == -1) {
			//close(ubi_ctrl);
			LOGE("failed to open attached UBI device\n");
			return ubi_num;
		}

		avail_lebs = ubi_dev_read_int(ubi_num, "avail_eraseblocks", 0);
		leb_size = ubi_dev_read_int(ubi_num, "eraseblock_size", 0);

	//Make UBI volume
		memset(&r, 0, sizeof(struct ubi_mkvol_req));
		r.vol_id = 0; //UBI_VOL_NUM_AUTO;
		r.alignment = 1;
		r.bytes = (long long)avail_lebs * leb_size;
		r.vol_type = UBI_DYNAMIC_VOLUME;
		ret = snprintf(r.name, UBI_MAX_VOLUME_NAME + 1, "%s", mount_point);
		r.name_len = ret;
		ret = ioctl(ubi_dev, UBI_IOCMKVOL, &r);

		close(ubi_dev);

		#ifdef UDEV_SETTLE_HACK
		//	if (system("udevsettle") == -1)
		//		return -1;
			usleep(100000);
		#endif

		if (ret==-1){
			LOGE("failed to make UBI volume\n");
			return ret;
		}
	}

	printf("make UBI volume success\n");
	return 0;

}

int ubi_rmvol_user(const char *mount_point)
{
	//int fd, ret;

	int ret, ubi_num, ubi_dev,vols;
	int vol_id = 0;
	char path[128];


	//memset(&r, 0, sizeof(struct ubi_mkvol_req));
	if (!(!strcmp(mount_point, "/system")||!strcmp(mount_point, "/data")||!strcmp(mount_point, "/cache")||!strcmp(mount_point, "/.cache")||!strcmp(mount_point, "/custom"))){
		LOGE("Invalid mount_point: %s\n", mount_point);
		return -1;
	}

	if (!strcmp(mount_point, "/system")){
		ubi_num = 0;
	}

	if (!strcmp(mount_point, "/data")){
		ubi_num = 1;
	}

	if (!strcmp(mount_point, "/cache")||!strcmp(mount_point, "/.cache")){
		ubi_num = 2;
	}

	if (!strcmp(mount_point, "/custom")){
		//ubi_num = 3;
	}

	vols = ubi_dev_read_int(ubi_num, "volumes_count", -1);
	if (vols == 0) {
		sprintf(path, "/dev/ubi%d", ubi_num);
		ubi_dev = open(path, O_RDONLY);
		if (ubi_dev == -1) {
			//close(ubi_ctrl);
			LOGE("failed to open attached UBI device\n");
			return ubi_num;
		}


/*
	//desc = desc;
	fd = open(ubi_dev, O_RDONLY);
	if (fd == -1)
		return -1; //sys_errmsg("cannot open \"%s\"", node);
*/

	ret = ioctl(ubi_dev, UBI_IOCRMVOL, &vol_id);
	if (ret == -1) {
		close(ubi_dev);
		LOGE("failed to remove UBI volume\n");
		return ret;
	}

	close(ubi_dev);

	#ifdef UDEV_SETTLE_HACK
	//	if (system("udevsettle") == -1)
	//		return -1;
		usleep(100000);
	#endif
	}

	printf("Remove UBI volume success\n");
	return 0;
}


int ubi_format(const char *mount_point){
	        //Detach UBI volume before formating
			printf("It's in ubi_format!!\n");
			const char* partition_name;
			char mtd_dev_name[20];
			const MtdPartition *partition;
			int32_t mtd_num;
			int32_t ubi_num = -1;

			if (!strcmp(mount_point, "/system")){
					ubi_num = 0;
					partition_name = "system";
			}

			if (!strcmp(mount_point, "/data")){
					ubi_num = 1;
					partition_name = "userdata";
			}

			if (!strcmp(mount_point, "/cache")||!strcmp(mount_point, "/.cache")){
					ubi_num = 2;
					partition_name = "cache";
			}


			if(ubi_num != -1){
				ubi_detach_dev(ubi_num);
				printf("Back to ubi_format!!\n");
			}
			else{
				printf("Can not find a ubi device![%s], error:%s\n", mount_point, strerror(errno));
				return -1;
			}

			//Run-time get mtd_dev_name
			mtd_scan_partitions();
			partition = mtd_find_partition_by_name(partition_name);
			if (partition == NULL) {
				LOGE("failed to find \"%s\" partition to mount at \"%s\"\n",
				partition_name,mount_point);
                return -1;
			}

			mtd_num = mtd_part_to_number(partition);
			printf("mtd_num = %d\n", mtd_num);
			sprintf(mtd_dev_name, "/dev/mtd/mtd%d", mtd_num);

			printf("Formatting %s -> %s\n", mount_point, mtd_dev_name);

			const char* binary_path = "/sbin/ubiformat";
			const char* skip_questions = "-y";

			int check;
			check = chmod(binary_path, 0777);
			printf("chmod = %d\n", check);

    		const char** args = (const char**)malloc(sizeof(char*) * 4);
    		args[0] = binary_path;
    		args[1] = (const char *)mtd_dev_name;
    		args[2] = skip_questions;
    		args[3] = NULL;

    		pid_t pid = fork();
    		if (pid == 0) {
        		execv(binary_path, (char* const*)args);
        		fprintf(stdout, "E:Can't run %s (%s)\n", binary_path, strerror(errno));
        		_exit(-1);
    		}

    		int status;
    		waitpid(pid, &status, 0);

    		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        		LOGE("Error in ubiformat\n(Status %d)\n", WEXITSTATUS(status));
        		return -1;
    		}


			//Attatch UBI device & Make UBI volume
			int n=-1;
			n = ubi_attach_mtd_user(mount_point);

			if((n!=-1) && (n<4)){
				printf("Try to attatch /dev/ubi%d_0... volume is attached \n", n);
			}
			else{
  				LOGE("failed to attach /dev/ubi%d_0%s\n", n);
				return -1;
			}

			return 0;
}

#endif

int ensure_path_mounted(const char* path) {
    Volume* v = volume_for_path(path);
    if (v == NULL) {
        LOGE("unknown volume for path [%s]\n", path);
        return -1;
    }
    if (strcmp(v->fs_type, "ramdisk") == 0) {
        // the ramdisk is always mounted.
        return 0;
    }

    int result;
    result = scan_mounted_volumes();
    if (result < 0) {
        LOGE("failed to scan mounted volumes\n");
        return -1;
    }

    const MountedVolume* mv =
        find_mounted_volume_by_mount_point(v->mount_point);
    if (mv) {
#if defined(CACHE_MERGE_SUPPORT)
        if (strncmp(path, "/cache", 6) == 0) {
            if (symlink(DATA_CACHE_ROOT, "/cache")) {
                if (errno != EEXIST) {
                    LOGE("create symlink from %s to %s failed(%s)\n",
                                        DATA_CACHE_ROOT, "/cache", strerror(errno));
                    return -1;
                }
            }
        }
#endif
        // volume is already mounted
        return 0;
    }

    mkdir(v->mount_point, 0755);  // in case it doesn't already exist

#if defined (UBIFS_SUPPORT)
	if(strcmp(v->fs_type, "ubifs") == 0){

		printf("Trying to mount %s \n", v->mount_point);

		//Attatch UBI device & Make UBI volum
		int n=-1;
		n = ubi_attach_mtd_user(v->mount_point);

		if((n!=-1) && (n<4)){
			printf("Try to attatch %s \n", v->device);
			printf("/dev/ubi%d_0 is attached \n", n);
		}
		else{
  			LOGE("failed to attach %s\n", v->device);
		}


		//Mount UBI volume
		const unsigned long flags = MS_NOATIME | MS_NODEV | MS_NODIRATIME;
		char tmp[64];
		sprintf(tmp, "/dev/ubi%d_0", n);

		result = mount(tmp, v->mount_point, v->fs_type, flags, "");
        if ( result < 0) {
            ubi_detach_dev(n);
            return -1;
        }
		else if (result == 0) goto mount_done;  //Volume  successfully  mounted

	}
#endif

    if (strcmp(v->fs_type, "yaffs2") == 0) {
        // mount an MTD partition as a YAFFS2 filesystem.
        mtd_scan_partitions();
        const MtdPartition* partition;
        partition = mtd_find_partition_by_name(v->device);
        if (partition == NULL) {
            LOGE("failed to find \"%s\" partition to mount at \"%s\"\n",
                 v->device, v->mount_point);
            return -1;
        }
        return mtd_mount_partition(partition, v->mount_point, v->fs_type, 0);
    } else if (strcmp(v->fs_type, "ext4") == 0 ||
               strcmp(v->fs_type, "vfat") == 0) {
        result = mount(v->device, v->mount_point, v->fs_type,
                       MS_NOATIME | MS_NODEV | MS_NODIRATIME, "");
        if (result == 0) {
            goto mount_done;
        } else {
#if 1 //wschen 2013-05-03 workaround for slowly SD
            if (strstr(v->mount_point, "/sdcard") && strstr(v->device, "/dev/block/mmcblk1")) {
                int retry = 0;
                for (; retry <=3; retry++) {
                    result = mount(v->device, v->mount_point, v->fs_type, MS_NOATIME | MS_NODEV | MS_NODIRATIME, "");
                    if (result == 0) {
                        goto mount_done;
                    } else {
                        sleep(1);
                    }
                }
                printf("Slowly SD retry failed (%s)\n", v->device);
            }
#endif
        }

        if (v->device2) {
#if 1 //wschen 2012-09-04
            //try mmcblk1 if mmcblk1p1 failed, then try internal FAT
            if (!strcmp(v->mount_point, "/sdcard") && !strcmp(v->device, "/dev/block/mmcblk1p1") && !strstr(v->device2, "/dev/block/mmcblk1")) {
                result = mount("/dev/block/mmcblk1", v->mount_point, v->fs_type, MS_NOATIME | MS_NODEV | MS_NODIRATIME, "");
                if (result == 0) {
                    return 0;
                }
            }
#endif
            LOGW("failed to mount %s (%s); trying %s\n",
                 v->device, strerror(errno), v->device2);
            result = mount(v->device2, v->mount_point, v->fs_type,
                           MS_NOATIME | MS_NODEV | MS_NODIRATIME, "");
            if (result == 0) goto mount_done;
        }

        LOGE("failed to mount %s (%s)\n", v->mount_point, strerror(errno));
        return -1;
    }

    LOGE("unknown fs_type \"%s\" for %s\n", v->fs_type, v->mount_point);
    return -1;

mount_done:
#if defined(CACHE_MERGE_SUPPORT)
    if (strcmp(v->mount_point, "/data") == 0) {
        if (mkdir(DATA_CACHE_ROOT, 0770)) {
            if (errno != EEXIST) {
                LOGE("mkdir %s error: %s\n", DATA_CACHE_ROOT, strerror(errno));
                return -1;
            } else if (need_clear_cache) {
                LOGI("cache exists, clear it...\n");
                if (remove_dir(DATA_CACHE_ROOT)) {
                    LOGE("remove_dir %s error: %s\n", DATA_CACHE_ROOT, strerror(errno));
                    return -1;
                }
                if (mkdir(DATA_CACHE_ROOT, 0770) != 0) {
                    LOGE("mkdir %s error: %s\n", DATA_CACHE_ROOT, strerror(errno));
                    return -1;
                }
            }
        }
        if (symlink(DATA_CACHE_ROOT, "/cache")) {
            if (errno != EEXIST) {
                LOGE("create symlink from %s to %s failed(%s)\n",
                                DATA_CACHE_ROOT, "/cache", strerror(errno));
                return -1;
            }
        }
        need_clear_cache = 0;
    }
#endif
    return 0;
}

int ensure_path_unmounted(const char* path) {
    Volume* v = volume_for_path(path);

#if defined(CACHE_MERGE_SUPPORT)
    if (strncmp(path, "/cache", 6) == 0) {
        unlink(path);
        return 0;
    }
#endif

    if (v == NULL) {
        LOGE("unknown volume for path [%s]\n", path);
        return -1;
    }
    if (strcmp(v->fs_type, "ramdisk") == 0) {
        // the ramdisk is always mounted; you can't unmount it.
        return -1;
    }

    int result;
    result = scan_mounted_volumes();
    if (result < 0) {
        LOGE("failed to scan mounted volumes\n");
        return -1;
    }

    const MountedVolume* mv =
        find_mounted_volume_by_mount_point(v->mount_point);
    if (mv == NULL) {
        // volume is already unmounted
        return 0;
    }

    return unmount_mounted_volume(mv);
}

int format_volume(const char* volume) {
#if defined(CACHE_MERGE_SUPPORT)
    char *target_volume = (char *)volume;

    if (strcmp(target_volume, "/cache") == 0) {
        // we cannot mount data since partition size changed
        // clear cache folder when data mounted
        if (part_size_changed) {
            LOGI("partition size changed, clear cache folder when data mounted...\n");
            need_clear_cache = 1;

            // change format volume name to format actual cache partition
            target_volume = "/.cache";
        } else {
            // clear DATA_CACHE_ROOT
            if (ensure_path_mounted(DATA_CACHE_ROOT) != 0) {
                LOGE("Can't mount %s while clearing cache!\n", DATA_CACHE_ROOT);
                return -1;
            }
            if (remove_dir(DATA_CACHE_ROOT)) {
                LOGE("remove_dir %s error: %s\n", DATA_CACHE_ROOT, strerror(errno));
                return -1;
            }
            if (mkdir(DATA_CACHE_ROOT, 0770) != 0) {
                LOGE("Can't mkdir %s (%s)\n", DATA_CACHE_ROOT, strerror(errno));
                return -1;
            }
            LOGI("format cache successfully!\n");
            return 0;
        }
    }

    Volume* v = volume_for_path(target_volume);
    if (v == NULL) {
        LOGE("unknown volume \"%s\"\n", target_volume);
        return -1;
    }
    if (strcmp(v->fs_type, "ramdisk") == 0) {
        // you can't format the ramdisk.
        LOGE("can't format_volume \"%s\"", target_volume);
        return -1;
    }
    if (strcmp(v->mount_point, target_volume) != 0) {
        LOGE("can't give path \"%s\" to format_volume\n", target_volume);
        return -1;
    }

    if (ensure_path_unmounted(target_volume) != 0) {
        LOGE("format_volume failed to unmount \"%s\"\n", v->mount_point);
        return -1;
    }
#else
    Volume* v = volume_for_path(volume);
    if (v == NULL) {
        LOGE("unknown volume \"%s\"\n", volume);
        return -1;
    }
    if (strcmp(v->fs_type, "ramdisk") == 0) {
        // you can't format the ramdisk.
        LOGE("can't format_volume \"%s\"", volume);
        return -1;
    }
    if (strcmp(v->mount_point, volume) != 0) {
        LOGE("can't give path \"%s\" to format_volume\n", volume);
        return -1;
    }

    if (ensure_path_unmounted(volume) != 0) {
        LOGE("format_volume failed to unmount \"%s\"\n", v->mount_point);
        return -1;
    }
#endif

#if defined (UBIFS_SUPPORT)
	if(strcmp(v->fs_type, "ubifs") == 0){

		int ret;
		ret = ubi_format(v->mount_point);

		if(!ret){
			return 0;
		}else{
			LOGE("Ubiformat failed on \"%s\"\n", v->mount_point);
		}


#if 0

			int ret;
			//Remove volume
			if(ubi_rmvol_user(v->mount_point)!=0){
				LOGE("failed to remove %s\n", v->device);
				return -1;
			}

			//Make volume
			ret = ubi_mkvol_user(v->mount_point);
			if(!ret){
				printf("%s volume made\n", v->device);
				return 0;
			}
#endif
	}
#endif

    if (strcmp(v->fs_type, "yaffs2") == 0 || strcmp(v->fs_type, "mtd") == 0) {
        mtd_scan_partitions();
        const MtdPartition* partition = mtd_find_partition_by_name(v->device);
        if (partition == NULL) {
            LOGE("format_volume: no MTD partition \"%s\"\n", v->device);
            return -1;
        }

        MtdWriteContext *write = mtd_write_partition(partition);
        if (write == NULL) {
            LOGW("format_volume: can't open MTD \"%s\"\n", v->device);
            return -1;
        } else if (mtd_erase_blocks(write, -1) == (off_t) -1) {
            LOGW("format_volume: can't erase MTD \"%s\"\n", v->device);
            mtd_write_close(write);
            return -1;
        } else if (mtd_write_close(write)) {
            LOGW("format_volume: can't close MTD \"%s\"\n", v->device);
            return -1;
        }
        return 0;
    }

    if (strcmp(v->fs_type, "ext4") == 0) {
#if defined(HAVE_ANDROID_OS) && !defined(ARCH_X86) //wschen 2012-07-10
        int fd;
        struct msdc_ioctl msdc_io;

        fd = open("/dev/misc-sd", O_RDONLY);
        if (fd < 0) {
            LOGE("open: /dev/misc-sd failed\n");
            return -1;
        }

        msdc_io.opcode = MSDC_ERASE_PARTITION;
#if defined(CACHE_MERGE_SUPPORT)
        if (!strcmp(target_volume, "/.cache")) {
            msdc_io.buffer = (unsigned int*) "cache";
            msdc_io.total_size = 6;
        } else if (!strcmp(target_volume, "/data")) {
            msdc_io.buffer = (unsigned int*) "usrdata";
            msdc_io.total_size = 8;
        }
#else
        if (!strcmp(volume, "/cache")) {
            msdc_io.buffer = (unsigned int*) "cache";
            msdc_io.total_size = 6;
        } else if (!strcmp(volume, "/data")) {
            msdc_io.buffer = (unsigned int*) "usrdata";
            msdc_io.total_size = 8;
        }
#endif
        ioctl(fd, 0, &msdc_io);
        close(fd);
#endif

#if defined(CACHE_MERGE_SUPPORT)
        int result = make_ext4fs(v->device, v->length, target_volume, sehandle);
#else
        int result = make_ext4fs(v->device, v->length, volume, sehandle);
#endif
        if (result != 0) {
            LOGE("format_volume: make_extf4fs failed on %s\n", v->device);
            return -1;
        }

        return 0;
    }

    LOGE("format_volume: fs_type \"%s\" unsupported\n", v->fs_type);
    return -1;
}

#if 0
int setup_install_mounts() {
    if (fstab == NULL) {
        LOGE("can't set up install mounts: no fstab loaded\n");
        return -1;
    }
    for (int i = 0; i < fstab->num_entries; ++i) {
        Volume* v = fstab->recs + i;

        if (strcmp(v->mount_point, "/tmp") == 0 ||
            strcmp(v->mount_point, "/cache") == 0) {
            if (ensure_path_mounted(v->mount_point) != 0) return -1;

        } else {
            if (ensure_path_unmounted(v->mount_point) != 0) return -1;
        }
    }
    return 0;
}
#endif
