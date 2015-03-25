/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

/*Hong-Rong: copy from uboot. FIXME!*/
 
#ifndef _PART_H
#define _PART_H

#include <sys/types.h>
#include <stdint.h>


typedef ulong lbaint_t; //Hong-Rong: Add for lk porting

typedef struct block_dev_desc {
	int		dev;		/* device number */
	lbaint_t		lba;		/* number of blocks */
	unsigned long	blksz;		/* block size */
	unsigned long	(*block_read)(int dev,
				      unsigned long start,
				      lbaint_t blkcnt,
				      void *buffer);
	unsigned long	(*block_write)(int dev,
				       unsigned long start,
				       lbaint_t blkcnt,
				       const void *buffer);
}block_dev_desc_t;


#endif /* _PART_H */
