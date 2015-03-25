#include <linux/string.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/vmalloc.h>
#include <linux/dma-mapping.h>

#include <asm/uaccess.h>

#include "ddp_debug.h"
#include "ddp_hal.h"
#include "ddp_ovl.h"
#include "ddp_reg.h"

#include <tlSvp_Api.h> // trustlet inter face for TLC
#include <drSvp_Api.h>

#include "mobicore_driver_api.h"

#define DRV_DBG_UUID { { 2, 0xf, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }

//------------------------------------------------------------------------------
// definitions
#define TPLAY_DEV_NAME        "t-play-drv-ddp"

#define TPLAY_DRV_DDP_CMD_1        1
#define TPLAY_DRV_DDP_CMD_2        2

//------------------------------------------------------------------------------
// debugging definitions
#define TPLAY_DRV_SESSION
#define TPLAY_TL_SESSION

//#define MANUAL_DEBUG
//#define USE_MC_MALLOC_WSM

//------------------------------------------------------------------------------
// Global
const struct mc_uuid_t uuid = SVP_TL_DBG_UUID; // use Svp TL
const struct mc_uuid_t uuid_dr = SVP_DRV_DBG_UUID;
const struct mc_uuid_t uuid_mem = DRV_DBG_UUID;
static struct mc_session_handle tlSessionHandle; // use TL
static struct mc_session_handle drSessionHandle; // use TDriver
static struct mc_session_handle memSessionHandle;
uint32_t mc_deviceId = MC_DEVICE_ID_DEFAULT;
tciMessage_t *pTci = NULL;
dciMessage_t *pDci = NULL;
dciMessage_t *pMemDci = NULL;

int debug_svp = 0;

unsigned int is_using_secure_debug_layer = 0;
unsigned int is_secure_port = 0;

//------------------------------------------------------------------------------
// Functions
int late_init_session(void);
void close_session(void);

enum mc_result late_init_session_tl(void);
enum mc_result late_init_session_drv(void);
enum mc_result late_init_session_mem(void);
enum mc_result late_open_mobicore_device(void);

void close_session_tl(void);
void close_session_drv(void);
void close_session_mem(void);
void close_mobicore_device(void);

int ovl_layer_dump_register(void);
int dummy_test_dci(void);
int dummy_test_tci(void);

//------------------------------------------------------------------------------
// Type
typedef struct{
    unsigned int layer_en[OVL_LAYER_NUM];
    unsigned int addr[OVL_LAYER_NUM];
    unsigned int size[OVL_LAYER_NUM];
} OVL_LAYER_INFO;

//------------------------------------------------------------------------------
// handle address for t-play
//------------------------------------------------------------------------------
unsigned int tplay_handle_virt_addr; // SW will access this
dma_addr_t handle_pa;

unsigned int init_tplay_handle(void)
{
    DISP_DBG("[SEC] init_tplay_handle \n");
    void *va = dma_alloc_coherent(NULL, sizeof(handle_pa), &handle_pa, GFP_KERNEL);
    if (0 == va)
    {
        DISP_MSG("[SEC] failed to allocate handle_pa \n");
    }
    tplay_handle_virt_addr = (unsigned int)va;
    return tplay_handle_virt_addr;
}

int write_tplay_handle(unsigned int handle_value)
{
    static int executed = 0; // this function can execute only once
    if (0 != tplay_handle_virt_addr && 0 == executed)
    {
        DISP_DBG("[SEC] write_tplay_handle 0x%x \n", handle_value);
        unsigned int *x = (unsigned int *)tplay_handle_virt_addr;
        *x = handle_value;
        executed = 1;
    }
}

unsigned int get_tplay_handle(void)
{
    return tplay_handle_virt_addr;
}

//------------------------------------------------------------------------------
// set the handle address to t-play driver
//------------------------------------------------------------------------------
#include "tlcApisec.h"

#define MC_UUID_SDRV_DEFINE { 0x05, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
static const struct mc_uuid_t MC_UUID_HACC = {MC_UUID_SDRV_DEFINE};
static struct mc_session_handle tplaySessionHandle;
static dapc_tciMessage_t *pTplayTci  = NULL;

/* DO NOT invoke this function unless you get HACC lock */
static int open_tplay_driver_connection(void)
{
    enum mc_result mcRet = MC_DRV_OK;

    if (tplaySessionHandle.session_id != 0)
    {
        DISP_MSG("tplay TDriver session already created\n");
        return 0;
    }

    DISP_DBG("=============== late init tplay TDriver session ===============\n");
    do
    {
        mcRet = mc_open_device(mc_deviceId);
        if (MC_DRV_OK != mcRet)
        {
            DISP_MSG("mc_open_device failed: %d @%s line %d\n", mcRet, __func__, __LINE__);
        }

        /* Initialize session handle data */
        memset(&tplaySessionHandle, 0, sizeof(tplaySessionHandle));

        /* Allocating WSM for DCI */
        mcRet = mc_malloc_wsm(mc_deviceId, 0, sizeof(dapc_tciMessage_t), (uint8_t **) &pTplayTci, 0);
        if (MC_DRV_OK != mcRet)
        {
            DISP_ERR("mc_malloc_wsm failed: %d @%s line %d\n", mcRet, __func__, __LINE__);
            return -1;
        }

        /* Open session the TDriver */
        memset(&drSessionHandle, 0, sizeof(drSessionHandle));
        drSessionHandle.device_id = mc_deviceId;
        mcRet = mc_open_session(&tplaySessionHandle,
                              &MC_UUID_HACC,
                              (uint8_t *) pTplayTci,
                              (uint32_t) sizeof(dapc_tciMessage_t));
        if (MC_DRV_OK != mcRet)
        {
            DISP_ERR("mc_open_session failed: %d @%s line %d\n", mcRet, __func__, __LINE__);
            // if failed clear session handle
            memset(&tplaySessionHandle, 0, sizeof(tplaySessionHandle));
            return -1;
        }
    } while (false);

    return (MC_DRV_OK == mcRet) ? 0 : -1;
}

