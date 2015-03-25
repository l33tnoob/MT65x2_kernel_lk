#ifndef __CAMERA_CUSTOM_FLICKER_H__
#define __CAMERA_CUSTOM_FLICKER_H__


void cust_getFlickerHalPara(int* defaultHz, int* maxDetExpUs); //default: 50 (50hz), 70000 (70ms)
namespace NSCamCustom
{
/*******************************************************************************
*
*******************************************************************************/
	typedef enum
	{
		eFLKSpeed_Slow =   0,
		eFLKSpeed_Normal,
		eFLKSpeed_Fast,

	}	eFlickerDetectSpeed;


};  //NSCamCustom


#endif //#ifndef __CAMERA_CUSTOM_FLICKER_H__



