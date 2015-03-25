#ifndef CUE_PARSER_H
#define CUE_PARSER_H

#define MAX_CHAR_PER_LINE 128
#define MAX_TRACK_PER_SHEET 32

typedef struct _cue_header {
    char title[MAX_CHAR_PER_LINE];
    char performer[MAX_CHAR_PER_LINE];
    char file[MAX_CHAR_PER_LINE];
} cue_header;

typedef struct _cue_track {
    char title[MAX_CHAR_PER_LINE];
    char performer[MAX_CHAR_PER_LINE];
    int start_from;
    int pregap;
} cue_track;

typedef struct _cue_sheet {
    cue_header header;
    int total_track;
    cue_track tracks[MAX_TRACK_PER_SHEET];
} cue_sheet;

int cue_sheet_init(char *filename,
                   cue_sheet *cue);

void cue_sheet_dump(cue_sheet *cue,
                    FILE *fp);

#endif // CUE_PARSER_H
