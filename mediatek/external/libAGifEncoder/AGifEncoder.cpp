/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <stdio.h>
#include <cutils/log.h>
#include <utils/Errors.h>
#include <fcntl.h>
#include <sys/mman.h>

//#include <errno.h>
#include <unistd.h>
#include <sys/types.h>

#include <cutils/xlog.h>

#include "SkBitmap.h"
#include "SkStream.h"
#include "SkTemplates.h"


#include "SkUtils.h"

#include <cutils/properties.h>
#include <cutils/xlog.h>


#include "AGifEncoder.h"
#include "GifEncoder.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif


#define LOG_TAG "AGifEncoder"

#define IMG_LOG(fmt, arg...)    XLOGW(fmt, ##arg)
#define IMG_DBG(fmt, arg...)    XLOGD(fmt, ##arg)
//#define JPG_DBG(fmt, arg...)

AGifEncoder *aGifEncoder = NULL;


AGifEncoder::AGifEncoder()
{
   
   IMG_DBG("AGifEncoder:: AGifEncoder()!!\n", __LINE__);      
   aGifEncoder = this;
   fWidth        = 0 ;
   fHeight       = 0 ;
   fPostTime     = 0 ;
   fEncodeFrame  = 0 ;
   fCloseGif     = 0 ;
   fWriteSize    = 0 ;   
   fEncodeTarget = 0 ;
   encoder = NULL ;
   fGifStream = NULL;   
   
}
      
      
AGifEncoder::~AGifEncoder()
{
   
   IMG_DBG("AGifEncoder::~AGifEncoder()!!\n", __LINE__);      
   if(encoder != NULL){
      closeGif();        
      GifEncoder::destroyEncoder(encoder);
      encoder = NULL;
   }
   if(fEncodeTarget == 2){ //from file
     if(fGifStream) delete fGifStream ;
     fGifStream = NULL ;
   }   
   aGifEncoder = NULL;
   
   
}

bool AGifEncoder::setWidth(unsigned int width) 
{
   IMG_DBG("AGIFE:: setWidth %d, L:%d!!\n", width , __LINE__); 
   fWidth = width; 
   return true ;
};

bool AGifEncoder::setHeight(unsigned int height) 
{  
   IMG_DBG("AGIFE:: setHeight %d, L:%d!!\n", height , __LINE__);     
   fHeight = height; 
   return true ;
};

bool AGifEncoder::setFrameDuration(unsigned int time)
{
   IMG_DBG("AGIFE:: setTime %d, L:%d!!\n", time , __LINE__);     
   fPostTime = time; 
   return true ;
};

bool AGifEncoder::setEncodeFile(const char file[])
{
   if(fEncodeTarget) return false ;
      
   //SkFILEWStream   stream(file);

   //IMG_DBG("AGIFE:: setEncodeFile stream %x, L:%d!!\n", (unsigned int)fGifStream , __LINE__);   
   
   fGifStream = new SkFILEWStream((const char *)file);//(SkWStream* )&stream ;

   IMG_DBG("AGIFE:: setEncodeFile stream %x, L:%d!!\n", (unsigned int)fGifStream ,__LINE__);   
   
   fEncodeTarget = 2;
   
   return true ;
   
}   

bool AGifEncoder::setEncodeStream(SkWStream* stream)
{
   IMG_DBG("AGIFE:: setEncodeStream stream %x, w h %d %d , L:%d!!\n", (unsigned int)stream, fWidth, fHeight , __LINE__);   
   
   //unsigned int buffer[4] = { 0x00FF00FF, 0x00FF00FF, 0x00FF00FF, 0x00FF00FF};
   //if(fEncodeTarget) return false ;
   fGifStream = stream ; 
   //IMG_DBG("AGifEncoder:: AGifEncoder(), fGifStream %x!!\n",fGifStream, __LINE__);      
   fEncodeTarget = 1;

   
   return true ;
}
      
      

