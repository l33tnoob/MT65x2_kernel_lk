#ifndef _OT_CONFIG_H
#define _OT_CONFIG_H

typedef struct
{
    int OBLoseTrackingFrm;
    int OCLoseTrackingFrm;
    float LtOcOb_ColorSimilarity_TH;
    float ARFA;
    int Numiter_shape_F;
    int LightResistance;
    int MaxObjHalfSize;
    int MinObjHalfSize;
    int IniwinW;
    int IniwinH;
    int AEAWB_LOCK;
}OT_Customize_PARA;


void get_ot_CustomizeData(OT_Customize_PARA  *OTDataOut);
	
#endif /* _OT_CONFIG_H */