/* DO NOT invoke this function unless you get HACC lock */
static int close_tplay_driver_connection(void)
{
    DISP_DBG("=============== close tplay TDriver session ===============\n");
        /* Close session*/
    enum mc_result mcRet = MC_DRV_OK;
    if (tplaySessionHandle.session_id != 0) // we have an valid session
    {
        mcRet = mc_close_session(&tplaySessionHandle);
        if (MC_DRV_OK != mcRet)
        {
            DISP_ERR("mc_close_session failed: %d @%s line %d\n", mcRet, __func__, __LINE__);
            return -1;
        }
    }

    mcRet = mc_free_wsm(mc_deviceId, (uint8_t *)pTplayTci);
    if (MC_DRV_OK != mcRet)
    {
        DISP_ERR("mc_free_wsm failed: %d @%s line %d\n", mcRet, __func__, __LINE__);
        return -1;
    }

    return 0;
}

// return 0 for success and -1 for error
int set_tplay_handle_addr_request(void)
{
    //int ret = SEC_OK;
    DISP_DBG("[SEC] set_tplay_handle_addr_request \n");
    int ret = 0;
    enum mc_result mcRet = MC_DRV_OK;

    open_tplay_driver_connection();
#if 1
    if (tplaySessionHandle.session_id == 0)
    {
        DISP_ERR("[SEC] invalid tplay session \n");
        return -1;
    }

    DISP_DBG("[SEC] tplay_handle_virt_addr=0x%x \n", tplay_handle_virt_addr);
    /* set other TCI parameter */
    pTplayTci->tplay_handle_low_addr = tplay_handle_virt_addr;
    pTplayTci->tplay_handle_high_addr = 0;
    /* set TCI command */
    pTplayTci->cmd.header.commandId = CMD_TPLAY_REQUEST;

    /* notify the trustlet */
    DISP_DBG("[SEC] notify Tlsec trustlet CMD_TPLAY_REQUEST \n");
    mcRet = mc_notify(&tplaySessionHandle);
    if (MC_DRV_OK != mcRet)
    {
        DISP_ERR("[SEC] mc_notify failed: %d @%s line %d\n", mcRet, __func__, __LINE__);
        ret = -1;
        goto _notify_to_trustlet_fail;
    }

    /* wait for response from the trustlet */
    if (MC_DRV_OK != mc_wait_notification(&tplaySessionHandle, /*MC_INFINITE_TIMEOUT*/20000))
    {
        DISP_ERR("[SEC] mc_wait_notification 20s timeout: %d @%s line %d\n", mcRet, __func__, __LINE__);
        ret = -1;
        goto _notify_from_trustlet_fail;
    }

    DISP_DBG("[SEC] CMD_TPLAY_REQUEST result=%d, return code=%d\n", pTplayTci->result, pTplayTci->rsp.header.returnCode);
#endif

_notify_from_trustlet_fail:
_notify_to_trustlet_fail:
    close_tplay_driver_connection();

    return ret;
}

//------------------------------------------------------------------------------
// Static variables
static struct task_struct *threadId;
static struct cdev tplay_cdev;

static struct mc_bulk_map mapped_info;
static OVL_CONFIG_STRUCT *ovl_config = NULL;

//------------------------------------------------------------------------------
// layer flag: the status of current ovl layer security setting.
//------------------------------------------------------------------------------
// stores the security flag each time the OVL layer is configurated.
static unsigned int ovl_layer_flag[OVL_LAYER_NUM] = {0};
DEFINE_MUTEX(ovlLayerFlagMutex);

// note that {secure} would be type MTK_FB_OVL_LAYER_SECURE_MODE
void update_layer_flag(unsigned int layer, unsigned int secure)
{
    mutex_lock(&ovlLayerFlagMutex);
    if (layer >= OVL_LAYER_NUM || layer < 0)
    {
        mutex_unlock(&ovlLayerFlagMutex);
        return; // nothing was done
    }
    if (ovl_layer_flag[layer] != secure)
    {
        DISP_MSG("update secure flag %d >> %d for layer #%d",
                ovl_layer_flag[layer], secure, layer);
        ovl_layer_flag[layer] = secure;
    }
    mutex_unlock(&ovlLayerFlagMutex);
}

// return layer buffer flag type as type MTK_FB_OVL_LAYER_SECURE_MODE
unsigned int is_ovl_secured(void)
{
    unsigned int secure = 0;
    int i;
    mutex_lock(&ovlLayerFlagMutex);
    for (i = 0; i < OVL_LAYER_NUM; i++)
    {
        secure |= ovl_layer_flag[i];
    }
    mutex_unlock(&ovlLayerFlagMutex);
    return secure;
}

//------------------------------------------------------------------------------
// last ovl layer info:
//------------------------------------------------------------------------------
static OVL_LAYER_INFO last_ovl_layer_info = {0};
static DEFINE_MUTEX(LastLayerInfoMutex);

void acquireLastLayerInfoMutex(void)
{
    mutex_lock(&LastLayerInfoMutex);
}

void releaseLastLayerInfoMutex(void)
{
    mutex_unlock(&LastLayerInfoMutex);
}

int get_ovl_layer_info(OVL_LAYER_INFO *pInfo)
{
    int i = 0;
    for (i = 0; i < OVL_LAYER_NUM; i++)
    {
        pInfo->layer_en[i] = last_ovl_layer_info.layer_en[i];
        pInfo->addr[i] = last_ovl_layer_info.addr[i];
        pInfo->size[i] = last_ovl_layer_info.size[i];
        printk("[DDP] get_ovl_layer_info layer=%d, layer_en=%d, addr=0x%x, size=%d \n",
            i, last_ovl_layer_info.layer_en[i], last_ovl_layer_info.addr[i], last_ovl_layer_info.size[i]);
    }
    return 0;
}

int update_ovl_layer_info(OVL_CONFIG_STRUCT *pConfig)
{
    unsigned int layer = pConfig->layer;
    last_ovl_layer_info.layer_en[layer] = pConfig->layer_en;
    last_ovl_layer_info.addr[layer] = pConfig->addr;
    last_ovl_layer_info.size[layer] = (pConfig->src_h + pConfig->src_y) * pConfig->src_pitch;
    return 0;
}

