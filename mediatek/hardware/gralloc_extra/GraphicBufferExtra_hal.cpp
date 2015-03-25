
#define LOG_TAG "GraphicBufferExtra_hal"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <stdint.h>
#include <errno.h>

#include <utils/Errors.h>
#include <utils/Log.h>
#include <utils/Trace.h>

#include <hardware/gralloc.h>
#include <hardware/gralloc_extra.h>

#include <ui/GraphicBufferExtra.h>

namespace android {
// ---------------------------------------------------------------------------

/* ----  IMPLEMENTATION macro  ---- */

#define GRALLOC_EXTRA_PROTOTYPE_(...)  ( __VA_ARGS__ )
#define GRALLOC_EXTRA_ARGS_(...)       ( __VA_ARGS__ )
#define GRALLOC_EXTRA_ARGS_2(...)      ( mExtraDev, __VA_ARGS__ )
#define GRALLOC_EXTRA_CHECK_FAIL_RETURN_(api)                           \
do {                                                                    \
if (!mExtraDev)                                                         \
{                                                                       \
	ALOGD("gralloc extra device is not supported");                     \
	return NO_INIT;                                                     \
}                                                                       \
                                                                        \
if (!mExtraDev->api)                                                    \
{                                                                       \
	ALOGW("gralloc extra device " #api "(...) is not supported");       \
	return NO_INIT;                                                     \
}                                                                       \
} while(0)

#define GRALLOC_EXTRA_IMPLEMENTATION_(api, prototype, args)             \
int GraphicBufferExtra:: api prototype                                  \
{                                                                       \
    ATRACE_CALL();                                                      \
    status_t err;                                                       \
                                                                        \
    GRALLOC_EXTRA_CHECK_FAIL_RETURN_(api);                              \
                                                                        \
    err = mExtraDev-> api GRALLOC_EXTRA_ARGS_2 args ;                   \
                                                                        \
    ALOGW_IF(err, #api "(...) failed %d (%s)", err, strerror(-err));    \
    return err;                                                         \
}                                                                       \
extern "C" int gralloc_extra_##api prototype                            \
{                                                                       \
    return GraphicBufferExtra::get(). api args;                         \
}

/* ----  IMPLEMENTATION start  ---- */

GRALLOC_EXTRA_IMPLEMENTATION_(getIonFd, 
    GRALLOC_EXTRA_PROTOTYPE_(buffer_handle_t handle, int *idx, int *num), 
    GRALLOC_EXTRA_ARGS_(handle, idx, num)
)

GRALLOC_EXTRA_IMPLEMENTATION_(getSecureBuffer, 
    GRALLOC_EXTRA_PROTOTYPE_(buffer_handle_t handle, int *type, int *hBuffer), 
    GRALLOC_EXTRA_ARGS_(handle, type, hBuffer)
)

GRALLOC_EXTRA_IMPLEMENTATION_(getBufInfo, 
    GRALLOC_EXTRA_PROTOTYPE_(buffer_handle_t handle, gralloc_buffer_info_t* bufInfo), 
    GRALLOC_EXTRA_ARGS_(handle, bufInfo)
)

GRALLOC_EXTRA_IMPLEMENTATION_(setBufParameter, 
    GRALLOC_EXTRA_PROTOTYPE_(buffer_handle_t handle, int mask, int value), 
    GRALLOC_EXTRA_ARGS_(handle, mask, value)
)
	
GRALLOC_EXTRA_IMPLEMENTATION_(getMVA, 
    GRALLOC_EXTRA_PROTOTYPE_(buffer_handle_t handle, void** mvaddr), 
    GRALLOC_EXTRA_ARGS_(handle, mvaddr)
)

GRALLOC_EXTRA_IMPLEMENTATION_(setBufInfo, 
    GRALLOC_EXTRA_PROTOTYPE_(buffer_handle_t handle, const char * str), 
    GRALLOC_EXTRA_ARGS_(handle, str)
)

/* ----  IMPLEMENTATION start end  ---- */

#undef GRALLOC_EXTRA_CHECK_FAIL_RETURN_
#undef GRALLOC_EXTRA_IMPLEMENTATION_
#undef GRALLOC_EXTRA_PROTOTYPE_
#undef GRALLOC_EXTRA_ARGS_
#undef GRALLOC_EXTRA_ARGS_2

// ---------------------------------------------------------------------------
}; // namespace android
