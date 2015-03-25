#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "cue_parser.h"

static int __scan_line(FILE *fp, char s[], int lim)
{
    char *t;
    int c = 0;

    t = s;
    while (--lim>1 && (c=getc(fp)) != EOF && c != '\n')
        *s++ = c;
    if (c == '\n')
        *s++ = c;
    else if (lim == 1) {
        *s++ = '\n';
        fprintf(stderr, "WARNING. __scan_line: Line too long, splitted.\n");
        }
    *s = '\0';
    return s - t;
}

static int __scan_token(char *line_buf_in, int *offset, char *token_buf_out)
{
    int start_pos = *offset;
    int advanced = 0;
    int token_start = 0;
    while (isspace(line_buf_in[start_pos + advanced]) &&
           line_buf_in[start_pos + advanced] != 0 ) { advanced++; }
    if (line_buf_in[start_pos + advanced] == 0) {
        return -1;//this line is completed and no token out
        }
    token_start = start_pos + advanced;
    if (line_buf_in[token_start] == '"') {
        char* temp = strstr(line_buf_in + token_start + 1, "\"");
        if (temp == 0) {
            return -2;//no secound ", this should be an error
            }
        memcpy(token_buf_out, line_buf_in + token_start + 1, (temp - line_buf_in) - token_start - 1);
        token_buf_out[(temp - line_buf_in) - token_start - 1] = 0;
        *offset = temp - line_buf_in + 1;
        return 0;
        }
    else {
        advanced = 0;
        while (!isspace(line_buf_in[token_start + advanced]) &&
               line_buf_in[token_start + advanced] != 0)
            advanced++;
        memcpy(token_buf_out, line_buf_in + token_start, advanced);
        token_buf_out[advanced] = 0;
        *offset = token_start + advanced;
        return 0;
        }
}

int cue_sheet_init(char *filename, cue_sheet *cue)
{
    char line[MAX_CHAR_PER_LINE];
    char token[MAX_CHAR_PER_LINE];
    int offset = 0;
    int ret = 0;
    FILE* fp = fopen(filename,"r");

    if (0 == fp) { return -1; }

    memset(cue, 0, sizeof(cue_sheet));

    while (1) {
        if (0 == __scan_line(fp, line, MAX_CHAR_PER_LINE)) {
            break;
            }
        offset = 0;
        while (!__scan_token(line, &offset, token)) {
            if (strcmp(token, "TRACK") == 0) {
                cue->total_track++;
                if (cue->total_track > MAX_TRACK_PER_SHEET) {
                    ret = -2;
                    goto err_out;
                    }
                }
            else if (strcmp(token, "FILE") == 0) {
                if (__scan_token(line, &offset, token)) {
                    ret = -3;
                    goto err_out;
                    }
                if (cue->total_track == 0) {
                    strcpy(cue->header.file, token);
                    }
                }
            else if (strcmp(token, "TITLE") == 0) {                
                if (__scan_token(line, &offset, token)) {
                    ret = -3;
                    goto err_out;
                    }
                if (cue->total_track == 0) {
                    strcpy(cue->header.title, token);
                    }
                else {
                    strcpy(cue->tracks[cue->total_track - 1].title, token);
                    }
                }
            else if (strcmp(token, "PERFORMER") == 0) {
                if (__scan_token(line, &offset, token)) {
                    ret = -3;
                    goto err_out;
                    }
                if (cue->total_track == 0) {
                    strcpy(cue->header.performer, token);
                    }
                else {
                    strcpy(cue->tracks[cue->total_track - 1].performer, token);
                    }
                }
            else if (strcmp(token, "INDEX") == 0) {
                if (__scan_token(line, &offset, token)) {
                    ret = -3;
                    goto err_out;
                    }
                if (strcmp(token, "00") == 0) {
                    char temp[4];
                    int pregap;
                    if (__scan_token(line, &offset, token)) {
                        ret = -3;
                        goto err_out;
                        }
                    temp[0] = token[0]; temp[1] = token[1]; temp[2] = 0;
                    pregap = atoi(temp);
                    temp[0] = token[3]; temp[1] = token[4]; temp[2] = 0;
                    pregap = atoi(temp) + pregap * 60;
                    cue->tracks[cue->total_track - 1].pregap = pregap;
                    }
                else if (strcmp(token, "01") == 0) {
                    char temp[4];
                    int start_from;
                    if (__scan_token(line, &offset, token)) {
                        ret = -3;
                        goto err_out;
                        }
                    temp[0] = token[0]; temp[1] = token[1]; temp[2] = 0;
                    start_from = atoi(temp);
                    temp[0] = token[3]; temp[1] = token[4]; temp[2] = 0;
                    start_from = atoi(temp) + start_from * 60;
                    cue->tracks[cue->total_track - 1].start_from = start_from;
                    }
                }
            }
        }
err_out:
    if (fp) { fclose(fp); }
    return ret;
}

void cue_sheet_dump(cue_sheet *cue, FILE *fp)
{
    int i = 0;
    if (cue == 0) { return; }
    printf("cue sheet tible: %s\n", cue->header.title);
    printf("cue sheet performer: %s\n", cue->header.performer);
    printf("cue sheet file: %s\n", cue->header.file);
    printf("cue sheet total track: %d\n", cue->total_track);

    for (; i < cue->total_track; i++) {
        printf("    track %d:\n", i + 1);
        printf("        title: %s\n", cue->tracks[i].title);
        printf("        performer: %s\n", cue->tracks[i].performer);
        printf("        pregap time:     %d\n", cue->tracks[i].pregap);
        printf("        start from time: %d\n", cue->tracks[i].start_from);
        }
}