static int notifyTrustletCommandValue(uint32_t command, uint32_t value1, uint32_t value2)
{
    enum mc_result ret = MC_DRV_OK;
    late_init_session();

    if (tlSessionHandle.session_id == 0)
    {
        DISP_ERR("invalid session handle of Trustlet @%s line %d\n", __func__, __LINE__);
        return -1;
    }

    /* prepare data */
    memset(pTci, 0, sizeof(tciMessage_t));
    pTci->cmdSvp.header.commandId = command;
    pTci->Value1 = value1;
    pTci->Value2 = value2;

    DISP_DBG("notify Trustlet CMD: %d \n", command);
    /* Notify the trustlet */
    ret = mc_notify(&tlSessionHandle);
    if (MC_DRV_OK != ret)
    {
        DISP_ERR("mc_notify failed: %d @%s line %d\n", ret, __func__, __LINE__);
        return -1;
    }

    DISP_DBG("Trustlet CMD: %d wait notification \n", command);
    /* Wait for response from the trustlet */
    ret = mc_wait_notification(&tlSessionHandle, MC_INFINITE_TIMEOUT);
    if (MC_DRV_OK != ret)
    {
        DISP_ERR("mc_wait_notification failed: %d @%s line %d\n", ret, __func__, __LINE__);
        return -1;
    }

    DISP_MSG("Trustlet CMD: %d done \n", command);

    return 0;
}

int switch_ovl_secure_port(unsigned int secure)
{
    // value1: 0 for DISP_OVL_0
    return notifyTrustletCommandValue(CMD_SVP_SWITCH_SECURE_PORT, 0, (uint32_t)secure);
}

int switch_debug_layer(unsigned int is_secure_debug_layer)
{
    // value2: not used
    return notifyTrustletCommandValue(CMD_SVP_SWITCH_DEBUG_LAYER, is_secure_debug_layer, 0);
}

//------------------------------------------------------------------------------
// TODO: remove. for debugging purpose
int call_dump_ovl_register(void)
{
    int ret = late_init_session();
    ret = ovl_layer_dump_register();
    close_session();
    return ret;
}

int call_dummy_test_dci(void)
{
    int ret = late_init_session();
    ret = dummy_test_dci();
    close_session();
    return ret;
}
// TODO end
//------------------------------------------------------------------------------


/* This is our main function which will be called by the init function once the
 * module is loaded and our tplay thread is started.
 * For the example here we do everything, open, close, and call a function in the
 * driver. However you can implement this to wait for event from the driver, or
 * to do whatever is required.
 */
int mainThread(void * uarg) {
    int ret = 0;
    //ret = late_init_session();
    return ret;
}

/*This is where we handle the IOCTL commands coming from user space*/

static long tplay_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
    int ret = 0;

    DISP_MSG("Entering ioctl switch \n");

    switch (cmd) {

    case TPLAY_DRV_DDP_CMD_1:
        DISP_MSG("TPLAY 1\n");
        break;

    case TPLAY_DRV_DDP_CMD_2:
        DISP_MSG("TPLAY 1\n");
        break;

    default:
        return -ENOTTY;
    }

    return ret;
}

static struct file_operations tplay_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = tplay_ioctl,
};

// This is the initial point of entry and first code to execute after the insmod command is sent
static int __init tlckTPlay_init(void)
{
    DISP_MSG("=============== Running TPlay Kernel TLC : ddp_svp driver ===============\n");

    dev_t devno;
    int err;
    static struct class *tplay_class;


    err = alloc_chrdev_region(&devno, 0, 1, TPLAY_DEV_NAME);
    if (err) {
        DISP_ERR(KERN_ERR "Unable to allocate TPLAY device number\n");
        return err;
    }

    cdev_init(&tplay_cdev, &tplay_fops);
    tplay_cdev.owner = THIS_MODULE;

    err = cdev_add(&tplay_cdev, devno, 1);
    if (err) {
        DISP_ERR(KERN_ERR "Unable to add Tplay char device\n");
        unregister_chrdev_region(devno, 1);
        return err;
    }

    tplay_class = class_create(THIS_MODULE, "tplay_ddp_svp");
    device_create(tplay_class, NULL, devno, NULL, TPLAY_DEV_NAME);

    // make sure the session handle cleared
#ifdef TPLAY_TL_SESSION
    memset(&tlSessionHandle, 0, sizeof(tlSessionHandle));
#endif
#ifdef TPLAY_DRV_SESSION
    memset(&drSessionHandle, 0, sizeof(drSessionHandle));
    memset(&memSessionHandle, 0, sizeof(memSessionHandle));
#endif
    /* Create the TlcTplay Main thread */
    threadId = kthread_run(mainThread, NULL, "dci_thread");
    if (!threadId) {
        DISP_ERR(KERN_ERR "Unable to start tplay main thread\n");
        return -1;
    }

    return 0;
}

static void __exit tlckTPlay_exit(void)
{
    DISP_MSG("=============== Unloading TPlay Kernel TLC : ddp_svp driver ===============\n");

    close_session();
}

module_init(tlckTPlay_init);
module_exit(tlckTPlay_exit);


MODULE_AUTHOR("Trustonic-MediaTek");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("TPlay Kernel TLC ddp_svp");

//------------------------------------------------------------------------------
int late_init_session(void)
{
    enum mc_result mcRet = MC_DRV_OK;

    late_open_mobicore_device();
    late_init_session_tl();
    late_init_session_drv();

    return 0;
}

enum mc_result late_init_session_tl(void)
{
    enum mc_result mcRet = MC_DRV_OK;

#ifdef TPLAY_TL_SESSION
    if (tlSessionHandle.session_id != 0)
    {
        DISP_DBG("trustlet session already created\n");
        return MC_DRV_OK;
    }
#endif

