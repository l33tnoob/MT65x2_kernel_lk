/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv)
{
    unsigned n;
    unsigned char *x;
    unsigned m;
    unsigned run_val;
    unsigned run_count;
 
    n = gimp_image.width * gimp_image.height;
    m = 0;
    x = gimp_image.pixel_data;

    printf("struct {\n");
    printf("  unsigned width;\n");
    printf("  unsigned height;\n");
    printf("  unsigned cwidth;\n");
    printf("  unsigned cheight;\n");
    printf("  unsigned char rundata[];\n");
    printf("} font = {\n");
    printf("  .width = %d,\n  .height = %d,\n  .cwidth = %d,\n  .cheight = %d,\n", gimp_image.width, gimp_image.height,
           gimp_image.width / 96, gimp_image.height);
    printf("  .rundata = {\n");
   
    run_val = (*x ? 0 : 255);
    run_count = 1;
    n--;
    x+=3;

    while(n-- > 0) {
        unsigned val = (*x ? 0 : 255);
        x+=3;
        if((val == run_val) && (run_count < 127)) {
            run_count++;
        } else {
eject:
            printf("0x%02x,",run_count | (run_val ? 0x80 : 0x00));
            run_val = val;
            run_count = 1;
            m += 5;
            if(m >= 75) {
                printf("\n");
                m = 0;
            }
        }
    }
    printf("0x%02x,",run_count | (run_val ? 0x80 : 0x00));
    printf("\n0x00,");
    printf("\n");
    printf("  }\n};\n");
    return 0;
}
