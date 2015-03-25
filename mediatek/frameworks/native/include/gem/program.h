#ifndef GEM_PROGRAM_H
#define GEM_PROGRAM_H

#include <utils/String8.h>
#include <utils/Vector.h>

// define related magic numbers as symbols
// GraphicBuffer
#define GBUF (0x46554247)
// GEMQueue BufferSlot
#define MQBS (0x5351474D)


namespace android
{


// define function pointer types
typedef void (*initFP)(void**);
typedef void (*teardownFP)(void*);
typedef void (*setvalueFP)(const char*, const char*, void*);
typedef void (*getvalueFP)(const char*, char*, int, void*);
typedef int  (*processFP)(const char**, const int*, int, char*, int, void*);
typedef void (*resetFP)(void*);


// get full program name string symbol
String8 makeFullProgramName(const String8& libname, const String8& progname);


// simplified and compatible to native_program
// of android media effect filter framework
class GEMProgram
{
public:
    // data adpater to android native program interface design
    // use general type from user side, and cast when calling in GEMProgram functions
    class Adapter
    {
    public:
        Vector<const void*> mInputs;
        Vector<int32_t>     mInputSizes;

        void*   mOutput;
        int32_t mOutputSize;

        Adapter()
        {
            mInputs.clear();
            mInputSizes.clear();
            mOutput = NULL;
            mOutputSize = -1;
        }
    };

private:
    String8 mName;

    // to the dynamic library
    void* mLib;

    // custom user data
    void* mData;

    // function pointers to the native function impl
    initFP     mInit;
    setvalueFP mSetvalue;
    getvalueFP mGetvalue;
    processFP  mProcess;
    resetFP    mReset;
    teardownFP mTeardown;

public:
    GEMProgram();
    ~GEMProgram();

    const String8& getName() { return mName; };

    // hook program functions up
    status_t openProgram(const String8& libname, const String8& progname);

    // hooked function entries
    status_t init();
    status_t setvalue(const String8& key, const String8& value);
    String8  getvalue(const String8& key);
    status_t process(const Adapter* adapter);
    status_t reset();
    status_t teardown();
};


} // namespace android
#endif
