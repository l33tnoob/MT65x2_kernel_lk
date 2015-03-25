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

/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     T_INTERFACE = 258,
     T_PREFIX = 259,
     T_ROUTE = 260,
     T_RDNSS = 261,
     T_DNSSL = 262,
     T_CLIENTS = 263,
     STRING = 264,
     NUMBER = 265,
     SIGNEDNUMBER = 266,
     DECIMAL = 267,
     SWITCH = 268,
     IPV6ADDR = 269,
     INFINITY = 270,
     T_IgnoreIfMissing = 271,
     T_AdvSendAdvert = 272,
     T_MaxRtrAdvInterval = 273,
     T_MinRtrAdvInterval = 274,
     T_MinDelayBetweenRAs = 275,
     T_AdvManagedFlag = 276,
     T_AdvOtherConfigFlag = 277,
     T_AdvLinkMTU = 278,
     T_AdvReachableTime = 279,
     T_AdvRetransTimer = 280,
     T_AdvCurHopLimit = 281,
     T_AdvDefaultLifetime = 282,
     T_AdvDefaultPreference = 283,
     T_AdvSourceLLAddress = 284,
     T_AdvOnLink = 285,
     T_AdvAutonomous = 286,
     T_AdvValidLifetime = 287,
     T_AdvPreferredLifetime = 288,
     T_DeprecatePrefix = 289,
     T_DecrementLifetimes = 290,
     T_AdvRouterAddr = 291,
     T_AdvHomeAgentFlag = 292,
     T_AdvIntervalOpt = 293,
     T_AdvHomeAgentInfo = 294,
     T_Base6Interface = 295,
     T_Base6to4Interface = 296,
     T_UnicastOnly = 297,
     T_HomeAgentPreference = 298,
     T_HomeAgentLifetime = 299,
     T_AdvRoutePreference = 300,
     T_AdvRouteLifetime = 301,
     T_RemoveRoute = 302,
     T_AdvRDNSSPreference = 303,
     T_AdvRDNSSOpenFlag = 304,
     T_AdvRDNSSLifetime = 305,
     T_FlushRDNSS = 306,
     T_AdvDNSSLLifetime = 307,
     T_FlushDNSSL = 308,
     T_AdvMobRtrSupportFlag = 309,
     T_BAD_TOKEN = 310
   };
#endif
/* Tokens.  */
#define T_INTERFACE 258
#define T_PREFIX 259
#define T_ROUTE 260
#define T_RDNSS 261
#define T_DNSSL 262
#define T_CLIENTS 263
#define STRING 264
#define NUMBER 265
#define SIGNEDNUMBER 266
#define DECIMAL 267
#define SWITCH 268
#define IPV6ADDR 269
#define INFINITY 270
#define T_IgnoreIfMissing 271
#define T_AdvSendAdvert 272
#define T_MaxRtrAdvInterval 273
#define T_MinRtrAdvInterval 274
#define T_MinDelayBetweenRAs 275
#define T_AdvManagedFlag 276
#define T_AdvOtherConfigFlag 277
#define T_AdvLinkMTU 278
#define T_AdvReachableTime 279
#define T_AdvRetransTimer 280
#define T_AdvCurHopLimit 281
#define T_AdvDefaultLifetime 282
#define T_AdvDefaultPreference 283
#define T_AdvSourceLLAddress 284
#define T_AdvOnLink 285
#define T_AdvAutonomous 286
#define T_AdvValidLifetime 287
#define T_AdvPreferredLifetime 288
#define T_DeprecatePrefix 289
#define T_DecrementLifetimes 290
#define T_AdvRouterAddr 291
#define T_AdvHomeAgentFlag 292
#define T_AdvIntervalOpt 293
#define T_AdvHomeAgentInfo 294
#define T_Base6Interface 295
#define T_Base6to4Interface 296
#define T_UnicastOnly 297
#define T_HomeAgentPreference 298
#define T_HomeAgentLifetime 299
#define T_AdvRoutePreference 300
#define T_AdvRouteLifetime 301
#define T_RemoveRoute 302
#define T_AdvRDNSSPreference 303
#define T_AdvRDNSSOpenFlag 304
#define T_AdvRDNSSLifetime 305
#define T_FlushRDNSS 306
#define T_AdvDNSSLLifetime 307
#define T_FlushDNSSL 308
#define T_AdvMobRtrSupportFlag 309
#define T_BAD_TOKEN 310




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1676 of yacc.c  */
#line 136 "gram.y"

	unsigned int		num;
	int			snum;
	double			dec;
	struct in6_addr		*addr;
	char			*str;
	struct AdvPrefix	*pinfo;
	struct AdvRoute		*rinfo;
	struct AdvRDNSS		*rdnssinfo;
	struct AdvDNSSL		*dnsslinfo;
	struct Clients		*ainfo;



/* Line 1676 of yacc.c  */
#line 177 "gram.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


