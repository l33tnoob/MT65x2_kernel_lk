/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.FMRadio;

import android.app.ActionBar;
import android.app.Activity;
import android.app.Dialog;
import android.app.LoaderManager.LoaderCallbacks;
import android.content.Context;
import android.content.CursorLoader;
import android.content.Intent;
import android.content.Loader;
import android.database.Cursor;
import android.media.AudioManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.ContextMenu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.AdapterView;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.SimpleCursorAdapter;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.FMRadio.FMRadioService.OnExitListener;
import com.mediatek.FMRadio.dialogs.AddFavoriteDialog;
import com.mediatek.FMRadio.dialogs.DeleteFavoriteDialog;
import com.mediatek.FMRadio.dialogs.EditFavoriteDialog;
import com.mediatek.FMRadio.ext.IProjectStringExt;
/**
 * This class interact with user, provider edit channel information, such as add to favorite,
 * edit favorite, delete from favorite
 *
 */

public class FMRadioFavorite extends Activity implements LoaderCallbacks<Cursor>,
    AddFavoriteDialog.AddFavoriteListener, EditFavoriteDialog.EditFavoriteListener,
    DeleteFavoriteDialog.DeleteFavoriteListener {
    public static final String TAG = "FmRx/Favorite"; // log tag
    
    private static final String ADD_FAVORITE = "AddFavorite";
    private static final String EDIT_FAVORITE = "EditFavorite";
    private static final String DELETE_FAVORITE = "DeleteFavorite";
    
    
    public static final String ACTIVITY_RESULT = "ACTIVITY_RESULT"; // activity result
    
    private static final int CONTMENU_ID_EDIT = 1; // content menu id edit
    private static final int CONTMENU_ID_DELETE = 2; // content menu id delete
    private static final int CONTMENU_ID_ADD = 3; // content menu id add
    
    private static final String FAVORITE_NAME = "FAVORITE_NAME";
    private static final String FAVORITE_FREQ = "FAVORITE_FREQ";
    
    private ListView mLvFavorites = null; // list view 
    private ChannelListAdapter mAdapter = null; // adapter use to update UI
    
    private String mDlgStationName = null; // Record the long clicked station name.
    private int mDlgStationFreq = 0; // Record the long clicked station frequency..
    private Context mContext = null; // application context
    
    IProjectStringExt mProjectStringExt = null;

    private OnExitListener mExitListener = null;

    /**
     * on create 
     * @param savedInstanceState save instance state
     */
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        LogUtils.d(TAG, "begin FMRadioFavorite.onCreate");
        // Bind the activity to FM audio stream.
        setVolumeControlStream(AudioManager.STREAM_FM);
        getWindow().requestFeature(Window.FEATURE_ACTION_BAR);
        setContentView(R.layout.favorite);
        // display action bar and navigation button
        ActionBar actionBar = getActionBar();
        actionBar.setTitle(R.string.favorite_manager);
        actionBar.setDisplayHomeAsUpEnabled(true);
        mContext = getApplicationContext();
        mProjectStringExt = ExtensionUtils.getExtension(mContext);
        if (savedInstanceState != null) {
            mDlgStationName = savedInstanceState.getString(FAVORITE_NAME);
            mDlgStationFreq = savedInstanceState.getInt(FAVORITE_FREQ);
        }
        
        mAdapter = new ChannelListAdapter(
                this,
                R.layout.simpleadapter, 
                null, 
                new String[] { FMRadioStation.Station.COLUMN_STATION_TYPE, FMRadioStation.Station.COLUMN_STATION_FREQ, 
                        FMRadioStation.Station.COLUMN_STATION_NAME },
                new int[] {R.id.lv_station_type, R.id.lv_station_freq, R.id.lv_station_name});
        mLvFavorites = (ListView)findViewById(R.id.station_list);
        TextView emptyView = (TextView) findViewById(R.id.empty);
        mLvFavorites.setEmptyView(emptyView);
        mLvFavorites.setAdapter(mAdapter); // set adapter
        getLoaderManager().initLoader(0, null, this); // initial loader
        mLvFavorites.setOnItemClickListener(
            new AdapterView.OnItemClickListener() {
                /**
                 * click list item will finish activity and pass value to other activity
                 * @param parent adapter view
                 * @param view item view
                 * @param position current position
                 * @param id current id
                 */
                public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                    // Set the selected frequency to main UI and finish the favorite manager.
                    TextView textView = (TextView) view.findViewById(R.id.lv_station_freq);
                    float frequency = 0;
                    try {
                        frequency = Float.parseFloat(textView.getText().toString());
                    } catch (NumberFormatException e) {
                        e.printStackTrace();
                    }       
                    Intent intentResult = new Intent();
                    intentResult.putExtra(ACTIVITY_RESULT, FMRadioUtils.computeStation(frequency));
                    setResult(RESULT_OK, intentResult);
                    finish();
                }
            }
        );
        mLvFavorites.setOnCreateContextMenuListener(
                /**
                 * create context menu
                 * @param menu context menu
                 * @param view context menu view
                 * @param menuInfo context menu information
                 */
            new View.OnCreateContextMenuListener() {
                public void onCreateContextMenu(ContextMenu menu, View view, ContextMenu.ContextMenuInfo menuInfo) {
                    menu.setHeaderTitle(R.string.contmenu_title);
                    Cursor cursor = mAdapter.getCursor();
                    cursor.moveToPosition(((AdapterView.AdapterContextMenuInfo)menuInfo).position);
                    int stationType = cursor.getInt(cursor.getColumnIndex(FMRadioStation.Station.COLUMN_STATION_TYPE));
                    if (FMRadioStation.STATION_TYPE_SEARCHED == stationType) {
                        // Searched station.
                        menu.add(0, CONTMENU_ID_ADD, 0, mProjectStringExt.getProjectString(mContext, 
                                R.string.add_to_favorite, R.string.add_to_favorite1));
                    } else {
                        // Favorite station.
                        menu.add(0, CONTMENU_ID_EDIT, 0, R.string.contmenu_item_edit);
                        menu.add(0, CONTMENU_ID_DELETE, 0, mProjectStringExt.getProjectString(mContext,
                                R.string.contmenu_item_delete, R.string.contmenu_item_delete1));
                    }
                }
            }
        );

        // ALPS01270783 Finish favorite when exit FM
        mExitListener = new FMRadioService.OnExitListener() {
            @Override
            public void onExit() {
                LogUtils.d(TAG, "onExit()");
                Handler mainHandler = new Handler(Looper.getMainLooper());
                mainHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        FMRadioFavorite.this.finish();
                    }
                });
            }
        };
        FMRadioService.registerExitListener(mExitListener);

        LogUtils.d(TAG, "end FMRadioFavorite.onCreate");
    }
    /**
     * handle the event when context menu selected
     * @param item selected menu item
     * @return whether need to handle other context item
     */
    public boolean onContextItemSelected(MenuItem item) {
        // get list view position
        int position = ((AdapterView.AdapterContextMenuInfo) item.getMenuInfo()).position;
        // get item view
        View itemView = mLvFavorites.getAdapter().getView(position, null, mLvFavorites);
        // get station frequency and station name of item view
        TextView stationFreqView = (TextView) itemView.findViewById(R.id.lv_station_freq);
        TextView stationNameView = (TextView) itemView.findViewById(R.id.lv_station_name);
        mDlgStationFreq = FMRadioUtils.computeStation(Float.parseFloat(stationFreqView.getText().toString()));
        mDlgStationName = stationNameView.getText().toString();
        switch (item.getItemId()) {
            case CONTMENU_ID_ADD:
            // Favorite list is full. Toast it.
            if (FMRadioStation.getStationCount(mContext, FMRadioStation.STATION_TYPE_FAVORITE)
                    >= FMRadioStation.MAX_FAVORITE_STATION_COUNT) {
                Toast.makeText(mContext, mProjectStringExt.getProjectString(mContext, R.string.toast_favorite_full,
                                R.string.toast_favorite_full1), Toast.LENGTH_SHORT).show();
                break;
            }
            // show add favorite dialog
            showAddFavoriteDialog();
            break;
            
            case CONTMENU_ID_EDIT:
            // show edit favorite dialog
            showEditFavoriteDialog();
            break;

            case CONTMENU_ID_DELETE: 
            // show delete favorite dialog
            showDeleteFavoriteDialog();
            break;
            
        default:
            LogUtils.d(TAG, "invalid menu item");
            break;
        }
        return false;
    }
    /**
     * handle navigation button
     * @param item selected menu item
     * @return true to consume it, false to can handle other
     */
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            super.onBackPressed();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }
    
    /**
     * create cursor loader to initial list view
     * @param id The id whose loader will be created
     * @param args Any arguments specified by caller
     * @return cursor loader according query result
     */
    public Loader<Cursor> onCreateLoader(int id, Bundle args) {
        Uri uri = FMRadioStation.Station.CONTENT_URI;
        String select = FMRadioStation.Station.COLUMN_STATION_TYPE + " IN (?, ?)";
        String order = FMRadioStation.Station.COLUMN_STATION_TYPE + "," + FMRadioStation.Station.COLUMN_STATION_FREQ;
        CursorLoader cursorLoader = new CursorLoader(
                this,
                uri,
                FMRadioStation.COLUMNS,
                select,
                new String[] { String.valueOf(FMRadioStation.STATION_TYPE_FAVORITE),
                        String.valueOf(FMRadioStation.STATION_TYPE_SEARCHED) },
                order);
        return cursorLoader;
    }

    /**
     * swap adapter cursor
     * 
     * @param loader
     *            cursor loader
     * @param data
     *            new cursor
     */
    public void onLoadFinished(Loader<Cursor> loader, Cursor data) {
        mAdapter.swapCursor(data);
    }

    /**
     * set adapter as null
     * 
     * @param loader
     *            cursor loader
     */
    public void onLoaderReset(Loader<Cursor> loader) {
        mAdapter.swapCursor(null);
    }

    /**
     * This class use associate cursor with adapter item view
     * 
     */
    static class ChannelListAdapter extends SimpleCursorAdapter {
        public ChannelListAdapter(Context context, int layout, Cursor c, String[] from, int[] to) {
            super(context, layout, c, from, to);
        }

        /**
         * class use to manage view
         * 
         */
        static class ViewHolder {
            ImageView mStationTypeView;
            TextView mStationFreqView;
            TextView mStationNameView;
        }

        /**
         * create item view
         * 
         * @param context
         *            The context
         * @param cursor
         *            The cursor
         * @param parent
         *            list view group
         */
        @Override
        public View newView(Context context, Cursor cursor, ViewGroup parent) {
            View view = super.newView(context, cursor, parent);
            ViewHolder viewHolder = new ViewHolder();
            viewHolder.mStationTypeView = (ImageView) view.findViewById(R.id.lv_station_type);
            viewHolder.mStationFreqView = (TextView) view.findViewById(R.id.lv_station_freq);
            viewHolder.mStationNameView = (TextView) view.findViewById(R.id.lv_station_name);
            view.setTag(viewHolder);
            return view;
        }

        /**
         * bind cursor data to list view
         * 
         * @param view
         *            item view
         * @param context
         *            The context
         * @param cursor
         *            The cursor
         */
        @Override
        public void bindView(View view, Context context, Cursor cursor) {
            ViewHolder viewHolder = (ViewHolder) view.getTag();
            int stationType = cursor.getInt(cursor.getColumnIndex(FMRadioStation.Station.COLUMN_STATION_TYPE));
            int stationFreq = cursor.getInt(cursor.getColumnIndex(FMRadioStation.Station.COLUMN_STATION_FREQ));
            String stationName = cursor.getString(cursor.getColumnIndex(FMRadioStation.Station.COLUMN_STATION_NAME));
            if (FMRadioStation.STATION_TYPE_FAVORITE == stationType) {
                viewHolder.mStationTypeView.setImageResource(R.drawable.btn_fm_favorite_on);
            } else {
                viewHolder.mStationTypeView.setImageResource(0);
            }
            viewHolder.mStationFreqView.setText(FMRadioUtils.formatStation(stationFreq));
            viewHolder.mStationNameView.setText(stationName);
        }
    }
    
    /**
     * show add favorite dialog
     */
    public void showAddFavoriteDialog() {
        AddFavoriteDialog fragment = AddFavoriteDialog.newInstance(mDlgStationName, mDlgStationFreq);
        fragment.show(getFragmentManager(), ADD_FAVORITE);

    }

    /**
     * show edite favorite dialog
     */
    public void showEditFavoriteDialog() {
        EditFavoriteDialog newFragment = EditFavoriteDialog.newInstance(mDlgStationName, mDlgStationFreq);
        newFragment.show(getFragmentManager(), EDIT_FAVORITE);
    }

    /**
     * show delete favorite dialog
     */
    public void showDeleteFavoriteDialog() {
        DeleteFavoriteDialog newFragment = DeleteFavoriteDialog.newInstance();
        newFragment.show(getFragmentManager(), DELETE_FAVORITE);
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        outState.putString(FAVORITE_NAME, mDlgStationName);
        outState.putInt(FAVORITE_FREQ, mDlgStationFreq);
        super.onSaveInstanceState(outState);
    }
    
    /**
     * add searched station as favorite station
     */
    public void addFavorite() {
        AddFavoriteDialog dialogFragment = ((AddFavoriteDialog) getFragmentManager().findFragmentByTag(ADD_FAVORITE));
        if (null == dialogFragment) {
            return;
        }

        Dialog dialog = dialogFragment.getDialog();

        if (null == dialog) {
            return;
        }
        
        EditText editText = (EditText) dialog.findViewById(R.id.dlg_add_station_name_text);

        if (null == editText) {
            return;
        }
        
        String newName = editText.getText().toString().trim();
        // if user not input String, use previous displayed station name
        // if not have previous displayed station name, use default station name
        if (0 != newName.length()) {
            mDlgStationName = newName;
        }
        // update the station name and station type in database
        // according the frequency
        FMRadioStation.updateStationToDB(mContext, mDlgStationName,
                FMRadioStation.STATION_TYPE_FAVORITE, mDlgStationFreq);
        mAdapter.notifyDataSetChanged();
    }
    
    /**
     * edit favorite station frequency and station name
     */
    public void editFavorite() {
        EditFavoriteDialog dialogFragment = (EditFavoriteDialog) getFragmentManager().findFragmentByTag(EDIT_FAVORITE);
        if (null == dialogFragment) {
            return;
        }
        Dialog dialog = dialogFragment.getDialog();

        if (null == dialog) {
            return;
        }
        
        EditText nameEditText = (EditText) dialog.findViewById(R.id.dlg_edit_station_name_text);
        EditText frequencyEditText = (EditText) dialog.findViewById(R.id.dlg_edit_station_freq_text);

        if ((null == nameEditText) || (null == frequencyEditText)) {
            return;
        }
        
        String newName = nameEditText.getText().toString().trim();
        String newStationFreqStr = frequencyEditText.getText().toString().trim();
        // if user not input String, use previous displayed station name
        // if not have previous displayed station name, use default station name
        if (0 != newName.length()) {
            mDlgStationName = newName;
        }
        
        float newStationFreq = 0;

        try {
            newStationFreq = Float.parseFloat(newStationFreqStr);
        } catch (NumberFormatException e) {
            e.printStackTrace();
        }

        int newStation = FMRadioUtils.computeStation(newStationFreq);
        if (FMRadioUtils.isValidStation(newStation)) {
            // if station is exist in channel list delete it
            // ignore current station, because current station not display in listview
            if (FMRadioStation.isStationExist(mContext, newStation, FMRadioStation.STATION_TYPE_FAVORITE)
                    && (newStation != mDlgStationFreq)) {
                FMRadioStation.deleteStationInDB(mContext, newStation, FMRadioStation.STATION_TYPE_FAVORITE);
            } else if (FMRadioStation.isStationExist(mContext, newStation, 
                    FMRadioStation.STATION_TYPE_SEARCHED)) {
                FMRadioStation.deleteStationInDB(mContext, newStation, FMRadioStation.STATION_TYPE_SEARCHED);
            }
            FMRadioStation.updateStationToDB(mContext, mDlgStationName, mDlgStationFreq, newStation,
                    FMRadioStation.STATION_TYPE_FAVORITE);
            mAdapter.notifyDataSetChanged();
        } else {
            Toast.makeText(getApplicationContext(), R.string.toast_invalid_frequency, 
                    Toast.LENGTH_SHORT).show();
        }
    }
    
    /**
     * delete favorite from favorite channel list, make it as searched station
     */
    public void deleteFavorite() {
        // update the station type from favorite to searched.
        FMRadioStation.updateStationToDB(mContext, mDlgStationName, FMRadioStation.STATION_TYPE_SEARCHED, mDlgStationFreq);
        mAdapter.notifyDataSetChanged();
    }

    @Override
    protected void onDestroy() {
        LogUtils.d(TAG, "onDestory()");
        FMRadioService.unregisterExitListener(mExitListener);
        super.onDestroy();
    }
}
