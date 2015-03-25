#ifndef MTK_GRALLOC_EXTRA_H
#define MTK_GRALLOC_EXTRA_H

#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include <system/window.h>

#include <hardware/gralloc.h>

__BEGIN_DECLS

int gralloc_extra_getIonFd(buffer_handle_t handle, int *idx, int *num);

typedef struct gralloc_buffer_info_t {
    /* static number, never change */
    int width;
    int height;
    int stride;
    int format;
    int vertical_stride;
    int usage;

    /* change by setBufParameter() */
    int status;
} gralloc_buffer_info_t;

int gralloc_extra_getBufInfo(buffer_handle_t handle, gralloc_buffer_info_t* bufInfo);

int gralloc_extra_getSecureBuffer(buffer_handle_t handle, int *type, int *hBuffer);

/* enum gralloc_extra_setBufParameter for */
#define GRALLOC_EXTRA_MAKE_BIT(start_bit, index)        ( (index) << (start_bit) )
#define GRALLOC_EXTRA_MAKE_MASK(start_bit, end_bit)     ( ( ((unsigned int)-1) >> (sizeof(int) * __CHAR_BIT__ - 1 - (end_bit) + (start_bit) ) ) << (start_bit) )
enum {
	/* TYPE: bit 0,1 */
    GRALLOC_EXTRA_BIT_TYPE_CPU          = GRALLOC_EXTRA_MAKE_BIT(0,0),
    GRALLOC_EXTRA_BIT_TYPE_GPU          = GRALLOC_EXTRA_MAKE_BIT(0,1),
    GRALLOC_EXTRA_BIT_TYPE_VIDEO        = GRALLOC_EXTRA_MAKE_BIT(0,2),
    GRALLOC_EXTRA_BIT_TYPE_CAMERA     	= GRALLOC_EXTRA_MAKE_BIT(0,3),
    GRALLOC_EXTRA_MASK_TYPE             = GRALLOC_EXTRA_MAKE_MASK(0,1),
        
    /* ClearMotion: bit 4,5 */
    GRALLOC_EXTRA_BIT_CM_YV12           = GRALLOC_EXTRA_MAKE_BIT(4,0),
    GRALLOC_EXTRA_BIT_CM_NV12_BLK       = GRALLOC_EXTRA_MAKE_BIT(4,1),
    GRALLOC_EXTRA_BIT_CM_NV12_BLK_FCM   = GRALLOC_EXTRA_MAKE_BIT(4,2),
    GRALLOC_EXTRA_BIT_CM_YUYV           = GRALLOC_EXTRA_MAKE_BIT(4,3),
    GRALLOC_EXTRA_MASK_CM               = GRALLOC_EXTRA_MAKE_MASK(4,5),

    /* DISPLAY: bit 8 */
    GRALLOC_EXTRA_BIT_DISPLAY_NORMAL    = GRALLOC_EXTRA_MAKE_BIT(8,0),
    GRALLOC_EXTRA_BIT_DISPLAY_DROP      = GRALLOC_EXTRA_MAKE_BIT(8,1),
    GRALLOC_EXTRA_MASK_DISPLAY          = GRALLOC_EXTRA_MAKE_MASK(8,8),

    /* DIRTY: bit 28 */
    GRALLOC_EXTRA_BIT_UNDIRTY           = GRALLOC_EXTRA_MAKE_BIT(28,0),
    GRALLOC_EXTRA_BIT_DIRTY             = GRALLOC_EXTRA_MAKE_BIT(28,1),
    GRALLOC_EXTRA_MASK_DIRTY            = GRALLOC_EXTRA_MAKE_MASK(28,28),
};

int gralloc_extra_setBufParameter(buffer_handle_t handle, int mask, int value);


///
enum {
    GRALLOC_EXTRA_BUFFER_TYPE               = GRALLOC_EXTRA_MASK_TYPE,
    GRALLOC_EXTRA_BUFFER_TYPE_VIDEO         = GRALLOC_EXTRA_BIT_TYPE_VIDEO,
    GRALLOC_EXTRA_STATUS_BIT                = GRALLOC_EXTRA_MASK_DIRTY,
    GRALLOC_EXTRA_STATUS_BUFFER_DIRTY_BIT   = GRALLOC_EXTRA_BIT_DIRTY,
};
///

int gralloc_extra_getMVA(buffer_handle_t handle, void ** mvaddr);

int gralloc_extra_setBufInfo(buffer_handle_t handle, const char * str);

__END_DECLS

#endif /* MTK_GRALLOC_EXTRA_H */
