#include <linux/kallsyms.h>
#include <linux/cache.h>
#include <linux/ftrace_event.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include "mtk_mali_trace.h"

static struct task_struct           *gTraceThread[MTK_MALI_MAX_CORE_COUNT];  // Trace task
static wait_queue_head_t            gTraceQueue[MTK_MALI_MAX_CORE_COUNT];    // Trace queue
static mtk_mali_trace_work          gTraceEvent[MTK_MALI_MAX_CORE_COUNT];    // Trace event
static spinlock_t                   gThreadLock[MTK_MALI_MAX_CORE_COUNT];    // Trace lock

// Mark address
static unsigned long __read_mostly  gMarkWriteAddr = 0;


static void inline mali_update_write_addr(void)
{
#if 0
    if(unlikely(0 == gMarkWriteAddr))
    {
        gMarkWriteAddr = kallsyms_lookup_name("tracing_mark_write");
    }
#endif // 0
}


static void mali_kernel_trace_begin(char *pName)
{
#if 0
    mali_update_write_addr();

    event_trace_printk(gMarkWriteAddr,
                       "B|%d|%s\n",
                       current->tgid, pName);
#endif // 0
}


static void mali_kernel_trace_counter(char *pName, int count)
{
#if 0
    mali_update_write_addr();

    event_trace_printk(gMarkWriteAddr,
                       "C|%d|%s|%d\n",
                       (in_interrupt()? -1: current->tgid),
                       pName,
                       count);
#endif // 0
}


static void mali_kernel_trace_end(void)
{
#if 0
    mali_update_write_addr();

    event_trace_printk(gMarkWriteAddr,
                       "E\n"); 
#endif // 0
}


static inline int mali_core_index_to_event_id(MALI_CORE_ENUM coreType,
                                              int            index,
                                              int            traceType)
{
    int eventID;

    eventID = 0;

    switch(coreType)
    {
        case MALI_CORE_TYPE_GP:
            eventID = ((1 == traceType)? 0: 1);
            break;
        case MALI_CORE_TYPE_PP:
            if (0 == index)
            {
                eventID = ((1 == traceType)? 2: 3);
            }
            else
            {
                eventID = ((1 == traceType)? 4: 5);
            }
            break;
        default:
            // assert(0);
            break;
    }

    return eventID;
}


static inline int mali_core_index_to_core_id(MALI_CORE_ENUM coreType,
                                             int            index)
{
    int coreID;

    coreID = 0;

    switch(coreType)
    {
        case MALI_CORE_TYPE_GP:
            coreID = 0;
            break;
        case MALI_CORE_TYPE_PP:
            coreID = ((0 == index)? 1: 2);
            break;
        default:
            // assert(0);
            break;
    }

    return coreID;
}


static inline int mali_core_id_to_core_name(int coreID)
{
    static const char *pName[] =
    {
        "MALI_GP0",
        "MALI_PP0",
        "MALI_PP1"
    };
    
    switch(coreID)
    {
        case 0:
        case 1:
        case 2:
            return pName[coreID];
        default:
            // assert(0);
            break;
    }

    return "MALI_ERR";
}


static int mtk_mali_trace_handler(struct mtk_mali_trace_work *pWork)
{
    struct task_struct  *pTask; 
    struct sched_param  param;;
    long                flags;
    int                 coreID;
    int                 eventID;
    char                *pName;

    pTask = current;

    //sched_getparam(0, &param);
    memset(&param, 0, sizeof(param));
    param.sched_priority = 90; 
    sched_setscheduler(pTask, SCHED_RR, &param);

    coreID = pWork->coreID;

    do
    {
        wait_event_interruptible(gTraceQueue[coreID], ((pWork->total > 0) || (1 == pWork->stop)));

        if(kthread_should_stop() || (1 == pWork->stop))
        {
            break;
        }

        spin_lock_irqsave(&gThreadLock[coreID], flags);

        smp_mb();

        if (pWork->total <= 0)
        {
            spin_unlock_irqrestore(&gThreadLock[coreID], flags); 
            continue;
        }
        
        eventID = pWork->event[pWork->read].theID;
        pName   = pWork->event[pWork->read].name;
        pWork->read++;
        if (pWork->read >= MTK_MALI_MAX_QUEUE_SIZE)
        {
            pWork->read = 0;
        }

        pWork->total--;
        spin_unlock_irqrestore(&gThreadLock[coreID], flags); 

        switch(eventID)
        {
            case 0:  // GP0 begin
                mali_kernel_trace_begin(pName);
                break;
            case 1:  // GP0 end
                mali_kernel_trace_end();
                break;
            case 2:  // PP0 begin
                mali_kernel_trace_begin(pName);
                break;
            case 3:  // PP0 end
                mali_kernel_trace_end();
                break;
            case 4:  // PP1 begin
                mali_kernel_trace_begin(pName);
                break;
            case 5:  // PP1 end
                mali_kernel_trace_end();
                break;
            break;
                //assert(0);
                break;
        }
    } while(1);

    return 0;
}


