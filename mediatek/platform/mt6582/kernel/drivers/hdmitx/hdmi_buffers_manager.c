#if defined(MTK_HDMI_SUPPORT)
#include <linux/err.h>
#include <linux/slab.h>
#include <asm/ioctl.h>

#include "hdmi_types.h"
#include "mtkfb_info.h"
#include "hdmi_utils.h"
#include "hdmi_buffers_manager.h"

//~~~~~~~~~~~~~~~~~~~~~~~the static variable~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static int hdmi_log_on  = 1;
static int hdmi_mmp_level = 7;

static atomic_t timeline_counter = ATOMIC_INIT(0);
static atomic_t fence_counter = ATOMIC_INIT(0);

static struct sw_sync_timeline *hdmi_timeline ;
static struct ion_client *ion_client;
static struct list_head  HDMI_Buffer_List;

#define FENCE_STEP_COUNTER 1

spinlock_t hdmi_lock;
DEFINE_SPINLOCK(hdmi_lock);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~~~~~~~~~~~extern declare~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
extern HDMI_MMP_Events_t HDMI_MMP_Events;
extern int hdmi_rdma_address_config(bool enable, hdmi_video_buffer_info buffer_info);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ---------------------------------------------------------------------------
//  function implement
// ---------------------------------------------------------------------------

static void hdmi_ion_init()
{
    struct ion_mm_data mm_data;

    if (!ion_client && g_ion_device)
    {
        ion_client = ion_client_create(g_ion_device, "hdmi");

        if (!ion_client)
        {
            HDMI_LOG("create ion client failed!\n");
            return;
        }

        HDMI_LOG("create ion client 0x%p\n", ion_client);
    }
}

static void hdmi_ion_deinit()
{
    if (ion_client)
    {
        ion_client_destroy(ion_client);
        ion_client = NULL;
        HDMI_LOG("destroy ion client 0x%p\n", ion_client);
    }
}

static struct ion_handle *hdmi_ion_import_handle(struct ion_client *client, int fd)
{
    struct ion_handle *handle = NULL;
    struct ion_mm_data mm_data;

    // If no need Ion support, do nothing!
    if (fd == MTK_HDMI_NO_ION_FD)
    {
        HDMI_LOG("NO NEED ion support");
        return handle;
    }

    if (!ion_client)
    {
        HDMI_LOG("invalid ion client!\n");
        return handle;
    }

    if (fd == MTK_HDMI_NO_FENCE_FD)
    {
        HDMI_LOG("invalid ion fd!\n");
        return handle;
    }

    handle = ion_import_dma_buf(client, fd);

    if (IS_ERR_OR_NULL(handle))
    {
        HDMI_LOG("import ion handle failed!\n");
        handle = 0;
        return handle;
    }

    mm_data.mm_cmd = ION_MM_CONFIG_BUFFER;
    mm_data.config_buffer_param.handle = handle;
    mm_data.config_buffer_param.eModuleID = 0;
    mm_data.config_buffer_param.security = 0;
    mm_data.config_buffer_param.coherent = 0;

    if (ion_kernel_ioctl(ion_client, ION_CMD_MULTIMEDIA, &mm_data))
    {
        HDMI_LOG("configure ion buffer failed!\n");
    }

    return handle;
}

static void hdmi_ion_free_handle(struct ion_client *client, struct ion_handle *handle)
{
    if (!ion_client)
    {
        HDMI_LOG("invalid ion client!\n");
        return ;
    }

    if (!handle)
    {
        return ;
    }

    ion_free(client, handle);
    HDMI_LOG("free ion handle 0x%p\n",  handle);
}

static size_t hdmi_ion_phys_mmu_addr(struct ion_client *client, struct ion_handle *handle, unsigned int *mva)
{
    size_t size;

    if (!ion_client)
    {
        HDMI_LOG("invalid ion client!\n");
        return 0;
    }

    if (!handle)
    {
        return 0;
    }

    ion_phys(client, handle, (ion_phys_addr_t *)mva, &size);
    HDMI_LOG("alloc mmu addr hnd=0x%p,mva=0x%08x\n",  handle, (unsigned int)*mva);
    return size;
}

/**
 * fence_counter records counter of last fence created.
 * fence_counter will be increased by FENCE_STEP_COUNTER
 */
static unsigned int hdmi_get_fence_counter()
{
    return atomic_add_return(FENCE_STEP_COUNTER, &fence_counter);
}

/**
 * timeline_counter records counter of last fence created.
 */
