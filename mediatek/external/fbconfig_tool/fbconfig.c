/* (C) Copyright 2009 
 * MediaTek <www.MediaTek.com>
 * Xiaokuan Shi <Xiaokuan.Shi@MediaTek.com>
 *
 * FBCONFIG TOOL 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "fbconfig.h"
#define CHAR_PER_LINE 680// Characters per record :680=128*5+30+x;
int tmp_val[128];
int fd = -1;
int data_type = 0 ;
CONFIG_RECORD  record_cmd ;
MIPI_TIMING timing;

static char _help[] =
	
    "\n"
    "USAGE\n"
    "		 Under /system/bin and run fbconfig"
    "    For Example:    ./fbconfig [PARAMETER] \n"
    "\n"
    "PARAMETER\n"
    "       driver_ic_config \n"
    "\n"
    "       driver_ic_reset \n"
    "\n"
    "       lcm_get_id \n"
    "\n"
    "       lcm_get_esd \n"
    "\n"    
    "       mipi_set_clock \n"
    "\n"    
    "       test_lcm_type \n"
    "\n"    
    "       mipi_set_clock_v2 \n"
    "\n"
    "       mipi_set_lane \n"
    "\n"
    "       mipi_set_timing \n"     
    "\n"
    "       mipi_set_non_cc \n"
    "\n"
    "       mipi_set_ssc \n"
    "\n"
    "       te_set_enable \n"
    "\n"    
    "       fb_layer_dump \n"
    "\n"    
    "       get_dsi_continuous \n"
    "\n"    
    "       get_dsi_clk \n"
    "\n"    
    "       get_dsi_clk_v2 \n"
    "\n"    
    "       get_dsi_lane_num \n"
    "\n"    
    "       get_dsi_timing \n"
    "\n"    
    "       get_dsi_te_enable \n"
    "\n"
    "       get_dsi_ssc \n"
    "\n"
    "       Reserved \n"
    ;

static char mipi_help[] =
	"\n"
	"\nUsage Example: ./fbconfig mipi_set_timing HS_ZERO 23\n"	
    "TIMCON0_REG:"  "LPX" "  " "HS_PRPR" "  "  "HS_ZERO" "  "  "HS_TRAIL\n"
    "\n"
    "TIMCON1_REG:" "TA_GO" "  " "TA_SURE" "  " "TA_GET" "  " "DA_HS_EXIT\n"
    "\n"
    "TIMCON2_REG:" "CLK_ZERO" "  " "CLK_TRAIL" "  " "CONT_DET\n"
    "\n"
    "TIMCON3_REG:" "CLK_HS_PRPR" "  " "CLK_HS_POST" "  " "CLK_HS_EXIT\n"
    "\n"
    "VDO MODE :" "HPW" "  " "HFP" "  " "HBP" "  " "VPW" "  " "VFP" "  " "VBP"
    "\n"
    ;
       
//---------------------Driver IC Config START-----------------------

static int  format_to_instrut(void)
{
int base_data =0 ;
/*
0x29==>0x2902
0x39==>0x3902
0x15==>0x1500
0x05==>0x0500
*/
switch(data_type)
{
case 0x39:
base_data = 0x3902;
break;
case 0x29:
base_data = 0x2902;
break;
case 0x15:
base_data = 0x1500;
break;
case 0x05:
base_data = 0x0500;
break;
default :
printf("No such data type ,error!!");
}

