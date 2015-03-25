#include <utils/String8.h>


#include <gui/mediatek/RingBuffer.h>


using namespace android;

//--------------------------------------------------------------------------------------------------
class IntPusher : public RingBuffer<int>::Pusher {
public:
    IntPusher(RingBuffer<int>& rb) : RingBuffer<int>::Pusher(rb) {}
    virtual bool push(const int& in) {
        int val = in * 2;

        int& head = editHead();
        head = val;

        XLOGD("    count:%u head:%d", mRingBuffer.getCount(), head);

        return true;
    }
};

class IntDumper : public RingBuffer<int>::Dumper {
public:
    IntDumper(RingBuffer<int>& rb) : RingBuffer<int>::Dumper(rb) {}
    virtual void dump() {
        for (uint32_t i = 0; i < mRingBuffer.getValidSize(); i++) {
            const int& item = getItem(i);
            XLOGD("    [%d] %d", i, item);
        }
    }
};

void int_test()
{
    RingBuffer<int> ringbuffer(20);
    int count;

    for (int i = 0; i < 123; i++) {
        ringbuffer.push(i);
    }
    ringbuffer.dump();

    ringbuffer.resize(10);
    for (int i = 0; i < 86; i++) {
        ringbuffer.push(i);
    }
    ringbuffer.dump();
}

void int_custom_test()
{
    RingBuffer<int> ringbuffer;
    sp< RingBuffer<int>::Pusher > pusher = new IntPusher(ringbuffer);
    sp< RingBuffer<int>::Dumper > dumper = new IntDumper(ringbuffer);
    ringbuffer.setPusher(pusher);
    ringbuffer.setDumper(dumper);

    for (int i = 0; i < 86; i++) {
        ringbuffer.push(i);
    }
    ringbuffer.dump();

    ringbuffer.resize(20);
    for (int i = 0; i < 125; i++) {
        ringbuffer.push(i);
    }
    ringbuffer.dump();
}

//--------------------------------------------------------------------------------------------------
class FloatPusher : public RingBuffer<float>::Pusher {
public:
    FloatPusher(RingBuffer<float>& rb) : RingBuffer<float>::Pusher(rb) {}
    virtual bool push(const float& in) {
        float val = in + 100.0;

        float& head = editHead();
        head = val;

        XLOGD("    count:%u head:%f", mRingBuffer.getCount(), head);

        return true;
    }
};

class FloatDumper : public RingBuffer<float>::Dumper {
public:
    FloatDumper(RingBuffer<float>& rb) : RingBuffer<float>::Dumper(rb) {}
    virtual void dump() {
        for (uint32_t i = 0; i < mRingBuffer.getValidSize(); i++) {
            const float& item = getItem(i);
            XLOGD("    [%d] %f", i, item);
        }
    }
};

void float_test()
{
    RingBuffer<float> ringbuffer(20);
    float f = 0.567;

    for (int i = 0; i < 123; i++) {
        ringbuffer.push(f);
        f += 1.0;
    }
    ringbuffer.dump();

    ringbuffer.resize(10);
    f = 0.123;
    for (int i = 0; i < 86; i++) {
        ringbuffer.push(f);
        f += 1.0;
    }
    ringbuffer.dump();
}

void float_custom_test()
{
    RingBuffer<float> ringbuffer;
    sp< RingBuffer<float>::Pusher > pusher = new FloatPusher(ringbuffer);
    sp< RingBuffer<float>::Dumper > dumper = new FloatDumper(ringbuffer);
    ringbuffer.setPusher(pusher);
    ringbuffer.setDumper(dumper);
    float f = 0.567;

    for (int i = 0; i < 86; i++) {
        ringbuffer.push(f);
        f += 1.0;
    }
    ringbuffer.dump();

    ringbuffer.resize(20);
    f = 0.123;
    for (int i = 0; i < 125; i++) {
        ringbuffer.push(f);
        f += 1.0;
    }
    ringbuffer.dump();
}

//--------------------------------------------------------------------------------------------------
class CustomStruct : virtual public RefBase {
public:
    static uint32_t sCount;

    uint32_t mCount;
    float    mFloat;
    String8  mString;

    CustomStruct() {
        mCount = sCount++;
        XLOGD("[CustomStruct] %d", mCount);
    }

    ~CustomStruct() {
        XLOGD("[~CustomStruct] %d",mCount);
    }
};
uint32_t CustomStruct::sCount = 0;

class SpPusher : public RingBuffer< sp<CustomStruct> >::Pusher {
public:
    SpPusher(RingBuffer< sp<CustomStruct> >& rb) : RingBuffer< sp<CustomStruct> >::Pusher(rb) {}
    virtual bool push(const sp<CustomStruct>& in) {
        sp<CustomStruct>& head = editHead();

        head = in;
        head->mFloat += 10000.0;

        XLOGD("    count:%u head:%f", mRingBuffer.getCount(), head->mFloat);

        return true;
    }
};

class SpDumper : public RingBuffer< sp<CustomStruct> >::Dumper {
public:
    SpDumper(RingBuffer< sp<CustomStruct> >& rb) : RingBuffer< sp<CustomStruct> >::Dumper(rb) {}
    virtual void dump() {
        for (uint32_t i = 0; i < mRingBuffer.getValidSize(); i++) {
            const sp<CustomStruct>& item = getItem(i);
            item->mString = String8::format("mCount:%d mFloat:%f", item->mCount, item->mFloat);

            XLOGD("    [%d] %s", i, item->mString.string());
        }
    }
};

void sp_custom_test(void)
{
    RingBuffer< sp<CustomStruct> > ringbuffer;
    sp<RingBuffer< sp<CustomStruct> >::Pusher> pusher = new SpPusher(ringbuffer);
    sp<RingBuffer< sp<CustomStruct> >::Dumper> dumper = new SpDumper(ringbuffer);
    ringbuffer.setPusher(pusher);
    ringbuffer.setDumper(dumper);

    float f = 0.567;
    for (int i = 0; i < 100; i++) {
        sp<CustomStruct> st = new CustomStruct();
        st->mFloat = f;
        ringbuffer.push(st);
        f += 1.0;
    }
    ringbuffer.dump();

    ringbuffer.resize(20);
    f = 0.123;
    for (int i = 0; i < 10; i++) {
        sp<CustomStruct> st = new CustomStruct();
        st->mFloat = f;
        ringbuffer.push(st);
        f += 1.0;
    }
    ringbuffer.dump();
}

//--------------------------------------------------------------------------------------------------
int main(void)
{
    int_test();
    int_custom_test();

    float_test();
    float_custom_test();

    sp_custom_test();

    return 0;
}