static unsigned int hdmi_get_timeline_counter_inc()
{
    return atomic_read(&timeline_counter);
}

static struct sw_sync_timeline *hdmi_create_timeline()
{
    char name[32];
    const char *prefix = "hdmi_timeline";
    sprintf(name, "%s", prefix);

    hdmi_timeline = timeline_create(name);

    if (hdmi_timeline == NULL)
    {
        printk(" error: cannot create timeline! \n");
    }
    else
    {
        HDMI_LOG(" hdmi_create_timeline() %s\n", name);
    }

    return hdmi_timeline;
}

static struct fence_data hdmi_create_fence()
{
    int fenceFd = MTK_HDMI_NO_FENCE_FD;
    struct fence_data data;
    const char *prefix = "hdmi_fence";

    spin_lock_bh(&hdmi_lock);
    data.value = hdmi_get_fence_counter();
    spin_unlock_bh(&hdmi_lock);
    sprintf(data.name, "%s-%d", prefix,  data.value);

    if (hdmi_timeline != NULL)
    {
        if (fence_create(hdmi_timeline, &data))
        {
            printk(" error: cannot create Fence Object! \n");
        }
        else
        {
            fenceFd = data.fence;
        }

        ///HDMI_LOG(" name %s, fenceFd=%d\n", data.name, fenceFd);
    }
    else
    {
        printk(" error: there is no Timeline to create Fence! \n");
    }

    MMProfileLogEx(HDMI_MMP_Events.FenceCreate, MMProfileFlagPulse, fenceFd, data.value);
    return data;
}


static void hdmi_release_fence()
{
    int inc = atomic_read(&fence_counter) - atomic_read(&timeline_counter);

    if (inc <= 0)
    {
        return ;
    }

    if (hdmi_timeline != NULL)
    {
        timeline_inc(hdmi_timeline, inc);
    }

    atomic_add(inc, &timeline_counter);
    MMProfileLogEx(HDMI_MMP_Events.FenceSignal, MMProfileFlagPulse, atomic_read(&fence_counter), inc);
}

/**
 * timeline_counter records counter of this timeline.
 * It should be always posterior to fence_counter when enable is true, otherwise
 * they're equaled
 * timeline_counter will step forward and present current hw used buff counter
 * NOTICE:
 *     Frame dropping maybe happen, we has no cache FIFO now!
 *     When a new buffer is coming, all prior to it will be released
 *     Buf will be released immediately if ovl_layer is disabled
 */
unsigned int hdmi_timeline_inc()
{
    unsigned int fence_cnt, timeline_cnt, inc;

    spin_lock_bh(&hdmi_lock);
    fence_cnt = atomic_read(&fence_counter);
    timeline_cnt = atomic_read(&timeline_counter);
    inc = fence_cnt - timeline_cnt;
    spin_unlock_bh(&hdmi_lock);

    if (inc < 0 || inc > 5)
    {
        if (hdmi_mmp_level > 0)
        {
            MMProfileLogEx(HDMI_MMP_Events.ErrorInfo, MMProfileFlagPulse, Timeline_Err, inc);
        }

        HDMI_LOG("fence error: inc=%d, fence_cnt=%d, timeline_cnt=%d! \n", inc, fence_cnt, timeline_cnt);
        inc = 0;
    }

    spin_lock_bh(&hdmi_lock);
    atomic_add(1, &timeline_counter);
    spin_unlock_bh(&hdmi_lock);
    return atomic_read(&timeline_counter);
}

/**
 * step forward timeline
 * all fence(sync_point) will be signaled prior to it's counter
 * refer to {@link sw_sync_timeline_inc}
 */
static void hdmi_signal_fence()
{
    unsigned int inc = 0;

    if (hdmi_timeline != NULL)
    {
        inc = 1;  ///hdmi_get_timeline_counter_inc();
        timeline_inc(hdmi_timeline, inc);

        if (hdmi_mmp_level > 0)
        {
            MMProfileLogEx(HDMI_MMP_Events.FenceSignal, MMProfileFlagPulse, atomic_read(&timeline_counter), atomic_read(&fence_counter));
        }

        ///HDMI_LOG("  %s:%d, tl %d, fd %d\n", hdmi_timeline->obj.name, hdmi_timeline->value, hdmi_timeline, fence_counter);
    }
    else
    {
        HDMI_LOG(" no Timeline to inc tl %d, fd %d\n", atomic_read(&timeline_counter), atomic_read(&fence_counter));
    }
}

