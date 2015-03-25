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
import android.widget.Toast;

import com.mediatek.engineermode.R;
import com.mediatek.engineermode.ShellExe;
import com.mediatek.xlog.Xlog;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class DesensePllsActivity extends Activity implements
        OnItemClickListener {

    private static final String TAG = "EM/DesensePll";
    private static final String COMMAND_GET_ALL_PLL = "cat /proc/clkmgr/pll_fsel";
    private static final String REGEX_PLL_GROUP = "\\[[\\s\\S]*?\\]";
    private static final String HEX_VALUE_DEFAULT = "-1";
    private String[] mNameArray;
    private int[] mIdArray;
    private String[] mValueArray;
    private List<String> mListData = new ArrayList<String>();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.desense_plls_activity);
        if (null == getAllPllInfo()) {
            Toast.makeText(this, R.string.desense_plls_error, Toast.LENGTH_SHORT).show();
            finish();
            return;
        }
        ListView pllMenuListView = (ListView) findViewById(R.id.pll_menu_listview);
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1, mListData);
        pllMenuListView.setOnItemClickListener(this);
        pllMenuListView.setAdapter(adapter);
    }
    
    @Override
    protected void onStart() {
        super.onStart();
        if (!updateData()) {
            Toast.makeText(this, R.string.desense_plls_updatefail,
                    Toast.LENGTH_SHORT).show();
        }
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position,
            long id) {
        Intent intent = new Intent(this, PllDetailActivity.class);
        intent.putExtra(PllDetailActivity.PLL_DETAIL_ITEM_NAME,
                mNameArray[position]);
        intent.putExtra(PllDetailActivity.PLL_DETAIL_ITEM_ID,
                mIdArray[position]);
        intent.putExtra(PllDetailActivity.PLL_DETAIL_ITEM_VALUE,
                mValueArray[position]);
        startActivity(intent);
    }

    private boolean updateData() {
        boolean result = false;
        ArrayList<PllStruct> list = getAllPllInfo();
        if (null != list) {
            mListData.clear();
            int size = list.size();
            Xlog.v(TAG, "PLLGetAllInfo list size = " + size);
            mNameArray = new String[size];
            mIdArray = new int[size];
            mValueArray = new String[size];
            for (int i = 0; i < size; i++) {
                PllStruct item = list.get(i);
                mNameArray[i] = item.mName;
                mIdArray[i] = item.mId;
                mValueArray[i] = item.mHexVal;
                mListData.add(item.mName);
                Xlog.v(TAG, "PLLGetAllInfo list.get(i) = " + item.toString());
            }
            result = true;
        }
        return result;
    }

    private static ArrayList<PllStruct> getAllPllInfo() {
        ArrayList<PllStruct> listResult = null;
        try {
            if (ShellExe.RESULT_SUCCESS == ShellExe
                    .execCommand(COMMAND_GET_ALL_PLL)) {
                String info = ShellExe.getOutput();
                Pattern p = Pattern.compile(REGEX_PLL_GROUP);
                Matcher m = p.matcher(info);
                int index = 0;
                listResult = new ArrayList<PllStruct>();
                String string = null;
                PllStruct struct = null;
                try {
                    while (m.find()) {
                        string = m.group();
                        if (null == string) {
                            continue;
                        }
                        if (index == 0) {
                            struct = new PllStruct();
                            struct.mId = Integer.valueOf(string.substring(1,
                                    string.length() - 1));
                            index++;
                        } else if (index == 1) {
                            struct.mName = string.substring(1,
                                    string.length() - 1).trim();
                            index++;
                        } else {
                            if (string.contains(HEX_VALUE_DEFAULT)) {
                                struct.mHexVal = HEX_VALUE_DEFAULT;
                            } else {
                                struct.mHexVal = string.substring(3, string
                                        .length() - 1);
                            }
                            index = 0;
                            listResult.add(struct);
                        }
                    }
                } catch (IllegalStateException e) {
                    Xlog.w(TAG, "getAllPllInfo IllegalStateException: "
                            + e.getMessage());
                } catch (NumberFormatException e) {
                    Xlog.w(TAG, "getAllPllInfo NumberFormatException: "
                            + e.getMessage());
                } catch (IndexOutOfBoundsException e) {
                    Xlog.w(TAG, "getAllPllInfo IndexOutOfBoundsException: "
                            + e.getMessage());
                }
            }
        } catch (IOException e) {
            Xlog.w(TAG, "getAllPllInfo IOException: " + e.getMessage());
        }
        return listResult;
    }

    static class PllStruct {
        public int mId;
        public String mName;
        public String mHexVal;

        @Override
        public String toString() {
            return String.format("PllStruct: mId: %d, mName: %s, mHexVal: %s",
                    mId, mName, mHexVal);
        }
    }

}
