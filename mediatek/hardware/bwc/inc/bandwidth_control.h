#ifndef __BANDWIDTH_CONTROL_H__
#define __BANDWIDTH_CONTROL_H__


/*-----------------------------------------------------------------------------
    BWC Primitive Data Type : Size
    (Avoid having virtual table in structure, try not to use inheritance)
  -----------------------------------------------------------------------------*/
class BWC_SIZE
{
public:
    long w;
    long h;

public:
    BWC_SIZE():
        w(0),h(0)
        {};
   
    BWC_SIZE( long _w, long _h )
        {
            w = _w; h = _h;
        };

    
    BWC_SIZE& operator=( const BWC_SIZE& rhs )
    {
        w = rhs.w; h = rhs.h;
        return *this;
    }

    bool operator==(const BWC_SIZE& rhs) const
    {
        return (  ( w == rhs.w ) && ( h == rhs.h ) );
    }

    bool operator!=(const BWC_SIZE& rhs) const
    {
        return !( *this == rhs );
    }

    void LoadFromProperty( const char* property_name );
    void SetToProperty( const char* property_name ) const;

};

/*-----------------------------------------------------------------------------
    BWC Primitive Data Type : Integer
  -----------------------------------------------------------------------------*/
class BWC_INT
{
public:
    int value;

public:
    BWC_INT():
        value(0)
        {};
   
    BWC_INT( int _value )
        {
            value = _value;
        };

    
    BWC_INT& operator=( const BWC_INT& rhs )
    {
        value = rhs.value;
        return *this;
    }

    bool operator==(const BWC_INT& rhs) const
    {
        return (  value == rhs.value  );
    }

    bool operator!=(const BWC_INT& rhs) const
    {
        return !( *this == rhs );
    }

    void LoadFromProperty( const char* property_name );
    void SetToProperty( const char* property_name ) const;

};

/*-----------------------------------------------------------------------------
    BWC Operation Scenario
  -----------------------------------------------------------------------------*/
typedef enum
{
    BWCPT_NONE = 0,            /*Lowest Priority*/

    BWCPT_VIDEO_NORMAL,

    BWCPT_CAMERA_PREVIEW,

    BWCPT_CAMERA_ZSD,

    BWCPT_CAMERA_CAPTURE,

    BWCPT_VIDEO_SWDEC_PLAYBACK,

    BWCPT_VIDEO_PLAYBACK,

    BWCPT_VIDEO_TELEPHONY,

    BWCPT_VIDEO_RECORD,

    BWCPT_VIDEO_RECORD_CAMERA,  
    
    BWCPT_VIDEO_RECORD_SLOWMOTION, 

    BWCPT_VIDEO_SNAPSHOT,
    
    BWCPT_VIDEO_LIVE_PHOTO,
    
    BWCPT_VIDEO_WIFI_DISPLAY /*Highest Priority*/

    
} BWC_PROFILE_TYPE;


typedef enum
{
    BWCVT_NONE,
        
    BWCVT_MPEG4,
    BWCVT_H264,
    BWCVT_VP8,
    BWCVT_VC1,
    BWCVT_MPEG2

} BWC_VCODEC_TYPE;


/*-----------------------------------------------------------------------------
    BWC Setting : a combination of MM setting. In charge of calculating out 
                  bandwidth consumption
  -----------------------------------------------------------------------------*/
class BWC_SETTING
{
    public:
        BWC_SIZE        sensor_size;
        BWC_SIZE        vr_size;
        BWC_SIZE        disp_size;
        BWC_SIZE        tv_size;
        int             fps;
        BWC_VCODEC_TYPE venc_codec_type;
        BWC_VCODEC_TYPE vdec_codec_type;

    public:
        BWC_SETTING(){};
        BWC_SETTING(    const BWC_SIZE& _sensor_size,
                        const BWC_SIZE& _vr_size,
                        const BWC_SIZE& _disp_size,
                        const BWC_SIZE& _tv_size,
                        int _fps,
                        BWC_VCODEC_TYPE _venc_codec_type,
                        BWC_VCODEC_TYPE _vdec_codec_type
                    ):
                    sensor_size( _sensor_size ),
                    vr_size(_vr_size),
                    disp_size(_disp_size),
                    tv_size(_tv_size),
                    fps(_fps),
                    venc_codec_type(_venc_codec_type),
                    vdec_codec_type(_vdec_codec_type)
                    {};


    public:
        unsigned long CalcThroughput_VR( void ) const;
        unsigned long CalcThroughput_VT( void ) const;

        
    public:
        void DumpInfo( void );
        
};





/*******************************************************************************

 *******************************************************************************/
/*-----------------------------------------------------------------------------
    BWC Core Object: Interface to BWC
  -----------------------------------------------------------------------------*/


class BWC_MONITOR{
    public:
        int start(void);
        int stop(void);
        unsigned int query_hwc_max_pixel(void);
        BWC_MONITOR();
        ~BWC_MONITOR();
    private:
        int smi_fd;
        unsigned int get_smi_bw_state(void);
};

class BWC
{
    public:
        int         Profile_Change( BWC_PROFILE_TYPE profile_type , bool bOn );

        

    public:
        void        SensorSize_Set( const BWC_SIZE &sensor_size );
        BWC_SIZE    SensorSize_Get( void );

        void        VideoRecordSize_Set( const BWC_SIZE &vr_size );
        BWC_SIZE    VideoRecordSize_Get( void );
        
        void        DisplaySize_Set( const BWC_SIZE &disp_size );
        BWC_SIZE    DisplaySize_Get( void );
        
        void        TvOutSize_Set( const BWC_SIZE &tv_size );
        BWC_SIZE    TvOutSize_Get( void );

        void        Fps_Set( int fps );
        int         Fps_Get( void );
        
        void            VideoEncodeCodec_Set( BWC_VCODEC_TYPE codec_type );
        BWC_VCODEC_TYPE VideoEncodeCodec_Get( void );

        void            VideoDecodeCodec_Set( BWC_VCODEC_TYPE codec_type );
        BWC_VCODEC_TYPE VideoDecodeCodec_Get( void );


    public:  /*Profile_XX is only for internal use , actually it is not precise, just for reference, kernel log is real case , but I am lazy to modify this interface....*/
        void                _Profile_Set( BWC_PROFILE_TYPE profile);
        void                _Profile_Add( BWC_PROFILE_TYPE profile);
        void                _Profile_Remove( BWC_PROFILE_TYPE profile);
        int                  _Profile_Get( void );
       

    private:
        int     property_name_str_get( const char* function_name , char* prop_name ); /*Auto generate property_name from given function name*/
        bool    check_profile_change_valid( BWC_PROFILE_TYPE profile_type );





    private:/*Platform-Depended Function*/

        int smi_bw_ctrl_set( BWC_PROFILE_TYPE profile_type, BWC_VCODEC_TYPE codec_type , bool bOn );
        int emi_bw_ctrl_set( BWC_PROFILE_TYPE profile_type, BWC_VCODEC_TYPE codec_type , bool bOn );
        
        typedef enum
        {
            EDT_LPDDR2 = 0,
            EDT_DDR3,
            EDT_LPDDR3,
            EDT_mDDR,

            EDT_NONE = -1,
        }EMI_DDR_TYPE;
        EMI_DDR_TYPE emi_ddr_type_get( void );

        typedef enum
        {
            MSP_NORMAL = 0,
            MSP_SCALE_DOWN,
        }MODEM_SPEED_PROFILE;
        int          modem_speed_profile_set( MODEM_SPEED_PROFILE profile );
        
    
};





#endif /*__BANDWIDTH_CONTROL_H__*/