int write_function(void *data, const char* buffer, int size)
{
  //IMG_DBG("AGIFE::write_function enc %x, data %x, buf %x, size %x, L:%d!!\n", (unsigned int)aGifEncoder,(unsigned int)data,(unsigned int)buffer,size, __LINE__);

  //memcpy(dst_buf_va+fWriteSize,buffer,size);
  //fWriteSize += size ;
  if(aGifEncoder != NULL)
    aGifEncoder->write_fun(data, buffer, size);
  
  return size ;  
}


int AGifEncoder::write_fun(void *data, const char* buffer, int size)
{
  //IMG_DBG("AGIFE::write_fun, data %x, buf %x, size %x, total %x, L:%d!!\n", (unsigned int)data,(unsigned int)buffer,size, fWriteSize,__LINE__);

#if 1  
  if(fGifStream == NULL){    
    IMG_DBG("AGIFE::skip!!null stream, data %x, buf %x, size %x, total %x, L:%d!!\n", (unsigned int)data,(unsigned int)buffer,size, fWriteSize,__LINE__);
  }else if(size>0){
    fGifStream->write(buffer, size) ;
    fGifStream->flush();
    //memcpy(dst_buf_va+fWriteSize,buffer,size);
    fWriteSize += size ;    
  }
#else
  IMG_DBG("AGIFE::skip!!write_fun, data %x, buf %x, size %x, total %x, L:%d!!\n", (unsigned int)data,(unsigned int)buffer,size, fWriteSize,__LINE__);
  
#endif  
  
  
  return size ;  
}



//bool AGifEncoder::encodeBitmap(const SkBitmap& bm)
//bool AGifEncoder::encodeBitmap(unsigned char* src_addr)
bool AGifEncoder::encodeBitmap(unsigned char* src_addr, SkWStream* stream)
{
   //unsigned char* src_addr ;
   //IMG_DBG("AGIFE:: encodeBitmap addr %x, stream %x, L:%d!!\n",src_addr, fGifStream, __LINE__);      
   if(src_addr == NULL){
     IMG_DBG("AGIFE:: encodeBitmap get NULL pointer, L:%d!!\n", __LINE__);      
     return false ;      
   }
   //if(stream != NULL)

     fGifStream = stream ;   
     IMG_DBG("AGIFE:: encodeBitmap addr %x, stream %x, L:%d!!\n",src_addr, stream,  __LINE__);      
      
   
   //IMG_DBG("AGIFE:: encodeBitmap, L:%d!!\n", __LINE__);      
   
   
   return encodeBuffer(src_addr) ;
   
}   

      
bool AGifEncoder::encodeBuffer(unsigned char *src_addr)
{
   
   if( fEncodeFrame == 0){
      fPostTime = 300;
   
       IMG_DBG("AGIFE:: createEncoder %d %d, L:%d!!\n",fWidth, fHeight, __LINE__);      
       //GifEncoder *
       encoder = GifEncoder::createEncoder(
                                           write_function,
                                           //this->write_fun,
                                           NULL,
                                           fWidth,
                                           fHeight,
                                           GifEncoder::ANIMATION,
                                           GIF_ENCODER_MAX_COLOR_BPP
                                           );   
       if(encoder == NULL){
         IMG_DBG("AGIFE:: create encoder fail L:%d!!\n", __LINE__);     
         return false ; 
       }
       IMG_DBG("AGIFE:: createEncoder successfully L:%d!!\n", __LINE__);         
       encoder->addLoopInfo(GifEncoder::LOOP_INFINITE);
       
   }
   if(encoder == NULL){
     IMG_DBG("AGIFE:: create encoder fail L:%d!!\n", __LINE__);     
     return false ; 
   }
     
   IMG_DBG("AGIFE:: appendImage addr %x, time %d L:%d!!\n",(unsigned int)src_addr, fPostTime, __LINE__); 
   if(!encoder->appendImage((const char *)src_addr, fPostTime)){     
     IMG_DBG("AGIFE:: encode fail, L:%d!!\n", __LINE__);
     return false ;
   }else{
     fEncodeFrame ++;
     IMG_DBG("AGIFE:: encode success, L:%d!!\n", __LINE__);
   }   
   
   return true ;
}

bool AGifEncoder::closeGif()
{
   if(!fCloseGif && encoder)
     encoder->close(); 
   fCloseGif  = 1; 
   

   return true;
}



