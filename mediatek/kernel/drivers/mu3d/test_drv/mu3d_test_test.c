#include <linux/init.h>
#include <linux/irq.h>
#include <linux/log2.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>

#include <linux/kernel.h>       /* printk() */
#include <linux/fs.h>           /* everything... */
#include <linux/errno.h>        /* error codes */
#include <linux/types.h>        /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>        /* O_ACCMODE */
#include <linux/seq_file.h>
#include <linux/cdev.h>
//#include <linux/pci.h>
#include <linux/string.h>
#include <linux/random.h>
#include <linux/scatterlist.h>
#include <asm/unaligned.h>
#include <linux/usb/ch9.h>
#include <asm/uaccess.h>
#include <linux/mu3d/test_drv/mu3d_test_test.h>
#include <linux/mu3d/test_drv/mu3d_test_unified.h>



struct file_operations xhci_mtk_test_fops_dev;


////////////////////////////////////////////////////////////////////////////

#define CLI_MAGIC 'C'
#define IOCTL_READ _IOR(CLI_MAGIC, 0, int)
#define IOCTL_WRITE _IOW(CLI_MAGIC, 1, int)

#define BUF_SIZE 200
#define MAX_ARG_SIZE 20

////////////////////////////////////////////////////////////////////////////

typedef struct
{
	char name[256];
	int (*cb_func)(int argc, char** argv);
} CMD_TBL_T;

static CMD_TBL_T _arPCmdTbl_dev[] =
{
	{"auto.dev", &TS_AUTO_TEST},
#ifdef SUPPORT_OTG
	{"auto.stop", &TS_AUTO_TEST_STOP},
#endif
	{"auto.u3i", &u3init},
	{"auto.u3w", &u3w},
	{"auto.u3r", &u3r},
	{"auto.u3d", &U3D_Phy_Cfg_Cmd},
	{"auto.link", &u3d_linkup},
	{"auto.eyeinit", &dbg_phy_eyeinit},
	{"auto.eyescan", &dbg_phy_eyescan},
#ifdef SUPPORT_OTG
	{"auto.otg", &otg_top},
#endif
	{"", NULL}
};

////////////////////////////////////////////////////////////////////////////

char wr_buf_dev[BUF_SIZE];
char rd_buf_dev[BUF_SIZE] = "this is a test";

////////////////////////////////////////////////////////////////////////////

int call_function_dev(char *buf)
{
	int i;
	int argc;
	char *argv[MAX_ARG_SIZE];

	argc = 0;
	do
	{
		argv[argc] = strsep(&buf, " ");
		printk(KERN_DEBUG "[%d] %s\r\n", argc, argv[argc]);
		argc++;
	} while (buf);
#ifdef SUPPORT_OTG
	#define CMD_NUM 10
#else
	#define CMD_NUM 8
#endif
	for (i = 0; i < CMD_NUM; i++)
	{
		if ((!strcmp(_arPCmdTbl_dev[i].name, argv[0])) && (_arPCmdTbl_dev[i].cb_func != NULL))
			return _arPCmdTbl_dev[i].cb_func(argc, argv);
	}

	return -1;
}


static int xhci_mtk_test_open(struct inode *inode, struct file *file)
{

    printk(KERN_DEBUG "xhci_mtk_test open: successful\n");
    return 0;
}

static int xhci_mtk_test_release(struct inode *inode, struct file *file)
{

    printk(KERN_DEBUG "xhci_mtk_test release: successful\n");
    return 0;
}

static ssize_t xhci_mtk_test_read(struct file *file, char *buf, size_t count, loff_t *ptr)
{

    printk(KERN_DEBUG "xhci_mtk_test read: returning zero bytes\n");
    return 0;
}

static ssize_t xhci_mtk_test_write(struct file *file, const char *buf, size_t count, loff_t * ppos)
{

    printk(KERN_DEBUG "xhci_mtk_test write: accepting zero bytes\n");
    return 0;
}

static long xhci_mtk_test_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{

    int len = BUF_SIZE;

	switch (cmd) {
		case IOCTL_READ:
			if(copy_to_user((char *) arg, rd_buf_dev, len))
				return -EFAULT;
			printk(KERN_DEBUG "IOCTL_READ: %s\n", rd_buf_dev);
			break;
		case IOCTL_WRITE:
			if(copy_from_user(wr_buf_dev, (char *) arg, len))
				return -EFAULT;
			printk("IOCTL_WRITE: %s\n", wr_buf_dev);

			//invoke function
			return call_function_dev(wr_buf_dev);
			break;
		default:
			printk("cmd=%d\r", cmd);
			return -ENOTTY;
	}

	return len;
}

struct file_operations xhci_mtk_test_fops_dev = {
    .owner =   THIS_MODULE,
    .read =    xhci_mtk_test_read,
    .write =   xhci_mtk_test_write,
    .unlocked_ioctl =   xhci_mtk_test_ioctl,
    .open =    xhci_mtk_test_open,
    .release = xhci_mtk_test_release,
};


static int __init mtk_test_init(void)
{
	int retval = 0;
	printk(KERN_DEBUG "xchi_mtk_test Init\n");
	retval = register_chrdev(XHCI_MTK_TEST_MAJOR, DEVICE_NAME, &xhci_mtk_test_fops_dev);
	if(retval < 0)
	{
		printk(KERN_DEBUG "xchi_mtk_test Init failed, %d\n", retval);
		goto fail;
	}

	return 0;
	fail:
		return retval;
}
module_init(mtk_test_init);

static void __exit mtk_test_cleanup(void)
{
	printk(KERN_DEBUG "xchi_mtk_test End\n");
	unregister_chrdev(XHCI_MTK_TEST_MAJOR, DEVICE_NAME);

}
module_exit(mtk_test_cleanup);

MODULE_LICENSE("GPL");
