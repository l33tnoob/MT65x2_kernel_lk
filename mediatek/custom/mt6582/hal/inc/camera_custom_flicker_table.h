#ifndef _CAMERA_CUSTOM_FLICKER_TABLE_H_
#define _CAMERA_CUSTOM_FLICKER_TABLE_H_

typedef struct
{
    MINT32* pPreviewTable1;
    MINT32* pPreviewTable2;
    MINT32  previewTableSize;
    MINT32* pVideoTable1;
    MINT32* pVideoTable2;
    MINT32  videoTableSize;
    MINT32* pCaptureTable1;
    MINT32* pCaptureTable2;
    MINT32  captureTableSize;
    MINT32* pZsdTable1;
    MINT32* pZsdTable2;
    MINT32  zsdTableSize;
} FlckerTable, *PFlckerTable;

#endif // _CAMERA_CUSTOM_FLICKER_TABLE_H_