static unsigned int hdmi_query_buf_mva(unsigned int idx)
{
    hdmi_video_buffer_list *buf = 0;

    list_for_each_entry(buf, &HDMI_Buffer_List, list)
    {
        if (buf->idx == idx)
        {
            HDMI_LOG("query buf list=%x, idx=%d, mva=0x%08x\n", buf, idx, buf->mva);
            return buf;
        }
    }

    return 0;
}

void hdmi_sync_init()
{

    hdmi_create_timeline();
    // Reset all counter to 0
    atomic_set(&timeline_counter, 0);
    atomic_set(&fence_counter, 0);
}

void hdmi_sync_destroy()
{

    if (hdmi_timeline != NULL)
    {
        HDMI_LOG(" destroy timeline %s:%d\n", hdmi_timeline->obj.name, hdmi_timeline->value);
        timeline_destroy(hdmi_timeline);
        hdmi_timeline = NULL;
    }

    // Reset all counter to 0
    atomic_set(&timeline_counter, 0);
    atomic_set(&fence_counter, 0);
}

void hdmi_buffer_list_init()
{
    INIT_LIST_HEAD(&HDMI_Buffer_List);
}

bool hdmi_buffer_list_empty()
{
    return list_empty(&HDMI_Buffer_List);
}

/*this function request a buffer that state is inserted from buffer list, and config the mva to RDMA*/
void hdmi_buffer_to_RDMA()
{
    int buf_configed = 0;
    if (!list_empty(&HDMI_Buffer_List))
    {
        hdmi_video_buffer_list *pBuffList = NULL;

        spin_lock_bh(&hdmi_lock);
        pBuffList = list_first_entry(&HDMI_Buffer_List, hdmi_video_buffer_list, list);

        while (pBuffList->buf_state != insert_new)
        {
            buf_configed++;

            if (list_is_last(&pBuffList->list, &HDMI_Buffer_List))
            {
                break;
            }

            pBuffList = list_entry(pBuffList->list.next, hdmi_video_buffer_list, list);
        }

        spin_unlock_bh(&hdmi_lock);

        if ((pBuffList == NULL) || (pBuffList->buf_state != insert_new)
            || (hdmi_rdma_address_config(true, pBuffList->buffer_info) < 0))
        {
            ///HDMI_LOG(" rdma config(pBuffList %x) error to exit\n", pBuffList);

            if (pBuffList && (hdmi_mmp_level > 0))
            {
                MMProfileLogEx(HDMI_MMP_Events.ErrorInfo, MMProfileFlagPulse, Buffer_Not_Enough, pBuffList);
            }
            else if (hdmi_mmp_level > 0)
            {
                MMProfileLogEx(HDMI_MMP_Events.ErrorInfo, MMProfileFlagPulse, Buffer_Not_Enough, buf_configed);
            }

            if ((pBuffList != NULL) && (pBuffList->buf_state == insert_new))
            {
                pBuffList->buf_state = buf_read_done;
                HDMI_LOG(" buffer config error to configed %x, state %d, idx %d\n", pBuffList, pBuffList->buf_state,  pBuffList->idx);
            }
        }
        else
        {
            spin_lock_bh(&hdmi_lock);
            pBuffList->buf_state = reg_configed;
            spin_unlock_bh(&hdmi_lock);

            buf_configed = 1;

            if (hdmi_mmp_level > 0)
            {
                MMProfileLogEx(HDMI_MMP_Events.BufferUsed, MMProfileFlagPulse, pBuffList, buf_configed);
            }
        }

    }
    else
    {
        MMProfileLogEx(HDMI_MMP_Events.ErrorInfo, MMProfileFlagPulse, Buffer_Empt_Err, Buffer_Empt_Err);
        HDMI_LOG(" rdma config buffer is NULL\n");
    }
}

