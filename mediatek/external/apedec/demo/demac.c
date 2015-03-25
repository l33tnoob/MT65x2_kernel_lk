#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wavwrite.h"
#include "parser.h"
#include "ape_decoder_exp.h"
#include "cue_parser.h"
#include "libprofiler.h"
#include "audip_exp.h"
IPCOMMON_PLUS

libprof_handle prof_entropy = 0;
libprof_handle prof_filter = 0;
libprof_handle prof_predictor = 0;
libprof_handle prof_interleave = 0;

static unsigned char *inbuffer;
static unsigned char *workbuffer;
static unsigned char *wavbuffer;
static unsigned int work_size, in_size, out_size;

int ape_decode(char* infile,
               char* outfile,
               FILE* prof_fp,
               libprof_handle prof_iter,
               int prof_count,
               int seek_millisecond)
{
    int bytesconsumed;
    struct ape_parser_ctx_t ape_parser_ctx;
    ape_decoder_handle ape_ctx;
    int n;
    int bytesinbuffer;
    int ret;
    unsigned int newframe = 0, filepos = 0, blocks_to_skip = 0, firstbyte = 0;

    FILE *fp = fopen(infile, "rb"), *fp_wav;
    if (fp == 0) { return -1; }

    if (ape_parseheader(fp,&ape_parser_ctx) < 0) {
        printf("Cannot read header\n");
        fclose(fp);
        return -1;
        }

    if ((ape_parser_ctx.fileversion < APE_MIN_VERSION) || (ape_parser_ctx.fileversion > APE_MAX_VERSION)) {
        printf("Unsupported file version - %.2f\n", ape_parser_ctx.fileversion/1000.0);
        fclose(fp);
        return -2;
        }

    ape_dumpinfo(&ape_parser_ctx);
    printf("Decoding file - v%.2f, compression level %d\n",
           ape_parser_ctx.fileversion/1000.0,ape_parser_ctx.compressiontype);

    if (seek_millisecond) {
        fp_wav = fopen(outfile, "wb");
        }
    else {
        fp_wav = open_wav(&ape_parser_ctx, outfile);
        }

    {
    struct ape_decoder_init_param ape_param;

    ape_decoder_get_mem_size(&in_size,
                             &work_size,
                             &out_size);

    inbuffer = (unsigned char*) malloc(in_size);
    workbuffer = (unsigned char*) malloc(work_size);
    wavbuffer = (unsigned char*) malloc(out_size);

    ape_param.blocksperframe = ape_parser_ctx.blocksperframe;
    ape_param.bps = ape_parser_ctx.bps;
    ape_param.channels = ape_parser_ctx.channels;
    ape_param.compressiontype = ape_parser_ctx.compressiontype;
    ape_param.fileversion = ape_parser_ctx.fileversion;
    ape_param.finalframeblocks = ape_parser_ctx.finalframeblocks;
    ape_param.totalframes = ape_parser_ctx.totalframes;

    ape_ctx = ape_decoder_init(workbuffer,
                               &ape_param);
    }

    fseek(fp, ape_parser_ctx.firstframe, SEEK_SET);
    bytesinbuffer = fread(inbuffer, 1, in_size, fp);

    while (1) {
        int bytesproduced = 0;

        if (seek_millisecond > 0) {
            if (ape_calc_seekpos_by_millisecond(&ape_parser_ctx,
                                                seek_millisecond,
                                                &newframe,
                                                &filepos,
                                                &firstbyte,
                                                &blocks_to_skip)) {
                printf("unknown seek position, skip once.\n");
                }
            else {
                fseek(fp, filepos, SEEK_SET);
                ape_decoder_reset(ape_ctx,
                                  firstbyte,
                                  newframe);
                bytesinbuffer = fread(inbuffer, 1, in_size, fp);
                }
            seek_millisecond = 0;
            }

        libprof_start_log(prof_iter);

        ret = ape_decoder_decode(ape_ctx,
                                 inbuffer,
                                 &bytesconsumed,
                                 wavbuffer,
                                 &bytesproduced);

        libprof_stop_log(prof_iter);
        libprof_dump_log(prof_iter, prof_fp);
        libprof_dump_log(prof_entropy, prof_fp);
        libprof_dump_log(prof_filter, prof_fp);
        libprof_dump_log(prof_predictor, prof_fp);
        libprof_dump_log(prof_interleave, prof_fp);

        if (ret == APE_ERR_EOS) {
            printf("decode complete.\n");
            ret = 0;
            break;
            }
        else if (ret) { break; }

        if (blocks_to_skip) {
            int blocks_produced = bytesproduced/(ape_parser_ctx.channels*(ape_parser_ctx.bps/8));
            if (blocks_to_skip >= blocks_produced) {
                blocks_to_skip -= blocks_produced;
                }
            else {
                fwrite(wavbuffer + blocks_to_skip, 1, blocks_produced - blocks_to_skip, fp_wav);
                blocks_to_skip = 0;
                }
            }
        else {
            fwrite(wavbuffer, 1, bytesproduced, fp_wav);
            }

        memmove(inbuffer,inbuffer + bytesconsumed, bytesinbuffer - bytesconsumed);
        bytesinbuffer -= bytesconsumed;
        n = fread(inbuffer + bytesinbuffer, 1, in_size - bytesinbuffer, fp);
        bytesinbuffer += n;

        if (--prof_count == 0) { break; }
        }

    fclose(fp);
    fclose(fp_wav);
    free(inbuffer);
    free(workbuffer);
    free(wavbuffer);

    return ret;
}

int main(int argc, char* argv[])
{
    int res;
    libprof_handle prof_iter = 0;
    FILE *prof_fp = 0;
    cue_sheet cue;
    int millisecond = 0;

    prof_fp = fopen("libdemac_prof.txt", "w");
    if (prof_fp == 0) { return -1; }

    libprof_init();
    prof_iter = libprof_get("ape decode iteration");
    prof_entropy = libprof_get("entropy");
    prof_filter = libprof_get("filter");
    prof_predictor = libprof_get("predictor");
    prof_interleave = libprof_get("interleave");

    if (argc < 2) {
        printf("ape_demo_pc.exe [input_file_name] [output_file_name]");
        return 0;
        }

    if (strstr(argv[1], ".cue")) {
        if (cue_sheet_init(argv[1], &cue)) {
            printf("cue file parse error\n");
            return -1;
            }
        cue_sheet_dump(&cue, stdout);
        return 0;
        }

    if (argc < 3) return 0;

    if (argc == 4) { millisecond = atoi(argv[3]); }

    res = ape_decode(argv[1],
                     argv[2],
                     prof_fp,
                     prof_iter,
                     0,
                     millisecond);

    if (res < 0) {
        fprintf(stderr,"DECODING ERROR %d, ABORTING\n", res);
        }
    else {
        fprintf(stderr,"DECODED OK - NO CRC ERRORS.\n");
        }

    return 0;
}