    DISP_DBG("=============== late init trustlet session ===============\n");
    do
    {
        /* Initialize session handle data */
#ifdef TPLAY_TL_SESSION
        memset(&tlSessionHandle, 0, sizeof(tlSessionHandle));
#endif

#ifdef TPLAY_TL_SESSION
        /* Allocating WSM for TCI */
        mcRet = mc_malloc_wsm(mc_deviceId, 0, sizeof(tciMessage_t), (uint8_t **) &pTci, 0);
        if (MC_DRV_OK != mcRet)
        {
            DISP_ERR("mc_malloc_wsm failed: %d @%s line %d\n", mcRet, __func__, __LINE__);
            break;
        }
#endif

#ifdef TPLAY_TL_SESSION
        /* Open session the trustlet */
        memset(&tlSessionHandle, 0, sizeof(tlSessionHandle));
        tlSessionHandle.device_id = mc_deviceId;
        mcRet = mc_open_session(&tlSessionHandle,
                              &uuid,
                              (uint8_t *) pTci,
                              (uint32_t) sizeof(tciMessage_t));
        if (MC_DRV_OK != mcRet)
        {
            DISP_ERR("mc_open_session failed: %d @%s line %d\n", mcRet, __func__, __LINE__);
            // if failed clear session handle
            memset(&tlSessionHandle, 0, sizeof(tlSessionHandle));
            break;
        }
#endif
    } while (false);

    return mcRet;
}

enum mc_result late_init_session_drv(void)
{
    enum mc_result mcRet = MC_DRV_OK;

#ifdef TPLAY_DRV_SESSION
    if (drSessionHandle.session_id != 0)
    {
        DISP_DBG("TDriver session already created\n");
        return MC_DRV_OK;
    }
#endif

    DISP_DBG("=============== late init TDriver session ===============\n");
    do
    {
        /* Initialize session handle data */
#ifdef TPLAY_DRV_SESSION
        memset(&drSessionHandle, 0, sizeof(drSessionHandle));
#endif

#ifdef TPLAY_DRV_SESSION
        /* Allocating WSM for DCI */
        mcRet = mc_malloc_wsm(mc_deviceId, 0, sizeof(dciMessage_t), (uint8_t **) &pDci, 0);
        if (MC_DRV_OK != mcRet)
        {
            DISP_ERR("mc_malloc_wsm failed: %d @%s line %d\n", mcRet, __func__, __LINE__);
            break;
        }
#endif

#ifdef TPLAY_DRV_SESSION
        /* Open session the TDriver */
        memset(&drSessionHandle, 0, sizeof(drSessionHandle));
        drSessionHandle.device_id = mc_deviceId;
        mcRet = mc_open_session(&drSessionHandle,
                              &uuid_dr,
                              (uint8_t *) pDci,
                              (uint32_t) sizeof(dciMessage_t));
        if (MC_DRV_OK != mcRet)
        {
            DISP_ERR("mc_open_session failed: %d @%s line %d\n", mcRet, __func__, __LINE__);
            // if failed clear session handle
            memset(&drSessionHandle, 0, sizeof(drSessionHandle));
            break;
        }
#endif
    } while (false);

    return mcRet;
}

enum mc_result late_init_session_mem(void)
{
    enum mc_result mcRet = MC_DRV_OK;

#ifdef TPLAY_DRV_SESSION
    if (memSessionHandle.session_id != 0)
    {
        DISP_DBG("sec_mem TDriver session already created\n");
        return MC_DRV_OK;
    }
#endif

    DISP_DBG("=============== late init sec_mem TDriver session ===============\n");
    do
    {
        /* Initialize session handle data */
#ifdef TPLAY_DRV_SESSION
        memset(&memSessionHandle, 0, sizeof(memSessionHandle));
#endif

#ifdef TPLAY_DRV_SESSION
        /* Allocating WSM for DCI */
        mcRet = mc_malloc_wsm(mc_deviceId, 0, sizeof(dciMessage_t), (uint8_t **) &pMemDci, 0);
        if (MC_DRV_OK != mcRet)
        {
            DISP_ERR("mc_malloc_wsm failed: %d @%s line %d\n", mcRet, __func__, __LINE__);
            break;
        }
#endif

#ifdef TPLAY_DRV_SESSION
        /* Open session the TDriver */
        memset(&memSessionHandle, 0, sizeof(memSessionHandle));
        memSessionHandle.device_id = mc_deviceId;
        mcRet = mc_open_session(&memSessionHandle,
                              &uuid_mem,
                              (uint8_t *) pMemDci,
                              (uint32_t) sizeof(dciMessage_t));
        if (MC_DRV_OK != mcRet)
        {
            DISP_ERR("mc_open_session failed: %d @%s line %d\n", mcRet, __func__, __LINE__);
            // if failed clear session handle
            memset(&memSessionHandle, 0, sizeof(memSessionHandle));
            break;
        }
#endif
    } while (false);

    return mcRet;
}

enum mc_result late_open_mobicore_device(void)
{
    DISP_DBG("=============== open mobicore device ===============\n");
    /* Open MobiCore device */
    enum mc_result mcRet = mc_open_device(mc_deviceId);
    if (MC_DRV_OK != mcRet)
    {
        DISP_ERR("mc_open_device failed: %d @%s line %d\n", mcRet, __func__, __LINE__);
        return mcRet;
    }
}

void close_session(void)
{
    close_session_tl();
    close_session_drv();
    close_mobicore_device();
}

void close_session_tl(void)
{
    DISP_DBG("=============== close trustlet session ===============\n");
        /* Close session*/
    enum mc_result mcRet = MC_DRV_OK;
#ifdef TPLAY_TL_SESSION
    if (tlSessionHandle.session_id != 0) // we have an valid session
    {
        mcRet = mc_close_session(&tlSessionHandle);
        if (MC_DRV_OK != mcRet)
        {
            DISP_ERR("mc_close_session failed: %d @%s line %d\n", mcRet, __func__, __LINE__);
            return;
        }
    }

    mcRet = mc_free_wsm(mc_deviceId, (uint8_t *)pTci);
    if (MC_DRV_OK != mcRet)
    {
        DISP_ERR("mc_free_wsm failed: %d @%s line %d\n", mcRet, __func__, __LINE__);
        return;
    }
#endif
}

