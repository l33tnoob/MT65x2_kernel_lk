package com.mediatek.common.search;

import com.mediatek.common.search.SearchEngineInfo;
import android.content.Context;
import java.util.List;

public interface IRegionalPhoneSearchEngineExt {
    /*
     * Read data from content provider of Regional Phone Manager to init search engines.
     * If has data and init success return true, else return false.
     */
	public List<SearchEngineInfo> initSearchEngineInfosFromRpm(Context context);
}
