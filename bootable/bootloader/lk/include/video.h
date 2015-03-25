/*
** MPC823 Video Controller
** =======================
** (C) 2000 by Paolo Scaffardi (arsenio@tin.it)
** AIRVENT SAM s.p.a - RIMINI(ITALY)
**
*/

#ifndef _VIDEO_H_
#define _VIDEO_H_

/* Video functions */
int drv_video_init(void);

int	video_init	(void *videobase);
void	video_putc	(const char c);
void	video_puts	(const char *s);
void	video_printf	(const char *fmt, ...);

//copy from mtk u-boot-1.1.6 /include/video.h
void video_clean_screen(void);
void video_set_cursor(int row, int col);
int  video_get_rows(void);
int  video_get_colums(void);

#endif
