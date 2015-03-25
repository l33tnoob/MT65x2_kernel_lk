
#include "mtk_debug.h"

#ifdef MTK_DEBUG

void MTKDebugInit()
{
#ifdef MTK_DEBUG_PROC_PRINT
	MTKPP_Init();
#endif
}

void MTKDebugDeinit()
{
#ifdef MTK_DEBUG_PROC_PRINT
	MTKPP_Deinit();
#endif
}

#endif

