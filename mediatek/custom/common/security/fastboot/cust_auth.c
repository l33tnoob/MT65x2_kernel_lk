/* MTK Proprietary Customization File */

#include <platform/mt_typedefs.h>

#define RSA2048_KEY_LENGTH          (256)
#define RSA1024_KEY_LENGTH          (128)
#define SHA256_LENGTH_IN_BYTE       (32)
#define SHA1_LENGTH_IN_BYTE         (20)

void cust_init_key (unsigned char *rsa_n, unsigned int n_len, unsigned char *rsa_e, unsigned int e_len)
{
    return;
}

int cust_hash (unsigned char* d_buf,  unsigned int d_len, unsigned char* h_buf, unsigned int h_len)
{
    //SHA1
 	if(h_len == SHA1_LENGTH_IN_BYTE)
    {

    }
    //SHA256
    else if(h_len == SHA256_LENGTH_IN_BYTE)
    {

    }

    //If length is not supported, then go here directly
    return -1;
}

int cust_verify (unsigned char* d_buf,  unsigned int d_len, unsigned char* s_buf, unsigned int s_len)
{
    //RSA1024
	if(s_len == RSA1024_KEY_LENGTH)
	{
        unsigned char sha1_buf[SHA1_LENGTH_IN_BYTE] = {0};    
            
        if( cust_hash(d_buf, d_len, sha1_buf, SHA1_LENGTH_IN_BYTE) != 0 )
        {
            return -1;
        }        
        
         return -1;        
    }
    //RSA2048
    else if(s_len == RSA2048_KEY_LENGTH)
    {
        unsigned char sha256_buf[SHA256_LENGTH_IN_BYTE] = {0};
       
        if( cust_hash(d_buf, d_len, sha256_buf, SHA256_LENGTH_IN_BYTE) != 0 )
        {
            return -1;
        }        
        
        return -1;        
    }

    //If length is not supported, then go here directly
    return -1;
}