if((base_data == 0x3902)||(base_data==0x2902))
{
int ins_num = 1+((tmp_val[1]+1)/4) + (((tmp_val[1]+1)%4) ? 1:0) ;//base ins+parameter instruction; 
int * ins_array = (int*)malloc(sizeof(int)*ins_num);
 ins_array[0] = ((tmp_val[1]+1)<<16 )+ base_data;    // (1)the first one instruction
 int ins_index = 1;
 int tmp_vi =2;//tmp_val_index
 printf("ins_num is %d; tmp_val[1] is %d\n",ins_num,tmp_val[1]);
 if(ins_num >1)
 	{
 	ins_array[ins_index] = tmp_val[0]+ (tmp_val[tmp_vi]<<8) +(tmp_val[tmp_vi+1]<<16)+(tmp_val[tmp_vi+2]<<24) ;
	//(2) the first-2nd instruction
	tmp_vi = 5 ;
	for(ins_index=2;ins_index < ((tmp_val[1]+1)/4+1); ins_index++)
		{
		ins_array[ins_index]= tmp_val[tmp_vi]+(tmp_val[tmp_vi+1]<<8)+(tmp_val[tmp_vi+2]<<16)+(tmp_val[tmp_vi+3]<<24);
		tmp_vi+=4;//(3)the middle instruction;
		}			
	if(((tmp_val[1]+1)%4 != 0)&&(tmp_val[1] >3 ))//(4) the last instruction ;
		{
		ins_array[ins_index]= tmp_val[tmp_vi];
		if(tmp_vi <= tmp_val[1] )
		ins_array[ins_index] +=	(tmp_val[tmp_vi+1]<<8) ;
		if(tmp_vi+1 <= tmp_val[1] )
		ins_array[ins_index] +=	(tmp_val[tmp_vi+2]<<16) ;
		}
	//print to test *****************************
	printf("\n the ins_index is %d\n",ins_index);
	int z ;
	for(z=0 ; z < ins_num;z++)
	printf("====>the instruction is :0x%x\n",ins_array[z]);	

	
	/***now the cmd instructions are stored in ins_array[]****/
	record_cmd.ins_num = ins_num;
	record_cmd.type = RECORD_CMD;
	//record_cmd.ins_array = ins_array ;
	memcpy(record_cmd.ins_array,ins_array,sizeof(int)*ins_num);	
	
 	}
 else
 	{
 		printf("only one instruction to apply!!\n");
		return 0 ;
 	}
}//at least 2 instructions
else if(base_data == 0x1500)
{
int tmp_inst=0;
tmp_inst = base_data + (tmp_val[0]<<16)+(tmp_val[2]<<24);
record_cmd.ins_num = 1;
record_cmd.type = RECORD_CMD;
printf("====>the instruction is :0x%x\n",tmp_inst);	
record_cmd.ins_array[0]= tmp_inst ;
}
else if(base_data == 0x0500)
{
int tmp_inst=0;
tmp_inst = base_data + (tmp_val[0]<<16);
record_cmd.ins_num = 1;
record_cmd.type = RECORD_CMD;
printf("====>the instruction is :0x%x\n",tmp_inst);
record_cmd.ins_array[0]= tmp_inst ;
}
return 0 ;

}

static int check_upper_case(char *tmp)
{
int ret =0 ;
while(*tmp != '\0')
{
	if(*tmp=='X')
	{
	ret = -1 ;
	printf("\nnow tmp is %c\n",*tmp);
	break;
	}
	tmp++;
}
return ret ;
}


static int convert_to_int(char *tmp)
{	
	unsigned int cmd ;
	cmd = DRIVER_IC_CONFIG;
	printf("\ni want to know the record ^0^:\n%s",tmp);
	if(strncmp(tmp,"TYPE",4)==0)
	{
	// it's cmd line ,parse it !!
	//format : CMD:ADDR:NUM:{PAR,PAR,.....}
	 int addr;
	 int num_par;
	sscanf(tmp+5,"0x%x",&data_type);
	memset(tmp_val,0x00,sizeof(int)*128);
	sscanf(tmp+14,"0x%x",&addr);
	sscanf(tmp+19,"0x%x",&num_par);	
	tmp_val[0] = addr ;
	tmp_val[1] = num_par ;
	printf("\nAddr is 0x%x\nNum_par is 0x%x\n",addr,num_par);
	tmp=tmp+23;//here is ":{"
	if(strncmp(tmp,":{",2)==0)//check !!
	printf("till now all is right ! and next is cmd value!\n");
	else
	printf("something is wrong before cmd value!! check please\n");
	
	int size = strlen(tmp);
	printf("the size of tmp[] is %d\n",size);
	int j=0;
	int n=2;
	tmp=tmp+2;
		while((strncmp(tmp,"}",1)!=0))//&&(j<size))
		{
		sscanf(tmp,"0x%x",&tmp_val[n]);
		printf("data value is 0x%x\n",tmp_val[n]);
		n++;
		j+=5;
		tmp+=5;
		}

		format_to_instrut();
		ioctl(fd, cmd, &record_cmd);
		
   }
   else if(strncmp(tmp,"MS",2)==0)
	{
	// not cmd line ;
	int ms;
	sscanf(tmp+3,"0x%x",&ms);
	record_cmd.ins_num =1;
	record_cmd.type = RECORD_MS ;
	record_cmd.ins_array[0] =ms;	
	printf("run here -->msleep :%d\n",ms);
	ioctl(fd, cmd, &record_cmd);
	}
   else if(strncmp(tmp,"PIN",3)==0)
   	{
	int enable;
	sscanf(tmp+4,"0x%x",&enable);
	record_cmd.ins_num =1;
	record_cmd.type = RECORD_PIN_SET ;
	record_cmd.ins_array[0] =enable;
	printf("run here -->pin set :%d\n",enable);
	ioctl(fd, cmd, &record_cmd);

    }	
	return 0;
}




