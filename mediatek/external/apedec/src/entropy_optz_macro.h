#ifndef ENTROPY_OPTZ_MACRO_H
#define ENTROPY_OPTZ_MACRO_H

#define RANGE_CODER_NORMALIZE\
    {\
    int guard = 5;\
    while (rc_range <= BOTTOM_VALUE) {\
        rc_buffer = (rc_buffer << 8) | read_byte(ape_dec_internal);\
        rc_low = (rc_low << 8) | ((rc_buffer >> 1) & 0xff);\
        rc_range <<= 8;\
        if (--guard == 0) { ape_dec_internal->err_flag = -1; return; }\
        }\
    }
    
#define RANGE_CODER_NORMALIZE_3970\
    {\
    int guard = 5;\
    while (rc_range <= BOTTOM_VALUE) {\
        rc_buffer = (rc_buffer << 8) | read_byte(ape_dec_internal);\
        rc_low = (rc_low << 8) | ((rc_buffer >> 1) & 0xff);\
        rc_range <<= 8;\
        if (--guard == 0) { ape_dec_internal->err_flag = -1; return 0; }\
        }\
    }

#define RANGE_CODER_UPDATE(a,b)\
    rc_low -= rc_help * (a);\
    rc_range = rc_help * (b);

#define RANGE_CODER_SEARCH_SYMBOL_AND_UPDATE(ret)\
{\
    rc_help = rc_range >> 16;\
    if (rc_low < counts_3980[1]*rc_help) { ret = 0; RANGE_CODER_UPDATE(counts_3980[0], counts_diff_3980[0]) }\
    else if (rc_low < counts_3980[2]*rc_help) { ret = 1; RANGE_CODER_UPDATE(counts_3980[1], counts_diff_3980[1]) }\
    else if (rc_low < counts_3980[3]*rc_help) { ret = 2; RANGE_CODER_UPDATE(counts_3980[2], counts_diff_3980[2]) }\
    else if (rc_low < counts_3980[4]*rc_help) { ret = 3; RANGE_CODER_UPDATE(counts_3980[3], counts_diff_3980[3]) }\
    else if (rc_low < counts_3980[5]*rc_help) { ret = 4; RANGE_CODER_UPDATE(counts_3980[4], counts_diff_3980[4]) }\
    else if (rc_low < counts_3980[6]*rc_help) { ret = 5; RANGE_CODER_UPDATE(counts_3980[5], counts_diff_3980[5]) }\
    else if (rc_low < counts_3980[7]*rc_help) { ret = 6; RANGE_CODER_UPDATE(counts_3980[6], counts_diff_3980[6]) }\
    else if (rc_low < counts_3980[8]*rc_help) { ret = 7; RANGE_CODER_UPDATE(counts_3980[7], counts_diff_3980[7]) }\
    else if (rc_low < counts_3980[9]*rc_help) { ret = 8; RANGE_CODER_UPDATE(counts_3980[8], counts_diff_3980[8]) }\
    else if (rc_low < counts_3980[10]*rc_help) { ret = 9; RANGE_CODER_UPDATE(counts_3980[9], counts_diff_3980[9]) }\
    else if (rc_low < counts_3980[11]*rc_help) { ret = 10; RANGE_CODER_UPDATE(counts_3980[10], counts_diff_3980[10]) }\
    else if (rc_low < counts_3980[12]*rc_help) { ret = 11; RANGE_CODER_UPDATE(counts_3980[11], counts_diff_3980[11]) }\
    else if (rc_low < counts_3980[13]*rc_help) { ret = 12; RANGE_CODER_UPDATE(counts_3980[12], counts_diff_3980[12]) }\
    else if (rc_low < counts_3980[14]*rc_help) { ret = 13; RANGE_CODER_UPDATE(counts_3980[13], counts_diff_3980[13]) }\
    else if (rc_low < counts_3980[15]*rc_help) { ret = 14; RANGE_CODER_UPDATE(counts_3980[14], counts_diff_3980[14]) }\
    else if (rc_low < counts_3980[16]*rc_help) { ret = 15; RANGE_CODER_UPDATE(counts_3980[15], counts_diff_3980[15]) }\
    else if (rc_low < counts_3980[17]*rc_help) { ret = 16; RANGE_CODER_UPDATE(counts_3980[16], counts_diff_3980[16]) }\
    else if (rc_low < counts_3980[18]*rc_help) { ret = 17; RANGE_CODER_UPDATE(counts_3980[17], counts_diff_3980[17]) }\
    else {\
        ret = UDIV32(rc_low, rc_help) - counts_3980[18] + 18; \
        if (ret <= (MODEL_ELEMENTS-1)) { RANGE_CODER_UPDATE(counts_3980[ret], counts_diff_3980[ret]) }\
        else { ape_dec_internal->err_flag = -1; return ; }\
        }\
}

