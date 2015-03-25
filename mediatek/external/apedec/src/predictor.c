#include <string.h>
#include "predictor.h"
#include <utils/Log.h>

/* Return 0 if x is zero, -1 if x is positive, 1 if x is negative */
#if defined(___CPU_ARM9E___) && defined(__CC_ARM)
    __inline int SIGN(int x)
    {
        __asm {
            CMP x, #0;
            MOVGT x, #-1;
            MOVMI x, #1;
            }
    return x;
    }
#else
    #define SIGN(x) (x) ? (((x) > 0) ? -1 : 1) : 0
#endif

static const int32_t predictor_init_coeff_tab[4] = {
  360, 317, -109, 98
};

#define YDELAY          (18 + PREDICTOR_LEN)
#define YADAPTCOEFFS    (5)
#define YDELAYA         (18 + PREDICTOR_LEN*4)
#define YADAPTCOEFFSA   (18)

#define XDELAY          (18 + PREDICTOR_LEN*3)
#define XADAPTCOEFFS    (10)
#define XDELAYA         (18 + PREDICTOR_LEN*2)
#define XADAPTCOEFFSA   (14)


STATIC_DECLARE void init_predictor_decoder(struct predictor_t* predictor)
{
    /* Zero the history buffers */
    memset(predictor->historybuffer, 0, PREDICTOR_SIZE * sizeof(int32_t));
    predictor->buffer = predictor->historybuffer;

    /* init Yfilter */
    predictor->YfilterA = 0;
    predictor->YfilterB = 0;
    predictor->YlastA = 0;
    memcpy(predictor->YcoeffsA, predictor_init_coeff_tab, sizeof(predictor_init_coeff_tab));
    memset(predictor->YcoeffsB, 0, sizeof(predictor->YcoeffsB));

    /* init Xfilter */
    predictor->XfilterA = 0;
    predictor->XfilterB = 0;
    predictor->XlastA = 0;
    memcpy(predictor->XcoeffsA, predictor_init_coeff_tab, sizeof(predictor_init_coeff_tab));
    memset(predictor->XcoeffsB, 0, sizeof(predictor->XcoeffsB));
}

