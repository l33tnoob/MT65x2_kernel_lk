#ifndef APE_DECODER_EXP_H
#define APE_DECODER_EXP_H

#ifdef __cplusplus
extern "C" {
#endif

#define APE_ERR_EOS -1
#define APE_ERR_CRC -2

typedef void* ape_decoder_handle;

/*----------------------------------------------------------------------*/
// FUNCTION
//  ape_decoder_GetVersion
//
// DESCRIPTION
//  This function was used to get current version of library
//
// RETURNS
//  B31-B24:  Project Type
//  B23-B16:  Compiler and Major Version
//  B15-B08:  Minor Version
//  B07-B00:  Release Version
//
/*----------------------------------------------------------------------*/
int ape_decoder_get_version(void);

struct ape_decoder_init_param {
    short           fileversion;
    unsigned short  compressiontype;
    unsigned int    blocksperframe;
    unsigned int    finalframeblocks;
    unsigned int    totalframes;
    unsigned short  bps;
    unsigned short  channels;
};

void
ape_decoder_get_mem_size(unsigned int *bs_buffer,
                         unsigned int *working_buffer,
                         unsigned int *pcm_buffer);

ape_decoder_handle
ape_decoder_init(void*  working_buffer,
                 struct ape_decoder_init_param* ape_param);

int
ape_decoder_reset(ape_decoder_handle handle,
                  int firstbyte,
                  int newframe);

int
ape_decoder_decode(ape_decoder_handle handle,
                   unsigned char* inbuffer,
                   int* bytes_consumed,
                   unsigned char* outbuffer,
                   int* bytes_produced);

#ifdef __cplusplus
}
#endif

#endif // APE_DECODER_EXP_H
