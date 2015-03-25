#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#ifndef max
    #define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif

#ifndef min
    #define min( a, b ) ( ((a) < (b)) ? (a) : (b) )
#endif

#define MAX_LINE_LEN 255

/*
 * MTB Trace Tags
 */
#define MTB_TRACE_SYSTRACE        (1<<0)
#define MTB_TRACE_MET             (1<<1)
#define MTB_TRACE_MMP             (1<<2)
#define MTB_TRACE_LAST            (MTB_TRACE_MMP)
#define MTB_TRACE_TYPE_VALID_MASK ((MTB_TRACE_LAST -1) | MTB_TRACE_LAST)


#define MTB_TAG_NEVER            0       // The "never" tag is never enabled.
#define MTB_TAG_GRAPHICS         (1<<1)
#define MTB_TAG_INPUT            (1<<2)
#define MTB_TAG_VIEW             (1<<3)
#define MTB_TAG_WEBVIEW          (1<<4)
#define MTB_TAG_WINDOW_MANAGER   (1<<5)
#define MTB_TAG_ACTIVITY_MANAGER (1<<6)
#define MTB_TAG_SYNC_MANAGER     (1<<7)
#define MTB_TAG_AUDIO            (1<<8)
#define MTB_TAG_VIDEO            (1<<9)
#define MTB_TAG_CAMERA           (1<<10)
#define MTB_TAG_WFD              (1LL<<32)
#define MTB_TAG_LAST             MTB_TAG_WFD

enum {
    ATRACE_ONESHOT_EVENT             = 0x00000001,
    ATRACE_ONESHOT_MESSAGE           = 0x00000002,
    ATRACE_ONESHOT_ADATA             = 0x00000004,
    ATRACE_ONESHOT_VDATA             = 0x00000008,
    ATRACE_ONESHOT_SPECIAL           = 0x00000010,
};

#endif
