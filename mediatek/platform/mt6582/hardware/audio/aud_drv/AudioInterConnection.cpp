#include "AudioInterConnection.h"
#include "AudioType.h"
#include "AudioDef.h"
#include "AudioDigitalType.h"

#define LOG_TAG "AudioInterConnection"
#ifndef ANDROID_DEFAULT_CODE
#include <cutils/xlog.h>
#ifdef ALOGE
#undef ALOGE
#endif
#ifdef ALOGW
#undef ALOGW
#endif ALOGI
#undef ALOGI
#ifdef ALOGD
#undef ALOGD
#endif
#ifdef ALOGV
#undef ALOGV
#endif
#define ALOGE XLOGE
#define ALOGW XLOGW
#define ALOGI XLOGI
#define ALOGD XLOGD
#define ALOGV XLOGV
#else
#include <utils/Log.h>
#endif

namespace android
{

/**
* here define conenction table for input and output
*/
const char mConnectionTable[AudioDigitalType::Num_Input][AudioDigitalType::Num_Output] =
{
    // 0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18
    {  3,  3, -1,  3,  3,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1,  3,  1, -1, -1}, // I00
    {  3,  3, -1,  3,  3, -1,  3, -1, -1, -1, -1, -1, -1, -1, -1,  1,  3, -1, -1}, // I01
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // I02
    {  1,  1, -1,  1,  1,  1, -1,  1, -1,  1, -1, -1, -1, -1, -1,  1,  1,  1, -1}, // I03
    {  1,  1, -1,  1,  1, -1,  1, -1,  1, -1,  1, -1, -1, -1, -1,  1,  1, -1,  1}, // I04
    {  3,  3, -1,  3,  3,  1, -1,  1, -1,  0, -1, -1, -1,  1,  1, -1, -1,  0, -1}, // I05
    {  3,  3, -1,  3,  3, -1,  1, -1,  1, -1,  0, -1,  0,  1,  1, -1, -1, -1,  0}, // I06
    {  3,  3, -1,  3,  3,  1, -1,  0, -1,  0, -1, -1, -1,  1,  1, -1, -1,  0, -1}, // I07
    {  3,  3, -1,  3,  3, -1,  1, -1,  0, -1,  0, -1,  0,  1,  1, -1, -1, -1,  0}, // I08
    {  1,  1, -1,  1,  1,  1, -1, -1, -1, -1, -1, -1,  1, -1, -1,  1,  1, -1, -1}, // I09
    {  3, -1, -1,  3, -1,  0, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1}, // I10
    { -1,  3, -1, -1,  3, -1,  0, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1}, // I11
    {  3, -1, -1,  3, -1,  0, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1}, // I12
    { -1,  3, -1, -1,  3, -1,  0, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1}, // I13
    {  1,  1, -1,  1,  1,  1, -1, -1, -1, -1, -1, -1,  1, -1, -1,  1,  1, -1, -1}, // I14
    //{  3,  3,  3,  3,  3,  3, -1, -1, -1,  1, -1, -1, -1, -1, -1,  3,  1, -1, -1}, // I15
    //{  3,  3,  3,  3,  3, -1,  3, -1, -1, -1,  1, -1, -1, -1, -1,  1,  3, -1, -1}, // I16
};

/**
* connection bits of certain bits
*/
const char mConnectionbits[AudioDigitalType::Num_Input][AudioDigitalType::Num_Output] =
{
    // 0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18
    {  0, 16, -1, 16,  0, 16, -1, -1, -1, -1, -1, -1, -1, -1, -1, 16, 22, -1, -1}, // I00
    {  1, 17, -1, 17,  1, -1, 22, -1, -1, -1, -1, -1, -1, -1, -1, 17, 23, -1, -1}, // I01
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // I02
    {  3, 19, -1, 19,  3, 18, -1, 26, -1,  0, -1, -1, -1, -1, -1, 19, 25, 13, -1}, // I03
    {  4, 20, -1, 20,  4, -1, 23, -1, 29, -1,  3, -1, -1, -1, -1, 20, 26, -1, 16}, // I04
    {  5, 21, -1, 21,  5, 19, -1, 27, -1,  1, -1, -1, -1, 16, 20, -1, -1, 14, -1}, // I05
    {  6, 22, -1, 22,  6, -1, 24, -1, 30, -1,  4, -1,  9, 17, 21, -1, -1, -1, 17}, // I06
    {  7, 23, -1, 23,  7, 20, -1, 28, -1,  2, -1, -1, -1, 18, 22, -1, -1, 15, -1}, // I07
    {  8, 24, -1, 24,  8, -1, 25, -1, 31, -1,  5, -1, 10, 19, 23, -1, -1, -1, 18}, // I08
    {  9, 25, -1, 25,  9, 21, 27, -1, -1, -1, -1, -1, 11, -1, -1, 21, 27, -1, -1}, // I09
    {  0, -1, -1,  8, -1, 12, -1, 14, -1, -1, -1, -1, -1, -1, -1, -1, -1, 24, -1}, // I10
    { -1,  2, -1, -1, 10, -1, 13, -1, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, 25}, // I11
    {  0, -1, -1,  8, -1, 12, -1, 14, -1, -1, -1, -1, -1, -1, -1, -1, -1,  6, -1}, // I12
    { -1,  2, -1, -1, 10, -1, 13, -1, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,  7}, // I13
    { 12, 17, -1, 27,  0,  5, -1, -1, -1, -1, -1, -1, 12, -1, -1, 28,  1, -1, -1}, // I14
    //{ 13, 18, 23, 28,  1,  6, -1, -1, -1, 10, -1, -1, -1, -1, -1, 29,  2, -1, -1}, // I15
    //{ 14, 19, 24, 29,  2, -1,  8, -1, -1, -1, 11, -1, -1, -1, -1, 30,  3, -1, -1}, // I16
};


/**
* connection shift bits of certain bits
*/
const char mShiftConnectionbits[AudioDigitalType::Num_Input][AudioDigitalType::Num_Output] =
{
    // 0   1   2   3   4   5   6    7  8   9  10  11  12  13  14  15  16  17  18
    { 10, 26, -1, 26, 10, 19, -1, -1, -1, -1, -1, -1, -1, -1, -1, 31, -1, -1, -1}, // I00
    { 11, 27, -1, 27, 11, -1, 20, -1, -1, -1, -1, -1, -1, -1, -1, -1,  4, -1, -1}, // I01
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // I02
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // I03
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // I04
    { 12, 28, -1, 28, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // I05
    { 13, 29, -1, 29, 13, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // I06
    { 14, 30, -1, 30, 14, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // I07
    { 15, 31, -1, 31, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // I08
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // I09
    {  1, -1, -1,  9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // I10
    { -1,  3, -1, -1, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // I11
    {  1, -1, -1,  9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // I12
    { -1,  3, -1, -1, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // I13
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // I14
    //{ 15, 20, 25, 30,  3,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0, -1, -1, -1}, // I15
    //{ 16, 21, 26, 31,  4, -1,  9, -1, -1, -1, -1, -1, -1, -1, -1, -1,  5, -1, -1}, // I16
};

/**
* connection of register
*/
const short mConnectionReg[AudioDigitalType::Num_Input][AudioDigitalType::Num_Output] =
{
    //   0      1      2      3      4      5      6      7      8      9     10     11    12      13     14     15     16     17     18
    { 0x20,  0x20,    -1,  0x24,  0x28,  0x28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1, 0x438, 0x438,    -1,    -1}, //I00
    { 0x20,  0x20,    -1,  0x24,  0x28,    -1,  0x28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1, 0x438, 0x438,    -1,    -1}, //I01
    {   -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1}, //I02
    { 0x20,  0x20,    -1,  0x24,  0x28,  0x28,    -1,  0x28,    -1,  0x2C,    -1,    -1,    -1,    -1,    -1, 0x438, 0x438,  0x30,    -1}, //I03
    { 0x20,  0x20,    -1,  0x24,  0x28,    -1,  0x28,    -1,  0x28,    -1,  0x2C,    -1,    -1,    -1,    -1, 0x438, 0x438,    -1,  0x30}, //I04
    { 0x20,  0x20,    -1,  0x24,  0x28,  0x28,    -1,  0x28,    -1,  0x2C,    -1,    -1,    -1, 0x420, 0x420,    -1,    -1,  0x30,    -1}, //I05
    { 0x20,  0x20,    -1,  0x24,  0x28,    -1,  0x28,    -1,  0x28,    -1,  0x2C,    -1,  0x2C, 0x420, 0x420,    -1,    -1,    -1,  0x30}, //I06
    { 0x20,  0x20,    -1,  0x24,  0x28,  0x28,    -1,  0x28,    -1,  0x2C,    -1,    -1,    -1, 0x420, 0x420,    -1,    -1,  0x30,    -1}, //I07
    { 0x20,  0x20,    -1,  0x24,  0x28,    -1,  0x28,    -1,  0x28,    -1,  0x2C,    -1,  0x2C, 0x420, 0x420,    -1,    -1,    -1,  0x30}, //I08
    { 0x20,  0x20,    -1,  0x24,  0x28,  0x28,    -1,    -1,    -1,    -1,    -1,    -1,  0x2C,    -1,    -1, 0x438, 0x438,    -1,    -1}, //I09
    {0x420,    -1,    -1, 0x420,    -1, 0x420,    -1, 0x420,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1, 0x420,    -1}, //I10
    {   -1, 0x420,    -1,    -1, 0x420,    -1, 0x420,    -1, 0x420,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1, 0x420}, //I11
    {0x438,    -1,    -1, 0x438,    -1, 0x438,    -1, 0x438,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1, 0x440,    -1}, //I12
    {   -1, 0x438,    -1,    -1, 0x438,    -1, 0x438,    -1, 0x438,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1, 0x440}, //I13
    { 0x2C,  0x2C,    -1,  0x2C,  0x30,  0x30,    -1,    -1,    -1,    -1,    -1,    -1,  0x30,    -1,    -1, 0x438, 0x440,    -1,    -1}, //I14
    //{ 0x2C,  0x2C,  0x2C,  0x2C,  0x30,  0x30,    -1,    -1,    -1,  0x30,    -1,    -1,    -1,    -1,    -1, 0x438, 0x440,    -1,    -1}, //I15
    //{ 0x2C,  0x2C,  0x2C,  0x2C,  0x30,    -1,  0x30,    -1,    -1,    -1,  0x30,    -1,    -1,    -1,    -1, 0x438, 0x440,    -1,    -1}, //I16
};


/**
* shift connection of register
*/
const short mShiftConnectionReg[AudioDigitalType::Num_Input][AudioDigitalType::Num_Output] =
{
    //    0      1      2      3      4      5     6       7      8      9     10     11     12     13     14     15     16     17     18
    {  0x20,  0x20,    -1,  0x24,  0x28,  0x30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1, 0x438,    -1,    -1,    -1}, //I00
    {  0x20,  0x20,    -1,  0x24,  0x28,    -1,  0x30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1, 0x440,    -1,    -1}, //I01
    {    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1}, //I02
    {    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1}, //I03
    {    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1}, //I04
    {  0x20,  0x20,    -1,  0x24,  0x28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1}, //I05
    {  0x20,  0x20,    -1,  0x24,  0x28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1}, //I06
    {  0x20,  0x20,    -1,  0x24,  0x28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1}, //I07
    {  0x20,  0x20,    -1,  0x24,  0x28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1}, //I08
    {    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1}, //I09
    { 0x420,    -1,    -1, 0x420,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1}, //I10
    { 0x420, 0x420,    -1,    -1, 0x420,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1}, //I11
    { 0x438,    -1,    -1, 0x438,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1}, //I12
    {    -1, 0x438,    -1,    -1, 0x438,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1}, //I13
    {    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1}, //I14
    //{  0x2C,  0x2C,    -1,  0x2C,  0x30,  0x30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1, 0x440,    -1,    -1,    -1}, //I15
    //{  0x2C,  0x2C,    -1,  0x2C,  0x30,    -1,  0x30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1, 0x440,    -1,    -1}, //I16
};

/**
* connection state of register
*/
char mConnectionState[AudioDigitalType::Num_Input][AudioDigitalType::Num_Output] =
{
    // 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // I00
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // I01
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // I02
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // I03
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // I04
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // I05
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // I06
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // I07
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // I08
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // I09
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // I10
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // I11
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // I12
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // I13
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // I14
    //    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // I15
    //    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}  // I16
};



AudioInterConnection::AudioInterConnection()
{
    ALOGD("AudioInterConnection contructor\n");
    mAduioAfeInstanse = AudioAfeReg::getInstance();
    if (!mAduioAfeInstanse)
    {
        ALOGD("AudioInterConnection get mAduioAfeInstanse error \n");
    }
    // reset all interconnection
    mAduioAfeInstanse->SetAfeReg(AFE_CONN0, 0, 0xffffffff);
    mAduioAfeInstanse->SetAfeReg(AFE_CONN1, 0, 0xffffffff);
    mAduioAfeInstanse->SetAfeReg(AFE_CONN2, 0, 0xffffffff);
    mAduioAfeInstanse->SetAfeReg(AFE_CONN3, 0, 0xffffffff);
}

AudioInterConnection::~AudioInterConnection()
{
    ALOGD("AudioInterConnection destructor\n");
}

bool AudioInterConnection::CheckBitsandReg(short regaddr , char bits)
{
    if (regaddr <= 0 || bits < 0)
    {
        ALOGD("regaddr = %x bits = %d \n", regaddr, bits);
        return false;
    }
    return true;
}

status_t AudioInterConnection::SetinputConnection(uint32 ConnectionState, uint32 Input, uint32 Output)
{
    ALOGD("SetinputConnection ConnectionState = %d Input = %d Output = %d\n", ConnectionState, Input, Output);
    if ((mConnectionTable[Input][Output]) < 0)
    {
        ALOGD("no connection mpConnectionTable[%d][%d] = %d\n", Input, Output, mConnectionTable[Input][Output]);
    }
    else if ((mConnectionTable[Input][Output]) == 0)
    {
        ALOGD("test only !! mpConnectionTable[%d][%d] = %d\n", Input, Output, mConnectionTable[Input][Output]);
    }
    else
    {
        if (mConnectionTable[Input][Output])
        {
            int connectionBits = 0;
            int connectReg = 0;
            switch (ConnectionState)
            {
                case (int)AudioDigitalType::DisConnect:
                {
                    ALOGD("nConnectionState = %d \n", ConnectionState);
                    if ((mConnectionState[Input][Output]&AudioDigitalType::Connection) == AudioDigitalType::Connection)
                    {
                        // here to disconnect connect bits
                        connectionBits = mConnectionbits[Input][Output];
                        connectReg = mConnectionReg[Input][Output];
                        if (CheckBitsandReg(connectReg, connectionBits))
                        {
                            mAduioAfeInstanse->SetAfeReg(connectReg, 0 << connectionBits, 1 << connectionBits);
                            mConnectionState[Input][Output] &= ~(AudioDigitalType::Connection);
                        }
                    }
                    if ((mConnectionState[Input][Output]&AudioDigitalType::ConnectionShift) == AudioDigitalType::ConnectionShift)
                    {
                        // here to disconnect connect shift bits
                        connectionBits = mShiftConnectionbits[Input][Output];
                        connectReg = mShiftConnectionReg[Input][Output];
                        if (CheckBitsandReg(connectReg, connectionBits))
                        {
                            mAduioAfeInstanse->SetAfeReg(connectReg, 0 << connectionBits, 1 << connectionBits);
                            mConnectionState[Input][Output] &= ~(AudioDigitalType::ConnectionShift);
                        }
                    }
                    break;
                }
                case (int)AudioDigitalType::Connection:
                {
                    ALOGD("nConnectionState = %d \n", ConnectionState);
                    // here to disconnect connect shift bits
                    connectionBits = mConnectionbits[Input][Output];
                    connectReg = mConnectionReg[Input][Output];
                    if (CheckBitsandReg(connectReg, connectionBits))
                    {
                        mAduioAfeInstanse->SetAfeReg(connectReg, 1 << connectionBits, 1 << connectionBits);
                        mConnectionState[Input][Output] |= AudioDigitalType::Connection;
                    }
                    break;
                }
                case (int)AudioDigitalType::ConnectionShift:
                {
                    ALOGD("nConnectionState = %d \n", ConnectionState);
                    if ((mConnectionTable[Input][Output]&AudioDigitalType::ConnectionShift) != AudioDigitalType::ConnectionShift)
                    {
                        ALOGD("donn't support shift opeartion");
                        break;
                    }
                    connectionBits = mShiftConnectionbits[Input][Output];
                    connectReg = mShiftConnectionReg[Input][Output];
                    if (CheckBitsandReg(connectReg, connectionBits))
                    {
                        mAduioAfeInstanse->SetAfeReg(connectReg, 1 << connectionBits, 1 << connectionBits);
                        mConnectionState[Input][Output] |= AudioDigitalType::ConnectionShift;
                    }
                    break;
                }
                default:
                    ALOGD("no this state ConnectionState = %d \n", ConnectionState);
                    break;
            }
        }
    }
    return NO_ERROR;
}


void AudioInterConnection::dump()
{
    for (int i = 0 ; i < AudioDigitalType::Num_Input ; i++)
    {
        char *temp = &mConnectionState[i][0];
        ALOGD("i%d %d %d %d %d %d %d %d %d %d", i,
              *temp, *(temp + 1), *(temp + 2), *(temp + 3), *(temp + 4), *(temp + 5), *(temp + 6), *(temp + 7), *(temp + 8));
        ALOGD(" %d %d %d %d %d %d %d %d %d \n",
              *(temp + 9), *(temp + 10), *(temp + 11), *(temp + 12), *(temp + 13), *(temp + 14), *(temp + 15), *(temp + 16), *(temp + 17));
    }
}

}

