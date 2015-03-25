#ifndef _SDIO_DETECT_H_
#define _SDIO_DETECT_H_

#include <linux/mmc/card.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio_ids.h>
#include <linux/module.h>



typedef struct _MTK_WCN_HIF_SDIO_CHIP_INFO_
{
  struct sdio_device_id deviceId;
	unsigned int chipId;
}MTK_WCN_HIF_SDIO_CHIP_INFO, *P_MTK_WCN_HIF_SDIO_CHIP_INFO;



extern int sdio_detect_exit(void);
extern int sdio_detect_init(void);
extern int sdio_detect_query_chipid(int waitFlag);
extern int hif_sdio_is_chipid_valid (int chipId);

#endif