static int file_parse(char path[30])

{	
	FILE * filed;
	
	char tmp[CHAR_PER_LINE];
	int test =0;
	int ret =0;
	printf("config file is: %s\n",path);
	filed = fopen(path,"r");
	if(filed != NULL)
		{
		while(fgets(tmp,CHAR_PER_LINE,filed)!=NULL)
		{
		ret=check_upper_case(tmp);
		if(ret !=0)
		{
		printf("Error!!!there is a upper case 'X' in config file \nLine: %s\n",tmp);
		break;
		}
		else
		convert_to_int(tmp);//parsing the record to tmp_val[128];		
		}		
		return 0;			

		 }
	else 
		{
		printf("can not open file in:%s",path);
		return 0 ;
		}
			
}

//---------------------Driver IC Config END-----------------------

static void check_mipi_type(char * type)
{
if(!strcmp(type,"HS_PRPR"))
	timing.type = HS_PRPR;
else if(!strcmp(type,"HS_ZERO"))
	timing.type = HS_ZERO;
else if(!strcmp(type,"HS_TRAIL"))
	timing.type = HS_TRAIL;
else if(!strcmp(type,"TA_GO"))
	timing.type = TA_GO;
else if(!strcmp(type,"TA_SURE"))
	timing.type = TA_SURE;
else if(!strcmp(type,"TA_GET"))
	timing.type = TA_GET;
else if(!strcmp(type,"DA_HS_EXIT"))
	timing.type = DA_HS_EXIT;
else if(!strcmp(type,"CLK_ZERO"))
	timing.type = CLK_ZERO;
else if(!strcmp(type,"CLK_TRAIL"))
	timing.type = CLK_TRAIL;
else if(!strcmp(type,"CONT_DET"))
	timing.type = CONT_DET;
else if(!strcmp(type,"CLK_HS_PRPR"))
	timing.type = CLK_HS_PRPR;
else if(!strcmp(type,"CLK_HS_POST"))
	timing.type = CLK_HS_POST;
else if(!strcmp(type,"CLK_HS_EXIT"))
	timing.type = CLK_HS_EXIT;
else if(!strcmp(type,"HPW"))
	timing.type = HPW;
else if(!strcmp(type,"HFP"))
	timing.type = HFP;
else if(!strcmp(type,"HBP"))
	timing.type = HBP;
else if(!strcmp(type,"VPW"))
	timing.type = VPW;
else if(!strcmp(type,"VFP"))
	timing.type = VFP;
else if(!strcmp(type,"VBP"))
	timing.type = VBP;
else if(!strcmp(type,"LPX"))
	timing.type = LPX;
else if(!strcmp(type,"SSC_EN"))
	timing.type = SSC_EN;
else
	printf("No such mipi timing control option!!\n");

}

static LAYER_H_SIZE get_layer_size(int layer_id)
{
//int size =0 ;
int cmd = FB_LAYER_GET_SIZE ;
LAYER_H_SIZE tmp;
tmp.layer_size=0;
tmp.height = layer_id ;
tmp.fmt = 0 ;
printf("[LAYER_DUMP]layer_id is %d\n",layer_id);

ioctl(fd, cmd, &tmp);	
return tmp ;
}

