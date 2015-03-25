#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "autok.h"
#define BUF_LEN     1024

int get_node_data(char *filename, char **data, int *len)
{
    int fd_rd;
    int result = -1;
    int length=0;
    char rBuf[BUF_LEN] = "";
    fd_rd = open(filename, O_RDONLY, 0000);
    if ((length = read(fd_rd, rBuf, BUF_LEN)) == -1) {
        LOGE("Can't read %s\n", filename);
        goto EXIT_GET_NODE_DATA;
    }
    //printf("GETNODE_LENGTH:%d\n", length);
    *len = length;
    *data = (char*)malloc(length+1);
    memset((*data)+length, 0, 1);
    //goto EXIT_GET_NODE_DATA;
    memcpy(*data, rBuf, length);
    
    result = 0;
EXIT_GET_NODE_DATA:
    close(fd_rd);
    return result;
}

int set_node_data(char *filename, char *data, int len)
{
    int fd_wr;
    int result = -1;
    fd_wr = open(filename, O_WRONLY, 0000);
    if(write(fd_wr, data, len) == -1){
        printf("Can't write %s\n", filename);
        goto EXIT_SET_NODE_DATA;
    }
    result = 0;
EXIT_SET_NODE_DATA:
    close(fd_wr);
    return result;
}

int from_dev_to_data(char *from, char *to)
{
    char *data_buf;
    int data_count;
    FILE * pFile;
    get_node_data(from, &data_buf, &data_count);
    pFile = fopen (to, "wb");
    fwrite (data_buf , sizeof(char), data_count, pFile);
    fclose (pFile);
    free(data_buf);
    return 0;
}

int data_copy(char *from, char *to)
{
    char *data_buf;
    int data_count;
    FILE * inFile;
    FILE * outFile;
    long lSize;
    
    inFile = fopen (from, "rb");
    if (inFile==NULL) {
        printf("File error"); 
        return -1;
    }
    fseek (inFile , 0 , SEEK_END);
    lSize = ftell (inFile);
    rewind (inFile);
    data_buf = (char*) malloc (sizeof(char)*lSize);
    fread (data_buf, 1, lSize, inFile);
    fclose (inFile);    
    
    outFile = fopen (to, "wb");
    if (outFile==NULL) {
        printf("File error"); 
        return -1;
    }
    fwrite (data_buf , sizeof(char), data_count, outFile);
    fclose (outFile);
    
    free (data_buf);
    return lSize;
}

int write_to_file(char *filename, char *data_buf, int length)
{
    FILE * outFile;
    outFile = fopen (filename, "wb");
    if (outFile==NULL) {
        printf("File error"); 
        return -1;
    }
    fwrite (data_buf , sizeof(char), length, outFile);
    fclose (outFile);
    return 0;
}