package com.mediatek.engineermode.desense;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import com.mediatek.engineermode.ChipSupport;
import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

import java.util.ArrayList;
import java.util.List;

public class DesenseActivity extends Activity implements OnItemClickListener {

    public static final String TAG = "EM/DesenseActivity";
    private static final String ITEM_PLLS = "PLLs";
    private static final String ITEM_FREQHOPPING = "Frequency Hopping Setting";
    private static final String ITEM_MEMPLL = "MEMPLL Setting";
    private static final String ITEMS[] = { ITEM_PLLS, ITEM_FREQHOPPING,
            ITEM_MEMPLL };
    private List<String> mListData;

    private ListView mListView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.desense_activity);
        mListView = (ListView) findViewById(R.id.desense_listview);
        mListView.setOnItemClickListener(this);
        mListData = getData();
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1, mListData);
        mListView.setAdapter(adapter);
    }

    private List<String> getData() {
        List<String> items = new ArrayList<String>();
        for (int i = 0; i < ITEMS.length; i++) {
            items.add(ITEMS[i]);
        }
        if (ChipSupport.isChipInSet(ChipSupport.CHIP_657X_SERIES_NEW)) {
            items.remove(ITEMS[2]);
        }
        return items;
    }

    public void onItemClick(AdapterView<?> parent, View view, int position,
            long id) {
        Xlog.v(TAG, mListData.get(position) + " item is clicked!");
        if (ITEM_PLLS.equals(mListData.get(position))) {
            startActivity(new Intent(this, DesensePllsActivity.class));
        } else if (ITEM_FREQHOPPING.equals(mListData.get(position))) {
        	if (ChipSupport.isChipInSet(ChipSupport.CHIP_657X_SERIES_NEW)) {
        		startActivity(new Intent(this, FreqHoppingSetting6572.class));
        	} else {
                startActivity(new Intent(this, FreqHoppingSet.class));
        	}
        	
        } else if (ITEM_MEMPLL.equals(mListData.get(position))) {
            startActivity(new Intent(this, MemPllSet.class));
        } else {
            Xlog.v(TAG, "other item is clicked!");
        }
    }

}