extern int g_count;
STATIC_DECLARE void predictor_decode_stereo(struct predictor_t* predictor,
                             int32_t* decoded0,
                             int32_t* decoded1,
                             int count)
{
    int32_t predictionA, predictionB;
    int32_t YlastA, YfilterA, XlastA, XfilterA;
    int32_t* buf = predictor->buffer;

    YlastA = predictor->YlastA;
    YfilterA = predictor->YfilterA;
    XlastA = predictor->XlastA;
    XfilterA = predictor->XfilterA;

    while (count--) {

        if (*decoded0 > 0) {
            int temp, delay, adaptcoeff;

            delay = buf[YDELAY - 1];
            adaptcoeff = buf[YADAPTCOEFFS - 1];
            temp = predictor->YcoeffsA[0];
            predictionA = delay * temp;
            predictor->YcoeffsA[0] = temp - adaptcoeff;

            delay = buf[YDELAY-1 - 1];
            adaptcoeff = buf[YADAPTCOEFFS-1 - 1];
            temp = predictor->YcoeffsA[1];
            predictionA += delay * temp;
            predictor->YcoeffsA[1] = temp - adaptcoeff;

            delay = buf[YDELAY-2 - 1];
            adaptcoeff = buf[YADAPTCOEFFS-2 - 1];
            temp = predictor->YcoeffsA[2];
            predictionA += delay * temp;
            predictor->YcoeffsA[2] = temp - adaptcoeff;

            delay = buf[YDELAY-3 - 1];
            adaptcoeff = buf[YADAPTCOEFFS-3 - 1];
            temp = predictor->YcoeffsA[3];
            predictionA += delay * temp;
            predictor->YcoeffsA[3] = temp - adaptcoeff;

            buf[XDELAY] = XlastA;
            buf[XDELAY-1] = buf[XDELAY] - buf[XDELAY-1];
            buf[XADAPTCOEFFS] = SIGN(buf[XDELAY]);
            buf[XADAPTCOEFFS-1] = SIGN(buf[XDELAY-1]);

            delay = buf[XDELAY];
            adaptcoeff = buf[XADAPTCOEFFS];
            temp = predictor->YcoeffsB[0];
            predictionB = delay * temp;
            predictor->YcoeffsB[0] = temp - adaptcoeff;

            delay = buf[XDELAY-1];
            adaptcoeff = buf[XADAPTCOEFFS-1];
            temp = predictor->YcoeffsB[1];
            predictionB += delay * temp;
            predictor->YcoeffsB[1] = temp - adaptcoeff;

            delay = buf[XDELAY-2];
            adaptcoeff = buf[XADAPTCOEFFS-2];
            temp = predictor->YcoeffsB[2];
            predictionB += delay * temp;
            predictor->YcoeffsB[2] = temp - adaptcoeff;

            delay = buf[XDELAY-3];
            adaptcoeff = buf[XADAPTCOEFFS-3];
            temp = predictor->YcoeffsB[3];
            predictionB += delay * temp;
            predictor->YcoeffsB[3] = temp - adaptcoeff;

            delay = buf[XDELAY-4];
            adaptcoeff = buf[XADAPTCOEFFS-4];
            temp = predictor->YcoeffsB[4];
            predictionB += delay * temp;
            predictor->YcoeffsB[4] = temp - adaptcoeff;

            YlastA = *decoded0 + ((predictionA + (predictionB >> 1)) >> 10);
            YfilterA =  YlastA + ((YfilterA * 31) >> 5);
            *(decoded0++) = YfilterA;
            }
        else if (*decoded0 < 0){
            int temp, delay, adaptcoeff;

            delay = buf[YDELAY - 1];
            adaptcoeff = buf[YADAPTCOEFFS - 1];
            temp = predictor->YcoeffsA[0];
            predictionA = delay * temp;
            predictor->YcoeffsA[0] = temp + adaptcoeff;

            delay = buf[YDELAY-1 - 1];
            adaptcoeff = buf[YADAPTCOEFFS-1 - 1];
            temp = predictor->YcoeffsA[1];
            predictionA += delay * temp;
            predictor->YcoeffsA[1] = temp + adaptcoeff;

            delay = buf[YDELAY-2 - 1];
            adaptcoeff = buf[YADAPTCOEFFS-2 - 1];
            temp = predictor->YcoeffsA[2];
            predictionA += delay * temp;
            predictor->YcoeffsA[2] = temp + adaptcoeff;

            delay = buf[YDELAY-3 - 1];
            adaptcoeff = buf[YADAPTCOEFFS-3 - 1];
            temp = predictor->YcoeffsA[3];
            predictionA += delay * temp;
            predictor->YcoeffsA[3] = temp + adaptcoeff;

            buf[XDELAY] = XlastA;
            buf[XDELAY-1] = buf[XDELAY] - buf[XDELAY-1];
            buf[XADAPTCOEFFS] = SIGN(buf[XDELAY]);
            buf[XADAPTCOEFFS-1] = SIGN(buf[XDELAY-1]);

            delay = buf[XDELAY];
            adaptcoeff = buf[XADAPTCOEFFS];
            temp = predictor->YcoeffsB[0];
            predictionB = delay * temp;
            predictor->YcoeffsB[0] = temp + adaptcoeff;

            delay = buf[XDELAY-1];
            adaptcoeff = buf[XADAPTCOEFFS-1];
            temp = predictor->YcoeffsB[1];
            predictionB += delay * temp;
            predictor->YcoeffsB[1] = temp + adaptcoeff;

            delay = buf[XDELAY-2];
            adaptcoeff = buf[XADAPTCOEFFS-2];
            temp = predictor->YcoeffsB[2];
            predictionB += delay * temp;
            predictor->YcoeffsB[2] = temp + adaptcoeff;

            delay = buf[XDELAY-3];
            adaptcoeff = buf[XADAPTCOEFFS-3];
            temp = predictor->YcoeffsB[3];
            predictionB += buf[XDELAY-3] * temp;
            predictor->YcoeffsB[3] = temp + adaptcoeff;

            temp = predictor->YcoeffsB[4];
            predictionB += buf[XDELAY-4] * temp;
            predictor->YcoeffsB[4] = temp + buf[XADAPTCOEFFS-4];

            YlastA = *decoded0 + ((predictionA + (predictionB >> 1)) >> 10);
            YfilterA =  YlastA + ((YfilterA * 31) >> 5);
            *(decoded0++) = YfilterA;
            }
        else {
            predictionA = (buf[YDELAY - 1] * predictor->YcoeffsA[0]) +
                          (buf[YDELAY-1 - 1] * predictor->YcoeffsA[1]) +
                          (buf[YDELAY-2 - 1] * predictor->YcoeffsA[2]) +
                          (buf[YDELAY-3 - 1] * predictor->YcoeffsA[3]);

            buf[XDELAY] = XlastA;
            buf[XDELAY-1] = buf[XDELAY] - buf[XDELAY-1];
            buf[XADAPTCOEFFS] = SIGN(buf[XDELAY]);
            buf[XADAPTCOEFFS-1] = SIGN(buf[XDELAY-1]);

            predictionB = (buf[XDELAY] * predictor->YcoeffsB[0]) +
                          (buf[XDELAY-1] * predictor->YcoeffsB[1]) +
                          (buf[XDELAY-2] * predictor->YcoeffsB[2]) +
                          (buf[XDELAY-3] * predictor->YcoeffsB[3]) +
                          (buf[XDELAY-4] * predictor->YcoeffsB[4]);

            YlastA = *decoded0 + ((predictionA + (predictionB >> 1)) >> 10);
            YfilterA =  YlastA + ((YfilterA * 31) >> 5);
            *(decoded0++) = YfilterA;
            }

#define MOVE_HISTORY_TO_HEAD\
        memmove(predictor->historybuffer, buf, PREDICTOR_SIZE * sizeof(int32_t));\
        buf = predictor->historybuffer;

        if (*decoded1 > 0) {
            int temp, delay, adaptcoeff;

            delay = buf[XDELAY];
            adaptcoeff = buf[XADAPTCOEFFS];
            temp = predictor->XcoeffsA[0];
            predictionA = delay * temp;
            predictor->XcoeffsA[0] = temp - adaptcoeff;

            delay = buf[XDELAY-1];
            adaptcoeff = buf[XADAPTCOEFFS-1];
            temp = predictor->XcoeffsA[1];
            predictionA += delay * temp;
            predictor->XcoeffsA[1] = temp - adaptcoeff;

            delay = buf[XDELAY-2];
            adaptcoeff = buf[XADAPTCOEFFS-2];
            temp = predictor->XcoeffsA[2];
            predictionA += delay * temp;
            predictor->XcoeffsA[2] = temp - adaptcoeff;

            delay = buf[XDELAY-3];
            adaptcoeff = buf[XADAPTCOEFFS-3];
            temp = predictor->XcoeffsA[3];
            predictionA += delay * temp;
            predictor->XcoeffsA[3] = temp - adaptcoeff;

            buf[YDELAY] = YlastA;
            buf[YADAPTCOEFFS] = SIGN(buf[YDELAY]);
            buf[YDELAY-1] = buf[YDELAY] - buf[YDELAY-1];
            buf[YADAPTCOEFFS-1] = SIGN(buf[YDELAY-1]);

            delay = buf[YDELAY];
            adaptcoeff = buf[YADAPTCOEFFS];
            temp = predictor->XcoeffsB[0];
            predictionB = delay * temp;
            predictor->XcoeffsB[0] = temp - adaptcoeff;

            delay = buf[YDELAY-1];
            adaptcoeff = buf[YADAPTCOEFFS-1];
            temp = predictor->XcoeffsB[1];
            predictionB += delay * temp;
            predictor->XcoeffsB[1] = temp - adaptcoeff;

            delay = buf[YDELAY-2];
            adaptcoeff = buf[YADAPTCOEFFS-2];
            temp = predictor->XcoeffsB[2];
            predictionB += delay * temp;
            predictor->XcoeffsB[2] = temp - adaptcoeff;

            delay = buf[YDELAY-3];
            adaptcoeff = buf[YADAPTCOEFFS-3];
            temp = predictor->XcoeffsB[3];
            predictionB += delay * temp;
            predictor->XcoeffsB[3] = temp - adaptcoeff;

            delay = buf[YDELAY-4];
            adaptcoeff = buf[YADAPTCOEFFS-4];
            temp = predictor->XcoeffsB[4];
            predictionB += delay * temp;
            predictor->XcoeffsB[4] = temp - adaptcoeff;

            XlastA = *decoded1 + ((predictionA + (predictionB >> 1)) >> 10);
            XfilterA =  XlastA + ((XfilterA * 31) >> 5);

            *(decoded1++) = XfilterA;
            buf++;
            if (buf == predictor->historybuffer + PREDICTOR_HISTORY_SIZE) {MOVE_HISTORY_TO_HEAD}
            }
        else if (*decoded1 < 0) {
            int temp, delay, adaptcoeff;

            delay = buf[XDELAY];
            adaptcoeff = buf[XADAPTCOEFFS];
            temp = predictor->XcoeffsA[0];
            predictionA = delay * temp;
            predictor->XcoeffsA[0] = temp + adaptcoeff;

            delay = buf[XDELAY-1];
            adaptcoeff = buf[XADAPTCOEFFS-1];
            temp = predictor->XcoeffsA[1];
            predictionA += delay * temp;
            predictor->XcoeffsA[1] = temp + adaptcoeff;

            delay = buf[XDELAY-2];
            adaptcoeff = buf[XADAPTCOEFFS-2];
            temp = predictor->XcoeffsA[2];
            predictionA += delay * temp;
            predictor->XcoeffsA[2] = temp + adaptcoeff;

            delay = buf[XDELAY-3];
            adaptcoeff = buf[XADAPTCOEFFS-3];
            temp = predictor->XcoeffsA[3];
            predictionA += delay * temp;
            predictor->XcoeffsA[3] = temp + adaptcoeff;

            buf[YDELAY] = YlastA;
            buf[YADAPTCOEFFS] = SIGN(buf[YDELAY]);
            buf[YDELAY-1] = buf[YDELAY] - buf[YDELAY-1];
            buf[YADAPTCOEFFS-1] = SIGN(buf[YDELAY-1]);

            delay = buf[YDELAY];
            adaptcoeff = buf[YADAPTCOEFFS];
            temp = predictor->XcoeffsB[0];
            predictionB = delay * temp;
            predictor->XcoeffsB[0] = temp + adaptcoeff;

            delay = buf[YDELAY-1];
            adaptcoeff = buf[YADAPTCOEFFS-1];
            temp = predictor->XcoeffsB[1];
            predictionB += delay * temp;
            predictor->XcoeffsB[1] = temp + adaptcoeff;

            delay = buf[YDELAY-2];
            adaptcoeff = buf[YADAPTCOEFFS-2];
            temp = predictor->XcoeffsB[2];
            predictionB += delay * temp;
            predictor->XcoeffsB[2] = temp + adaptcoeff;

            delay = buf[YDELAY-3];
            adaptcoeff = buf[YADAPTCOEFFS-3];
            temp = predictor->XcoeffsB[3];
            predictionB += delay * temp;
            predictor->XcoeffsB[3] = temp + adaptcoeff;

            delay = buf[YDELAY-4];
            adaptcoeff = buf[YADAPTCOEFFS-4];
            temp = predictor->XcoeffsB[4];
            predictionB += delay * temp;
            predictor->XcoeffsB[4] = temp + adaptcoeff;

            XlastA = *decoded1 + ((predictionA + (predictionB >> 1)) >> 10);
            XfilterA =  XlastA + ((XfilterA * 31) >> 5);

            *(decoded1++) = XfilterA;
            buf++;
            if (buf == predictor->historybuffer + PREDICTOR_HISTORY_SIZE) {MOVE_HISTORY_TO_HEAD}
            }
        else {
            predictionA = (buf[XDELAY] * predictor->XcoeffsA[0]) +
                          (buf[XDELAY-1] * predictor->XcoeffsA[1]) +
                          (buf[XDELAY-2] * predictor->XcoeffsA[2]) +
                          (buf[XDELAY-3] * predictor->XcoeffsA[3]);

            buf[YDELAY] = YlastA;
            buf[YADAPTCOEFFS] = SIGN(buf[YDELAY]);
            buf[YDELAY-1] = buf[YDELAY] - buf[YDELAY-1];
            buf[YADAPTCOEFFS-1] = SIGN(buf[YDELAY-1]);

            predictionB = (buf[YDELAY] * predictor->XcoeffsB[0]) +
                          (buf[YDELAY-1] * predictor->XcoeffsB[1]) +
                          (buf[YDELAY-2] * predictor->XcoeffsB[2]) +
                          (buf[YDELAY-3] * predictor->XcoeffsB[3]) +
                          (buf[YDELAY-4] * predictor->XcoeffsB[4]);

            XlastA = *decoded1 + ((predictionA + (predictionB >> 1)) >> 10);
            XfilterA =  XlastA + ((XfilterA * 31) >> 5);

            *(decoded1++) = XfilterA;
            buf++;
            if (buf == predictor->historybuffer + PREDICTOR_HISTORY_SIZE) {MOVE_HISTORY_TO_HEAD}
            }
        }

    predictor->buffer = buf;
    predictor->YlastA = YlastA;
    predictor->YfilterA = YfilterA;
    predictor->XlastA = XlastA;
    predictor->XfilterA = XfilterA;

}

