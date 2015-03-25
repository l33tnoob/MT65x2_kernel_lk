#ifndef __MTK_GPU_UTILITY_H__
#define __MTK_GPU_UTILITY_H__

#ifdef __cplusplus
extern "C"
{
#endif

// returning false indicated no implement 
bool mtk_get_gpu_memory_usage(unsigned int* pMemUsage);

// returning false indicated no implement 
bool mtk_get_gpu_loading(unsigned int* pLoading);

#ifdef __cplusplus
}
#endif

#endif
