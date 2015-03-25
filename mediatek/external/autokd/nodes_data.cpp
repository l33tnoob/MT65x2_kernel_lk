#include "autok.h"
#include <string>
#include <iostream>
#include <list>
#define BUF_LEN     1024

int get_param_count()
{
    char *data_buf;
    int data_count;
    int param_count = 0;
    get_node_data(PARAM_COUNT_DEVNODE, &data_buf, &data_count);
    sscanf (data_buf,"%d", &param_count);
    free(data_buf);
    return param_count;
}   

int get_debug()
{
    int debug_value = -1;
    char *data_buf;
    int data_count;
    get_node_data(DEBUG_DEVNODE, &data_buf, &data_count);
    sscanf(data_buf, "%d\n", &debug_value);
    free(data_buf);
    return debug_value;
}

int set_debug(int debug)
{
    int result = -1;
    char data_buf[BUF_LEN]="";
    int data_count = 0;
    data_count = snprintf(data_buf, BUF_LEN, "%d", debug);
    result = set_node_data(DEBUG_DEVNODE, data_buf, data_count);
    return result;
}

std::list<struct host_progress*> get_ready()
{
    std::list<struct host_progress*> proglist;
    int debug_value = -1;
    char *data_buf;
    int data_count;
    get_node_data(READY_DEVNODE, &data_buf, &data_count);
    
    int host_id = -1;
	int is_done = 0;
	std::list<struct host_progress*>::iterator it_prog;
	char *line;
	line = strtok(data_buf,"\n");
	char *s_idx, *e_idx;
	char temp_str[512];
	while (line != NULL)
	{
		//printf ("%s\n",line);
		s_idx = line;
		if((e_idx = strstr(line, ":"))){
			memcpy(temp_str, s_idx, e_idx-s_idx);
			memset(temp_str+(e_idx-s_idx), 0, 1);
			sscanf(temp_str, "%d", &host_id);
			s_idx = e_idx+1;
			if((e_idx = strstr(s_idx, "\t")) || (e_idx = strstr(s_idx, " "))){
				memcpy(temp_str, s_idx, e_idx-s_idx);
				memset(temp_str+(e_idx-s_idx), 0, 1);
				sscanf(temp_str, "%d", &is_done);
				//printf("host_id:%d, done:%d\n", host_id, is_done);
			}
			for (it_prog=proglist.begin(); it_prog!=proglist.end() ; ++it_prog){
				struct host_progress *temp_prog = *it_prog;
				if(temp_prog->host_id == host_id){
					temp_prog->is_done = is_done;
					break;
				}
			}
			if(it_prog == proglist.end()){
				struct host_progress *prog = (struct host_progress *)malloc(sizeof(struct host_progress));
				prog->host_id = host_id;
				prog->is_done = is_done;
				proglist.push_back(prog);
			}
		}
		line = strtok (NULL, "\n");
	}      
    free(data_buf);
    return proglist;
}

int set_ready(int id)
{
    int result = -1;
    char data_buf[BUF_LEN]="";
    int data_count = 0;
    std::list<struct host_progress*>::iterator it_prog;
    data_count = snprintf(data_buf, BUF_LEN, "%d", id);
    result = set_node_data(READY_DEVNODE, data_buf, data_count);
    /*for (it_prog=proglist.begin(); it_prog!=proglist.end() ; ++it_prog){
		struct host_progress *temp_prog = *it_prog;
		if(temp_prog->host_id == id){
			temp_prog->is_done ^= 1;
			break;
		}
	}*/
    return result;
}

struct autok_predata get_stage2(int id)
{
    char devnode[BUF_LEN]=""; 
    snprintf(devnode, BUF_LEN, "%s/%d", STAGE2_DEVNODE, id);
    struct autok_predata predata = get_param(devnode);
    return predata;
}

int set_stage2(int id, struct autok_predata *predata)
{
    char *buf;
    char devnode[2*BUF_LEN]=""; 
    int offset;
    int result = -1;
    snprintf(devnode, 2*BUF_LEN, "%s/%d", STAGE2_DEVNODE, id);
    offset = serilize_predata(predata, &buf);
    printf("stage2 length[%d]\n", offset);
    result = set_node_data(devnode, buf, offset);
    free(buf);
    return result;
}

int get_stage1_done(int id)
{
    char devnode[BUF_LEN]="";    
    int value = -1;
    char *data_buf = NULL;
    int data_count;
    snprintf(devnode, BUF_LEN, "%s/%d/%s", STAGE1_DEVNODE, id, "DONE");
    get_node_data(devnode, &data_buf, &data_count);
    if(data_buf == NULL)
        return 0;
    sscanf(data_buf, "%d\n", &value);
    free(data_buf);
    return value;     
}

int set_stage1_done(int id, int data)
{
    char devnode[BUF_LEN]="";    
    int result = -1;
    char data_buf[BUF_LEN]="";
    int data_count = 0;
    snprintf(devnode, BUF_LEN, "%s/%d/%s", STAGE1_DEVNODE, id, "DONE");
    data_count = snprintf(data_buf, BUF_LEN, "%d", data);
    result = set_node_data(devnode, data_buf, data_count);
    return result; 
}

struct autok_predata get_stage1_params(int id)
{
    char devnode[BUF_LEN]="";  
    snprintf(devnode, BUF_LEN, "%s/%d/%s", STAGE1_DEVNODE, id, "PARAMS");
    struct autok_predata predata = get_param(devnode);
    return predata;
}

int set_stage1_params(int id, struct autok_predata *predata)
{
    char *buf;
    int offset;
    int result = -1;
    char devnode[BUF_LEN]="";  
    snprintf(devnode, BUF_LEN, "%s/%d/%s", STAGE1_DEVNODE, id, "PARAMS");
    offset = serilize_predata(predata, &buf);
    result = set_node_data(devnode, buf, offset);
    free(buf);
    return result;
}

int get_stage1_voltage(int id)
{
    char devnode[BUF_LEN]="";    
    int value = -1;
    char *data_buf;
    int data_count;
    snprintf(devnode, BUF_LEN, "%s/%d/%s", STAGE1_DEVNODE, id, "VOLTAGE");
    get_node_data(devnode, &data_buf, &data_count);
    sscanf(data_buf, "%d\n", &value);
    free(data_buf);
    return value;     
}

int set_stage1_voltage(int id, int data)
{
    char devnode[BUF_LEN]="";    
    int result = -1;
    char data_buf[BUF_LEN]="";
    int data_count = 0;
    snprintf(devnode, BUF_LEN, "%s/%d/%s", STAGE1_DEVNODE, id, "VOLTAGE");
    data_count = snprintf(data_buf, BUF_LEN, "%d", data);
    result = set_node_data(devnode, data_buf, data_count);
    return result; 
}