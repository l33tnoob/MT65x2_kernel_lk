#include "CAPEWrapper.h"
#include <binder/IMemory.h>
#include "OMX_Core.h"

#define LOG_TAG "CAPEWrapper"

CAPEWrapper::CAPEWrapper():apeHandle(NULL),working_BUF_size(0),in_size(0),out_size(0),pWorking_BUF(NULL),
                                                        pTempBuff(NULL),pTempBuffEnabled(false),bTempBuffFlag(false),Tempbuffersize(0),
                                                        mSourceRead(false),mNewInBufferRequired(true),mNewOutBufRequired(true),ptemp(NULL)
{
    memset(&mApeConfig, 0, sizeof(mApeConfig));
}

status_t CAPEWrapper::Create(const Parcel &para, Parcel *reply)
{
    ape_decoder_get_mem_size(&in_size, &working_BUF_size, &out_size);
    LOGD("create:in_size:%d,working_BUF_size:%d,out_size:%d",in_size,working_BUF_size,out_size);
    reply->writeInt32(in_size);
    reply->writeInt32(out_size);
#ifdef MTK_HIGH_RESOLUTION_AUDIO_SUPPORT
    ptemp = (unsigned char*)malloc(out_size*sizeof(char));
    memset(ptemp, 0, out_size);
#endif
    return OK;
}

status_t CAPEWrapper::Destroy(const Parcel &para)
{
    apeHandle = NULL;

    if (pTempBuff != NULL)
    {
        free(pTempBuff);
        pTempBuff = NULL;
    }

    if (pWorking_BUF != NULL)
    {
        free(pWorking_BUF);
        pWorking_BUF = NULL;
    }
    if (ptemp != NULL)
    {
        free(ptemp);
        ptemp = NULL;
    }
    return OK;
}

status_t CAPEWrapper::Init(const Parcel &para)
{
    ape_param.blocksperframe = para.readInt32();
    ape_param.bps = para.readInt32();
    ape_param.channels = para.readInt32();
    ape_param.compressiontype = para.readInt32();
    ape_param.fileversion = para.readInt32();
    ape_param.finalframeblocks = para.readInt32();
    ape_param.totalframes = para.readInt32();
    LOGD("Init:blocksperframe:%d,bps:%d,channels:%d,compressiontype:%d,fileversion:%d,finalframeblocks:%d,totalframes:%d",
        ape_param.blocksperframe,ape_param.bps,ape_param.channels,ape_param.compressiontype,
        ape_param.fileversion,ape_param.finalframeblocks,ape_param.totalframes);

    if (pTempBuff == NULL)
    {
        pTempBuff = (unsigned char *)malloc(in_size*sizeof(char));
    }
    if (pWorking_BUF == NULL)
    {
        pWorking_BUF = (unsigned char *)malloc(working_BUF_size*sizeof(char));
    }

    if (apeHandle == NULL)
    {
        apeHandle = ape_decoder_init(pWorking_BUF, &ape_param);
    }
    mNewInBufferRequired=mNewOutBufRequired=true;
    return OK;
}

status_t CAPEWrapper::DeInit(const Parcel &para)
{
    return OK;
}

