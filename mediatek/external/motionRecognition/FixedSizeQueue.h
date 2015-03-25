#ifndef FIXEDSIZEQUEUE_H
#define FIXEDSIZEQUEUE_H

#include <utils/Vector.h>

template<typename ContainedType>
class FixedSizeQueue
{
    public:
        FixedSizeQueue(const size_t fixedSize);

        void enqueue(const ContainedType& value);

        int getLatestValue(ContainedType* rtnValueArray, int demandSize); //return actual size in the rtnValueArray

    protected:
        const size_t fixedSize_;
        android::Vector<ContainedType> queue_;
        int beginIndex_;
        int endIndex_;
};

template<typename ContainedType>
FixedSizeQueue<ContainedType>::FixedSizeQueue(const size_t fixedSize):
    fixedSize_(fixedSize),
    beginIndex_(0),
    endIndex_(0)
{
    queue_.reserve(fixedSize_);
    for(size_t i=0;i<fixedSize_;++i)
    {queue_.push();}
}

template<typename ContainedType>
void FixedSizeQueue<ContainedType>::enqueue(const ContainedType& value)
{
    //queue_[endIndex_] = value;
    queue_.replaceAt(value,endIndex_);
    endIndex_=(endIndex_+1)%fixedSize_;
    if(endIndex_==beginIndex_)//drop the oldest value
    {beginIndex_=(beginIndex_+1)%fixedSize_;}
}

//return actual size in the rtnValueArray
template<typename ContainedType>
int FixedSizeQueue<ContainedType>::getLatestValue(ContainedType* rtnValueArray, int demandSize)
{
    static const int increment = fixedSize_-1;
    int gottenSize = 0;
    int indexArray[demandSize];
    for(int i=(endIndex_+increment)%fixedSize_;
        i!=beginIndex_;i=(i+increment)%fixedSize_)
    {
        indexArray[gottenSize]=i;
        ++gottenSize ;
        if(gottenSize >=demandSize)
        {break;}
    }

    //fill in rtnValueArray
    for(int i=gottenSize-1;i>=0;--i)
    {
        rtnValueArray[i] = queue_[indexArray[gottenSize-1-i]];
    }
    return gottenSize ;
}


#endif // FIXEDSIZEQUEUE_H
