/*
 * (C) Copyright 2008
 * MediaTek <www.mediatek.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <sys/types.h>
#include <debug.h>
#include <err.h>
#include <reg.h>

#include <platform/mt_typedefs.h>
#include <platform/boot_mode.h>
#include <platform/mt_reg_base.h>
#include <platform/mtk_key.h>
#include <target/cust_key.h>
#include <platform/meta.h>
#include <platform/mt_rtc.h>
#include <mt_partition.h>

/**************************************************************************
 *  CONSTANT DEFINITION
 **************************************************************************/
#define META_STR_READY         "READY"         /* Ready Signal          */
#define	META_STR_REQ           "METAMETA"      /* PC META Request      */
#define META_STR_ACK  	       "ATEMATEM"      /* TARGET META Ack Response */
#define META_STR_MOD_PREF      "[META]"        /* Log prefix of meta module */
#define META_LOCK              "LOCK"          /* Meta lock */

#define META_ADV_REQ       "ADVMETA"
#define META_ADV_ACK       "ATEMVDA"

#define META_SZ_MAX_PBUF       20			   /* max protocol buffer size */

#define ATE_STR_REQ           "FACTORYM"      /* PC ATE Request      */
#define ATE_STR_ACK  	        "MYROTCAF"      /* TARGET ATE Ack Response */
#define ATE_STR_MOD_PREF      "[ATE]"        /* Log prefix of ate module */

#define ATE_SZ_MAX_PBUF       20			   /* max protocol buffer size */


/**************************************************************************
 *  DEBUG FLAG
 **************************************************************************/
 //#define META_DEBUG

/**************************************************************************
 *  LOCAL VARIABLE DECLARATION
 **************************************************************************/

/**************************************************************************
 *  FUNCTION IMPLEMENTATION
 **************************************************************************/

/******************************************************************************
 * meta_detection
 * 
 * DESCRIPTION:
 *   Detect META mode is on or off
 *
******************************************************************************/
extern BOOT_ARGUMENT *g_boot_arg;
BOOL meta_detection(void)
{  
  int mode = 0;
  mode = g_boot_arg->boot_mode &= 0x000000FF;

  dprintf(INFO,"%s Check meta info from pre-loader: %x, %x, %d\n", META_STR_MOD_PREF, g_boot_arg->boot_mode, g_boot_arg->maggic_number, mode);
  
  if (g_boot_arg->maggic_number == BOOT_ARGUMENT_MAGIC)
  {
    if (mode == META_BOOT)
     {
      g_boot_mode = META_BOOT;
      return TRUE;
    }
    else if (mode == ADVMETA_BOOT)
    {
      g_boot_mode = ADVMETA_BOOT;
      return TRUE;
    }
    else if (mode == ATE_FACTORY_BOOT)
    {
      g_boot_mode = ATE_FACTORY_BOOT;
      return TRUE;
    }
    else if (mode == ALARM_BOOT)
    {
      g_boot_mode = ALARM_BOOT;
      return TRUE;
    }
    else if (mode == FASTBOOT)
    {
      g_boot_mode = FASTBOOT;
      return TRUE;
    }
    else if (mode == FACTORY_BOOT)
    {
      g_boot_mode = FACTORY_BOOT;
      return TRUE;
    }
    else
    {
      return FALSE;
    }
  }
	return FALSE;
}

