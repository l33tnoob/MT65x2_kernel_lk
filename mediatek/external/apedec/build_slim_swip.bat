:: DEMO SOURCES
armcc --cpu ARM9EJ-S -Iinc -Isrc -Iimport -c  -O3 -Otime demo\cue_parser.c
armcc --cpu ARM9EJ-S -Iinc -Isrc -Iimport -c  -O3 -Otime demo\wavwrite.c
armcc --cpu ARM9EJ-S -Iinc -Isrc -Iimport -c  -O3 -Otime demo\demac.c
armcc --cpu ARM9EJ-S -Iinc -Isrc -Iimport -c  -O3 -Otime demo\libprofiler.c
armcc --cpu ARM9EJ-S -Iinc -Isrc -Iimport -c  -O3 -Otime demo\parser.c

:: LIBRARY SOURCES
armcc --cpu ARM9EJ-S --split_sections -Iinc -Isrc -Iimport -c  -O3 -Otime -DUP_TO_L3 -DSTATIC_DECLARE= -DEXTERN=extern src\predictor.c
armcc --cpu ARM9EJ-S --split_sections -Iinc -Isrc -Iimport -c  -O3 -Otime -DUP_TO_L3 -DSTATIC_DECLARE= -DEXTERN=extern src\filter_16_11.c
armcc --cpu ARM9EJ-S --split_sections -Iinc -Isrc -Iimport -c  -O3 -Otime -DUP_TO_L3 -DSTATIC_DECLARE= -DEXTERN=extern src\filter_32_10.c
armcc --cpu ARM9EJ-S --split_sections -Iinc -Isrc -Iimport -c  -O3 -Otime -DUP_TO_L3 -DSTATIC_DECLARE= -DEXTERN=extern src\filter_64_11.c
armcc --cpu ARM9EJ-S --split_sections -Iinc -Isrc -Iimport -c  -O3 -Otime -DUP_TO_L3 -DSTATIC_DECLARE= -DEXTERN=extern src\filter_256_13.c
armcc --cpu ARM9EJ-S --split_sections -Iinc -Isrc -Iimport -c  -O3 -Otime -DUP_TO_L3 -DSTATIC_DECLARE= -DEXTERN=extern src\filter_1280_15.c
armcc --cpu ARM9EJ-S --split_sections -Iinc -Isrc -Iimport -c  -O3 -Otime -DUP_TO_L3 -DSTATIC_DECLARE= -DEXTERN=extern src\entropy.c
armcc --cpu ARM9EJ-S --split_sections -Iinc -Isrc -Iimport -c  -O3 -Otime -DUP_TO_L3 -DSTATIC_DECLARE= -DEXTERN=extern src\crc.c
armcc --cpu ARM9EJ-S --split_sections -Iinc -Isrc -Iimport -c  -O3 -Otime -DUP_TO_L3 -DSTATIC_DECLARE= -DEXTERN=extern src\decoder.c
armcc --cpu ARM9EJ-S --split_sections -Iinc -Isrc -Iimport -c  -O3 -Otime -DUP_TO_L3 -DSTATIC_DECLARE= -DEXTERN=extern src\ape_decoder_swip.c

:: LIBRARY SOURCES (assembly)
armcc --cpu ARM9EJ-S --split_sections -c src\unaligned_dot_and_add_64.s
armcc --cpu ARM9EJ-S --split_sections -c src\unaligned_dot_and_sub_64.s
armcc --cpu ARM9EJ-S --split_sections -c src\dot_product_arm9m.s
armcc --cpu ARM9EJ-S --split_sections -c src\aligned_dot_and_add_64.s
armcc --cpu ARM9EJ-S --split_sections -c src\aligned_dot_and_sub_64.s

:: ARCHIVE TO LIBRARY
armar --create slim_ape_dec_max_L3.a predictor.o filter_16_11.o filter_32_10.o filter_64_11.o filter_256_13.o filter_1280_15.o entropy.o crc.o decoder.o ape_decoder_swip.o unaligned_dot_and_add_64.o unaligned_dot_and_sub_64.o dot_product_arm9m.o aligned_dot_and_add_64.o aligned_dot_and_sub_64.o

:: LINK AD DEMO IMAGE
armlink slim_ape_dec_max_L3.a cue_parser.o wavwrite.o demac.o libprofiler.o parser.o -o slim_ape_demo_max_L3.axf

