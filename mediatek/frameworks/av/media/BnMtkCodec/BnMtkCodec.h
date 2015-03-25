#ifndef BNMTKCODEC_H
#define BNMTKCODEC_H

#include "IMtkCodec.h"
#include <utils/KeyedVector.h>
using namespace android;

class BnMtkCodec: public BBinder
{
public:
    BnMtkCodec();
    static int instantiate();
    virtual status_t   onTransact( uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags = 0);
	
private:
    KeyedVector<unsigned long, IMtkCodec*> mvCodec;
    int mCodecId;
    IMtkCodec * fn_CreateCodec(const char *pmime,  CODECTYPE pcodectype);
};

#endif
