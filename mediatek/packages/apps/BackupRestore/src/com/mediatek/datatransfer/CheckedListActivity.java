package com.mediatek.datatransfer;

import android.app.ListActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.ListAdapter;
import android.widget.ListView;

import com.mediatek.datatransfer.utils.MyLogger;
import com.mediatek.datatransfer.utils.Constants.LogTag;

import java.util.ArrayList;

public class CheckedListActivity extends ListActivity {

    public interface OnCheckedCountChangedListener {
        public void onCheckedCountChanged();
    }

    private final String TAG = "CheckListActivity/";
    private final String SAVE_STATE_UNCHECKED_IDS = "CheckedListActivity/unchecked_ids";
    protected ArrayList<Long> mUnCheckedIds = new ArrayList<Long>();
    protected ArrayList<Long> mDisabledIds = new ArrayList<Long>();
    private ArrayList<OnCheckedCountChangedListener> mListeners;

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.backup);
        Log.i(LogTag.LOG_TAG, TAG + "onCreate");
        if (savedInstanceState != null) {
            restoreInstanceState(savedInstanceState);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.i(LogTag.LOG_TAG, TAG + "onResume");
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.i(LogTag.LOG_TAG, TAG + "onDestroy");
    }

    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        int size = mUnCheckedIds.size();
        long[] array = new long[size];
        for (int position = 0; position < size; position++) {
            array[position] = mUnCheckedIds.get(position);
        }
        outState.putLongArray(SAVE_STATE_UNCHECKED_IDS, array);
    }

    private void restoreInstanceState(final Bundle savedInstanceState) {
        long array[] = savedInstanceState.getLongArray(SAVE_STATE_UNCHECKED_IDS);
        if (array != null) {
            for (long item : array) {
                mUnCheckedIds.add(item);
            }
        }
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        super.onListItemClick(l, v, position, id);
        revertItemCheckedByPosition(position);
    }

    public int getCount() {
        int count = 0;
        ListAdapter adapter = getListAdapter();
        if (adapter != null) {
            count = adapter.getCount();
        }
        return count;
    }

    public int getCheckedCount() {
        return getCount() - getUnCheckedCount();
    }

    public int getUnCheckedCount() {
        return mUnCheckedIds.size();
    }

    public boolean isItemCheckedById(long id) {
        boolean ret = true;
        if (mUnCheckedIds != null && mUnCheckedIds.contains(id)) {
            ret = false;
        }
        return ret;
    }

    public boolean isItemCheckedByPosition(int position) {
        boolean ret = true;
        ListAdapter adapter = getListAdapter();
        if (adapter != null) {
            long itemId = adapter.getItemId(position);
            ret = isItemCheckedById(itemId);
        }
        return ret;
    }

    public void setItemCheckedByPosition(int position, boolean checked) {
        ListAdapter adapter = getListAdapter();
        if (adapter != null && mUnCheckedIds != null) {
            long itemId = adapter.getItemId(position);
            if (checked) {
                mUnCheckedIds.remove(itemId);
            } else {
                if (!mUnCheckedIds.contains(itemId)) {
                    mUnCheckedIds.add(itemId);
                }
            }
            notifyItemCheckChanged();
        }

    }

    public void setItemCheckedById(long id, boolean checked) {
        ListAdapter adapter = getListAdapter();
        if (adapter != null) {
            if (checked) {
                mUnCheckedIds.remove(id);
            } else {
                if (!mUnCheckedIds.contains(id)) {
                    mUnCheckedIds.add(id);
                }
            }
            notifyItemCheckChanged();
        }
    }

    public void revertItemCheckedByPosition(int position) {
        boolean checked = isItemCheckedByPosition(position);
        setItemCheckedByPosition(position, !checked);
    }

    public void setItemDisabledById(long id, boolean bDisabled) {
        if (mDisabledIds == null || mUnCheckedIds == null) {
            return;
        }
        if (!bDisabled) {
            mDisabledIds.remove(id);
        } else {
            if (!mDisabledIds.contains(id)) {
                mDisabledIds.add(id);
            }
            if (!mUnCheckedIds.contains(id)) {
                mUnCheckedIds.add(id);
            }
        }
        notifyItemCheckChanged();
    }

    public void setItemDisabledByPosition(int position, boolean bDisabled) {
        ListAdapter adapter = getListAdapter();
        if (adapter != null) {
            long itemId = adapter.getItemId(position);
            setItemDisabledById(itemId, bDisabled);
        }
    }

    public boolean isItemDisabledByPosition(int position) {
        boolean ret = true;
        ListAdapter adapter = getListAdapter();
        if (adapter != null) {
            long itemId = adapter.getItemId(position);
            ret = isItemDisabledById(itemId);
        }
        return ret;
    }

    public boolean isItemDisabledById(long id) {
        boolean ret = true;
        if (mDisabledIds != null && mDisabledIds.contains(id)) {
            ret = false;
        }
        return ret;
    }

    public int getDisabledCount() {
        return mDisabledIds.size();
    }

    public int getEnabledCount() {
        return getCount() - getDisabledCount();
    }

    /**
     * 
     * @param checked
     * @param notify
     *            is to notify
     */
    public void setAllChecked(boolean checked) {

        mUnCheckedIds.clear();

        if (!checked) {
            ListAdapter adapter = getListAdapter();
            if (adapter != null) {
                int count = adapter.getCount();
                for (int position = 0; position < count; position++) {
                    long itemId = adapter.getItemId(position);
                    mUnCheckedIds.add(itemId);
                }
            }
        } else {
            for (int i = 0; i < mDisabledIds.size(); i++) {
                mUnCheckedIds.add(mDisabledIds.get(i));
            }
        }
        notifyItemCheckChanged();
    }

    public boolean isAllChecked(boolean checked) {

        boolean ret = true;
        if (checked) {
            // is it all checked
            if (getUnCheckedCount() - getDisabledCount() > 0) {
                ret = false;
            }
        } else {
            // is it all unchecked
            if (getCheckedCount() > 0) {
                ret = false;
            }
        }
        return ret;
    }

    public Object getItemByPosition(int position) {
        ListAdapter adapter = getListAdapter();
        if (adapter == null) {
            MyLogger.logE(LogTag.LOG_TAG, TAG + "getItemByPosition: adapter is null, please check");
            return null;
        }
        return adapter.getItem(position);
    }

    /*
     * after data changed(item increase or decrease), must sync unchecked list
     */
    protected void syncUnCheckedItems() {
        ListAdapter adapter = getListAdapter();
        if (adapter == null) {
            mUnCheckedIds.clear();
        } else {
            ArrayList<Long> list = new ArrayList<Long>();
            int count = adapter.getCount();
            for (int position = 0; position < count; position++) {
                long itemId = adapter.getItemId(position);
                if (mUnCheckedIds.contains(itemId)) {
                    list.add(itemId);
                }
            }
            mUnCheckedIds.clear();
            mUnCheckedIds = list;
        }
    }

    protected void registerOnCheckedCountChangedListener(OnCheckedCountChangedListener listener) {
        if (mListeners == null) {
            mListeners = new ArrayList<OnCheckedCountChangedListener>();
        }
        mListeners.add(listener);
    }

    protected void unRegisterOnCheckedCountChangedListener(OnCheckedCountChangedListener listener) {
        if (mListeners != null) {
            mListeners.remove(listener);
        }
    }

    private void notifyItemCheckChanged() {
        if (mListeners != null) {
            for (OnCheckedCountChangedListener listener : mListeners) {
                listener.onCheckedCountChanged();
            }
        }
    }
}
