#ifndef __CUST_AAL__
#define __CUST_AAL__

#ifdef __cplusplus
extern "C" {
#endif

#define CUST_ALS_LEVEL 12
#define CUST_LUMINANCE_LEVEL 33

struct CUST_AAL_ALS_DATA {
	float als[CUST_ALS_LEVEL];
    float lux[CUST_ALS_LEVEL];
};

struct CUST_AAL_LCM_DATA {
    float luminance[CUST_LUMINANCE_LEVEL];
};

struct CUST_AAL_PARAM {
    int brightnessLevel;
    int darkeningSpeedLevel;
    int brighteningSpeedLevel;
    int smartBacklightLevel;
    int toleranceRatioLevel;
    int readabilityLevel;
};


struct CUST_AAL_ALS_DATA *getALSCalData();
struct CUST_AAL_LCM_DATA *getLCMCalData();
struct CUST_AAL_PARAM *getAALParam();

#ifdef __cplusplus
}
#endif

#endif