void close_session_drv(void)
{
    DISP_DBG("=============== close TDriver session ===============\n");
        /* Close session*/
    enum mc_result mcRet = MC_DRV_OK;
#ifdef TPLAY_DRV_SESSION
    if (drSessionHandle.session_id != 0) // we have an valid session
    {
        mcRet = mc_close_session(&drSessionHandle);
        if (MC_DRV_OK != mcRet)
        {
            DISP_ERR("mc_close_session failed: %d @%s line %d\n", mcRet, __func__, __LINE__);
            return;
        }
    }

    mcRet = mc_free_wsm(mc_deviceId, (uint8_t *)pDci);
    if (MC_DRV_OK != mcRet)
    {
        DISP_ERR("mc_free_wsm failed: %d @%s line %d\n", mcRet, __func__, __LINE__);
        return;
    }
#endif
}

void close_session_mem(void)
{
    DISP_DBG("=============== close sec_mem TDriver session ===============\n");
        /* Close session*/
    enum mc_result mcRet = MC_DRV_OK;
#ifdef TPLAY_DRV_SESSION
    if (memSessionHandle.session_id != 0) // we have an valid session
    {
        mcRet = mc_close_session(&memSessionHandle);
        if (MC_DRV_OK != mcRet)
        {
            DISP_ERR("mc_close_session failed: %d @%s line %d\n", mcRet, __func__, __LINE__);
            return;
        }
    }

    mcRet = mc_free_wsm(mc_deviceId, (uint8_t *)pMemDci);
    if (MC_DRV_OK != mcRet)
    {
        DISP_ERR("mc_free_wsm failed: %d @%s line %d\n", mcRet, __func__, __LINE__);
        return;
    }
#endif
}

void close_mobicore_device(void)
{
    DISP_DBG("=============== close mobicore device ===============\n");
        /* Close MobiCore device */
    enum mc_result mcRet = mc_close_device(mc_deviceId);
    if (MC_DRV_OK != mcRet)
    {
        DISP_ERR("mc_close_device failed: %d @%s, line %d\n", mcRet, __func__, __LINE__);
        return;
    }
}

//------------------------------------------------------------------------------
// TODO: is these functions used?
void prepare_ovl_config_session(void)
{
    enum mc_result ret = MC_DRV_OK;
    if (NULL == ovl_config)
    {
        /*ret = mc_malloc_wsm(mc_deviceId, 0, sizeof(OVL_CONFIG_STRUCT), (uint8_t **)(&ovl_config), 0);
        if (MC_DRV_OK != ret)
        {
            DISP_ERR("mc_malloc_wsm failed: %d @%s line %d\n", ret, __func__, __LINE__);
            return;
        }*/
        ovl_config = kmalloc(sizeof(OVL_CONFIG_STRUCT), GFP_KERNEL);
        DISP_DBG("kernel malloc memory for ovl config structure %x \n", ovl_config);
        memset(ovl_config, 0, sizeof(OVL_CONFIG_STRUCT));
    }

    ret = mc_map(&tlSessionHandle, (void*)ovl_config, sizeof(OVL_CONFIG_STRUCT), &mapped_info);
    if (MC_DRV_OK != ret)
    {
        DISP_ERR("failed to bulk map memory between kernel and secure world %d\n", ret);
    }
}

void release_ovl_config_session(void)
{
    enum mc_result ret = MC_DRV_OK;
    /* unmap configuration structure data buffer after finished */
    ret = mc_unmap(&tlSessionHandle, (void*)ovl_config, &mapped_info);
    if (MC_DRV_OK != ret)
    {
        DISP_ERR("mc_unmap returned: %d @%s, line %d\n", ret, __func__, __LINE__);
        return;
    }

    if (NULL != ovl_config)
    {
        /*ret = mc_free_wsm(mc_deviceId, (uint8_t *)ovl_config);
        if (MC_DRV_OK != ret)
        {
            DISP_ERR("mc_free_wsm failed: %d @%s line %d\n", ret, __func__, __LINE__);
        }*/
        kfree(ovl_config);
        DISP_DBG("kernel free memory for ovl config structure \n");
    }
}
// TODO end
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// return 0 for success and -1 for failure
int ovl_layer_config_secure(void* config, uint32_t len, uint32_t secure)
{
    enum mc_result ret = MC_DRV_OK;
    late_init_session();

    if (tlSessionHandle.session_id == 0)
    {
        DISP_ERR("invalid session handle of Trustlet @%s line %d\n", __func__, __LINE__);
        return -1;
    }

    memcpy(ovl_config, config, len);

    /* prepare data */
    memset(pTci, 0, sizeof(tciMessage_t));
    pTci->cmdSvp.header.commandId = CMD_SVP_CONFIG_OVL_LAYER;
    pTci->Value1 = secure;

    DISP_DBG("notify Trustlet CMD_SVP_CONFIG_OVL_LAYER \n");
    /* Notify the trustlet */
    ret = mc_notify(&tlSessionHandle);
    if (MC_DRV_OK != ret)
    {
        DISP_ERR("mc_notify failed: %d @%s line %d\n", ret, __func__, __LINE__);
        return -1;
    }

    DISP_DBG("Trustlet CMD_SVP_CONFIG_OVL_LAYER wait notification \n");
    /* Wait for response from the trustlet */
    ret = mc_wait_notification(&tlSessionHandle, MC_INFINITE_TIMEOUT);
    if (MC_DRV_OK != ret)
    {
        DISP_ERR("mc_wait_notification failed: %d @%s line %d\n", ret, __func__, __LINE__);
        return -1;
    }

    DISP_MSG("Trustlet CMD_SVP_CONFIG_OVL_LAYER done \n");

    return 0;
}

