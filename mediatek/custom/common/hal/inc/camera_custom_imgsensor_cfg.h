#ifndef _CAMERA_CUSTOM_IMGSENSOR_CFG_
#define _CAMERA_CUSTOM_IMGSENSOR_CFG_
//
#include "camera_custom_types.h"
//
namespace NSCamCustomSensor
{
//
//
enum EDevId
{
    eDevId_ImgSensor0, //main sensor
    eDevId_ImgSensor1, //sub sensor
    eDevId_ImgSensor2, //main2 sensor (for 3D)    
};

enum EMclkId
{
	eMclk_1 = 0x0,	//mclk1
	eMclk_2 = 0x1,	//mclk2
	eMclk_3	= 0x2,	//mclk3
};

enum EMipiPort
{
    ePort_1 = 0x0, // 4 lane
    ePort_2 = 0x1, // 4 lane
    ePort_3 = 0x2, // 2 lane   
};

/*******************************************************************************
* Sensor Input Data Bit Order
*   Return:
*       0   : raw data input [9:2]
*       1   : raw data input [7:0]
*       -1  : error
*******************************************************************************/
MINT32  getSensorInputDataBitOrder(EDevId const eDevId);

/*******************************************************************************
* Sensor Pixel Clock Inverse in PAD side.
*   Return:
*       0   : no inverse
*       1   : inverse
*       -1  : error
*******************************************************************************/
MINT32  getSensorPadPclkInv(EDevId const eDevId);

/*******************************************************************************
* Sensor Placement Facing Direction
*   Return:
*       0   : Back side  
*       1   : Front side (LCD side)
*       -1  : error
*******************************************************************************/
MINT32  getSensorFacingDirection(EDevId const eDevId);
/*******************************************************************************
* Sensor layout using mclk
*   Return: EMclkId
*******************************************************************************/
MINT32  getSensorMclkConnection(EDevId const eDevId);

/*******************************************************************************
* MIPI sensor pad usage
*   Return: EMipiPort
*******************************************************************************/
MINT32  getMipiSensorPort(EDevId const eDevId);

/*******************************************************************************
* Image Sensor Orientation
*******************************************************************************/
typedef struct SensorOrientation_S
{
    MUINT32 u4Degree_0;     //  main sensor in degree (0, 90, 180, 270)
    MUINT32 u4Degree_1;     //  sub  sensor in degree (0, 90, 180, 270)
    MUINT32 u4Degree_2;     //  main2 sensor in degree (0, 90, 180, 270)
} SensorOrientation_T;

SensorOrientation_T const&  getSensorOrientation();

/*******************************************************************************
* Return fake orientation for front sensor in degree 0/180 or not
*******************************************************************************/
MBOOL isRetFakeSubOrientation();
/*******************************************************************************
* Return fake orientation for back sensor in degree 0/180 or not
*******************************************************************************/
MBOOL isRetFakeMainOrientation();
/*******************************************************************************
* Return fake orientation for back2 (3D)sensor in degree 0/180 or not
*******************************************************************************/
MBOOL isRetFakeMain2Orientation();


/*******************************************************************************
* Image Sensor Module FOV
*******************************************************************************/
typedef struct SensorViewAngle_S
{
    MUINT32 MainSensorHorFOV;     //  main sensor horizontal view angle, 0: not support
    MUINT32 MainSensorVerFOV;     //  main sensor vertical view angle, 0: not support
    MUINT32 SubSensorHorFOV;     //  sub sensor horizontal view angle, 0: not support
    MUINT32 SubSensorVerFOV;     //  sub sensor vertical view angle, 0: not support
    MUINT32 Main2SensorHorFOV;     //  main2 sensor horizontal view angle, 0: not support
    MUINT32 Main2SensorVerFOV;     //  main2 sensor vertical view angle, 0: not support
} SensorViewAngle_T;

SensorViewAngle_T const&  getSensorViewAngle();

};  //NSCamCustomSensor
#endif  //  _CAMERA_CUSTOM_IMGSENSOR_CFG_

