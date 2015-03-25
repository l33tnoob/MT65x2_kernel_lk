#define LOG_TAG "GraphicBufferExtra"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <stdint.h>
#include <errno.h>

#include <utils/Errors.h>
#include <utils/Log.h>
#include <utils/Trace.h>
#include <utils/String8.h>

#include <cutils/xlog.h>

#include <hardware/gralloc.h>
#include <hardware/gralloc_extra.h>

#include <ui/GraphicBufferExtra.h>
#include <ui/PixelFormat.h>
#include <ui/GraphicBuffer.h>

#include <png.h>
#define ALIGN_CEIL(x,a) (((x) + (a) - 1L) & ~((a) - 1L))

namespace android {
// ---------------------------------------------------------------------------

ANDROID_SINGLETON_STATIC_INSTANCE( GraphicBufferExtra )

GraphicBufferExtra::GraphicBufferExtra()
    : mExtraDev(0)
{
    hw_module_t const* module;
    int err = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &module);

    ALOGE_IF(err, "FATAL: can't find the %s module", GRALLOC_HARDWARE_MODULE_ID);
    if (err == 0) 
    {
        gralloc_extra_open(module, &mExtraDev);
    }
}

GraphicBufferExtra::~GraphicBufferExtra()
{
    if (mExtraDev)
    {
        gralloc_extra_close(mExtraDev);
    }
}

void GraphicBufferExtra::dump(const sp<GraphicBuffer> &gb,
                              const char *prefix,
                              const char *dir)
{
    if (gb == NULL)
    {
        XLOGE("[%s] gb is NULL", __func__);
        return;
    }

    void *id = gb.get();
    uint32_t width = gb->getWidth();
    uint32_t height = gb->getHeight();
    uint32_t stride = gb->getStride();
    PixelFormat format = gb->getPixelFormat();

    XLOGD("[%s] gb:%p +", __func__, id);
    XLOGD("    prefix:%s dir:%s", prefix, dir);

    // make file name, default path to /data/[ADDR]_[W]_[H]_[S]
    String8 path;
    if ((NULL == dir) || (0 == strlen(dir)))
    {
        path.setPathName("/data/");
    }
    else
    {
        path.setPathName(dir);
    }

    if ((NULL == prefix) || (0 == strlen(prefix)))
    {
        path.append(String8::format("/%p_w%d_h%d_s%d",
            id, width, height, stride));
    }
    else
    {
        path.append(String8::format("/%s_%p_w%d_h%d_s%d",
            prefix, id, width, height, stride));
    }
    XLOGD("    path:%s", path.string());

    int inputFormat = format;
    int dumpHeight = height;

#ifndef EMULATOR_SUPPORT
    if (inputFormat == HAL_PIXEL_FORMAT_YUV_PRIVATE)
    {
        gralloc_buffer_info_t buffInfo;
        GraphicBufferExtra::get().getBufInfo(gb->handle, &buffInfo);

        // check real format within private format
        int fillFormat = (buffInfo.status & GRALLOC_EXTRA_MASK_CM);
        switch (fillFormat)
        {
            case GRALLOC_EXTRA_BIT_CM_YV12:
                inputFormat = HAL_PIXEL_FORMAT_YV12;
                break;
            case GRALLOC_EXTRA_BIT_CM_NV12_BLK:
                inputFormat = HAL_PIXEL_FORMAT_NV12_BLK;
                dumpHeight = ALIGN_CEIL(height, 32);
                break;
            case GRALLOC_EXTRA_BIT_CM_NV12_BLK_FCM:
                inputFormat = HAL_PIXEL_FORMAT_NV12_BLK_FCM;
                dumpHeight = ALIGN_CEIL(height, 32);
                break;
            default:
                XLOGE("    CANNOT get real format: (inputFormat=0x%x, fillFormat=0x%x)", inputFormat, fillFormat);
                return;
        }
    }
#endif

    float bpp = 4.0;
    bool  isRaw = false;
    bool  is565 = false;

    switch (inputFormat)
    {
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_RGBX_8888:
            path += ".png";
            break;
        case HAL_PIXEL_FORMAT_BGRA_8888:
        case 0x1ff:                     // tricky format for SGX_COLOR_FORMAT_BGRX_8888 in fact
            path += "(RBswapped).png";
            break;
        case HAL_PIXEL_FORMAT_RGB_565:
            bpp = 2.0;
            is565 = true;
            path += ".png";
            break;
        case HAL_PIXEL_FORMAT_I420:
            bpp = 1.5;
            isRaw = true;
            path += ".i420";
            break;
        case HAL_PIXEL_FORMAT_NV12_BLK:
            bpp = 1.5;
            isRaw = true;
            path += ".nv12_blk";
            break;
        case HAL_PIXEL_FORMAT_NV12_BLK_FCM:
            bpp = 1.5;
            isRaw = true;
            path += ".nv12_blk_fcm";
            break;
        case HAL_PIXEL_FORMAT_YV12:
            bpp = 1.5;
            isRaw = true;
            path += "%s.yv12";
            break;
        default:
            XLOGE("    CANNOT dump (format:0x%x)", inputFormat);
            return;
    }
    XLOGD("    path:%s", path.string());

    void        *ptr     = NULL;
    FILE        *f       = NULL;
    png_structp png_ptr  = NULL;
    png_infop   info_ptr = NULL;

    // dump to file with final path and file type
    gb->lock(GraphicBuffer::USAGE_SW_READ_OFTEN, &ptr);
    {
        if (NULL == ptr)
        {
            XLOGE("    lock() FAILED");
            goto finalize;
        }

        f = fopen(path.string(), "wb");
        if (NULL == f)
        {
            XLOGE("    fopen() FAILED");
            goto finalize;
        }

        if (true == isRaw)
        {
            // simplely write binary data to file
            fwrite(ptr, stride * dumpHeight * bpp, 1, f);
        }
        else
        {
            // init write struct
            png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
            if (NULL == png_ptr)
            {
                XLOGE("    init png FAILED (1)");
                goto finalize;
            }

            // init info struct
            info_ptr = png_create_info_struct(png_ptr);
            if (NULL == info_ptr)
            {
                XLOGE("    init png FAILED (2)");
                goto finalize;
            }

            // set file to write
            png_init_io(png_ptr, f);

            // setup color format info
            png_set_IHDR(png_ptr, info_ptr,
                stride, dumpHeight, 8, PNG_COLOR_TYPE_RGB_ALPHA,
                PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

            png_color_8 sigBit;
            if (true == is565)
            {
                sigBit.red   = 5;
                sigBit.green = 6;
                sigBit.blue  = 5;
                sigBit.alpha = 0;
            }
            else
            {
                sigBit.red   = 8;
                sigBit.green = 8;
                sigBit.blue  = 8;
                sigBit.alpha = 8;
            }
            png_set_sBIT(png_ptr, info_ptr, &sigBit);

            // start
            png_write_info(png_ptr, info_ptr);

            // dump data row by row
            uint32_t strideBytes = stride * bpp;
            int32_t y;
            for (y = 0; y < dumpHeight; y++)
            {
                png_write_rows(png_ptr, (png_bytepp)&ptr, 1);
                ptr = (uint8_t *)ptr + strideBytes;
            }

            // end
            png_write_end(png_ptr, info_ptr);
        }

finalize:
        // clean up
        if (NULL != f)        fclose(f);
        if (NULL != info_ptr) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
        if (NULL != png_ptr)  png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    }
    gb->unlock();

    XLOGD("[%s] -", __func__);
}


// ---------------------------------------------------------------------------
}; // namespace android