// return 0 for success and -1 for failure
// directly access TDriver via TCI
int ovl_layer_switch_secure(uint32_t layer, int en)
{
    enum mc_result ret = MC_DRV_OK;
    late_init_session();

    if (drSessionHandle.session_id == 0)
    {
        DISP_ERR("invalid session handle of TDriver @%s line %d\n", __func__, __LINE__);
        return -1;
    }

    /* prepare data */
    pDci->command.header.commandId = CMD_SVP_DRV_SWITCH_OVL_LAYER;
    pDci->Value1 = layer;
    pDci->Value2 = (uint32_t)en;

    DISP_DBG("notify TDriver CMD_SVP_DRV_SWITCH_OVL_LAYER \n");
    /* Notify the trustlet */
    ret = mc_notify(&drSessionHandle);
    if (MC_DRV_OK != ret)
    {
        DISP_ERR("mc_notify failed: %d @%s line %d\n", ret, __func__, __LINE__);
        return -1;
    }

    DISP_DBG("TDriver CMD_SVP_DRV_SWITCH_OVL_LAYER wait notification \n");
    /* Wait for response from the trustlet */
    ret = mc_wait_notification(&drSessionHandle, MC_INFINITE_TIMEOUT);
    if (MC_DRV_OK != ret)
    {
        DISP_ERR("mc_wait_notification failed: %d @%s line %d\n", ret, __func__, __LINE__);
        return -1;
    }

    DISP_MSG("TDriver CMD_SVP_DRV_SWITCH_OVL_LAYER done \n");

    return 0;
}

//------------------------------------------------------------------------------
// debugging / UT / IT
// start:
//------------------------------------------------------------------------------
#ifdef TPLAY_DRV_SESSION
// return -1 for failure and 0 for success
int ovl_layer_dump_register(void)
{
    if (drSessionHandle.session_id == 0)
    {
        DISP_ERR("invalid session handle of TDriver @%s line %d\n", __func__, __LINE__);
        return -1;
    }

    enum mc_result ret = MC_DRV_OK;

    /* prepare data */
    pDci->command.header.commandId = CMD_SVP_DRV_DUMP_OVL_REGISTER;

    DISP_DBG("notify TDriver CMD_SVP_DRV_DUMP_OVL_REGISTER \n");
    /* Notify the trustlet */
    ret = mc_notify(&drSessionHandle);
    if (MC_DRV_OK != ret)
    {
        DISP_ERR("mc_notify failed: %d @%s line %d\n", ret, __func__, __LINE__);
        return -1;
    }

    DISP_DBG("TDriver CMD_SVP_DRV_DUMP_OVL_REGISTER wait notification \n");
    /* Wait for response from the trustlet */
    ret = mc_wait_notification(&drSessionHandle, MC_INFINITE_TIMEOUT);
    if (MC_DRV_OK != ret)
    {
        DISP_ERR("mc_wait_notification failed: %d @%s line %d\n", ret, __func__, __LINE__);
        return -1;
    }

    DISP_MSG("TDriver CMD_SVP_DRV_DUMP_OVL_REGISTER done \n");

    return 0;
}
#endif // TPLAY_DRV_SESSION

#ifdef TPLAY_DRV_SESSION
int dummy_test_dci(void)
{
    if (drSessionHandle.session_id == 0)
    {
        DISP_ERR("invalid session handle of TDriver @%s line %d\n", __func__, __LINE__);
        return -1;
    }

    enum mc_result ret = MC_DRV_OK;

    /* allocate a buffer and map it to SWD */
    ovl_config = kmalloc(sizeof(OVL_CONFIG_STRUCT), GFP_KERNEL);
    memset(ovl_config, 0, sizeof(OVL_CONFIG_STRUCT));

    ret = mc_map(&drSessionHandle, (void*)ovl_config, sizeof(OVL_CONFIG_STRUCT), &mapped_info);
    if (MC_DRV_OK != ret)
    {
        DISP_ERR("mc_map failed: %d @%s line %d\n", ret, __func__, __LINE__);
        /* continue anyway */
    }

    /* prepare data */
    memset(pDci, 0, sizeof(dciMessage_t));
    pDci->command.header.commandId = CMD_SVP_DRV_DUMMY;

    DISP_DBG("notify TDriver CMD_SVP_DRV_DUMMY \n");
    /* Notify the trustlet */
    ret = mc_notify(&drSessionHandle);
    if (MC_DRV_OK != ret)
    {
        DISP_ERR("mc_notify failed: %d @%s line %d\n", ret, __func__, __LINE__);
        return -1;
    }

    DISP_DBG("TDriver CMD_SVP_DRV_DUMMY wait notification \n");
    /* Wait for response from the trustlet */
    ret = mc_wait_notification(&drSessionHandle, MC_INFINITE_TIMEOUT);
    if (MC_DRV_OK != ret)
    {
        DISP_ERR("mc_wait_notification failed: %d @%s line %d\n", ret, __func__, __LINE__);
        return -1;
    }

    DISP_MSG("TDriver CMD_SVP_DRV_DUMMY done \n");

    /* unmap and free the buffer */
    ret = mc_unmap(&drSessionHandle, (void*)ovl_config, &mapped_info);
    if (MC_DRV_OK != ret)
    {
        DISP_ERR("mc_unmap failed: %d @%s line %d\n", ret, __func__, __LINE__);
        /* continue anyway */;
    }

    kfree(ovl_config);

    return 0;
}
#endif // TPLAY_DRV_SESSION

