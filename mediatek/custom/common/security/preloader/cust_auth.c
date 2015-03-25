/* MTK Proprietary Customization File */

#include "sec_platform.h"
#include "sec_auth.h"

#define RSA2048_KEY_LENGTH          (256)
#define RSA1024_KEY_LENGTH          (128)
#define SHA256_LENGTH_IN_BYTE       (32)
#define SHA1_LENGTH_IN_BYTE         (20)

U32 da_auth_init (void)
{
    return 0;
}

U32 img_auth_init (void)
{       
    return 0;   
}
int sec_hash (U8* d_buf, U32 d_len, U8* h_buf, U32 h_len)
{
    //SHA1
 	if(h_len == SHA1_LENGTH_IN_BYTE)
    {
        return -1;
    }
    //SHA256
    else if(h_len == SHA256_LENGTH_IN_BYTE)
    {
        return -2; 
    }

    //If length is not supported, then go here directly
    
    return -3;
}

int sec_auth (U8* d_buf, U32 d_len, U8* s_buf, U32 s_len)
{
    //RSA1024
	if(s_len == RSA1024_KEY_LENGTH)
	{
        U8 sha1_buf[SHA1_LENGTH_IN_BYTE] = {0};    
            
        if( sec_hash(d_buf, d_len, sha1_buf, SHA1_LENGTH_IN_BYTE) != 0 )
        {
            return -1;
        }        
        
         return -2;        
    }
    //RSA2048
    else if(s_len == RSA2048_KEY_LENGTH)
    {
        U8 sha256_buf[SHA256_LENGTH_IN_BYTE] = {0};
       
        if( sec_hash(d_buf, d_len, sha256_buf, SHA256_LENGTH_IN_BYTE) != 0 )
        {
            return -3;
        }        
        
        return -4;        
    }

    //If length is not supported, then go here directly
    return -5;
}

