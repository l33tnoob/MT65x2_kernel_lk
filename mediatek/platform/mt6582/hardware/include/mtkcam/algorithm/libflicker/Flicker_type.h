#ifndef _FLICKER_TYPE_H_
#define _FLICKER_TYPE_H_

#define HAL_FLICKER_AUTO_50HZ    50
#define HAL_FLICKER_AUTO_60HZ    60
#define HAL_FLICKER_AUTO_OFF     0

typedef struct flkEISVector
{
	MINT32 vx[16];
	MINT32 vy[16];
} flkEISVector;

typedef struct flkSensorInfo
{
	MUINT32 pixelClock;
	MUINT32 fullLineWidth;
} flkSensorInfo;

typedef struct flkAEInfo
{
	MUINT32 previewShutterValue;
} flkAEInfo;

typedef enum FLICKER_STATUS_t {INCONCLUSIVE, FK000, FK100, FK120} FLICKER_STATUS;           // inconclusive, no flicker, 100 Hz, 120 Hz

#endif
