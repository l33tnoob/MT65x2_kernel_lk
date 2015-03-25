#include "dram_buffer.h"
#include "typedefs.h"
#include "platform.h"
#include "emi.h"

#define MOD "[Dram_Buffer]"
#define DRAM_BASE 0x80000000
#define RESERVED_DRAM_BUF_SIZE (5 * 1024 * 1024)
#define RESERVED_PAGE_TABLE_SIZE (1 * 1024 * 1024)

dram_buf_t* g_dram_buf = 0;
dram_page_table* g_page_table = 0; 


void init_dram_buffer(){
	u32 dram_rank_size[4] = {0,0,0,0};
	u32 dram_size = 0;
	u32 structure_size = sizeof(dram_buf_t);
	/*get memory size*/
	get_dram_rank_size(dram_rank_size);
	dram_size= dram_rank_size[0] + dram_rank_size[1] + dram_rank_size[2] + dram_rank_size[3];
	print("%s dram size:%d \n" ,MOD, dram_size);
	print("%s structure size: %d \n" ,MOD, structure_size);
        print("%s MAX_TEE_DRAM_SIZE: %d \n" ,MOD, MAX_TEE_DRAM_SIZE);
	/*allocate dram_buf*/
	ASSERT(dram_size >= RESERVED_DRAM_BUF_SIZE + MAX_TEE_DRAM_SIZE);
	g_dram_buf = DRAM_BASE  + dram_size - RESERVED_DRAM_BUF_SIZE - MAX_TEE_DRAM_SIZE;
	/*allocate page_table, must align 16384*/
	g_page_table = DRAM_BASE + dram_size - RESERVED_PAGE_TABLE_SIZE - MAX_TEE_DRAM_SIZE;

	if((u32)g_dram_buf + sizeof(dram_buf_t) >= (u32)g_page_table )
	{
	    print("ERROR! DRAM buffer over 4MB, it overlap with page_table buffer\n", MOD);
            ASSERT(0);   
	}

}