status_t CAPEWrapper::DoCodec(const Parcel &para, Parcel *replay)
{
//param order: input buf offset, input flag, input buf, output buf
    //input buffer offset
    int inOffset = para.readInt32();
    //inputbuf flag
    int iflag = para.readInt32();
    sp<IMemory> inputmem = interface_cast<IMemory>(para.readStrongBinder());
    sp<IMemory> outputmem = interface_cast<IMemory>(para.readStrongBinder());
    ALOGD("inOffset:%d,iflag:%d,in:%p,out:%p",inOffset,iflag,inputmem.get(),outputmem.get());
    unsigned char * pinputbuf = (unsigned char *)inputmem->pointer();
    int inAlloclen = inputmem->size();
    unsigned char * poutputbuf = (unsigned char *)outputmem->pointer();
    int32_t consumeBS = APE_ERR_EOS;
    bool decoderEosErr = false;
    LOGV("Docodec+,inOffset:%d,iflag:%d,inAllocLen:%d",inOffset,iflag,inAlloclen);
//decode
    if (pTempBuffEnabled == false)
    {
        LOGV("tmpbuf enabled:F");
        if ((mSourceRead == true))
        {
            LOGD("buffer mSourceRead done in_offset %d, in_filllen %d", inOffset, inAlloclen);
            mApeConfig.pInputBuffer = (uint8_t *)pTempBuff;
            memset(pTempBuff, 0, in_size);    
        }
        else
        {
            LOGV("cfg.inputbuf=inpubuf+offset:%d",inOffset);    
            mApeConfig.pInputBuffer = (uint8_t *)pinputbuf + inOffset;    
        }
    }
    else
    {
        LOGV("tmpbuf enabled:T");
        mApeConfig.pInputBuffer = (uint8_t *)pTempBuff;
        if (bTempBuffFlag == true)
        {
            ALOGD("tembufflag:T,Tempbuffersize:%d,in_size:%d", Tempbuffersize, in_size);
            memcpy(pTempBuff + Tempbuffersize, pinputbuf, in_size - Tempbuffersize);
            bTempBuffFlag = false;
        }
    }
    mApeConfig.pOutputBuffer = poutputbuf;

#ifdef MTK_HIGH_RESOLUTION_AUDIO_SUPPORT
        if (ape_param.bps == 24)
        {
            consumeBS = ape_decoder_decode(apeHandle, mApeConfig.pInputBuffer, (int *)&mApeConfig.inputBufferUsedLength,
                               ptemp, (int *)&mApeConfig.outputFrameSize);
            int i = 0, j = 0;
            int  temp = 0;

            for (j = 0; j < mApeConfig.outputFrameSize; j+=3)
            {
                temp = ptemp[j+2];
                temp <<= 8;
                temp |= ptemp[j+1];
                temp <<= 8;                
                temp |= ptemp[j];
                temp <<= 8;
                *((int*)mApeConfig.pOutputBuffer+i) = temp>>8;
                i++;
            }
        }
        else
            consumeBS = ape_decoder_decode(apeHandle, mApeConfig.pInputBuffer, (int *)&mApeConfig.inputBufferUsedLength,
                               mApeConfig.pOutputBuffer, (int *)&mApeConfig.outputFrameSize);
#else
    consumeBS = ape_decoder_decode(apeHandle, mApeConfig.pInputBuffer, (int *)&mApeConfig.inputBufferUsedLength,
                                   mApeConfig.pOutputBuffer, (int *)&mApeConfig.outputFrameSize);
#endif

    LOGV("decode: pTempBuffEnabled %d, consumeBS %d,in_used %d out_len %d",
           pTempBuffEnabled, consumeBS, mApeConfig.inputBufferUsedLength, mApeConfig.outputFrameSize);
    
//consumeBS should be returned to the MtkOmxApeDec component...>>>
    if ((consumeBS == APE_ERR_CRC))
    {
        LOGD("APEDEC_INVALID_Frame CRC ERROR code %d", consumeBS);                    
        mApeConfig.outputFrameSize = 0;
        mNewInBufferRequired = OMX_TRUE;
        memset(mApeConfig.pOutputBuffer, 0, mApeConfig.outputFrameSize);
        mApeConfig.inputBufferUsedLength = 0;    
    }
    else if (consumeBS == APE_ERR_EOS)
    {
        mSourceRead = false;
        mNewInBufferRequired = OMX_FALSE;
        mNewOutBufRequired = OMX_FALSE;
        LOGV("Decode Frame ERROR EOS");
    }
    else
    {
        mNewOutBufRequired = OMX_TRUE;
        if (pTempBuffEnabled == true)
        {
            LOGV("tmpbuf true");
            mNewInBufferRequired = OMX_FALSE;
    //tempbuffer was decoded to end;why inputBufferUsedLength > Tempbuffersize?
            if (mApeConfig.inputBufferUsedLength >= Tempbuffersize) ///&& (mSourceRead == false))
            {
                LOGV("tmpbuf true:1,in used len:%d>temsize:%d",mApeConfig.inputBufferUsedLength,Tempbuffersize);
                mApeConfig.inputBufferUsedLength -= Tempbuffersize;
                pTempBuffEnabled = false;
                Tempbuffersize = 0;
            }
    //tempbuffer suplus
            else
            {
                LOGV("tmpbuf true:2");
                Tempbuffersize -= mApeConfig.inputBufferUsedLength;
                memmove(pTempBuff, pTempBuff + mApeConfig.inputBufferUsedLength, Tempbuffersize);
                if (mSourceRead == true)
                    memset(pTempBuff + Tempbuffersize, 0, in_size - Tempbuffersize);
                else
                    memcpy(pTempBuff + Tempbuffersize, pinputbuf + inOffset, in_size - Tempbuffersize);
                mApeConfig.inputBufferUsedLength = 0;
            }
        }
        else
        {
         //input buffer not docodec end,copy suplus data in input buffer to temp buffer
            LOGV("tempbuf false:alloclen:%d,offset:%d,inUsedLen:%d",inAlloclen, inOffset, mApeConfig.inputBufferUsedLength);
            if ((inAlloclen - inOffset - mApeConfig.inputBufferUsedLength) <= in_size)
            {
                LOGV("tempbuf false1");
                pTempBuffEnabled = true;
                bTempBuffFlag = true;
                memset(pTempBuff, 0, in_size);
                
                if ((iflag & OMX_BUFFERFLAG_EOS))
                {
                    mNewInBufferRequired = false;
                    mSourceRead = true;
                }
                else
                {
                    mNewInBufferRequired = true;
                }

                Tempbuffersize = inAlloclen - inOffset - mApeConfig.inputBufferUsedLength;
                memcpy(pTempBuff, (uint8_t *)(pinputbuf + inOffset + mApeConfig.inputBufferUsedLength), Tempbuffersize);
            }
            else
            {
                LOGV("tembuf false:2");
                mNewInBufferRequired = false;
            }
        }
    }

//output param order: consumeBS, outputFrameSize, inputBufferUsedLength, mNewInputBufferRequired, mNewOutBufRequired
    replay->writeInt32(consumeBS);
#ifdef MTK_HIGH_RESOLUTION_AUDIO_SUPPORT
            if (ape_param.bps == 24)
                mApeConfig.outputFrameSize = mApeConfig.outputFrameSize/3*4;
#endif
    replay->writeInt32(mApeConfig.outputFrameSize);
    replay->writeInt32(mApeConfig.inputBufferUsedLength);
    replay->writeInt32(mNewInBufferRequired);
    replay->writeInt32(mNewOutBufRequired);
    LOGV("Docodec-,consumeBS:%d,outputFrameSize:%d,inputBufferUsedLength:%d,mNewInBufferRequired:%d,mNewOutBufRequired:%d",
        consumeBS,mApeConfig.outputFrameSize,mApeConfig.inputBufferUsedLength,mNewInBufferRequired,mNewOutBufRequired);
    return OK;
}

status_t CAPEWrapper::Reset(const Parcel &para)
{
    LOGD("reset+");
    int seekbyte = para.readInt32();
    int newframe = para.readInt32();
    ape_decoder_reset(apeHandle, seekbyte, newframe);
    pTempBuffEnabled = false;
    bTempBuffFlag = false;
    Tempbuffersize = 0;
    if (ptemp != NULL)
        memset(ptemp, 0, out_size);
    LOGD("reset-");
    return OK;
}

