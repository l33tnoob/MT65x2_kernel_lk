#include <stdio.h>
#include <string.h>
#include "bandwidth_control.h"
#include "bandwidth_control_private.h"


#ifdef FLAG_SUPPORT_PROPERTY
//Property
#include <cutils/properties.h>
#else
#define PROPERTY_VALUE_MAX  (512)
#endif



#define MAX_PROP_NAME_CHAR  (512)


/*******************************************************************************
    Bandwidth Control Limitation Table
 *******************************************************************************/
#if 0
const static BWC_SETTING BWCLIMIT_VR_1066(  BWC_SIZE( 3072, 2044 ),     //_sensor_size   //6M
                                            BWC_SIZE( 1920, 1080 ),     //_vr_size      //1080P
                                            BWC_SIZE( 1280,  768 ),     //_disp_size    //WXGA
                                            BWC_SIZE(    0,    0 ),     //_tv_size      //no tv out
                                            30,                         //_fps          //30 fps
                                            BWCVT_MPEG4,                //_venc_codec_type//MPEG4
                                            BWCVT_NONE                  //_vdec_codec_type//no decode
                                            );

const static BWC_SETTING BWCLIMIT_VT_1066(  BWC_SIZE( 1920, 1080 ),     //_sensor_size   //FHD = 2M
                                            BWC_SIZE( 1280,  720 ),     //_vr_size      //720P decode/decode
                                            BWC_SIZE( 1280,  768 ),     //_disp_size    //WXGA
                                            BWC_SIZE( 1280,  768 ),     //_tv_size      //WXGA tv out
                                            30,                         //_fps          //30 fps
                                            BWCVT_H264,                 //_venc_codec_type//H.264
                                            BWCVT_H264                  //_vdec_codec_type//H.264
                                            );

#endif
static const char* BwcProfileType_GetStr( BWC_PROFILE_TYPE profile )
{
    switch( profile )
    {
    case BWCPT_VIDEO_NORMAL:        return "BWCPT_VIDEO_NORMAL";
    case BWCPT_VIDEO_RECORD_CAMERA: return "BWCPT_VIDEO_RECORD_CAMERA";
    case BWCPT_VIDEO_RECORD:        return "BWCPT_VIDEO_RECORD";
    case BWCPT_VIDEO_PLAYBACK:      return "BWCPT_VIDEO_PLAYBACK";
    case BWCPT_VIDEO_SNAPSHOT:      return "BWCPT_VIDEO_SNAPSHOT";
    case BWCPT_VIDEO_TELEPHONY:     return "BWCPT_VIDEO_TELEPHONY";
    case BWCPT_CAMERA_PREVIEW:      return "BWCPT_CAMERA_PREVIEW";
    case BWCPT_CAMERA_CAPTURE:      return "BWCPT_CAMERA_CAPTURE";
    case BWCPT_CAMERA_ZSD:          return "BWCPT_CAMERA_ZSD";
    case BWCPT_VIDEO_LIVE_PHOTO:    return "BWCPT_VIDEO_LIVE_PHOTO";
    case BWCPT_VIDEO_WIFI_DISPLAY:  return "BWCPT_VIDEO_WIFI_DISPLAY";
    case BWCPT_NONE:                return "BWCPT_NONE";
    }

    BWC_ERROR("Unknown profile:0x%08x\n", (unsigned int)profile); 
    return "BWCPT_UNKNOWN";
}

/*******************************************************************************
    Bandwidth Control Primitive Datatypes
 *******************************************************************************/
/*-----------------------------------------------------------------------------
    BWC_SIZE
  -----------------------------------------------------------------------------*/
void BWC_SIZE::LoadFromProperty( const char* property_name )
{
    char value[PROPERTY_VALUE_MAX];

    
    #ifdef FLAG_SUPPORT_PROPERTY
    if( property_get( property_name, value, NULL ) < 0 )
    {
        BWC_ERROR("[%s] Get Error!!\n", property_name);
    }
    #endif

    sscanf( value, "%ld %ld", &w, &h );

    BWC_INFO("[%s] Get : %s\n", property_name, value );
    
}