static int fbconfig_layer_dump(int layer_id)
{
void* base = NULL;
void * tmp_base = NULL;
BMF_HEADER bmp_file ;
int fd_store= -1 ;
int cmd = FB_LAYER_DUMP ;
char store_path[30];
LAYER_H_SIZE layer_info;
int i ;
sprintf(store_path,"%s%d%s", "/data/layer",layer_id,"_dump.bmp");
fd_store = open(store_path,O_WRONLY | O_CREAT,0644);
if(fd_store <0)
printf("[LAYER_DUMP]create /data/lay_dump fail !!\n");

memset(&layer_info,0,sizeof(LAYER_H_SIZE));
memset(&bmp_file,0,sizeof(bmp_file));

layer_info= get_layer_size(layer_id);
printf("[LAYER_DUMP]layer%d size is %dbyte\n",layer_id ,layer_info.layer_size);

if(layer_info.layer_size >0)
{
bmp_file.type = 0x4D42 ; //'BM'
bmp_file.fsize= layer_info.layer_size + 54;
bmp_file.res1 = 0;
bmp_file.res2 = 0;
bmp_file.offset = 54;//40+14 bytes
//below 40 bytes are for BMP INFO HEADER 
bmp_file.this_struct_size = 0x28;
bmp_file.width= (layer_info.layer_size)/(layer_info.height)/(layer_info.fmt/8);
bmp_file.height = layer_info.height ;
bmp_file.planes = 0x01;
bmp_file.bpp = layer_info.fmt ;//32 
bmp_file.compression = 0x0;
bmp_file.raw_size = layer_info.layer_size;
bmp_file.x_per_meter = 0x0ec4;
bmp_file.y_per_meter = 0x0ec4;
bmp_file.color_used = 0x0;
bmp_file.color_important = 0x0;

printf("[LAYER_DUMP]size of bmp_file is %d\n",sizeof(bmp_file));
printf("[LAYER_DUMP]file size  is 0x%x\n",(layer_info.layer_size+54));
printf("[LAYER_DUMP]raw size  is 0x%x\n",layer_info.layer_size);
printf("[LAYER_DUMP]height  is %d\n",layer_info.height);
printf("[LAYER_DUMP]bpp  is %d\n",layer_info.fmt);

base = malloc(layer_info.layer_size);
if(base == NULL){	
printf("[LAYER_DUMP]malloc for layer dump fail !!!\n");
close(fd_store);
return -1 ;
}
tmp_base = base ;
//memcpy(base,&bmp_file,sizeof(bmp_file));
printf("[LAYER_DUMP]malloc :0x%x \n",(unsigned int)base);
if(ioctl(fd, cmd, base)!=0){
printf("[LAYER_DUMP]ioctl memcpy fail !!!\n");
close(fd_store);
free(base);
tmp_base = base = NULL;
return -2 ;
}
write(fd_store, &bmp_file, sizeof(bmp_file));//write BMF header to bmp file ;
base = base + layer_info.layer_size; // reposition pointer ;
for(i=0;i<=layer_info.height;i++)//  write raw data to bmp file;
{
base =  base -  bmp_file.width*(layer_info.fmt/8);
write(fd_store, base, bmp_file.width*(layer_info.fmt/8));
}
free(tmp_base);
tmp_base = base = NULL ;
close(fd_store);
return 0 ;
}
else{
close(fd_store);
return -2;
}
}

