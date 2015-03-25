#include <gui/BufferQueue.h>
#include <cutils/xlog.h>
#include <cutils/properties.h>

#include <ui/GraphicBufferExtra.h>

#include <gui/mediatek/BufferQueueDump.h>


#define PROP_DUMP_NAME      "debug.bq.dump"
#define PROP_DUMP_BUFCNT    "debug.bq.bufscnt"
#define DEFAULT_DUMP_NAME   "GOD'S IN HIS HEAVEN, ALL'S RIGHT WITH THE WORLD."
#define DEFAULT_BUFCNT      "0"
#define STR_DUMPALL         "dump_all"
#define PREFIX_NODUMP       "[:::nodump:::]"
#define DUMP_FILE_PATH      "/data/SF_dump/"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define ST_LOGV(x, ...) XLOGV("[%s](this:%p) "x, mName.string(), this, ##__VA_ARGS__)
#define ST_LOGD(x, ...) XLOGD("[%s](this:%p) "x, mName.string(), this, ##__VA_ARGS__)
#define ST_LOGI(x, ...) XLOGI("[%s](this:%p) "x, mName.string(), this, ##__VA_ARGS__)
#define ST_LOGW(x, ...) XLOGW("[%s](this:%p) "x, mName.string(), this, ##__VA_ARGS__)
#define ST_LOGE(x, ...) XLOGE("[%s](this:%p) "x, mName.string(), this, ##__VA_ARGS__)


