#ifndef __MTK_MALI_TRACE_H__
#define __MTK_MALI_TRACE_H__

#include <linux/sched.h>

#define MTK_MALI_MAX_QUEUE_SIZE     32
#define MTK_MALI_MAX_CORE_COUNT     3  // For 6582 MALI: 1 GP core + 2 PP cores
#define MTK_MALI_MAX_EVENT_COUNT    6  // MALI_CORE_MAX_NUM * 2 (i.e. begin and end)
#define MTK_MALI_MAX_NAME_SIZE      32

typedef enum MALI_CORE_ENUM
{
    MALI_CORE_TYPE_GP,
    MALI_CORE_TYPE_PP,
} MALI_CORE_ENUM;


typedef struct mtk_mali_trace_event
{
    int                  theID;
    char                 name[MTK_MALI_MAX_NAME_SIZE];
} mtk_mali_trace_event;

typedef struct mtk_mali_trace_work
{
    mtk_mali_trace_event event[MTK_MALI_MAX_QUEUE_SIZE];
    int                  read;
    int                  write;
    int                  total; 
    int                  stop;   
    int                  coreID;
} mtk_mali_trace_work;


#define MTK_TRACE_CONTAINER_OF(ptr, type, member)       \
    ((type *)( ((char *)ptr) - offsetof(type, member)))

#ifdef __cplusplus
extern "C" {
#endif

int mtk_mali_kernel_trace_init();

void mtk_mali_kernel_trace_begin(MALI_CORE_ENUM     coreType,
                                 int                index,
                                 struct task_struct *pTask);

void mtk_mali_kernel_trace_end(MALI_CORE_ENUM coreType,
                               int            index);

int mtk_mali_kernel_trace_exit();

#ifdef __cplusplus
}
#endif

#endif  // __MTK_MALI_TRACE_H__