#define RANGE_CODER_SEARCH_SYMBOL_AND_UPDATE_3970(ret)\
{\
    rc_help = rc_range >> 16;\
    if (rc_low < counts_3970[1]*rc_help) { ret = 0; RANGE_CODER_UPDATE(counts_3970[0], counts_diff_3970[0]) }\
    else if (rc_low < counts_3970[2]*rc_help) { ret = 1; RANGE_CODER_UPDATE(counts_3970[1], counts_diff_3970[1]) }\
    else if (rc_low < counts_3970[3]*rc_help) { ret = 2; RANGE_CODER_UPDATE(counts_3970[2], counts_diff_3970[2]) }\
    else if (rc_low < counts_3970[4]*rc_help) { ret = 3; RANGE_CODER_UPDATE(counts_3970[3], counts_diff_3970[3]) }\
    else if (rc_low < counts_3970[5]*rc_help) { ret = 4; RANGE_CODER_UPDATE(counts_3970[4], counts_diff_3970[4]) }\
    else if (rc_low < counts_3970[6]*rc_help) { ret = 5; RANGE_CODER_UPDATE(counts_3970[5], counts_diff_3970[5]) }\
    else if (rc_low < counts_3970[7]*rc_help) { ret = 6; RANGE_CODER_UPDATE(counts_3970[6], counts_diff_3970[6]) }\
    else if (rc_low < counts_3970[8]*rc_help) { ret = 7; RANGE_CODER_UPDATE(counts_3970[7], counts_diff_3970[7]) }\
    else if (rc_low < counts_3970[9]*rc_help) { ret = 8; RANGE_CODER_UPDATE(counts_3970[8], counts_diff_3970[8]) }\
    else if (rc_low < counts_3970[10]*rc_help) { ret = 9; RANGE_CODER_UPDATE(counts_3970[9], counts_diff_3970[9]) }\
    else if (rc_low < counts_3970[11]*rc_help) { ret = 10; RANGE_CODER_UPDATE(counts_3970[10], counts_diff_3970[10]) }\
    else if (rc_low < counts_3970[12]*rc_help) { ret = 11; RANGE_CODER_UPDATE(counts_3970[11], counts_diff_3970[11]) }\
    else if (rc_low < counts_3970[13]*rc_help) { ret = 12; RANGE_CODER_UPDATE(counts_3970[12], counts_diff_3970[12]) }\
    else if (rc_low < counts_3970[14]*rc_help) { ret = 13; RANGE_CODER_UPDATE(counts_3970[13], counts_diff_3970[13]) }\
    else if (rc_low < counts_3970[15]*rc_help) { ret = 14; RANGE_CODER_UPDATE(counts_3970[14], counts_diff_3970[14]) }\
    else if (rc_low < counts_3970[16]*rc_help) { ret = 15; RANGE_CODER_UPDATE(counts_3970[15], counts_diff_3970[15]) }\
    else if (rc_low < counts_3970[17]*rc_help) { ret = 16; RANGE_CODER_UPDATE(counts_3970[16], counts_diff_3970[16]) }\
    else if (rc_low < counts_3970[18]*rc_help) { ret = 17; RANGE_CODER_UPDATE(counts_3970[17], counts_diff_3970[17]) }\
    else if (rc_low < counts_3970[19]*rc_help) { ret = 18; RANGE_CODER_UPDATE(counts_3970[18], counts_diff_3970[18]) }\
    else if (rc_low < counts_3970[20]*rc_help) { ret = 19; RANGE_CODER_UPDATE(counts_3970[19], counts_diff_3970[19]) }\
    else if (rc_low < counts_3970[21]*rc_help) { ret = 20; RANGE_CODER_UPDATE(counts_3970[20], counts_diff_3970[20]) }\
    else {\
        ret = UDIV32(rc_low, rc_help) - counts_3970[21] + 21; \
        if (ret <= (MODEL_ELEMENTS-1)) { RANGE_CODER_UPDATE(counts_3970[ret], counts_diff_3970[ret]) }\
        else { ape_dec_internal->err_flag = -1; return -1; }\
        }\
}