STATIC_DECLARE void predictor_decode_mono(struct predictor_t* predictor,
                           int32_t* decoded0,
                           int count)
{
    int32_t predictionA, currentA, A;

    currentA = predictor->YlastA;

    while (count--) {
        A = *decoded0;

        predictor->buffer[YDELAYA] = currentA;
        predictor->buffer[YDELAYA-1] = predictor->buffer[YDELAYA] - predictor->buffer[YDELAYA-1];

        predictionA = (predictor->buffer[YDELAYA] * predictor->YcoeffsA[0]) +
                      (predictor->buffer[YDELAYA-1] * predictor->YcoeffsA[1]) +
                      (predictor->buffer[YDELAYA-2] * predictor->YcoeffsA[2]) +
                      (predictor->buffer[YDELAYA-3] * predictor->YcoeffsA[3]);

        currentA = A + (predictionA >> 10);

        predictor->buffer[YADAPTCOEFFSA] = SIGN(predictor->buffer[YDELAYA]);
        predictor->buffer[YADAPTCOEFFSA-1] = SIGN(predictor->buffer[YDELAYA-1]);

        if (A != 0) {
            if (A > 0) {
                predictor->YcoeffsA[0] -= predictor->buffer[YADAPTCOEFFSA];
                predictor->YcoeffsA[1] -= predictor->buffer[YADAPTCOEFFSA-1];
                predictor->YcoeffsA[2] -= predictor->buffer[YADAPTCOEFFSA-2];
                predictor->YcoeffsA[3] -= predictor->buffer[YADAPTCOEFFSA-3];
                }
            else {
                predictor->YcoeffsA[0] += predictor->buffer[YADAPTCOEFFSA];
                predictor->YcoeffsA[1] += predictor->buffer[YADAPTCOEFFSA-1];
                predictor->YcoeffsA[2] += predictor->buffer[YADAPTCOEFFSA-2];
                predictor->YcoeffsA[3] += predictor->buffer[YADAPTCOEFFSA-3];
                }
            }

        predictor->buffer++;

        /* Have we filled the history buffer? */
        if (predictor->buffer == predictor->historybuffer + PREDICTOR_HISTORY_SIZE) {
            memmove(predictor->historybuffer, predictor->buffer,
                    PREDICTOR_SIZE * sizeof(int32_t));
            predictor->buffer = predictor->historybuffer;
            }

        predictor->YfilterA =  currentA + ((predictor->YfilterA * 31) >> 5);
        *(decoded0++) = predictor->YfilterA;
        }

    predictor->YlastA = currentA;
}