int mtk_mali_kernel_trace_init(void)
{
    int  index;
    char *pName;

    for (index = 0; index < MTK_MALI_MAX_CORE_COUNT; index++)
    {
        init_waitqueue_head(&gTraceQueue[index]);

        memset(&gTraceEvent[index], 0x0, sizeof(mtk_mali_trace_work));

        // Record the core ID
        gTraceEvent[index].coreID = index;

        spin_lock_init(&gThreadLock[index]);

        gTraceThread[index] = kthread_run(mtk_mali_trace_handler,
                                          &gTraceEvent[index],
                                          "%s",
                                          mali_core_id_to_core_name(index));      
        if(IS_ERR(gTraceThread[index]))
        {
            printk("Unable to start kernel thread for core%d\n", index);  
        }
    }

    return 0;
}


void mtk_mali_kernel_trace_begin(MALI_CORE_ENUM     coreType,
                                 int                index,
                                 struct task_struct *pTask)
{
    long flags;
    int  coreID;
    int  eventID;
    int  slotID;

    coreID  = mali_core_index_to_core_id(coreType, index);
    eventID = mali_core_index_to_event_id(coreType, index, 1);

    if (eventID < MTK_MALI_MAX_EVENT_COUNT)
    {
        spin_lock_irqsave(&gThreadLock[coreID], flags);

        smp_mb();

        slotID = gTraceEvent[coreID].write;
        gTraceEvent[coreID].write++;
        if (gTraceEvent[coreID].write >= MTK_MALI_MAX_QUEUE_SIZE)
        {
            gTraceEvent[coreID].write = 0;
        }
       
        gTraceEvent[coreID].total++;
        spin_unlock_irqrestore(&gThreadLock[coreID], flags); 

        gTraceEvent[coreID].event[slotID].theID = eventID;
        memcpy(gTraceEvent[coreID].event[slotID].name, pTask->comm, sizeof(current->comm));

        wake_up_interruptible(&gTraceQueue[coreID]);
    }
}


void mtk_mali_kernel_trace_end(MALI_CORE_ENUM coreType,
                               int            index)
{
    long flags;
    int  coreID;
    int  eventID;
    int  slotID;

    coreID  = mali_core_index_to_core_id(coreType, index);
    eventID = mali_core_index_to_event_id(coreType, index, 0);

    if (eventID < MTK_MALI_MAX_EVENT_COUNT)
    {
        spin_lock_irqsave(&gThreadLock[coreID], flags);

        smp_mb();

        slotID = gTraceEvent[coreID].write;
        gTraceEvent[coreID].write++;
        if (gTraceEvent[coreID].write >= MTK_MALI_MAX_QUEUE_SIZE)
        {
            gTraceEvent[coreID].write = 0;
        }

        gTraceEvent[coreID].total++;
        spin_unlock_irqrestore(&gThreadLock[coreID], flags);

        gTraceEvent[coreID].event[slotID].theID = eventID;

        wake_up_interruptible(&gTraceQueue[coreID]);
    }
}


int mtk_mali_kernel_trace_exit()
{
    int index;

    for (index = 0; index < MTK_MALI_MAX_CORE_COUNT; index++)
    {
        gTraceEvent[index].stop = 1;
        kthread_stop(gTraceThread[index]);
    }
    
    return 0;
}