#define DIV_LOOKUP(x,p,q)\
{\
    switch (p) {\
        case 1: q = x; break;\
        case 2: q = x / 2; break;\
        case 3: q = x / 3; break;\
        case 4: q = x / 4; break;\
        case 5: q = x / 5; break;\
        case 6: q = x / 6; break;\
        case 7: q = x / 7; break;\
        case 8: q = x / 8; break;\
        case 9: q = x / 9; break;\
        case 10: q = x / 10; break;\
        case 11: q = x / 11; break;\
        case 12: q = x / 12; break;\
        case 13: q = x / 13; break;\
        case 14: q = x / 14; break;\
        case 15: q = x / 15; break;\
        case 16: q = x / 16; break;\
        case 17: q = x / 17; break;\
        case 18: q = x / 18; break;\
        case 19: q = x / 19; break;\
        case 20: q = x / 20; break;\
        case 21: q = x / 21; break;\
        case 22: q = x / 22; break;\
        case 23: q = x / 23; break;\
        case 24: q = x / 24; break;\
        case 25: q = x / 25; break;\
        case 26: q = x / 26; break;\
        case 27: q = x / 27; break;\
        case 28: q = x / 28; break;\
        case 29: q = x / 29; break;\
        case 30: q = x / 30; break;\
        case 31: q = x / 31; break;\
        case 32: q = x / 32; break;\
        case 33: q = x / 33; break;\
        case 34: q = x / 34; break;\
        case 35: q = x / 35; break;\
        case 36: q = x / 36; break;\
        case 37: q = x / 37; break;\
        case 38: q = x / 38; break;\
        case 39: q = x / 39; break;\
        case 40: q = x / 40; break;\
        case 41: q = x / 41; break;\
        case 42: q = x / 42; break;\
        case 43: q = x / 43; break;\
        case 44: q = x / 44; break;\
        case 45: q = x / 45; break;\
        case 46: q = x / 46; break;\
        case 47: q = x / 47; break;\
        case 48: q = x / 48; break;\
        case 49: q = x / 49; break;\
        case 50: q = x / 50; break;\
        case 51: q = x / 51; break;\
        case 52: q = x / 52; break;\
        case 53: q = x / 53; break;\
        case 54: q = x / 54; break;\
        case 55: q = x / 55; break;\
        case 56: q = x / 56; break;\
        case 57: q = x / 57; break;\
        case 58: q = x / 58; break;\
        case 59: q = x / 59; break;\
        case 60: q = x / 60; break;\
        case 61: q = x / 61; break;\
        case 62: q = x / 62; break;\
        case 63: q = x / 63; break;\
        case 64: q = x / 64; break;\
        case 65: q = x / 65; break;\
        case 66: q = x / 66; break;\
        case 67: q = x / 67; break;\
        case 68: q = x / 68; break;\
        case 69: q = x / 69; break;\
        case 70: q = x / 70; break;\
        case 71: q = x / 71; break;\
        case 72: q = x / 72; break;\
        case 73: q = x / 73; break;\
        case 74: q = x / 74; break;\
        case 75: q = x / 75; break;\
        case 76: q = x / 76; break;\
        case 77: q = x / 77; break;\
        case 78: q = x / 78; break;\
        case 79: q = x / 79; break;\
        case 80: q = x / 80; break;\
        case 81: q = x / 81; break;\
        case 82: q = x / 82; break;\
        case 83: q = x / 83; break;\
        case 84: q = x / 84; break;\
        case 85: q = x / 85; break;\
        case 86: q = x / 86; break;\
        case 87: q = x / 87; break;\
        case 88: q = x / 88; break;\
        case 89: q = x / 89; break;\
        case 90: q = x / 90; break;\
        case 91: q = x / 91; break;\
        case 92: q = x / 92; break;\
        case 93: q = x / 93; break;\
        case 94: q = x / 94; break;\
        case 95: q = x / 95; break;\
        case 96: q = x / 96; break;\
        case 97: q = x / 97; break;\
        case 98: q = x / 98; break;\
        case 99: q = x / 99; break;\
        case 100: q = x / 100; break;\
        case 101: q = x / 101; break;\
        case 102: q = x / 102; break;\
        case 103: q = x / 103; break;\
        case 104: q = x / 104; break;\
        case 105: q = x / 105; break;\
        case 106: q = x / 106; break;\
        case 107: q = x / 107; break;\
        case 108: q = x / 108; break;\
        case 109: q = x / 109; break;\
        case 110: q = x / 110; break;\
        case 111: q = x / 111; break;\
        case 112: q = x / 112; break;\
        case 113: q = x / 113; break;\
        case 114: q = x / 114; break;\
        case 115: q = x / 115; break;\
        case 116: q = x / 116; break;\
        case 117: q = x / 117; break;\
        case 118: q = x / 118; break;\
        case 119: q = x / 119; break;\
        case 120: q = x / 120; break;\
        case 121: q = x / 121; break;\
        case 122: q = x / 122; break;\
        case 123: q = x / 123; break;\
        case 124: q = x / 124; break;\
        case 125: q = x / 125; break;\
        case 126: q = x / 126; break;\
        case 127: q = x / 127; break;\
        case 128: q = x / 128; break;\
        default: q = UDIV32(x,p);\
        }\
}

#endif // ENTROPY_OPTZ_MACRO_H
