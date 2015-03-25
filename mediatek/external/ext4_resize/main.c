#include <utils/Log.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include "ext2fs/ext2fs.h"

#define LOG_TAG "Resize"
#define LK_FILE_MAX_SIZE (16 * 1024)
#define STATS_RESIZED 1
#define DATA_PATH "/emmc@usrdata" 
#define RESERVE_SIZE (1 * 1024 * 1024)

struct resize_stats
{
    int state;
    unsigned long long size;
};

static unsigned long long get_block_size(const char *path);
static int get_resize_stats(struct resize_stats *stats);
static errcode_t adjust_fs_size(ext2_filsys fs, unsigned long long *new_size);

int main(int argc, char* agrv[])
{
    struct resize_stats stats = {0, 0};
    int result = 0;
    if ((result = get_resize_stats(&stats)) < 0) {
        ALOGE("process system env fail.\n");
        return -1;
    }

    if (STATS_RESIZED == stats.state) {
        ALOGI("Partition has been resized,so nothing to do!\n");
        return 0;
    }else {
        //Get block size of data partition.
        pid_t pid;
        int status;
        int child_return_val;
        char size[16]; // data_partition max size is 999999M
        int ret;
        int resize_ret = 0;

        if((ret = sprintf(size, "%lluK", (unsigned long long)(stats.size / 1024))) < 0) {
            ALOGE("Convert size fail\n");
            return -1;
        }

        ALOGI("Resize data partition to %s. Starting...\n", size);

        //Fork a new thread to run resize2fs
        pid = fork(); 
        switch(pid) {
            case -1:
                ALOGE("fork system call fail.\n");
                break;
            case 0: // child 
                execl("/system/bin/resize2fs", "resize2fs", "-f", DATA_PATH, size, NULL);
                ALOGE("execl fail.\n");
                _exit(127);
                break;
            default: //parent 
            // wait child process end
            while (pid != wait(&status));

            if (WIFEXITED(status)){
                child_return_val = WEXITSTATUS(status);
                if (child_return_val != 0) {
                    ALOGE("resize data partition fail. We will run e2fsck -pDf...");
                    resize_ret = 0;
                } else {
                    ALOGE("resize run ok. We will run e2fsck -pDf to fix journal size...");
                    resize_ret = 1;
                }
            }
                
        }
 
        //For a new thread to run e2fsck -pDf
        pid = fork(); 
        switch(pid) {
            case -1:
                ALOGE("fork system call fail.\n");
                break;
            case 0: // child 
                execl("/sbin/e2fsck", "e2fsck", "-pDf", DATA_PATH, NULL);
                ALOGE("execl fail.\n");
                _exit(127);
                break;
            default: //parent 
            // wait child process end
            while (pid != wait(&status));

            if (WIFEXITED(status)){
                child_return_val = WEXITSTATUS(status);
                if (child_return_val != 0 && child_return_val != 1){
                    ALOGE("Fail to correct filesystem...\n");
                    resize_ret = 0;
                } else {
                    ALOGE("successful to run e2fsck...\n");
                }
            }
                
        }

        ALOGI("successful to Resize data partition.\n");
        return 0;
    }
}

static int get_resize_stats(struct resize_stats *stats)
{
	errcode_t	retval;
	ext2_filsys	fs;
	ext2_filsys	dup_fs;
	io_manager	io_ptr;
    unsigned long long data_partition_size = 0;

    io_ptr = unix_io_manager;
	retval = ext2fs_open2(DATA_PATH, NULL , 0,
			      0, 0, io_ptr, &fs);
	if (retval) {
		ALOGE("Couldn't find valid filesystem superblock.\n");
	    return -1;	
	}

	retval = ext2fs_dup_handle(fs, &dup_fs);
	if (retval) {
		ALOGE("Couldn't duplicate filesys.\n");
        ext2fs_close(fs);
	    return -1;	
	}

    if((data_partition_size = get_block_size(DATA_PATH)) < 0) {
        ALOGE("Get data partition size fail.\n");
        ext2fs_close(fs);
        ext2fs_free(dup_fs);
        return -1;
    }
    ALOGE("size for emmc@data is %lluK.\n", data_partition_size / 1024);
    ALOGE("size in superblock is %lluK.\n", (unsigned long long)fs->super->s_blocks_count * fs->blocksize / 1024);

    data_partition_size -= RESERVE_SIZE;
    //adjust partition size to meet resizefs rule.
    //resizefs will adjust partition size in some case.
    retval = adjust_fs_size(dup_fs, &data_partition_size);
	if (retval) {
		ALOGE("Couldn't adjust partition size.\n");
        ext2fs_close(fs);
        ext2fs_free(dup_fs);
	    return -1;	
	}
    ALOGE("size will (maybe) resize to(after adjust) is %lluK.\n", data_partition_size / 1024);

    if((unsigned int)(data_partition_size / fs->blocksize) == (unsigned int)(fs->super->s_blocks_count)) {
        ALOGI("The size of data already meet the request(size in superblock = size after adjust).\n");
        stats->state = STATS_RESIZED;
    }
    stats->size = data_partition_size;
    ext2fs_close(fs);
    ext2fs_free(dup_fs);
    return 0;
}

static errcode_t adjust_fs_size(ext2_filsys fs, unsigned long long *new_size)
{
	errcode_t	retval;
	int		overhead = 0;
	int		rem;

	fs->super->s_blocks_count = (unsigned int)(*new_size / fs->blocksize);

retry:
	fs->group_desc_count = ext2fs_div_ceil(fs->super->s_blocks_count -
				       fs->super->s_first_data_block,
				       EXT2_BLOCKS_PER_GROUP(fs->super));
	if (fs->group_desc_count == 0)
		return EXT2_ET_TOOSMALL;
	fs->desc_blocks = ext2fs_div_ceil(fs->group_desc_count,
					  EXT2_DESC_PER_BLOCK(fs->super));

	/*
	 * Overhead is the number of bookkeeping blocks per group.  It
	 * includes the superblock backup, the group descriptor
	 * backups, the inode bitmap, the block bitmap, and the inode
	 * table.
	 */
	overhead = (int) (2 + fs->inode_blocks_per_group);

	if (ext2fs_bg_has_super(fs, fs->group_desc_count - 1))
		overhead += 1 + fs->desc_blocks +
			fs->super->s_reserved_gdt_blocks;

	/*
	 * See if the last group is big enough to support the
	 * necessary data structures.  If not, we need to get rid of
	 * it.
	 */
	rem = (fs->super->s_blocks_count - fs->super->s_first_data_block) %
		fs->super->s_blocks_per_group;
	if ((fs->group_desc_count == 1) && rem && (rem < overhead))
		return EXT2_ET_TOOSMALL;
	if (rem && (rem < overhead+50)) {
		fs->super->s_blocks_count -= rem;
		goto retry;
	}

    *new_size = ((unsigned long long)fs->super->s_blocks_count * (unsigned long long)fs->blocksize);
    return 0;
}

static unsigned long long get_block_size(const char *path)
{
    int fd;
    int ret;
    unsigned long long block_size = 0;
    if ((fd = open(path, O_RDONLY)) < 0) {
        ALOGE("Open block fail:%s.\n", (char*)strerror(errno));
        return -1;
    }

	ret = ioctl(fd, BLKGETSIZE64, &block_size);
    close(fd);

	if (ret)
		return -1;

	return block_size;
}