void BWC_SIZE::SetToProperty( const char* property_name ) const
{
    char value[PROPERTY_VALUE_MAX];
    
    sprintf( value, "%ld %ld", w, h );

    #ifdef FLAG_SUPPORT_PROPERTY
    if( property_set( property_name, value) < 0 )
    {
        BWC_ERROR("[%s] Set Error!!\n", property_name);
    }
    #endif

    BWC_INFO("[%s] Set : %s\n", property_name, value );
}

/*-----------------------------------------------------------------------------
    BWC_INT
  -----------------------------------------------------------------------------*/

void BWC_INT::LoadFromProperty( const char* property_name )
{
    char value_str[PROPERTY_VALUE_MAX];
    
    #ifdef FLAG_SUPPORT_PROPERTY
    if( property_get( property_name, value_str, NULL ) < 0 )
    {
        BWC_ERROR("[%s] Get Error!!\n", property_name);
    }
    #endif

    sscanf( value_str, "%d", &value );

    BWC_INFO("[%s] Get : %s\n", property_name, value_str );
    
}

void BWC_INT::SetToProperty( const char* property_name ) const
{
    char value_str[PROPERTY_VALUE_MAX];

    sprintf( value_str, "%d", value );
    
    #ifdef FLAG_SUPPORT_PROPERTY
    if( property_set( property_name, value_str) < 0 )
    {
        BWC_ERROR("[%s] Set Error!!\n", property_name);
    }
    #endif

    BWC_INFO("[%s] Set : %s\n", property_name, value_str );
}


/*-----------------------------------------------------------------------------
    BWC_SETTING
  -----------------------------------------------------------------------------*/
unsigned long BWC_SETTING::CalcThroughput_VR( void ) const
{
    unsigned long throughput = 0;

    const float bpp = 1.5;  //use yuv422 as limit calculation

    throughput  =   sensor_size.w * sensor_size.h * 2;  //sensor out + cdp in
    throughput +=   vr_size.w * vr_size.h * 2;          //cdp out + venc in
    throughput +=   disp_size.w * disp_size.h * 2;      //cdp out + disp in

    if( tv_size.w * tv_size.h )
    {
        throughput += tv_size.w * tv_size.h;    //TV in 
    }

    return (unsigned long)(throughput*fps*bpp);

}

unsigned long BWC_SETTING::CalcThroughput_VT( void ) const
{
    unsigned long throughput = 0;

    const float bpp = 1.5;  //use yuv422 as limit calculation

    throughput  =   sensor_size.w * sensor_size.h * 2;  //sensor out + cdp in
    throughput +=   vr_size.w * vr_size.h * 2;          //cdp out + venc in
    throughput +=   disp_size.w * disp_size.h * 2;      //cdp out + disp in

    throughput +=   vr_size.w * vr_size.h * 1;          //vdec out

    if( tv_size.w * tv_size.h )
    {
        throughput += tv_size.w * tv_size.h * 2;    //disp out + TV in 
    }

    return (unsigned long)(throughput*fps*bpp);

}


