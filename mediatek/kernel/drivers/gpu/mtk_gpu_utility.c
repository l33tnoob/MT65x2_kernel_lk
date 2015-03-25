#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mtk_gpu_utility.h>

unsigned int (*mtk_get_gpu_memory_usage_fp)(void) = NULL;
EXPORT_SYMBOL(mtk_get_gpu_memory_usage_fp);

bool mtk_get_gpu_memory_usage(unsigned int* pMemUsage)
{
    if (NULL != mtk_get_gpu_memory_usage_fp)
    {
        if (pMemUsage)
        {
            *pMemUsage = mtk_get_gpu_memory_usage_fp();
            return true;
        }
    }
    return false;
}

unsigned int (*mtk_get_gpu_loading_fp)(void) = NULL;
EXPORT_SYMBOL(mtk_get_gpu_loading_fp);

bool mtk_get_gpu_loading(unsigned int* pLoading)
{
    if (NULL != mtk_get_gpu_loading_fp)
    {
        if (pLoading)
        {
            *pLoading = mtk_get_gpu_loading_fp();
            return true;
        }
    }
    return false;
}