:: DEMO SOURCES
armcc --cpu ARM9EJ-S -Iinc -Isrc -Iimport -c  -O3 -Otime demo\cue_parser.c
armcc --cpu ARM9EJ-S -Iinc -Isrc -Iimport -c  -O3 -Otime demo\wavwrite.c
armcc --cpu ARM9EJ-S -Iinc -Isrc -Iimport -c  -O3 -Otime demo\demac.c
armcc --cpu ARM9EJ-S -Iinc -Isrc -Iimport -c  -O3 -Otime demo\libprofiler.c
armcc --cpu ARM9EJ-S -Iinc -Isrc -Iimport -c  -O3 -Otime demo\parser.c

:: LIBRARY SOURCES
armcc --cpu ARM9EJ-S --split_sections -Iinc -Isrc -Iimport -c  -O3 -Otime -DUP_TO_L4 -DSTATIC_DECLARE= -DEXTERN=extern src\predictor.c
armcc --cpu ARM9EJ-S --split_sections -Iinc -Isrc -Iimport -c  -O3 -Otime -DUP_TO_L4 -DSTATIC_DECLARE= -DEXTERN=extern src\filter_16_11.c
armcc --cpu ARM9EJ-S --split_sections -Iinc -Isrc -Iimport -c  -O3 -Otime -DUP_TO_L4 -DSTATIC_DECLARE= -DEXTERN=extern src\filter_32_10.c
armcc --cpu ARM9EJ-S --split_sections -Iinc -Isrc -Iimport -c  -O3 -Otime -DUP_TO_L4 -DSTATIC_DECLARE= -DEXTERN=extern src\filter_64_11.c
armcc --cpu ARM9EJ-S --split_sections -Iinc -Isrc -Iimport -c  -O3 -Otime -DUP_TO_L4 -DSTATIC_DECLARE= -DEXTERN=extern src\filter_256_13.c
armcc --cpu ARM9EJ-S --split_sections -Iinc -Isrc -Iimport -c  -O3 -Otime -DUP_TO_L4 -DSTATIC_DECLARE= -DEXTERN=extern src\filter_1280_15.c
armcc --cpu ARM9EJ-S --split_sections -Iinc -Isrc -Iimport -c  -O3 -Otime -DUP_TO_L4 -DSTATIC_DECLARE= -DEXTERN=extern src\entropy.c
armcc --cpu ARM9EJ-S --split_sections -Iinc -Isrc -Iimport -c  -O3 -Otime -DUP_TO_L4 -DSTATIC_DECLARE= -DEXTERN=extern src\crc.c
armcc --cpu ARM9EJ-S --split_sections -Iinc -Isrc -Iimport -c  -O3 -Otime -DUP_TO_L4 -DSTATIC_DECLARE= -DEXTERN=extern src\decoder.c
armcc --cpu ARM9EJ-S --split_sections -Iinc -Isrc -Iimport -c  -O3 -Otime -DUP_TO_L4 -DSTATIC_DECLARE= -DEXTERN=extern src\ape_decoder_swip.c

:: LIBRARY SOURCES (assembly)
armcc --cpu ARM9EJ-S --split_sections -c src\unaligned_dot_and_add_64.s
armcc --cpu ARM9EJ-S --split_sections -c src\unaligned_dot_and_sub_64.s
armcc --cpu ARM9EJ-S --split_sections -c src\dot_product_arm9m.s
armcc --cpu ARM9EJ-S --split_sections -c src\aligned_dot_and_add_64.s
armcc --cpu ARM9EJ-S --split_sections -c src\aligned_dot_and_sub_64.s

:: ARCHIVE TO LIBRARY
armar --create slim_ape_dec_max_L4.a predictor.o filter_16_11.o filter_32_10.o filter_64_11.o filter_256_13.o filter_1280_15.o entropy.o crc.o decoder.o ape_decoder_swip.o unaligned_dot_and_add_64.o unaligned_dot_and_sub_64.o dot_product_arm9m.o aligned_dot_and_add_64.o aligned_dot_and_sub_64.o

:: LINK AD DEMO IMAGE
armlink slim_ape_dec_max_L4.a cue_parser.o wavwrite.o demac.o libprofiler.o parser.o -o slim_ape_demo_max_L4.axf

del *.o