void BWC_SETTING::DumpInfo( void )
{
    #define _DUMP_SIZE( _field_name_ ) \
        BWC_INFO("%20s = %6ld x %6ld\n", #_field_name_, _field_name_.w , _field_name_.h );
    
    #define _DUMP_INT( _scalar_ ) \
        BWC_INFO("%20s = %6ld\n", #_scalar_, (long)_scalar_ );

    BWC_INFO("BWC_SETTING::DumpInfo-------\n\n");
    _DUMP_SIZE( sensor_size );
    _DUMP_SIZE( vr_size );
    _DUMP_SIZE( disp_size );
    _DUMP_SIZE( tv_size );

    _DUMP_INT(fps);
    _DUMP_INT(venc_codec_type);
    _DUMP_INT(vdec_codec_type);
    BWC_INFO("----------------------------\n\n");

    
    
}

/*******************************************************************************
    Bandwidth Control Change Profile
 *******************************************************************************/
/*-----------------------------------------------------------------------------
    BWC
  -----------------------------------------------------------------------------*/

int BWC::Profile_Change( BWC_PROFILE_TYPE profile_type , bool bOn )
{
    BWC_SETTING mmsetting;

    mmsetting.sensor_size       = SensorSize_Get();
    mmsetting.vr_size          = VideoRecordSize_Get();
    mmsetting.disp_size        = DisplaySize_Get();
    mmsetting.tv_size          = TvOutSize_Get();
    mmsetting.fps              = Fps_Get();
    mmsetting.venc_codec_type  = VideoEncodeCodec_Get();
    mmsetting.vdec_codec_type  = VideoDecodeCodec_Get();

    mmsetting.DumpInfo();

    /*Get DDR Type*/
    BWC_INFO("DDR Type = %d\n", emi_ddr_type_get() );

    /*Change SMI Setting*/
    smi_bw_ctrl_set( profile_type, mmsetting.venc_codec_type, bOn );

    /*Change EMI Setting*/
    emi_bw_ctrl_set( profile_type, mmsetting.venc_codec_type, bOn );

    if(bOn)
    {
        _Profile_Add( profile_type);
    }
    else
    {
        _Profile_Remove( profile_type);
    }

    BWC_INFO("Profile_Change:[%s]:%s,current concurrency is 0x%x\n", BwcProfileType_GetStr( profile_type ), bOn ? "ON" : "OFF" , _Profile_Get());

    return 0;
    
    
}

/*******************************************************************************
    Bandwidth Control Various Property
 *******************************************************************************/
void        BWC::SensorSize_Set( const BWC_SIZE &sensor_size )
{
    char prop_name[MAX_PROP_NAME_CHAR];
    
    property_name_str_get( __FUNCTION__ , prop_name );

    sensor_size.SetToProperty( prop_name );
}


BWC_SIZE    BWC::SensorSize_Get( void )
{
    BWC_SIZE    sensor_size;
        
    char prop_name[MAX_PROP_NAME_CHAR];
    
    property_name_str_get( __FUNCTION__ , prop_name );

    sensor_size.LoadFromProperty( prop_name );

    return sensor_size;
}


void        BWC::VideoRecordSize_Set( const BWC_SIZE &vr_size )
{
    char prop_name[MAX_PROP_NAME_CHAR];
    
    property_name_str_get( __FUNCTION__ , prop_name );

    vr_size.SetToProperty( prop_name );
}


BWC_SIZE    BWC::VideoRecordSize_Get( void )
{
    BWC_SIZE    vr_size;
        
    char prop_name[MAX_PROP_NAME_CHAR];
    
    property_name_str_get( __FUNCTION__ , prop_name );

    vr_size.LoadFromProperty( prop_name );

    return vr_size;
}


void        BWC::DisplaySize_Set( const BWC_SIZE &disp_size )
{
    char prop_name[MAX_PROP_NAME_CHAR];
    
    property_name_str_get( __FUNCTION__ , prop_name );

    disp_size.SetToProperty( prop_name );
}

BWC_SIZE    BWC::DisplaySize_Get( void )
{
    BWC_SIZE    disp_size;
        
    char prop_name[MAX_PROP_NAME_CHAR];
    
    property_name_str_get( __FUNCTION__ , prop_name );

    disp_size.LoadFromProperty( prop_name );

    return disp_size;
}


void        BWC::TvOutSize_Set( const BWC_SIZE &tv_size )
{
    char prop_name[MAX_PROP_NAME_CHAR];
    
    property_name_str_get( __FUNCTION__ , prop_name );

    tv_size.SetToProperty( prop_name );
}

BWC_SIZE    BWC::TvOutSize_Get( void )
{
    BWC_SIZE    tv_size;
        
    char prop_name[MAX_PROP_NAME_CHAR];
    
    property_name_str_get( __FUNCTION__ , prop_name );

    tv_size.LoadFromProperty( prop_name );

    return tv_size;
}


void        BWC::Fps_Set( int fps )
{
    BWC_INT fps_int( fps );
    char    prop_name[MAX_PROP_NAME_CHAR];

    property_name_str_get( __FUNCTION__ , prop_name );

    fps_int.SetToProperty( prop_name );
}

int         BWC::Fps_Get( void )
{
    BWC_INT fps_int;
        
    char prop_name[MAX_PROP_NAME_CHAR];
    
    property_name_str_get( __FUNCTION__ , prop_name );

    fps_int.LoadFromProperty( prop_name );

    return fps_int.value;
}


void            BWC::VideoEncodeCodec_Set( BWC_VCODEC_TYPE codec_type )
{
    BWC_INT codec( (int)codec_type );
    char    prop_name[MAX_PROP_NAME_CHAR];

    property_name_str_get( __FUNCTION__ , prop_name );

    codec.SetToProperty( prop_name );
}

BWC_VCODEC_TYPE BWC::VideoEncodeCodec_Get( void )
{
    BWC_INT codec;
        
    char prop_name[MAX_PROP_NAME_CHAR];
    
    property_name_str_get( __FUNCTION__ , prop_name );

    codec.LoadFromProperty( prop_name );

    return (BWC_VCODEC_TYPE)codec.value;
}
    

void            BWC::VideoDecodeCodec_Set( BWC_VCODEC_TYPE codec_type )
{
    BWC_INT codec( (int)codec_type );
    char    prop_name[MAX_PROP_NAME_CHAR];

    property_name_str_get( __FUNCTION__ , prop_name );

    codec.SetToProperty( prop_name );
}

BWC_VCODEC_TYPE BWC::VideoDecodeCodec_Get( void )
{
    BWC_INT codec;
        
    char prop_name[MAX_PROP_NAME_CHAR];
    
    property_name_str_get( __FUNCTION__ , prop_name );

    codec.LoadFromProperty( prop_name );

    return (BWC_VCODEC_TYPE)codec.value;
}

void BWC::_Profile_Set( BWC_PROFILE_TYPE profile)
{
//Null function
}

void BWC::_Profile_Add( BWC_PROFILE_TYPE profile)
{
    BWC_INT profile_value;

    char    prop_name[MAX_PROP_NAME_CHAR];

    property_name_str_get( __FUNCTION__ , prop_name );

    profile_value.LoadFromProperty( prop_name );

    profile_value.value = (profile_value.value | (1 << profile));

    profile_value.SetToProperty( prop_name );
}

void BWC::_Profile_Remove( BWC_PROFILE_TYPE profile)
{
    BWC_INT profile_value;

    char    prop_name[MAX_PROP_NAME_CHAR];

    property_name_str_get( __FUNCTION__ , prop_name );

    profile_value.LoadFromProperty( prop_name );

    profile_value.value = (profile_value.value & (~(1 << profile)));

    profile_value.SetToProperty( prop_name );
}

int BWC::_Profile_Get( void )
{
    BWC_INT profile;
        
    char prop_name[MAX_PROP_NAME_CHAR];
    
    property_name_str_get( __FUNCTION__ , prop_name );

    profile.LoadFromProperty( prop_name );

    return profile.value;
}



/*-----------------------------------------------------------------------------
    Auto generate property_name from given function name
  -----------------------------------------------------------------------------*/
int BWC::property_name_str_get( const char* function_name , char* prop_name )
{
    #define PROP_PREFIX_STR "bwc.mm."
    char     *p_str;
    char     src_str[MAX_PROP_NAME_CHAR - sizeof(PROP_PREFIX_STR)];
    char     dst_str[MAX_PROP_NAME_CHAR];

    strcpy( src_str, function_name );
    
    p_str = strrchr( src_str, '_' );

    if( p_str != NULL ){
        *p_str = '\0';
    }

    strcpy( dst_str , PROP_PREFIX_STR );
    strcat( dst_str , src_str );
    strcpy( prop_name, dst_str );

    return 0;
    
}


unsigned int BWC_MONITOR::query_hwc_max_pixel(){
    unsigned int hwc_max_pixel = -1;
    hwc_max_pixel = this->get_smi_bw_state();
    //BWC_INFO("query_hwc_max_pixel: get_smi_bw_state return %d\n", hwc_max_pixel );
    if( hwc_max_pixel <= 0 ){
        return 4389000;
    }else{
        return hwc_max_pixel;
    }
}

