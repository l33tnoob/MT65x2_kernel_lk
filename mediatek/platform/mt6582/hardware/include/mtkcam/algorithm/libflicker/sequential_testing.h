#ifndef __SEQUENTIAL_TESTING__
#define __SEQUENTIAL_TESTING__

#define LIKELIHOOD_BUFFER_DEPTH 150
typedef struct {
	MINT32	m;
	MINT32	b_l;
	MINT32	b_r;
	MINT32	offset;
} FLICKER_STATISTICS;

typedef enum { Hz50 = 0 , Hz60 = 1 } EV_TABLE;

//#define MIN_PAST_FRAMES 3
//#define MAX_PAST_FRAMES 14

typedef struct {
	MINT32 vals[LIKELIHOOD_BUFFER_DEPTH];
	MINT32 front;
	MINT32 count;
} LL_BUFFER;



FLICKER_STATUS test_next_frame(const MINT32 * frame_data, MUINT32 dim, MBOOL is_valid, MUINT32 n_history);

void reset_flicker_queue();

void set_flicker_state(EV_TABLE);

MINT32 get_slrt_score(MUINT32 history_length);

void set_flicker_index(MINT32* peak_index);

MINT32 get_current_llr(const MINT32 * frame_data, MUINT32 dim);

#endif
