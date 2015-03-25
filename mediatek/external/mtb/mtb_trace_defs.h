#ifndef __MTB_TRACE_DEFS_H__
#define __MTB_TRACE_DEFS_H__
#include "constants.h"
#include "mtb.h"

#define ATRACE_TAG_WFD              (1LL<<32)
#define ATRACE_TAG_MTK_LAST         ATRACE_TAG_WFD
#ifdef ATRACE_TAG_VALID_MASK
#undef ATRACE_TAG_VALID_MASK
#endif
#define ATRACE_TAG_VALID_MASK ((ATRACE_TAG_MTK_LAST - 1) | ATRACE_TAG_MTK_LAST)


#define ATRACE_BEGIN(name) atrace_begin(ATRACE_TAG, name)
static inline void atrace_begin(uint64_t tag, const char* name)
{
    if (CC_UNLIKELY(atrace_is_tag_enabled(tag))) {
        uint32_t types = mtb_trace_get_types();
        if (types & MTB_TRACE_SYSTRACE) {
            char buf[ATRACE_MESSAGE_LENGTH];
            size_t len;

            len = snprintf(buf, ATRACE_MESSAGE_LENGTH, "B|%d|%s", getpid(), name);
            write(atrace_marker_fd, buf, len);
        }
        if (types & (MTB_TRACE_MET | MTB_TRACE_MMP)) {
            mtb_trace_begin(tag, name, gettid());
        }
    }
}

#define ATRACE_END() atrace_end(ATRACE_TAG)
static inline void atrace_end(uint64_t tag)
{
    if (CC_UNLIKELY(atrace_is_tag_enabled(tag))) {
        uint32_t types = mtb_trace_get_types();
        if (types & MTB_TRACE_SYSTRACE) {
            char c = 'E';
            write(atrace_marker_fd, &c, 1);
        }
        
        if (types & (MTB_TRACE_MET | MTB_TRACE_MMP)) {
            mtb_trace_end(tag, NULL, gettid());
        }
    }
}

#define ATRACE_ASYNC_BEGIN(name, cookie) \
    atrace_async_begin(ATRACE_TAG, name, cookie)
static inline void atrace_async_begin(uint64_t tag, const char* name,
        int32_t cookie)
{
    if (CC_UNLIKELY(atrace_is_tag_enabled(tag))) {
        uint32_t types = mtb_trace_get_types();
        if (types & MTB_TRACE_SYSTRACE) {
            char buf[ATRACE_MESSAGE_LENGTH];
            size_t len;

            len = snprintf(buf, ATRACE_MESSAGE_LENGTH, "S|%d|%s|%d", getpid(),
                    name, cookie);
            write(atrace_marker_fd, buf, len);
        }

        if (types & (MTB_TRACE_MET | MTB_TRACE_MMP)) {
            mtb_trace_begin(tag, name, gettid());
        }
    }
}

#define ATRACE_ASYNC_END(name, cookie) atrace_async_end(ATRACE_TAG, name, cookie)
static inline void atrace_async_end(uint64_t tag, const char* name,
        int32_t cookie)
{
    if (CC_UNLIKELY(atrace_is_tag_enabled(tag))) {
        uint32_t types = mtb_trace_get_types();
        if (types & MTB_TRACE_SYSTRACE) {
            char buf[ATRACE_MESSAGE_LENGTH];
            size_t len;

            len = snprintf(buf, ATRACE_MESSAGE_LENGTH, "F|%d|%s|%d", getpid(),
                    name, cookie);
            write(atrace_marker_fd, buf, len);
        }
        
        if (types & (MTB_TRACE_MET | MTB_TRACE_MMP)) {
            mtb_trace_end(tag, name, gettid());
        }
    }
}

/** 
 * Traces an oneshot event.  event type can be control, event, memssage, adata, vdata, special, etc.
 * 
 */
#define ATRACE_ONESHOT(type, ...)  atrace_oneshot(ATRACE_TAG, type, __FUNCTION__, __LINE__ , __VA_ARGS__)
static inline void atrace_oneshot(uint64_t tag, uint32_t type, 
                                const char* fun, 
                                uint32_t line, 
                                const char *fmt, ...) {
    
    if (CC_UNLIKELY(atrace_is_tag_enabled(tag)))  {
        va_list ap;
        char buf[256];
        va_start(ap, fmt);
        vsnprintf(buf, 256, fmt, ap);
        va_end(ap);
        
        uint32_t types = mtb_trace_get_types();
        if (types & (MTB_TRACE_MET | MTB_TRACE_MMP)) {
            mtb_trace_oneshot(tag, type, buf, gettid());
        }
    }
}

#endif // __MTB_TRACE_DEFS_H__