#ifdef TPLAY_TL_SESSION
int dummy_test_tci(void)
{
    if (tlSessionHandle.session_id == 0)
    {
        DISP_ERR("invalid session handle of Trustlet @%s line %d\n", __func__, __LINE__);
        return -1;
    }

    enum mc_result ret = MC_DRV_OK;

    /* allocate a buffer and map it to SWD */
#ifndef USE_MC_MALLOC_WSM
    ovl_config = vmalloc(sizeof(OVL_CONFIG_STRUCT));
#else
    ret = mc_malloc_wsm(mc_deviceId, 1, sizeof(OVL_CONFIG_STRUCT), (uint8_t **)(&ovl_config), 0);
#endif
    memset(ovl_config, 0, sizeof(OVL_CONFIG_STRUCT));

    ret = mc_map(&tlSessionHandle, (void*)ovl_config, sizeof(OVL_CONFIG_STRUCT), &mapped_info);
    if (MC_DRV_OK != ret)
    {
        DISP_ERR("mc_map failed: %d @%s line %d\n", ret, __func__, __LINE__);
        /* continue anyway */
    }

    /* prepare data */
    memset(pTci, 0, sizeof(tciMessage_t));
    pTci->cmdSvp.header.commandId = CMD_SVP_DUMMY;

    DISP_DBG("notify Trustlet CMD_SVP_DUMMY \n");
    /* Notify the trustlet */
    ret = mc_notify(&tlSessionHandle);
    if (MC_DRV_OK != ret)
    {
        DISP_ERR("mc_notify failed: %d @%s line %d\n", ret, __func__, __LINE__);
        return -1;
    }

    DISP_DBG("Trustlet CMD_SVP_DUMMY wait notification \n");
    /* Wait for response from the trustlet */
    ret = mc_wait_notification(&tlSessionHandle, MC_INFINITE_TIMEOUT);
    if (MC_DRV_OK != ret)
    {
        DISP_ERR("mc_wait_notification failed: %d @%s line %d\n", ret, __func__, __LINE__);
        return -1;
    }

    DISP_MSG("Trustlet CMD_SVP_DUMMY done \n");

    /* unmap and free the buffer */
    ret = mc_unmap(&tlSessionHandle, (void*)ovl_config, &mapped_info);
    if (MC_DRV_OK != ret)
    {
        DISP_ERR("mc_unmap failed: %d @%s line %d\n", ret, __func__, __LINE__);
        /* continue anyway */;
    }

#ifndef USE_MC_MALLOC_WSM
    vfree(ovl_config);
#else
    ret = mc_free_wsm(mc_deviceId, (uint8_t *)ovl_config);
#endif // USE_MC_MALLOC_WSM

    return 0;
}
#endif // TPLAY_TL_SESSION

// debugging / UT / IT end
//------------------------------------------------------------------------------

// return 0 for success and -1 for failure
// allocate a buffer to place OVL_CONFIG_STRUCT and notify to secured driver
// make sure a tlSessionHandle is valid.
static int notify_ovl_config(void)
{
    enum mc_result ret = MC_DRV_OK;

    if (NULL == ovl_config)
    {
        /* allocate a buffer and map it to SWD */
#ifndef USE_MC_MALLOC_WSM
        ovl_config = vmalloc(sizeof(OVL_CONFIG_STRUCT));
#else
        ret = mc_malloc_wsm(mc_deviceId, 1, sizeof(OVL_CONFIG_STRUCT), (uint8_t **)(&ovl_config), 0);
#endif
        memset(ovl_config, 0, sizeof(OVL_CONFIG_STRUCT));
    }

    ret = mc_map(&tlSessionHandle, (void*)ovl_config, sizeof(OVL_CONFIG_STRUCT), &mapped_info);
    if (MC_DRV_OK != ret)
    {
        DISP_ERR("mc_map failed: %d @%s line %d\n", ret, __func__, __LINE__);
        return -1;
    }

    /* prepare data */
    memset(pTci, 0, sizeof(tciMessage_t));
    pTci->cmdSvp.header.commandId = CMD_SVP_NOTIFY_OVL_CONFIG;
    pTci->Value1 = (uint32_t)(mapped_info.secure_virt_addr);
    pTci->Value2 = mapped_info.secure_virt_len;

    DISP_DBG("notify Trustlet CMD_SVP_NOTIFY_OVL_CONFIG \n");
    /* Notify the trustlet */
    ret = mc_notify(&tlSessionHandle);
    if (MC_DRV_OK != ret)
    {
        DISP_ERR("mc_notify failed: %d @%s line %d\n", ret, __func__, __LINE__);
        return -1;
    }

    DISP_DBG("Trustlet CMD_SVP_NOTIFY_OVL_CONFIG wait notification \n");
    /* Wait for response from the trustlet */
    ret = mc_wait_notification(&tlSessionHandle, MC_INFINITE_TIMEOUT);
    if (MC_DRV_OK != ret)
    {
        DISP_ERR("mc_wait_notification failed: %d @%s line %d\n", ret, __func__, __LINE__);
        return -1;
    }

    DISP_MSG("Trustlet CMD_SVP_NOTIFY_OVL_CONFIG done \n");
    return 0;
}