/*this function updates buffers state in buffer list, and free the buffers that state is buf_read_done*/
void hdmi_buffer_state_update()
{
    int buf_sequence = 0;
    hdmi_video_buffer_list *pUpdateList = NULL;

    if (!list_empty(&HDMI_Buffer_List))
    {
        hdmi_video_buffer_list *pBuffList = NULL;

        spin_lock_bh(&hdmi_lock);
        pBuffList = list_first_entry(&HDMI_Buffer_List, hdmi_video_buffer_list, list);

        while (pBuffList)
        {
            if (pBuffList->buf_state == insert_new)
            {
                break;
            }
            else if (pBuffList->buf_state == reg_configed)
            {
                buf_sequence++;
                pBuffList->buf_state = reg_updated;
                if (buf_sequence > 1)
                {
                    pUpdateList->buf_state = buf_read_done;
                }
                pUpdateList = pBuffList;
            }
            else if (pBuffList->buf_state == reg_updated)
            {
                pBuffList->buf_state = buf_read_done;
            }


            if (!list_is_last(&pBuffList->list, &HDMI_Buffer_List))
            {
                pBuffList = list_entry(pBuffList->list.next, hdmi_video_buffer_list, list);
            }
            else
            {
                pBuffList = NULL;
            }

        }

        pBuffList = NULL;
        pBuffList = list_first_entry(&HDMI_Buffer_List, hdmi_video_buffer_list, list);
        spin_unlock_bh(&hdmi_lock);

        int remove_buffer_cnt = 0;
        int using_buf_cnt = 0;

        while (!list_is_last(&pBuffList->list, &HDMI_Buffer_List))
        {
            if (pBuffList && (pBuffList->buf_state == buf_read_done))
            {
                do
                {
                    if (hdmi_mmp_level > 0)
                    {
                        MMProfileLogEx(HDMI_MMP_Events.BufferRemove, MMProfileFlagPulse, pBuffList, pBuffList->buffer_info.src_phy_addr);
                    }
                    else
                    {
                        HDMI_LOG("remove list %x-->buffer %x \n", pBuffList, pBuffList->buffer_info.src_phy_addr);
                    }

#ifdef MTK_HDMI_ION_SUPPORT

                    if (pBuffList->va)
                    {
                        ion_unmap_kernel(ion_client, pBuffList->hnd);
                    }

                    hdmi_ion_free_handle(ion_client, pBuffList->hnd);
#endif
                    spin_lock_bh(&hdmi_lock);
                    list_del(&pBuffList->list);
                    kfree(pBuffList);
                    pBuffList = NULL;
                    spin_unlock_bh(&hdmi_lock);

                    hdmi_timeline_inc();
                    hdmi_signal_fence();

                    remove_buffer_cnt++;

                    spin_lock_bh(&hdmi_lock);
                    pBuffList = list_first_entry(&HDMI_Buffer_List, hdmi_video_buffer_list, list);
                    spin_unlock_bh(&hdmi_lock);
                }
                while (using_buf_cnt--);

                using_buf_cnt = 0;
            }
            else if (pBuffList && (pBuffList->buf_state == create_new))
            {
                using_buf_cnt++;
                pBuffList = list_entry(pBuffList->list.next, hdmi_video_buffer_list, list);
            }
            else
            {
                break;
            }
        }

        if (remove_buffer_cnt > 1)
        {
            if (hdmi_mmp_level > 0)
            {
                MMProfileLogEx(HDMI_MMP_Events.ErrorInfo, MMProfileFlagPulse, Buff_Dup_Err1, remove_buffer_cnt);
            }

            printk("[hdmi] %s, %d remove two buffer one time", __func__, __LINE__);
        }
    }
}

/*this function removes all buffer in buffer list*/
void hdmi_remove_buffers()
{
    HDMI_LOG("fence stop rdma \n");
    hdmi_video_buffer_list *pBuffList = NULL;

    while (!list_empty(&HDMI_Buffer_List))
    {
        spin_lock_bh(&hdmi_lock);
        pBuffList = list_first_entry(&HDMI_Buffer_List, hdmi_video_buffer_list, list);
        spin_unlock_bh(&hdmi_lock);

        if (hdmi_mmp_level > 0)
        {
            MMProfileLogEx(HDMI_MMP_Events.BufferRemove, MMProfileFlagPulse, pBuffList, pBuffList->buffer_info.src_phy_addr);
        }
        else
        {
            HDMI_LOG("delete list %x-->buffer %x \n", pBuffList, pBuffList->buffer_info.src_phy_addr);
        }

#ifdef MTK_HDMI_ION_SUPPORT

        if (pBuffList->va)
        {
            ion_unmap_kernel(ion_client, pBuffList->hnd);
        }

        hdmi_ion_free_handle(ion_client, pBuffList->hnd);
#endif

        spin_lock_bh(&hdmi_lock);
        list_del(&pBuffList->list);
        kfree(pBuffList);
        pBuffList = NULL;
        spin_unlock_bh(&hdmi_lock);
    }

    hdmi_release_fence();
    HDMI_LOG("fence stop rdma done\n");
}

