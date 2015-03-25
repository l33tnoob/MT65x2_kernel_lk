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

#ifndef SQLITE3_CUSTOM_H
#define SQLITE3_CUSTOM_H

#include <sqlite3.h>

namespace android {

/**
 * add for CT NEW FEATURE to show items follow order 09-004
 */
#define DS_FULL_INITIALS_MATCH          20
#define DS_FULL_FULLSPEL_MATCH          16

#define DS_INITIALS_MATCH               17
#define DS_FULLSPELL_MATCH              15
#define DS_NUMBER_MATCH                 13
#define DS_MIXED_MATCH                  11
#define DS_NOT_MATCH                    0

/**
 * FUNCTION POINTER
 *  FUNC_MATCH
 *
 * DESCRIPTION
 *  Function pointer for dialer search algorithm. It is used to do matching and
 *  get the matched positions and matched count, which are used to highlight
 *  matched characters.
 *  Search resource can be names or numbers.
 *  The matching rules should be consistent with the rules used in FUNC_FILTER.
 *
 * PARAMETERS:
 *  searchKey           [IN]  user input string.
 *  matchText          	[IN]  search key contain contact name or phone number
 *  matchOffset         [IN]  name character offset in the corresponding string
 *  						  NULL if the matchType is DS_SEARCH_TYPE_NUMBER.
 *  matchType         	[IN]  A flag indicates the search is to query names or numbers.
 *                            Its value should be DS_SEARCH_TYPE_NAME or DS_SEARCH_TYPE_NUMBER,
 *                            which are defined in DialerSearchUtils.h.
 *  res      			[OUT] matched positions sequence
 *  resLen				[OUT] this count of matched substrings
 *
 * RETURNS:
 *  DS_RET_OK           Defined in DialerSearchUtils.h, its values is 0.
 *                      If get matched results successfully, return DS_RET_OK.
 *  DS_RET_ERR          Defined in DialerSearchUtils.h, its values is -1.
 *                      If error happens when to get matched results, return DS_RET_ERR.
 *  DS_RET_NO_RESULT    Defined in DialerSearchUtils.h, its values is -2.
 *                      If there is no matched result, return this DS_RET_NO_RESULT.
 */
typedef int (*FUNC_MATCH) (const char *searchKey, \
		const char *matchText, \
		const char *matchOffset, \
		int matchType,  \
		char *res, \
		int *resLen);
/**
 * FUNCTION POINTER
 *  FUNC_FILTER
 *
 * DESCRIPTION
 *  Function pointer for dialer search algorithm. It is used to do matching and
 *  filter the un-matched results.
 *  Search resource can be names or numbers.
 *
 * PARAMETERS:
 *  searchKey           [IN]  user input string.
 *  matchText          	[IN]  search key contain contact name or phone number
 *  matchOffset         [IN]  name character offset in the corresponding string
 *  						  NULL if the matchType is DS_SEARCH_TYPE_NUMBER.
 *  matchType         	[IN]  A flag indicates the search is to query names or numbers.
 *                            Its value should be DS_SEARCH_TYPE_NAME or DS_SEARCH_TYPE_NUMBER,
 *                            which are defined in DialerSearchUtils.h.
 * RETURNS:
 *  DS_RET_OK           Defined in DialerSearchUtils.h, its values is 0.
 *                      If get matched results successfully, return DS_RET_OK.
 *  DS_RET_ERR          Defined in DialerSearchUtils.h, its values is -1.
 *                      If error happens when to get matched results, return DS_RET_ERR.
 *  DS_RET_NO_RESULT    Defined in DialerSearchUtils.h, its values is -2.
 *                      If there is no matched result, return this DS_RET_NO_RESULT.
 */
typedef int (*FUNC_FILTER) (const char *searchKey, \
		const char *matchText, \
		const char *matchOffset, \
		int matchType);
/**
 * FUNCTION POINTER
 *  FUNC_NAME_MATCH_PTR
 *
 * DESCRIPTION
 *  Function pointer for dialer search algorithm. It is used to do matching on
 *  contacts' names.
 *
 * PARAMETERS:
 *  searchKey           [IN]  user input string.
 *  matchText          	[IN]  search key contain contact name or phone number
 *  matchOffset         [IN]  name character offset in the corresponding string
 *  						  NULL if the matchType is DS_SEARCH_TYPE_NUMBER.
 *  matchType         	[IN]  A flag indicates the search is to query names or numbers.
 *                            Its value should be DS_SEARCH_TYPE_NAME or DS_SEARCH_TYPE_NUMBER,
 *                            which are defined in DialerSearchUtils.h.
 *  needResult			[IN]  A flag indicates whether this fuction should return matched info.
 *  						  If needResult is true, res and resLen must be filled.
 *  res      			[OUT] matched positions sequence
 *  resLen				[OUT] this count of matched substrings
 *
 *  RETURNS:
 *  DS_RET_OK           Defined in DialerSearchUtils.h, its values is 0.
 *                      If get matched results successfully, return DS_RET_OK.
 *  DS_RET_ERR          Defined in DialerSearchUtils.h, its values is -1.
 *                      If error happens when to get matched results, return DS_RET_ERR.
 *  DS_RET_NO_RESULT    Defined in DialerSearchUtils.h, its values is -2.
 *                      If there is no matched result, return this DS_RET_NO_RESULT.
 */
typedef int (*FUNC_NAME_MATCH_PTR)(const char *searchKey, \
		const char *matchText, \
		const char *matchOffset, \
		char *res, \
		int *resLen, \
		bool needResult);
/**
 * FUNCTION POINTER
 *  FUNC_NUMBER_MATCH_PTR
 *
 * DESCRIPTION
 *  Function pointer for dialer search algorithm. It is used to do matching on
 *  contacts' phone numbers.
 *
 * PARAMETERS:
 *  searchKey           [IN]  user input string.
 *  matchText          	[IN]  search key contain contact name or phone number
 *  needResult			[IN]  A flag indicates whether this fuction should return matched info.
 *  						  If needResult is true, res and resLen must be filled.
 *  res      			[OUT] matched positions sequence
 *  resLen				[OUT] this count of matched substrings
 *
 * RETURNS:
 *  DS_RET_OK           Defined in DialerSearchUtils.h, its values is 0.
 *                      If get matched results successfully, return DS_RET_OK.
 *  DS_RET_ERR          Defined in DialerSearchUtils.h, its values is -1.
 *                      If error happens when to get matched results, return DS_RET_ERR.
 *  DS_RET_NO_RESULT    Defined in DialerSearchUtils.h, its values is -2.
 *                      If there is no matched result, return this DS_RET_NO_RESULT.
 */
typedef int (*FUNC_NUMBER_MATCH_PTR)(const char *searchKey, \
		const char *matchText, \
		char *res, \
		int *resLen, \
		bool needResult);

/**
 * FUNCTION
 *  create_dialer_search_function
 *
 * DESCRIPTION
 * customer register dialer search matching algorithm. It contains matching function
 *  and filter function, and they both apply to contacts names and nubmers.
 *
 * PARAMETERS:
 *  func1           [IN]  matching function pointer.
 *  func2          	[IN]  filter function pinter.
 *
 * RETURNS:
 *  bool
 */
extern bool create_dialer_search_function(FUNC_MATCH func1, FUNC_FILTER func2);

/**
 * FUNCTION
 *  create_name_match_function
 *
 * DESCRIPTION
 * Create custom name matching function.
 *
 * PARAMETERS:
 *  func           [IN]  name matching function pinter. Null if its values is zero and it
 *  					 would take the default function according to defaultFun value.
 *  defaultFun     [IN]  A flag indicats which default name matching function is used.
 *                       The valid values are 0,1.
 *                       If 0, a name matching function
 *                       If 1, A new name matching function 
 *
 * RETURNS:
 *  bool
 */
extern bool create_name_match_function(FUNC_NAME_MATCH_PTR func, int defaultFun);

/**
 * FUNCTION
 *  create_number_match_function
 *
 * DESCRIPTION
 *  Create custom number matching function.
 *  Besides, it also supports to choose which predefined function is for use
 *  if custom function pointer is NULL(0).
 *
 *  However, it should be carefull to choose middle match function for use
 *  since this function may increase the results count and reduce the query
 *  speed.
 *
 * PARAMETERS:
 *  func           [IN]  number matching function pinter.
 *  defaultFun     [IN]  A flag indicats which number matching function is used.
 *  					 if 0, get a function that does not support number middle match,
 *  					 and this is also the default function.
 *                       if 1, get a function that supports number middle match.
 *                       if other values, the default function is used.
 *
 * RETURNS:
 *  bool            return true if create custom functions successfully.
 */
extern bool create_number_match_function(FUNC_NUMBER_MATCH_PTR func, int defaultFun);

/**
 * FUNCTION
 *  register_dialer_search_sqlite_custom_functions
 *
 * DESCRIPTION
 *	Rigister matching functions for Dialer search algorithm
 *
 * PARAMETERS:
 *  void
 *
 * RETURNS:
 *  void
 */
void register_dialer_search_local_functions();

}//android namespace

/**
 * FUNCTION
 *  register_dialer_search_custom_functions
 *
 * DESCRIPTION
 *	This function is used to register original SQLite3 functions for dialer
 *	search.
 *
 * PARAMETERS:
 *  handle           [IN] A pinter connects to a SQLite3 instance.
 *
 * RETURNS:
 *  int
 */

#ifdef __cplusplus
extern "C" {
#endif

int  register_dialer_search_custom_functions(sqlite3 * handle);

#ifdef __cplusplus
} // extern "C"
#endif
#endif
