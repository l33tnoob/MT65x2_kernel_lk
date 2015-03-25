#include <mt_partition.h>
#include <debug.h>

#include "sparse_format.h"
#include "sparse_state_machine.h"



#define DBG_LV 0
#define DBG_INFO 2

#define SDMMC_BLK_SIZE BLK_SIZE

extern void* memset(void* s, int c, int count);
extern void* memcpy(void* dest, const void* src, int count);

//only engine data structure.
static unsparse_data_t m_unsparse_data;

static void mmc_set_unsparse_status( unsparse_status_t* status,
							 STATUS_E handle_status,
							 unsparse_phase_t wait_phase,
							 u32 size,
							 u8 *buf	)
{
	status->handle_status = handle_status;
	status->wait_phase = wait_phase;
	status->size = size;
	status->buf = buf;
}

void mmc_init_unsparse_status(unsparse_status_t* status, u64 image_base_addr, BOOL enable_chk)
{
	mmc_set_unsparse_status(status, S_DONE, UNSPARSE_WAIT_SPARSE_HEADER, 0, 0);
	status->image_base_addr = image_base_addr;
	status->checksum_enabled = enable_chk;
}

static void mmc_write_sparse_data_internal(unsparse_status_t* status)
{
	u32 size = status->size;
	u8 *buf = status->buf;
	u32 sizeOfX=0;
	u32 pre_chksum = 0;
	u32 post_chksum = 0;
	u64 size_wrote = 0;

	//dprintf(DBG_INFO, "[UNSPARSE] wait_phase=%d, buf=0x%x, size=%u \n", status->wait_phase, buf, size);
	dprintf(DBG_INFO, "[UNSPARSE] chunk_remain_data_size=%llx, image_address_offse=%llx \n", m_unsparse_data.chunk_remain_data_size, m_unsparse_data.image_address_offset);
	if (size==0)
	{
		return ;
	}

	switch (status->wait_phase)
	{
	case UNSPARSE_WAIT_SPARSE_HEADER:
		{
			if (size >=sizeof (sparse_header_t))
			{

				memset((void*)&m_unsparse_data, 0x00, sizeof(unsparse_data_t));
				memcpy((void*)&m_unsparse_data.sparse_hdr, buf, sizeof (sparse_header_t));

				dprintf(DBG_LV, "[UNSPARSE] Got SPARSE_HEADER \n");

				size -= sizeof (sparse_header_t);
				buf+= sizeof (sparse_header_t);
				m_unsparse_data.unhandle_buf_size = 0;
				mmc_set_unsparse_status(status, S_DONE, UNSPARSE_WAIT_CHUNK_HEADER, size, buf);
			}
			break;
		}

	case UNSPARSE_WAIT_CHUNK_HEADER:
		{
			if (m_unsparse_data.unhandle_buf_size + size >=sizeof (chunk_header_t))
			{
				if (m_unsparse_data.unhandle_buf_size > 0) {
					u32 sizeOfUsedBuf = sizeof (chunk_header_t)- m_unsparse_data.unhandle_buf_size;
					memcpy(&m_unsparse_data.chunk_hdr, m_unsparse_data.unhandle_buf, m_unsparse_data.unhandle_buf_size);
					memcpy( ((u8*)&m_unsparse_data.chunk_hdr) + m_unsparse_data.unhandle_buf_size, buf, sizeOfUsedBuf);

					size -= sizeOfUsedBuf;
					buf+= sizeOfUsedBuf;
					m_unsparse_data.unhandle_buf_size = 0;
				}
				else
				{
					memcpy(&m_unsparse_data.chunk_hdr, buf, sizeof (chunk_header_t));
					size -= sizeof (chunk_header_t);
					buf+= sizeof (chunk_header_t);
				}

				m_unsparse_data.chunk_remain_data_size = (u64)m_unsparse_data.chunk_hdr.chunk_sz*m_unsparse_data.sparse_hdr.blk_sz;

				//dprintf(DBG_INFO, "[UNSPARSE] Got CHUNK_HEADER, TYPE=0x%x,  BUF:%08X, size:%llX\n", m_unsparse_data.chunk_hdr.chunk_type, buf, m_unsparse_data.chunk_remain_data_size);

				switch (m_unsparse_data.chunk_hdr.chunk_type)
				{
				case CHUNK_TYPE_RAW:
					{
						mmc_set_unsparse_status(status, S_DONE, UNSPARSE_WAIT_CHUNK_DATA, size, buf);
						break;
					}
				case CHUNK_TYPE_DONT_CARE:
					{
						m_unsparse_data.image_address_offset+=m_unsparse_data.chunk_remain_data_size;
						m_unsparse_data.chunk_remain_data_size = 0;
						mmc_set_unsparse_status(status, S_DONE, UNSPARSE_WAIT_CHUNK_HEADER, size, buf);
						break;
					}
				default:
					dprintf(DBG_LV, "[UNSPARSE] No such CHUNK_TYPE: 0x%x \n", m_unsparse_data.chunk_hdr.chunk_type);
					status->handle_status = S_DA_SDMMC_WRITE_FAILED;
					return;
				}
			}
			else
			{
				m_unsparse_data.unhandle_buf_size = size;
				memcpy(m_unsparse_data.unhandle_buf, buf, size);
				size = 0; // force to jump out while loop
				mmc_set_unsparse_status(status, S_DONE, UNSPARSE_WAIT_CHUNK_HEADER, size, buf);
			}
			break;
		}

	case UNSPARSE_WAIT_CHUNK_DATA:
		{
			if (size >= m_unsparse_data.chunk_remain_data_size)
			{
#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
				size_wrote = emmc_write(status->part_id, status->image_base_addr+m_unsparse_data.image_address_offset, buf,  m_unsparse_data.chunk_remain_data_size);
#else
				size_wrote = emmc_write(status->image_base_addr+m_unsparse_data.image_address_offset, buf,  m_unsparse_data.chunk_remain_data_size);
#endif
				if (size_wrote  != m_unsparse_data.chunk_remain_data_size)
				{
					status->handle_status = S_DA_SDMMC_WRITE_FAILED;
					dprintf(DBG_LV, "[UNSPARSE] S_DA_SDMMC_WRITE_FAILED:%d, status=%d, size=%d\n",__LINE__, status->handle_status, status->size);
					return;
				}

				if(status->checksum_enabled)
				{
					pre_chksum = calc_checksum(buf, (u32)m_unsparse_data.chunk_remain_data_size);
#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
					if(m_unsparse_data.chunk_remain_data_size !=
						emmc_read(status->part_id, status->image_base_addr+m_unsparse_data.image_address_offset, buf,  m_unsparse_data.chunk_remain_data_size))
#else
					if(m_unsparse_data.chunk_remain_data_size !=
						emmc_read(status->image_base_addr+m_unsparse_data.image_address_offset, buf,  m_unsparse_data.chunk_remain_data_size))
#endif
					{
						status->handle_status = S_DA_SDMMC_READ_FAILED;
						dprintf(DBG_LV, "[UNSPARSE] S_DA_SDMMC_READ_FAILED:%d, status=%d, size=%d\n",__LINE__, status->handle_status, status->size);
						return;
					}

					post_chksum = calc_checksum(buf, (u32)m_unsparse_data.chunk_remain_data_size);

					if(post_chksum != pre_chksum)
					{
						status->handle_status = S_DA_SDMMC_CHECKSUM_ERR;
						dprintf(DBG_LV, "[UNSPARSE] S_DA_SDMMC_CHECKSUM_ERR:%d, post_chksum=%d, pre_chksum=%d\n",__LINE__, post_chksum, pre_chksum);
						return;
					}
				}

				buf += m_unsparse_data.chunk_remain_data_size;
				size -= m_unsparse_data.chunk_remain_data_size;
				m_unsparse_data.image_address_offset += m_unsparse_data.chunk_remain_data_size;
				m_unsparse_data.chunk_remain_data_size = 0;
				mmc_set_unsparse_status(status, S_DONE, UNSPARSE_WAIT_CHUNK_HEADER, size, buf);
			}
			else
			{   //big trunk, so it need more than 1 package.
				sizeOfX = size;
				if(size &  (SDMMC_BLK_SIZE-1)) //not 512 aligned.
				{
					if(size < SDMMC_BLK_SIZE) //last fragment in a package. this package must have successive package that contain the identical trunk.
					{
						mmc_set_unsparse_status(status, S_DA_SDMMC_SPARSE_INCOMPLETE, UNSPARSE_WAIT_CHUNK_DATA, size, buf);
						dprintf(DBG_INFO, "S_DA_SDMMC_SPARSE_INCOMPLETE\n");
						break;
					}
					else
					{
						size &= ~(SDMMC_BLK_SIZE-1);
					}
				}

#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
				size_wrote = emmc_write(status->part_id, status->image_base_addr+m_unsparse_data.image_address_offset, buf,  size);
#else
				size_wrote = emmc_write(status->image_base_addr+m_unsparse_data.image_address_offset, buf,  size);
#endif
				if (size_wrote  != size)
				{
					status->handle_status = S_DA_SDMMC_WRITE_FAILED;
					dprintf(DBG_LV, "[UNSPARSE] S_DA_SDMMC_WRITE_FAILED:%d, status=%d, size=%d\n",__LINE__, status->handle_status, status->size);
					return;
				}

				if(status->checksum_enabled)
				{
					pre_chksum = calc_checksum(buf, (u32)size);
#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
					if(size !=
						emmc_read(status->part_id, status->image_base_addr+m_unsparse_data.image_address_offset, buf,  size))
#else
					if(size !=
						emmc_read(status->image_base_addr+m_unsparse_data.image_address_offset, buf,  size))
#endif
					{
						status->handle_status = S_DA_SDMMC_READ_FAILED;
						dprintf(DBG_LV, "[UNSPARSE] S_DA_SDMMC_READ_FAILED:%d, status=%d, size=%d\n",__LINE__, status->handle_status, status->size);
						return;
					}

					post_chksum = calc_checksum(buf, (u32)size);

					if(post_chksum != pre_chksum)
					{
						status->handle_status = S_DA_SDMMC_CHECKSUM_ERR;
						dprintf(DBG_LV, "[UNSPARSE] S_DA_SDMMC_CHECKSUM_ERR:%d, post_chksum=%d, pre_chksum=%d\n",__LINE__, post_chksum, pre_chksum);
						return;
					}
				}

				m_unsparse_data.image_address_offset += size;
				m_unsparse_data.chunk_remain_data_size -= size;
				buf += size;
				size = sizeOfX & (SDMMC_BLK_SIZE-1);   // size = 0 org
				mmc_set_unsparse_status(status, S_DONE, UNSPARSE_WAIT_CHUNK_DATA, size, buf);
			}
			break;
		}
	}

	return ;
}


void mmc_write_sparse_data(unsparse_status_t* status)
{
	do
	{
		mmc_write_sparse_data_internal(status);
		if (status->handle_status !=S_DONE)
		{
			return;
		}

	} while (status->size > 0);

	return;
}