/*this function insert buffer to buffer list*/
int hdmi_insert_buffer(hdmi_buffer_info *buffer_info)
{
    struct fence_data data;
    data = hdmi_create_fence();

    hdmi_video_buffer_list *pBuffList = NULL;
    struct ion_handle *handle = NULL;
    unsigned int mva = 0x0;
    unsigned int va = 0x0;

    pBuffList = kmalloc(sizeof(hdmi_video_buffer_list), GFP_KERNEL);
    memset(pBuffList, 0 , sizeof(hdmi_video_buffer_list));
#ifdef MTK_HDMI_ION_SUPPORT

    if (!ion_client)
    {
        hdmi_ion_init();

        if (!ion_client)
        {
            HDMI_LOG(": get ion_client fail \n", ion_client);
            return -1;
        }
    }

    handle = hdmi_ion_import_handle(ion_client, buffer_info->ion_fd);
    hdmi_ion_phys_mmu_addr(ion_client, handle, &mva);
    va = ion_map_kernel(ion_client, handle);
#endif
    spin_lock_bh(&hdmi_lock);
    pBuffList->buf_state = create_new;
    pBuffList->fence = data.fence;
    pBuffList->idx = data.value;
    pBuffList->hnd = handle;
    pBuffList->mva = mva;
    pBuffList->va = va;
    buffer_info->fence_fd = data.fence;
    buffer_info->index = data.value;
    INIT_LIST_HEAD(&pBuffList->list);
    list_add_tail(&pBuffList->list, &HDMI_Buffer_List);
    spin_unlock_bh(&hdmi_lock);
    HDMI_LOG(": add list :%x, index %d, fd %d\n", pBuffList, pBuffList->idx, pBuffList->fence);

    if (hdmi_mmp_level > 0)
    {
        MMProfileLogEx(HDMI_MMP_Events.FenceCreate, MMProfileFlagEnd, buffer_info->fence_fd, pBuffList);
    }

    return 0;
}

/*this function notify hdmitx that this buffer can be used, and modify the buffer's state to insert_new*/
int hdmi_post_buffer(hdmi_video_buffer_info *buffer_info, bool clock_on)
{
    hdmi_video_buffer_list *pBuffList = NULL;
    spin_lock_bh(&hdmi_lock);

    if (clock_on)
    {

        //MMP_MetaDataBitmap_t Bitmap;
#ifdef MTK_HDMI_ION_SUPPORT
        pBuffList = hdmi_query_buf_mva(buffer_info->next_buff_idx);

        if (pBuffList)
        {
            memcpy(&(pBuffList->buffer_info), buffer_info, sizeof(hdmi_video_buffer_info));

            if (pBuffList->hnd != 0)
            {
                if (buffer_info->src_phy_addr != NULL)
                {
                    HDMI_LOG("Warning: ion enable, but phy is not null \n");
                    MMProfileLogEx(HDMI_MMP_Events.ErrorInfo, MMProfileFlagPulse, Buff_ION_Err1, 1);
                }
                else
                {
                    HDMI_LOG("ion disable, bufflist %x, vir %x, phy %x \n", pBuffList, buffer_info->src_base_addr, buffer_info->src_phy_addr);
                }

                buffer_info->src_phy_addr = pBuffList->mva;
                buffer_info->src_base_addr = pBuffList->va;
                pBuffList->buffer_info.src_phy_addr = pBuffList->mva;
                pBuffList->buffer_info.src_base_addr = pBuffList->va;
            }
        }
        else
        {
            spin_unlock_bh(&hdmi_lock);
            MMProfileLogEx(HDMI_MMP_Events.ErrorInfo, MMProfileFlagPulse, Buff_ION_Err1, 0);
            HDMI_LOG("Warning: buffer list no buffers! \n");
            return -1;
        }

#endif
        pBuffList->buf_state = insert_new;
        spin_unlock_bh(&hdmi_lock);
    }
    else
    {
        spin_unlock_bh(&hdmi_lock);
    }

    if (hdmi_mmp_level > 0)
    {
        MMProfileLogEx(HDMI_MMP_Events.BufferInsert, MMProfileFlagPulse, pBuffList, buffer_info->next_buff_idx);
    }

    if (hdmi_mmp_level > 0)
    {
        if (pBuffList)
        {
            MMProfileLogEx(HDMI_MMP_Events.BufferPost, MMProfileFlagEnd, pBuffList, pBuffList->buffer_info.src_phy_addr);
        }
        else
        {
            MMProfileLogEx(HDMI_MMP_Events.BufferPost, MMProfileFlagEnd, pBuffList, 0);
        }
    }

    return 0;
}

void hdmi_mmp_debug_enable(int mmp_debug_level)
{
    hdmi_mmp_level = mmp_debug_level;
}
#endif