// return 0 for success and -1 for failure.
// map the current ovl buffers to secure world page table so that it can be accessed after
static int map_ovl_config(void)
{
    enum mc_result ret = MC_DRV_OK;

    // the current ovl setting is stored in the [last_ovl_layer_info] variable
    // note: we assume the vmalloc allocation is successful
    OVL_LAYER_INFO *p_last_ovl_info = vmalloc(sizeof(OVL_LAYER_INFO));
    memset(p_last_ovl_info, 0, sizeof(OVL_LAYER_INFO));
    int result = get_ovl_layer_info(p_last_ovl_info);

    // buffer mapped to secure world
    ret = mc_map(&tlSessionHandle, (void*)p_last_ovl_info, sizeof(OVL_LAYER_INFO), &mapped_info);
    if (MC_DRV_OK != ret)
    {
        DISP_ERR("mc_map failed: %d @%s line %d\n", ret, __func__, __LINE__);
        vfree(p_last_ovl_info);
        return -1;
    }

    /* prepare data */
    memset(pTci, 0, sizeof(tciMessage_t));
    pTci->cmdSvp.header.commandId = CMD_SVP_MAP_OVL_CONFIG;
    pTci->Value1 = (uint32_t)(mapped_info.secure_virt_addr);
    pTci->Value2 = mapped_info.secure_virt_len;

    DISP_DBG("notify Trustlet CMD_SVP_MAP_OVL_CONFIG \n");
    /* Notify the trustlet */
    ret = mc_notify(&tlSessionHandle);
    if (MC_DRV_OK != ret)
    {
        DISP_ERR("mc_notify failed: %d @%s line %d\n", ret, __func__, __LINE__);
        ret = mc_unmap(&tlSessionHandle, (void*)p_last_ovl_info, &mapped_info);
        if (MC_DRV_OK != ret)
        {
            DISP_ERR("mc_unmap failed: %d @%s line %d\n", ret, __func__, __LINE__);
            /* continue anyway */
        }
        vfree(p_last_ovl_info);
        return -1;
    }

    DISP_DBG("Trustlet CMD_SVP_MAP_OVL_CONFIG wait notification \n");
    /* Wait for response from the trustlet */
    ret = mc_wait_notification(&tlSessionHandle, MC_INFINITE_TIMEOUT);
    if (MC_DRV_OK != ret)
    {
        DISP_ERR("mc_wait_notification failed: %d @%s line %d\n", ret, __func__, __LINE__);
        ret = mc_unmap(&tlSessionHandle, (void*)p_last_ovl_info, &mapped_info);
        if (MC_DRV_OK != ret)
        {
            DISP_ERR("mc_unmap failed: %d @%s line %d\n", ret, __func__, __LINE__);
            /* continue anyway */
        }
        vfree(p_last_ovl_info);
        return -1;
    }

    DISP_MSG("Trustlet CMD_SVP_MAP_OVL_CONFIG done \n");
    // now unmap the buffer and free it
    ret = mc_unmap(&tlSessionHandle, (void*)p_last_ovl_info, &mapped_info);
    if (MC_DRV_OK != ret)
    {
        DISP_ERR("mc_unmap failed: %d @%s line %d\n", ret, __func__, __LINE__);
        /* continue anyway */;
    }
    vfree(p_last_ovl_info);
    return 0;
}

int config_ovl_secure(void)
{
    enum mc_result ret = MC_DRV_OK;
    late_init_session();

    if (tlSessionHandle.session_id == 0)
    {
        DISP_ERR("invalid session handle of Trustlet @%s line %d\n", __func__, __LINE__);
        return -1;
    }

    if (-1 == notify_ovl_config())
    {
        return -1;
    }

    if (-1 == map_ovl_config())
    {
        return -1;
    }
#ifdef MANUAL_DEBUG
    /* prepare data */
    memset(pTci, 0, sizeof(tciMessage_t));
    pTci->cmdSvp.header.commandId = CMD_SVP_CONFIG_OVL_SECURE;

    DISP_DBG("notify Trustlet CMD_SVP_CONFIG_OVL_SECURE \n");
    /* Notify the trustlet */
    ret = mc_notify(&tlSessionHandle);
    if (MC_DRV_OK != ret)
    {
        DISP_ERR("mc_notify failed: %d @%s line %d\n", ret, __func__, __LINE__);
        return -1;
    }

    DISP_DBG("Trustlet CMD_SVP_CONFIG_OVL_SECURE wait notification \n");
    /* Wait for response from the trustlet */
    ret = mc_wait_notification(&tlSessionHandle, MC_INFINITE_TIMEOUT);
    if (MC_DRV_OK != ret)
    {
        DISP_ERR("mc_wait_notification failed: %d @%s line %d\n", ret, __func__, __LINE__);
        return -1;
    }

    DISP_MSG("Trustlet CMD_SVP_CONFIG_OVL_SECURE done \n");
#endif // MANUAL_DEBUG
    return 0;
}

int config_ovl_nonsecure(void)
{
    enum mc_result ret = MC_DRV_OK;
    late_init_session();

    if (tlSessionHandle.session_id == 0)
    {
        DISP_ERR("invalid session handle of Trustlet @%s line %d\n", __func__, __LINE__);
        return -1;
    }

    acquireLastLayerInfoMutex();
#ifdef MANUAL_DEBUG
    /* prepare data */
    memset(pTci, 0, sizeof(tciMessage_t));
    pTci->cmdSvp.header.commandId = CMD_SVP_CONFIG_OVL_NONSECURE;

    DISP_DBG("notify Trustlet CMD_SVP_CONFIG_OVL_NONSECURE \n");
    /* Notify the trustlet */
    ret = mc_notify(&tlSessionHandle);
    if (MC_DRV_OK != ret)
    {
        DISP_ERR("mc_notify failed: %d @%s line %d\n", ret, __func__, __LINE__);
        return -1;
    }

    DISP_DBG("Trustlet CMD_SVP_CONFIG_OVL_NONSECURE wait notification \n");
    /* Wait for response from the trustlet */
    ret = mc_wait_notification(&tlSessionHandle, MC_INFINITE_TIMEOUT);
    if (MC_DRV_OK != ret)
    {
        DISP_ERR("mc_wait_notification failed: %d @%s line %d\n", ret, __func__, __LINE__);
        return -1;
    }

    DISP_MSG("Trustlet CMD_SVP_CONFIG_OVL_NONSECURE done \n");
#endif // MANUAL_DEBUG
    debug_svp = 0;
    releaseLastLayerInfoMutex();

    /* unmap and free the buffer */
    ret = mc_unmap(&tlSessionHandle, (void*)ovl_config, &mapped_info);
    if (MC_DRV_OK != ret)
    {
        DISP_ERR("mc_unmap failed: %d @%s line %d\n", ret, __func__, __LINE__);
        /* continue anyway */;
    }

#ifndef USE_MC_MALLOC_WSM
    vfree(ovl_config);
#else
    ret = mc_free_wsm(mc_deviceId, (uint8_t *)ovl_config);
#endif // USE_MC_MALLOC_WSM

    return 0;
}


int disp_path_update_secure_port(void)
{
#ifdef MANUAL_DEBUG
    if (is_secure_port == 1 && is_using_secure_debug_layer == 0 && is_ovl_secured() == 0)
#else
    if (is_secure_port == 1 && is_ovl_secured() == 0)
#endif // MANUAL_DEBUG
    {
        DISP_MSG("[SEC] accessing secure port, switch to non-secure port \n");
        switch_ovl_secure_port(0);
        is_secure_port = 0;
    }
    return 0;
}

