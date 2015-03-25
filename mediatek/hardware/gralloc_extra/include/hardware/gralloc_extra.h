#ifndef MTK_GRALLOC_EXTRA_DEVICE_H
#define MTK_GRALLOC_EXTRA_DEVICE_H

#include <system/window.h>
#include <hardware/hardware.h>
#include <hardware/gralloc.h>

#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include <ui/gralloc_extra.h>

__BEGIN_DECLS

#define GRALLOC_HARDWARE_EXTRA "extra"

typedef struct extra_device_t {
    struct hw_device_t common;

    /*
     * (*getIonFd)() is called for getting ion share fd from buffer handle
     * It should return the beginning index of native_handle.data[]
     * for ion shard fds and number of ion share fds
     */
    int (*getIonFd)(struct extra_device_t* dev,
            buffer_handle_t handle, int *idx, int *num);

    /*
     * (*getBufInfo)() is called for getting the buffer information 
     * which includes width, height, format, stride by casting
     * native_handle_t as GPU private handle.  
     */
    int (*getBufInfo)(struct extra_device_t *dev,
            buffer_handle_t handle, gralloc_buffer_info_t* bufInfo);

    /*
     * (*getSecureBuffer)()
     */
    int (*getSecureBuffer)(struct extra_device_t* dev,
            buffer_handle_t handle, int *type, int *hBuffer);

    /*
     * (*setBufParameter)()
     */
    int (*setBufParameter)(struct extra_device_t* dev,
            buffer_handle_t handle, int mask, int value);

    /*
     * (*getMVA)()
     */
    int (*getMVA)(struct extra_device_t* dev,
            buffer_handle_t handle, void** mvaddr);

    /*
     * (*setBufParameter2)()
     * Note: 
     *   values must be "int[2]" 
     *   str must be "const char *"
     */
    int (*setBufInfo)(struct extra_device_t* dev,
            buffer_handle_t handle, const char* str);

    void* reserved_proc[4];
} extra_device_t;


/** convenience API for opening and closing a supported device */

static inline int gralloc_extra_open(const struct hw_module_t* module,
        struct extra_device_t** device) {
    return module->methods->open(module,
            GRALLOC_HARDWARE_EXTRA, (struct hw_device_t**)device);
}

static inline int gralloc_extra_close(struct extra_device_t* device) {
    return device->common.close(&device->common);
}

__END_DECLS

#endif // MTK_GRALLOC_EXTRA_DEVICE_H