namespace android {

BufferQueueDump::BufferQueueDump() :
        mBackupBufInited(false),
        mName("") {
    checkBackupCount();
}

void BufferQueueDump::setName(String8& name) {
    mName = name;

    // update dumper's name
    if (mBackupBufDumper != NULL) {
        mBackupBufDumper->setName(name);
    }
}

void BufferQueueDump::dumpBuffer() const {
    char value[PROPERTY_VALUE_MAX];

    property_get(PROP_DUMP_NAME, value, DEFAULT_DUMP_NAME);

    if (strstr(value, PREFIX_NODUMP) == value) {
        // find prefix for no dump
        return;
    }

    if (!((!strcmp(value, STR_DUMPALL)) || (-1 != mName.find(value)))) {
        // no dump for me
        return;
    }

    // at first, dump backup buffer if needed
    if (mBackupBuf.getSize() > 0) {
        // dump all backup buffer
        mBackupBuf.dump();
    }

    String8 name;
    String8 prefix;

    getDumpFileName(name, mName);

    uint32_t offset = mBackupBuf.getValidSize();

    // dump acquired buffers
    for (uint32_t i = 0; i < mAcquiredBufs.size(); i++) {
        if (mAcquiredBufs[i]->mGraphicBuffer != NULL) {
            prefix = String8::format("%s_%u_ts%lldms", name.string(), offset + i,
                                        ns2ms(mAcquiredBufs[i]->mTimeStamp));
            if (mAcquiredBufs[i]->mFence != NULL)
                mAcquiredBufs[i]->mFence->waitForever("BufferQueue::Dump::dumpBuffer");
            GraphicBufferExtra::dump(mAcquiredBufs[i]->mGraphicBuffer,
                    prefix.string(), DUMP_FILE_PATH);
            ST_LOGD("dumpBuffer: dump acquired buffer %u", i);
        }
    }
}

void BufferQueueDump::getDumpFileName(String8& fileName, const String8& name) {
    fileName = name;

    // check file name, filter out invalid chars
    const char invalidChar[] = {'\\', '/', ':', '*', '?', '"', '<', '>', '|'};
    size_t size = fileName.size();
    char *buf = fileName.lockBuffer(size);
    for (unsigned int i = 0; i < ARRAY_SIZE(invalidChar); i++) {
        for (size_t c = 0; c < size; c++) {
            if (buf[c] == invalidChar[i]) {
                // find invalid char, replace it with '_'
                buf[c] = '_';
            }
        }
    }
    fileName.unlockBuffer(size);
}

int BufferQueueDump::checkBackupCount() const {
    char value[PROPERTY_VALUE_MAX];
    char *name = value;
    int count;

    property_get(PROP_DUMP_NAME, value, DEFAULT_DUMP_NAME);

    if (strstr(value, PREFIX_NODUMP) == value) {
        // find prefix for no dump, skip it
        name = &value[strlen(PREFIX_NODUMP)];
    }

    if ((!strcmp(name, STR_DUMPALL)) || (-1 != mName.find(name))) {
        property_get(PROP_DUMP_BUFCNT, value, DEFAULT_BUFCNT);
        count = atoi(value);
    } else {
        count = 0;
    }

    if (count > 0) {
        // create backup buffer if needed
        if (!mBackupBufInited) {
            mBackupBufPusher = new BackupBufPusher(mBackupBuf);
            mBackupBufDumper = new BackupBufDumper(mBackupBuf, mName);
            if ((mBackupBufPusher != NULL) && (mBackupBufDumper != NULL)) {
                sp< RingBuffer< sp<BackupBuffer> >::Pusher > proxyPusher = mBackupBufPusher;
                sp< RingBuffer< sp<BackupBuffer> >::Dumper > proxyDumper = mBackupBufDumper;
                mBackupBuf.setPusher(proxyPusher);
                mBackupBuf.setDumper(proxyDumper);
                mBackupBufInited = true;
            } else {
                mBackupBufPusher.clear();
                mBackupBufDumper.clear();
                count = 0;
                XLOGE("[%s] create Backup pusher or dumper failed", __func__);
            }
        }

        // resize backup buffer
        mBackupBuf.resize(count);
    } else {
        mBackupBuf.resize(0);
    }

    return count;
}

void BufferQueueDump::onAcquireBuffer(const int slot, const sp<GraphicBuffer>& buffer, const sp<Fence>& fence) {
    if (buffer == NULL) {
        ST_LOGW("onAcquireBuffer: The GraphicBuffer of slot%d is NULL, ignore it", slot);
        return;
    }

    // check this slot in vector
    for (uint32_t i = mAcquiredBufs.size(); i > 0; i--) {
        uint32_t index = i - 1;
        if ((mAcquiredBufs[index]->mSlot == slot) ||
                (mAcquiredBufs[index]->mSlot == BufferQueue::INVALID_BUFFER_SLOT)) {
            // remove the items which are same with slot id or freed
            mAcquiredBufs.removeAt(index);
            ST_LOGD("onAcquireBuffer: find slot%d has been in acquired buffer[%u], remove it",
                    slot, index);
        }
    }

    // push new buffer into vector
    sp<AcquiredBuffer> acquiredBuffer = new AcquiredBuffer(slot, buffer, fence, systemTime());
    if (acquiredBuffer == NULL) {
        XLOGE("[%s] alloc AcquiredBuffer failed", __func__);
        return;
    }
    mAcquiredBufs.push_back(acquiredBuffer);
}

void BufferQueueDump::updateBuffer(const int slot) {
    // search the slot
    uint32_t index;
    for (index = 0; index < mAcquiredBufs.size(); index++) {
        if (mAcquiredBufs[index]->mSlot == slot) {
            break;
        }
    }

    if (index >= mAcquiredBufs.size()) {
        // can not find the slot acquired
        return;
    }

    if (mBackupBuf.getSize() == 0) {
        // no backup buffer
        // only remove item for the slot if there are other buffer in vector
        if (mAcquiredBufs.size() > 1) {
            // remove the item for this slot
            mAcquiredBufs.removeAt(index);
        } else {
            // only this one in vector
            // hold it and mark to freed with 'INVALID_BUFFER_SLOT'
            sp<AcquiredBuffer>& item = mAcquiredBufs.editItemAt(index);
            item->mSlot = BufferQueue::INVALID_BUFFER_SLOT;
        }
    } else {
        // push GraphicBuffer into backup buffer
        sp<BackupBuffer> buffer = new BackupBuffer(
                                mAcquiredBufs[index]->mGraphicBuffer,
                                mAcquiredBufs[index]->mTimeStamp);
        if (buffer != NULL) {
            mBackupBuf.push(buffer);
        } else {
            XLOGE("[%s] alloc BackupBuffer failed", __func__);
        }

        // remove the item for this slot
        mAcquiredBufs.removeAt(index);
    }
}

void BufferQueueDump::onReleaseBuffer(const int slot) {
    updateBuffer(slot);
}

void BufferQueueDump::onFreeBuffer(const int slot) {
    updateBuffer(slot);
}

bool BackupBufPusher::push(const sp<BackupBuffer>& in) {
    if ((in == NULL) || (in->mGraphicBuffer == NULL)) {
        XLOGW("[%s] input buffer is NULL", __func__);
        return false;
    }

    sp<BackupBuffer>& buffer = editHead();

    // check property of GraphicBuffer, realloc if needed
    bool needCreate = false;
    if ((buffer == NULL) || (buffer->mGraphicBuffer == NULL)) {
        needCreate = true;
        XLOGD("[%s] buffer head is NULL, create it", __func__);
    } else {
        if ((buffer->mGraphicBuffer->width != in->mGraphicBuffer->width) ||
                (buffer->mGraphicBuffer->height != in->mGraphicBuffer->height) ||
                (buffer->mGraphicBuffer->format != in->mGraphicBuffer->format)) {
            needCreate = true;
            XLOGD("[%s] geometry changed, backup=(%d, %d, %d) ==> active=(%d, %d, %d)",
                __func__, buffer->mGraphicBuffer->width, buffer->mGraphicBuffer->height,
                buffer->mGraphicBuffer->format, in->mGraphicBuffer->width,
                in->mGraphicBuffer->height, in->mGraphicBuffer->format);
        }
    }

    if (needCreate) {
        sp<GraphicBuffer> newGraphicBuffer = new GraphicBuffer(
                                            in->mGraphicBuffer->width, in->mGraphicBuffer->height,
                                            in->mGraphicBuffer->format, in->mGraphicBuffer->usage);
        if (newGraphicBuffer == NULL) {
            XLOGE("[%s] alloc GraphicBuffer failed", __func__);
            return false;
        }

        if (buffer == NULL) {
            buffer = new BackupBuffer();
            if (buffer == NULL) {
                XLOGE("[%s] alloc BackupBuffer failed", __func__);
                return false;
            }
        }

        buffer->mGraphicBuffer = newGraphicBuffer;
    }

    float bpp = 0.0f;
    int width = in->mGraphicBuffer->width;
    int height = in->mGraphicBuffer->height;
    int format = in->mGraphicBuffer->format;
    int usage = in->mGraphicBuffer->usage;
    int stride = in->mGraphicBuffer->stride;
    status_t err;

    switch (format) {
        case PIXEL_FORMAT_RGBA_8888:
        case PIXEL_FORMAT_BGRA_8888:
        case PIXEL_FORMAT_RGBX_8888:
        case 0x1ff:
            // tricky format for SGX_COLOR_FORMAT_BGRX_8888 in fact
            bpp = 4.0;
            break;
        case PIXEL_FORMAT_RGB_565:
            bpp = 2.0;
            break;
        case HAL_PIXEL_FORMAT_I420:
            bpp = 1.5;
            break;
        case HAL_PIXEL_FORMAT_YV12:
            bpp = 1.5;
            break;
        default:
            XLOGE("[%s] cannot dump format:%d", __func__, format);
            break;
    }
    
    // backup
    void *src;
    void *dst;
    err = in->mGraphicBuffer->lock(GraphicBuffer::USAGE_SW_READ_OFTEN, &src);
    if (err != NO_ERROR) {
        XLOGE("[%s] lock GraphicBuffer failed", __func__);
        return false;
    }

    err = buffer->mGraphicBuffer->lock(GraphicBuffer::USAGE_SW_READ_OFTEN | GraphicBuffer::USAGE_SW_WRITE_OFTEN, &dst);
    if (err != NO_ERROR) {
        in->mGraphicBuffer->unlock();
        XLOGE("[%s] lock backup buffer failed", __func__);
        return false;
    }

    memcpy(dst, src, stride * height * bpp);

    buffer->mGraphicBuffer->unlock();
    in->mGraphicBuffer->unlock();

    // update timestamp
    buffer->mTimeStamp = in->mTimeStamp;

    return true;
}

void BackupBufDumper::dump() {
    String8 name;
    String8 prefix;

    BufferQueueDump::getDumpFileName(name, mName);

    for (uint32_t i = 0; i < mRingBuffer.getValidSize(); i++) {
        const sp<BackupBuffer>& buffer = getItem(i);
        prefix = String8::format("%s_%u_ts%lldms", name.string(), i,
                                ns2ms(buffer->mTimeStamp));
        GraphicBufferExtra::dump(buffer->mGraphicBuffer, prefix.string(), DUMP_FILE_PATH);
        ST_LOGI("dump: handle=%p", buffer->mGraphicBuffer->handle);
    }
}

AcquiredBuffer::AcquiredBuffer() :
    mSlot(BufferQueue::INVALID_BUFFER_SLOT),
    mFence(Fence::NO_FENCE) {
}

AcquiredBuffer::AcquiredBuffer(const int slot, const sp<GraphicBuffer> buffer,
        const sp<Fence>& fence, const nsecs_t timestamp) :
    mSlot(slot),
    mGraphicBuffer(buffer),
    mFence(fence),
    mTimeStamp(timestamp) {
}


}

