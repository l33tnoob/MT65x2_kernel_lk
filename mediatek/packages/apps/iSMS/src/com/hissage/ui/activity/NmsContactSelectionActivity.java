package com.hissage.ui.activity;

import java.util.ArrayList;
import java.util.HashSet;

import android.app.ActionBar;
import android.app.Activity;
import android.app.LoaderManager;
import android.content.Context;
import android.content.CursorLoader;
import android.content.Intent;
import android.content.Loader;
import android.content.res.Configuration;
import android.database.Cursor;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.provider.ContactsContract.Contacts;
import android.text.TextUtils;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.LinearLayout.LayoutParams;
import android.widget.ListView;
import android.widget.PopupMenu;
import android.widget.ProgressBar;
import android.widget.SearchView;
import android.widget.SearchView.OnCloseListener;
import android.widget.SearchView.OnQueryTextListener;
import android.widget.TextView;
import android.widget.Toast;
import com.hissage.R;
import com.hissage.contact.NmsContactManager;
import com.hissage.contact.NmsUIContact;
import com.hissage.ui.adapter.NmsContactSelectionAdapter;
import com.hissage.ui.view.NmsPinnedHeaderListView;
import com.hissage.util.log.NmsLog;

public class NmsContactSelectionActivity extends Activity implements OnQueryTextListener,
        OnCloseListener {

    public enum SelectionMode {
        SINGLE, MUTIL
    }

    public String Tag = "NmsContactSelectionActivity";

    private final static int HANDLER_DATA_READY = 1002;

    public static String SELECTMAX = "SELECTMAX";
    public static String SELECTEDID = "SELECTID";
    public static String LOADTYPE = "LOADTYPE";
    public static String SIMID = "SIMID";

    public static int DEFAUT_SIMID = -9;
    public static String CONTACTTAG = "contactId";

    private Context mContext;

    private SelectionMode mSelection;

    private boolean mSearchMode;
    private boolean isReady = false;
    private String mQueryString;
    private SearchView mSearchView;
    private int mSelectMax;
    private int mLoadType;
    private int msimId;

    private TextView tvEmpty;
    private TextView emptyView;
    private NmsPinnedHeaderListView mLvRecipient;
    private NmsContactSelectionAdapter mAdapter;
    private Button mBtnSelect;
    private HashSet<String> mSelectId;

    private ArrayList<NmsUIContact> mContacList = new ArrayList<NmsUIContact>();

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);

            NmsLog.trace(Tag, "handler received msg type: " + msg.what);
            switch (msg.what) {
            case HANDLER_DATA_READY:
                initAllList();
                setEmptyView();
                isReady = true;
                break;
            default:
                break;
            }
        }
    };

    private void setEmptyView() {
        if (emptyView == null) {
            emptyView = new TextView(this);
            emptyView.setLayoutParams(new LinearLayout.LayoutParams(LayoutParams.FILL_PARENT,
                    LayoutParams.FILL_PARENT));
            emptyView.setGravity(Gravity.CENTER_HORIZONTAL | Gravity.TOP);
            emptyView.setTextColor(Color.BLACK);

            emptyView.setPadding(0, 40, 0, 0);
            ViewGroup vg = (ViewGroup) mLvRecipient.getParent();
            if (vg != null)
                vg.addView(emptyView);
        }

        ProgressBar pbWait = (ProgressBar) this.findViewById(R.id.pb_wait);
        View view = (View) findViewById(R.id.ll_prompt);

        if (mLoadType == NmsContactManager.TYPE_HISSAGE) {
            emptyView.setText(R.string.STR_NMS_SELECT_CONTAT_EMPTY);
        } else if (mLoadType == NmsContactManager.TYPE_ALL) {
            emptyView.setText(R.string.STR_NMS_EMPTY);
        } else {
            emptyView.setText(R.string.STR_NMS_EMPTY);
        }
        pbWait.setVisibility(View.GONE);
        view.setVisibility(View.GONE);

        mLvRecipient.setEmptyView(emptyView);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.contact_list);
        mContext = this;

        NmsLog.trace(Tag, "Start ContactListActivity...");

        mSelectMax = getIntent().getIntExtra(SELECTMAX, 0);

        msimId = getIntent().getIntExtra(SIMID, DEFAUT_SIMID);
        if (mSelectMax > 1) {
            mSelection = SelectionMode.MUTIL;
        } else {
            mSelection = SelectionMode.SINGLE;
        }

        mLoadType = getIntent().getIntExtra(LOADTYPE, NmsContactManager.TYPE_ALL);
        NmsLog.trace(Tag, "Max contact selected:" + mSelectMax);

        mSelectId = (HashSet<String>) getIntent().getSerializableExtra((SELECTEDID));

        mLvRecipient = (NmsPinnedHeaderListView) this.findViewById(R.id.lv_recipient);
        mLvRecipient.setScrollBarStyle(ListView.SCROLLBARS_OUTSIDE_OVERLAY);

        mLvRecipient.setFastScrollEnabled(true);

        mLvRecipient.setFastScrollAlwaysVisible(true);

        mLvRecipient.setVerticalScrollbarPosition(View.SCROLLBAR_POSITION_RIGHT);

        mLvRecipient.setDividerHeight(0);

        // We manually save/restore the listview state
        mLvRecipient.setSaveEnabled(false);
        mLvRecipient.setPinnedHeaderView(getLayoutInflater().inflate(R.layout.list_section,
                mLvRecipient, false));

        int leftPadding = 0;
        int rightPadding = 0;
        rightPadding = mContext.getResources().getDimensionPixelOffset(
                R.dimen.list_select_right_padding);

        mLvRecipient.setPadding(leftPadding, mLvRecipient.getPaddingTop(), rightPadding,
                mLvRecipient.getPaddingBottom());
        View emptyView = (View) findViewById(R.id.ll_prompt);
        mLvRecipient.setEmptyView(emptyView);
        tvEmpty = (TextView) this.findViewById(R.id.tv_prompt);

        loadContactData();

        setupActionBar();

        getLoaderManager().initLoader(0, null, mLoaderCallbacks);
    }

    private void loadContactData() {
        NmsLog.trace(Tag, "thread to load the list info");

        new Thread() {
            @Override
            public void run() {
                super.run();
                mContacList = NmsContactManager.getInstance(mContext).getContacts(mLoadType);
                mHandler.sendEmptyMessage(HANDLER_DATA_READY);
            }
        }.start();
    }

    private boolean check(int index) {
        if (mAdapter.checkSelection(index)) {
            return true;
        }

        if (mSelectId == null) {
            if (mAdapter.getSelectedCount() < mSelectMax) {
                return true;
            }
        } else {
            if (mSelectId.size() + mAdapter.getSelectedCount() < mSelectMax + 1) {
                return true;
            }
        }

        return false;
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();

        inflater.inflate(R.menu.search_menu, menu);

        MenuItem item = menu.findItem(R.id.menu_conatact_search);

        item.setVisible(!mSearchMode);

        return true;
    }

    private void cancel() {
        this.finish();
    }

    private void finishSelected() {
        if (!isReady) {
            return;
        }

        if (mLoadType == NmsContactManager.TYPE_HISSAGE) {
            String[] contactId = mAdapter.getSelectContactId();
            if (contactId.length > 0) {
                Intent i = new Intent();
                i.putExtra(CONTACTTAG, contactId);
                i.putExtra(SIMID, msimId);
                this.setResult(RESULT_OK, i);
                this.finish();
            } else {
                Toast.makeText(this, R.string.STR_NMS_SELECT_MEMBER_EMPTY, Toast.LENGTH_SHORT)
                        .show();
            }
        } else {
            String selectionPhone = mAdapter.getSelectPhoneNumber();
            if (!TextUtils.isEmpty(selectionPhone)) {
                Intent i = new Intent();
                i.putExtra(CONTACTTAG, selectionPhone);
                this.setResult(RESULT_OK, i);
                this.finish();
            } else {
                Toast.makeText(this, R.string.STR_NMS_SELECT_MEMBER_EMPTY, Toast.LENGTH_SHORT)
                        .show();
            }
        }
    }

    private void setSelectCount(int count) {
        String s = String.format(getString(R.string.STR_NMS_SELECTED), String.valueOf(count));
        mBtnSelect.setText(s);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        NmsLog.trace(Tag, "Options item selected:" + item.getTitle());

        if (item.getItemId() == R.id.menu_conatact_search) {
            if (isReady) {
                setSearchMode(true);
            } else {
                return false;
            }

        } else {
            NmsLog.trace(Tag, "Options item selected is unknown");
        }

        return false;
    }

    public void initAllList() {

        if (null != mSelectId && mSelectId.size() != 0) {
            ArrayList<NmsUIContact> tempList = new ArrayList<NmsUIContact>();
            for (NmsUIContact uiC : mContacList) {
                String id = String.valueOf(uiC.getEngineContactId());
                if (!mSelectId.contains(id)) {
                    tempList.add(uiC);
                }
            }

            mContacList = tempList;
        }

        mAdapter = new NmsContactSelectionAdapter(this, mLvRecipient, mContacList, mSelection,
                msimId);

        // setupActionBar();

        mLvRecipient.setAdapter(mAdapter);
        mLvRecipient.setOnScrollListener(mAdapter);

        mLvRecipient.setOnItemClickListener(new OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                // TODO Auto-generated method stub
                if (mSelection == SelectionMode.SINGLE) {
                    selectContact(arg2);
                } else {
                    if (check(arg2)) {
                        mAdapter.check(arg2);
                        setSelectCount(mAdapter.getSelectedCount());
                    } else {
                        Toast.makeText(mContext, R.string.STR_NMS_SELECT_MEMBER_FULL,
                                Toast.LENGTH_SHORT).show();
                    }
                }
            }
        });
    }

    public void selectContact(int index) {

        long sysContactId = mAdapter.selectSingleContact(index);
        Intent intent = new Intent();
        intent.putExtra(SELECTEDID, sysContactId);

        setResult(RESULT_OK, intent);

        this.finish();

    }

    private void setupActionBar() {
        ActionBar actionBar = getActionBar();

        actionBar.setDisplayShowTitleEnabled(false);
        actionBar.setDisplayShowHomeEnabled(true);
        actionBar.setDisplayHomeAsUpEnabled(false);
        actionBar.setDisplayUseLogoEnabled(true);
        actionBar.setLogo(R.drawable.contacts);

        ViewGroup v = (ViewGroup) LayoutInflater.from(this).inflate(R.layout.action_bar_contact,
                null);
        actionBar.setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM, ActionBar.DISPLAY_SHOW_CUSTOM);
        actionBar.setCustomView(v, new ActionBar.LayoutParams(ActionBar.LayoutParams.MATCH_PARENT,
                ActionBar.LayoutParams.MATCH_PARENT, Gravity.CENTER_VERTICAL | Gravity.LEFT));

        Button btnCancel = ((Button) v.findViewById(R.id.btn_cancel));
        btnCancel.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                cancel();
            }
        });

        Button tvDone = ((Button) v.findViewById(R.id.btn_done));
        tvDone.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                finishSelected();
            }
        });

        mBtnSelect = ((Button) v.findViewById(R.id.btn_select));
        mSearchView = (SearchView) v.findViewById(R.id.search_view);

        if (mSearchMode) {
            mBtnSelect.setVisibility(View.GONE);
            mSearchView.setVisibility(View.VISIBLE);
            mSearchView.setIconifiedByDefault(true);
            mSearchView.setQueryHint(mContext.getString(R.string.STR_NMS_CONTACT_SEARCH));
            mSearchView.setOnQueryTextListener(this);
            mSearchView.setOnCloseListener(this);
            mSearchView.setQuery(null, false);

        } else {
            mBtnSelect.setVisibility(View.VISIBLE);
            mSearchView.setVisibility(View.GONE);

            if (mAdapter != null && mAdapter.getSelectedCount() > 0) {
                setSelectCount(mAdapter.getSelectedCount());
            }

            mBtnSelect.setOnClickListener(new OnClickListener() {

                @Override
                public void onClick(View v) {
                    // TODO Auto-generated method stub
                    PopupMenu popup = new PopupMenu(mContext, v);
                    popup.getMenuInflater().inflate(R.menu.select_menu, popup.getMenu());

                    popup.setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener() {
                        public boolean onMenuItemClick(MenuItem item) {
                            if (item.getItemId() == R.id.menu_select_all) {
                                setContactSelectMode(0);
                            } else if (item.getItemId() == R.id.menu_select_cancel) {
                                setContactSelectMode(1);
                            } else {
                                NmsLog.trace(Tag, "Options item selected is unknown");
                            }

                            return false;
                        }
                    });

                    popup.show();
                }
            });
        }
    }

    private void setContactSelectMode(int mode) {
        if (!isReady) {
            return;
        }

        int num = 0;
        if (mode == 0) {
            if ((mSelectId == null ? 0 : mSelectId.size()) + mAdapter.getCount() <= mSelectMax) {
                num = mAdapter.checkAll(true);
            } else {
                Toast.makeText(this, R.string.STR_NMS_SELECT_MEMBER_FULL, Toast.LENGTH_SHORT)
                        .show();
                return;
            }
        } else {
            num = mAdapter.checkAll(false);
        }

        setSelectCount(num);
    }

    public void setSearchMode(boolean flag) {
        if (mSearchMode != flag) {
            mSearchMode = flag;
            setupActionBar();
            if (mSearchView == null) {
                return;
            }
            if (mSearchMode) {
                setFocusOnSearchView();
            } else {
                mSearchView.setQuery(null, false);
                mAdapter.exitSearch();
                if (mAdapter.getCount() == 0) {
                    if (mLoadType == NmsContactManager.TYPE_HISSAGE) {
                        emptyView.setText(R.string.STR_NMS_SELECT_CONTAT_EMPTY);
                    } else if (mLoadType == NmsContactManager.TYPE_ALL) {
                        emptyView.setText(R.string.STR_NMS_EMPTY);
                    } else {
                        emptyView.setText(R.string.STR_NMS_EMPTY);
                    }
                }
            }
        }

        invalidateOptionsMenu();
    }

    private void setFocusOnSearchView() {
        mSearchView.requestFocus();
        mSearchView.setIconified(false);
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        invalidateOptionsMenu();
    }

    @Override
    public boolean onClose() {
        // TODO Auto-generated method stub
        InputMethodManager imm = (InputMethodManager) mContext
                .getSystemService(Context.INPUT_METHOD_SERVICE);
        if (imm != null) {
            imm.hideSoftInputFromWindow(mSearchView.getWindowToken(), 0);
        }
        mSearchView.clearFocus();

        setSearchMode(false);
        return false;
    }

    @Override
    public boolean onQueryTextChange(String queryString) {
        // TODO Auto-generated method stub
        if (queryString.equals(mQueryString)) {
            return false;
        }
        mQueryString = queryString;
        if (!mSearchMode) {
            if (!TextUtils.isEmpty(queryString)) {
                setSearchMode(true);
            }
        } else {
            // int n = mAdapter.search(mQueryString);
            // if (n == 0) {
            // emptyView.setText(R.string.STR_NMS_SELECT_NO_RESULT);
            // }
            getLoaderManager().restartLoader(0, null, mLoaderCallbacks);
        }

        return true;
    }

    @Override
    public void onBackPressed() {
        if (mSearchMode) {
            setSearchMode(false);
        } else {
            if (!isReady) {
                NmsContactManager.getInstance(mContext).setCancel(true);
            }
            super.onBackPressed();
        }
    }

    @Override
    public boolean onQueryTextSubmit(String query) {
        // TODO Auto-generated method stub
        return false;
    }

    LoaderManager.LoaderCallbacks<Cursor> mLoaderCallbacks = new LoaderManager.LoaderCallbacks<Cursor>() {
        @Override
        public Loader<Cursor> onCreateLoader(int id, Bundle args) {
            // TODO Auto-generated method stub

            Uri baseUri;
            if (!TextUtils.isEmpty(mQueryString)) {
                baseUri = Uri.withAppendedPath(Contacts.CONTENT_FILTER_URI,
                        Uri.encode(mQueryString));
            } else {
                baseUri = Contacts.CONTENT_URI;
            }
            return new CursorLoader(NmsContactSelectionActivity.this, baseUri,
                    new String[] { Contacts._ID }, null, null, null);
        }

        @Override
        public void onLoadFinished(Loader<Cursor> loader, Cursor data) {
            // TODO Auto-generated method stub
            if (mSearchMode) {
                if (!TextUtils.isEmpty(mQueryString)) {
                    if (mAdapter.search(data, mQueryString) == 0) {
                        emptyView.setText(R.string.STR_NMS_SELECT_NO_RESULT);
                    }
                } else {
                    if (data != null && !data.isClosed()) {
                        data.close();
                    }
                    if (mAdapter.search(null, mQueryString) == 0) {
                        emptyView.setText(R.string.STR_NMS_SELECT_NO_RESULT);
                    }
                }
            } else {
                if (data != null && !data.isClosed()) {
                    data.close();
                }
            }
        }

        @Override
        public void onLoaderReset(Loader<Cursor> loader) {
            // TODO Auto-generated method stub
        }
    };
}
