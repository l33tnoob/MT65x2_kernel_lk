#include    <stdio.h>

#if 1
#include    <bandwidth_control.h>
#endif

void Show_Current_Props(void)
{
#ifdef  __BANDWIDTH_CONTROL_H__
    BWC bwc;

    bwc.SensorSize_Get();

    bwc.VideoRecordSize_Get();

    bwc.DisplaySize_Get();

    bwc.TvOutSize_Get();

    bwc.Fps_Get();

    bwc.VideoEncodeCodec_Get();

    bwc.VideoDecodeCodec_Get();

    bwc._Profile_Get();

#endif
}


void Test_Case_01(void)
{
#ifdef  __BANDWIDTH_CONTROL_H__
    BWC bwc;

    bwc.SensorSize_Set( BWC_SIZE(123,456) );
    bwc.SensorSize_Get();

    
    bwc.VideoRecordSize_Set( BWC_SIZE(789,111) );
    bwc.VideoRecordSize_Get();

    
    bwc.DisplaySize_Set( BWC_SIZE(222,333) );
    bwc.DisplaySize_Get();


    
    bwc.TvOutSize_Set( BWC_SIZE(444,555) );
    bwc.TvOutSize_Get();


    
    bwc.Fps_Set( 30 );
    bwc.Fps_Get();

    
    
    bwc.VideoEncodeCodec_Set( BWCVT_H264 );
    bwc.VideoEncodeCodec_Get();

    
    bwc.VideoDecodeCodec_Set( BWCVT_MPEG4 );
    bwc.VideoDecodeCodec_Get();



    bwc.Profile_Change( BWCPT_VIDEO_NORMAL, true );
    bwc._Profile_Get();
#endif
 
    
}

/*-----------------------------------------------------------------------------
    Main Test
  -----------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
    int     i;
    char    command;
    
    if( argc < 2 )
    {
        printf("bwc_test <command>\n");
        printf("\n");
        printf("<command>:\n");
        printf("\tr : Show current properties.\n");
        printf("\t1 : Test case 1.\n");
            
        return -1;
    }
    

    for( i = 0; i < argc; i++ )
    {
        switch( i )
        {
        case 1://<command>
            sscanf( argv[i], "%c", &command );
            break;
        }
    }


    switch( command )
    {
    case 'r':
        printf("Exe Show_Current_Props()\n");
        Show_Current_Props();
        break;
        
    case '1':
        printf("Exe Test_Case_01()\n");
        Test_Case_01();
        break;
        
    default:
        printf("Unknown command.\n");
        break;
        
    }

    

    
}