int main (int argc, char **argv)
{
	
	unsigned int cmd ;
	char * tmp ;
	if (argc ==1)
	printf("%s",_help);
	else
		{
		fd = open("/sys/kernel/debug/fbconfig", O_RDWR);
		if(fd >0)
			{
		if(!strcmp(argv[1],"lcm_get_id"))
			{
			int id_num = 0 ;
		   cmd = LCM_GET_ID;
		   ioctl(fd, cmd, &id_num);
		   printf("lcm_get_id :%d",id_num);
		   if(id_num == 0)
		   	printf("\n====please make sure you have implemented get_lcm_id() in lcm driver==");
			}
		else if(!strcmp(argv[1],"driver_ic_config"))
			{
			//lcm driver IC config ,this will parse config file and process in lcm driver .
				if(argc !=3)
				{
				printf("[W]please indicate config file name for lcm configuration!!\n");
				printf("[W]Config file should be placed under /data folder !\n");
				close(fd);
				return 0 ;
				}
				else
				{
					char path[128] ={0};
					int x ;
					if(strlen(argv[2])<120){
					sprintf(path,"/data/%s",argv[2]);
					memset(&record_cmd,0x00,sizeof(CONFIG_RECORD));
					file_parse(path);
					}
					else
					{
					printf("\nThe name of config file is too long !!\n");
					return 0;
					}
				}
			}
		else if(!strcmp(argv[1],"driver_ic_reset"))
			{
			//lcm driver IC config ,this will parse config file and process in lcm driver .
			if(argc != 2){				
			printf("\nUsage: <./fbconfig driver_ic_reset > \n");				
			close(fd);
			return 0 ;
			}
			else
			{
			printf("\nIn order to Reset Driver IC config to lcm_init setting\n");
			cmd = DRIVER_IC_RESET ;				
			ioctl(fd, cmd, NULL);							
			}
			}
		else if (!strcmp(argv[1],"mipi_set_clock"))
			{
			if(argc !=3)
			printf("\nUsage: <./fbconfig mipi_set_clock CLOCK>\n");
			else{
				unsigned int clock = atoi(argv[2]);
				cmd = MIPI_SET_CLK ;
				printf("mipi_set_clock :%d",clock);
				ioctl(fd, cmd, &clock);
				}
			}
		else if (!strcmp(argv[1],"mipi_set_clock_v2"))
			{
			if(argc !=5)
			printf("\nUsage: <./fbconfig mipi_set_clock_v2 Div1 Div2 Fbk_div>\n");
			else{
				MIPI_CLK_V2 clock_v2 ;
				clock_v2.div1 = atoi(argv[2]);
				clock_v2.div2 = atoi(argv[3]);
				clock_v2.fbk_div = atoi(argv[4]);
				if((clock_v2.div1 > 3)||(clock_v2.div2 >3)||(clock_v2.fbk_div > 31))
				{
				printf("Out of range: div1 and div2 must be in [0,3]; fbk_div must be in [0,31] !!\n");
				return 0;
				}
				cmd = MIPI_SET_CLK_V2;
				printf("mipi_set_clock_v2 :%d %d %d",clock_v2.div1,clock_v2.div2,clock_v2.fbk_div);
				ioctl(fd, cmd, &clock_v2);
				}
			}
		else if (!strcmp(argv[1],"mipi_set_ssc"))
			{
			if(argc !=3)
			printf("\nUsage: <./fbconfig mipi_set_ssc SSC_RANGE>\n");
			else{
				unsigned int ssc = atoi(argv[2]);
				if((ssc >8)||(ssc <1))
				{
				printf("the value for spread frequency is NOT correct ,must in [1,8]\n");
				return 0 ;
				}
				cmd = MIPI_SET_SSC ;
				printf("mipi_set_ssc :%d",ssc);
				ioctl(fd, cmd, &ssc);
				}
			}
		else if (!strcmp(argv[1],"mipi_set_lane"))
			{
			if(argc !=3)
			printf("\nUsage: <./fbconfig mipi_set_lane LANE_ID>\n");
			else{
				cmd = MIPI_SET_LANE ;
				unsigned int lane_num = atoi(argv[2]);
				printf("mipi_set_lane :%d",lane_num);
				if((lane_num > 4)||(lane_num ==0))
				printf("====lane number is not correct ,must be 1,2,3 or 4===");
				else
				ioctl(fd, cmd, &lane_num);
				}
			}
		else if (!strcmp(argv[1],"mipi_set_timing"))
			{
			if(argc !=4)
			{
			printf("%s\n",mipi_help);				
			close(fd);
			return 0 ;
			}			
			cmd = MIPI_SET_TIMING ;
			#if 0
			if(strstr(argv[3], "0x"))
			sscanf(argv[3],"0x%x",&(timing.value));
			else if(strstr(argv[3], "0X"))
			sscanf(argv[3],"0X%x",&(timing.value));
			#endif
			timing.value = atoi(argv[3]);
			check_mipi_type(argv[2]);
			printf("mipi_set_timing :type is %d;value is %d\n",timing.type,timing.value);			
			if(ioctl(fd, cmd, &timing)!= 0)
				{
				printf("==Error !! Do you have ever put your phone in suspend mode ?==\n");
				printf("==Please make sure your phone NOT in suspend mode!!");
				}
			}		
		else if (!strcmp(argv[1],"mipi_set_non_cc"))
			{
			if(argc !=3)
			printf("\nUsage: <./fbconfig mipi_set_non_cc Enable>\n");
			else{
				int enable = atoi(argv[2]);
				cmd = MIPI_SET_CC ;
				printf("mipi_set_non_cc :%d",enable);
				ioctl(fd, cmd, &enable);
				}
			}
		else if (!strcmp(argv[1],"te_set_enable"))
			{
			if(argc !=3)
			printf("\nUsage: <./fbconfig te_set_enable Enable>\n");
			else{
				char enable = atoi(argv[2]);
				cmd = TE_SET_ENABLE ;
				printf("te_set_enable :%d",enable);
				ioctl(fd, cmd, &enable);
				}
			}
		else if (!strcmp(argv[1],"fb_layer_dump"))
			{
				if(argc !=3)
				{
				int cmd = FB_LAYER_GET_EN;
				FBCONFIG_LAYER_INFO layers ;
				if(ioctl(fd, cmd, &layers)==0)
				printf("The current layer enable/disable info is :\nlayer_0:%s\nlayer_1:%s\nlayer_2:%s\nlayer_3:%s\n",
				(layers.layer_enable[0]==1)?"Enable":"Disable",(layers.layer_enable[1]==1)?"Enable":"Disable",
				(layers.layer_enable[2]==1)?"Enable":"Disable",(layers.layer_enable[3]==1)?"Enable":"Disable");
				
				printf("\nUsage: <./fbconfig fb_layer_dump LAYER_ID> to dump LAYER_ID\n");
				}
			else{
				int layer_id= atoi(argv[2]);
				if(layer_id >=4)
				printf("Currently we only have 0,1,2,3 layer to be dumped !!");
				else{
				int ret = fbconfig_layer_dump(layer_id);
				if(ret ==0)
					printf("Layer dump Correctly!!");
				else
					printf("Layer dump Fail!!");
					}
				}
			}	
		else if (!strcmp(argv[1],"lcm_get_esd"))
			{
			if(argc !=5)
			{
				printf("\nUsage: <./fbconfig lcm_get_esd Address TYPE Parameter_number>\n");
			}
			else{
				int addr,para_num,type;
				ESD_PARA esd_para;
				if(strstr(argv[2], "0x"))
				sscanf(argv[2],"0x%x",&addr);
				else if(strstr(argv[2], "0X"))
				sscanf(argv[2],"0X%x",&addr);
				type = atoi(argv[3]);
				para_num = atoi(argv[4]);
				if(para_num>4)
				{
				printf("the para_num must less than 4!!\n");
				return 0;
				}
				printf("lcm_get_esd:addr=0x%x type=%d para_num=%d\n",addr,type,para_num);
				printf("lcm_get_esd:type==0 means:DCS Read;  type==1 means GERNERIC READ\n");
				cmd = LCM_GET_ESD ;	
				esd_para.addr = addr ;
				esd_para.type= type ;
				esd_para.para_num = para_num ;
				if(ioctl(fd, cmd, &esd_para)==0)
				{
				char * esd_ret_buffer =malloc(sizeof(char)*(para_num+6));
				memset(esd_ret_buffer,0,(para_num+6));
				if(esd_ret_buffer != NULL)
					{
					cmd = LCM_GET_ESD_RET ;
					if(ioctl(fd, cmd, esd_ret_buffer)==0)
						{int i ;
						for(i=0;i<(para_num+6);i++)
						printf("\nLCM_GET_ESD:esd_get[%d]==>0x%x\n",i,esd_ret_buffer[i]);
						}
					else
					printf("Something WRONG in LCM_GET_ESD_RET\n");	
					
					}
				}
				else
				printf("Something WRONG in LCM_GET_ESD\n");	
			}
		}
		else if (!strcmp(argv[1],"get_dsi_continuous"))
			{
			int dsi_contin =0 ;
			ioctl(fd, LCM_GET_DSI_CONTINU, &dsi_contin);
			printf("get_dsi_continuous:%d\n",dsi_contin);
			}
		else if (!strcmp(argv[1],"get_dsi_clk_v2"))
			{
			MIPI_CLK_V2 clock_v2;
			ioctl(fd, LCM_GET_DSI_CLK_V2, &clock_v2);
			printf("get_dsi_clk_v2:div1==>%d div2=>%d fbk_div=>%d\n",clock_v2.div1,clock_v2.div2,clock_v2.fbk_div);
			}
		else if (!strcmp(argv[1],"get_dsi_clk"))
			{
			int dsi_clk =0 ;
			ioctl(fd, LCM_GET_DSI_CLK, &dsi_clk);
			printf("get_dsi_clk:%d\n",dsi_clk);
			}
		else if (!strcmp(argv[1],"test_lcm_type"))
			{			
			LCM_TYPE_FB lcm_fb ;
			lcm_fb.clock =0;
			lcm_fb.lcm_type =0;
			ioctl(fd, LCM_TEST_DSI_CLK, &lcm_fb);
			printf("get_dsi_type ==>clk:%d \n",lcm_fb.clock);
			/*
			{
		CMD_MODE = 0,
		SYNC_PULSE_VDO_MODE = 1,
		SYNC_EVENT_VDO_MODE = 2,
		BURST_VDO_MODE = 3

			*/
			switch(lcm_fb.lcm_type)
			{
				case 0:
					printf("get_dsi_type ==> CMD_MODE\n");
					break;
				case 1:
					printf("get_dsi_type ==> SYNC_PULSE_VDO_MODE \n");
					break;
				case 2:
					printf("get_dsi_type ==> SYNC_EVENT_VDO_MODE\n");
					break;
				case 3:
					printf("get_dsi_type ==> BURST_VDO_MODE\n");
					break;
				default :
					printf("get_dsi_type ==> Error: no such type!!\n");
					break;
			}			
			}
		else if (!strcmp(argv[1],"get_dsi_ssc"))
			{
			int ssc =0 ;
			ioctl(fd, LCM_GET_DSI_SSC, &ssc);
			printf("get_dsi_ssc:%d\n",ssc);
			}
		else if (!strcmp(argv[1],"get_dsi_lane_num"))
			{
			int dsi_lane_num =0 ;
			ioctl(fd, LCM_GET_DSI_LANE_NUM, &dsi_lane_num);
			printf("get_dsi_lane_num:%d\n",dsi_lane_num);
			}
		else if (!strcmp(argv[1],"get_dsi_timing"))
			{
			int dsi_timing_ret =0 ;
			if(argc !=3)
			printf("\nUsage: <./fbconfig get_dsi_timing VBP>\n");
			else
				{
				check_mipi_type(argv[2]);
				timing.value =0;
				ioctl(fd, LCM_GET_DSI_TIMING, &timing);
				dsi_timing_ret=timing.value;
				printf("get_dsi_timing==>%s:%d\n",argv[2],dsi_timing_ret);
				}
			}
		else if (!strcmp(argv[1],"get_dsi_te_enable"))
			{
			int dsi_te_enable =0 ;
			ioctl(fd, LCM_GET_DSI_TE, &dsi_te_enable);
			printf("get_dsi_te_enable:%d\n",dsi_te_enable);
			}
		else 
			{
			printf("parameter is not correct !!%s",_help);
			return 0 ;
			}
		}
	
	printf(" \n***finish for this query or setting*** \n");	
	close(fd);
}
	return 0;	
}

