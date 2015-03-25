/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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

package com.mediatek.rcse.activities.widgets;

import android.R.integer;
import android.content.Context;
import android.content.res.Resources;
import android.graphics.drawable.Drawable;
import android.os.Handler;
import android.os.Looper;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AbsListView;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.SectionIndexer;
import android.widget.TextView;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.fragments.GroupChatFragment;
import com.mediatek.rcse.fragments.ChatFragment.FileTransfer;
import com.mediatek.rcse.fragments.One2OneChatFragment;
import com.mediatek.rcse.fragments.One2OneChatFragment.ReceivedFileTransfer;
import com.mediatek.rcse.fragments.One2OneChatFragment.SentFileTransfer;
import com.mediatek.rcse.interfaces.ChatView.IChatEventInformation;
import com.mediatek.rcse.interfaces.ChatView.IReceivedChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;

import com.orangelabs.rcs.R;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Vector;
import java.util.concurrent.Semaphore;

public class ChatAdapter extends BaseAdapter implements SectionIndexer {
    public static final String TAG = "ChatAdapter";

    public static final int ITEM_TYPE_DATE_LABEL = 0;

    public static final int ITEM_TYPE_SENT_MESSAGE = 1;

    public static final int ITEM_TYPE_RECEIVED_MESSAGE = 2;

    public static final int ITEM_TYPE_SENT_FILE_TRANSFER = 3;

    public static final int ITEM_TYPE_RECEIVED_FILE_TRANSFER = 4;

    public static final int ITEM_TYPE_RECEIVED_FILE_TRANSFER_ACCEPT = 5;

    public static final int ITEM_TYPE_SENT_FILE_TRANSFER_REJECT = 6;

    public static final int ITEM_TYPE_RECEIVED_FILE_TRANSFER_REJECT = 7;

    public static final int ITEM_TYPE_RECEIVED_FILE_TRANSFER_FINISHED = 8;

    public static final int ITEM_TYPE_SENT_FILE_TRANSFER_CANCEL = 9;

    public static final int ITEM_TYPE_RECEIVED_FILE_TRANSFER_CANCEL = 10;

    public static final int ITEM_TYPE_SENT_FILE_TRANSFER_CANCELED = 11;

    public static final int ITEM_TYPE_RECEIVED_FILE_TRANSFER_CANCELED = 12;

    public static final int ITEM_TYPE_SENT_FILE_TRANSFER_PENDING = 13;

    public static final int ITEM_TYPE_CHAT_EVENT_INFORMATION = 14;

    public static final int ITEM_TYPE_SENT_FILE_TRANSFER_FINISHED = 15;

    public static final int ITEM_TYPE_SENT_FILE_TRANSFER_FAILED = 16;

    public static final int ITEM_TYPE_RECEIVED_FILE_TRANSFER_FAILED = 17;

    public static final int ITEM_TYPE_SENT_FILE_TRANSFER_TIMEOUT = 18;

    public static final int ITEM_TYPE_COUNT = ITEM_TYPE_SENT_FILE_TRANSFER_TIMEOUT + 1;

    private static final long THREAD_ID_MAIN = 1;

    private Map<Integer, Integer> mSectionToPosition = null;
    
    private static final String FAST_SCROLLER = "mFastScroller";
    
    private static final String OVERLAY_DRAWABLE = "mOverlayDrawable";

    private final Object mLock = new Object();

    public final Semaphore available = new Semaphore(100, true);

    /**
     * This class specify a view binder for a specific item
     */
    public abstract static class AbsItemBinder {
        public static final String TAG = "AbsItemBinder";
        private int mSectionIndex = -1;

        /**
         * Bind an item view
         * 
         * @param itemView The item view to be bound
         */
        abstract void bindView(View itemView);

        /**
         * Get the type id of this item binder
         * 
         * @return The type id of this item binder
         */
        abstract int getItemType();

        /**
         * Called while the adapter needs to create a new item view
         * 
         * @return The layout resource id
         */
        abstract int getLayoutResId();
    }

    /**
     * This factory class is used to create an item binder from one kind of item
     */
    private static class ItemBinderFactory {
        private static AbsItemBinder createItemBinder(Object item, int position) {
            if (item instanceof Date) {
                return new DateLabelItemBinder((Date) item, position);
            } else if (item instanceof ISentChatMessage) {
                if (item instanceof GroupChatFragment.SentMessage && Logger.getIsIntegrationMode()) {
                    return new PluginGroupSendMesItemBinder((ISentChatMessage) item);
                } else {
                    return new SentMessageItemBinder((ISentChatMessage) item);
                }
            } else if (item instanceof IReceivedChatMessage) {
                if (item instanceof GroupChatFragment.ReceivedMessage && Logger.getIsIntegrationMode()) {
                    return new PluginGroupReceivedMesItemBinder((IReceivedChatMessage) item);
                } else {
                    return new ReceivedMessageItemBinder((IReceivedChatMessage) item);
                }
            } else if (item instanceof One2OneChatFragment.SentFileTransfer || item instanceof GroupChatFragment.SentFileTransfer) {
                return new SentFileTransferItemBinder((FileTransfer)item);
            } else if (item instanceof ReceivedFileTransfer) {
                return new ReceivedFileTransferItemBinder((ReceivedFileTransfer) item);
            } else if (item instanceof IChatEventInformation) {
                return new ChatEventItemBinder((IChatEventInformation) item);
            } else {
                // TODO Add more types of item here (File Transfer etc.)
                return null;
            }
        }
    }

    protected List<AbsItemBinder> mItemBinderList = new Vector<AbsItemBinder>();

    private LayoutInflater mInflater;

    private ListView mListView = null;

    private Handler mUiHandler = new Handler(Looper.getMainLooper());

    public RelativeLayout mHeaderView = null;

    protected TextView mHeaderText = null;

    public ChatAdapter(ListView listView) {
        if (null != listView) {
            mInflater = LayoutInflater.from(listView.getContext());
            mListView = listView;
            updateAdapter();
            mHeaderView = (RelativeLayout) mInflater.inflate(R.layout.load_history_header, null);
            if (mHeaderView != null) {
                mListView.addHeaderView(mHeaderView);
                mHeaderText = (TextView) mHeaderView.findViewById(R.id.load_chat_history);
                showHeaderView(false);
            } else {
                Logger.e(TAG, "ChatAdapter, mHeaderTextView is null!");
            }
        } else {
            throw new RuntimeException("listView is null");
        }
    }
    
    /**
     * Defined to update the ChatAdapter
     */
    public final void updateAdapter() {
        Context context = mInflater.getContext();
        if (context != null) {
            Resources resources = context.getResources();
            Field field = null;
            try {
                field = AbsListView.class.getDeclaredField(FAST_SCROLLER);
                field.setAccessible(true);
                Object fastScroller = field.get(mListView);
                field = fastScroller.getClass().getDeclaredField(OVERLAY_DRAWABLE);
                field.setAccessible(true);
                Drawable myOverlayDrawable = resources.getDrawable(R.drawable.fast_scroller);
                field.set(fastScroller, myOverlayDrawable);
            } catch (NoSuchFieldException e) {
                e.printStackTrace();
            } catch (IllegalArgumentException e) {
                e.printStackTrace();
            } catch (IllegalAccessException e) {
                e.printStackTrace();
            }
            int fastScrollerFontSize =
                    resources.getDimensionPixelSize(R.dimen.fast_scroll_font_size);
            //mListView.setFastScrollerPopupTextSize(fastScrollerFontSize);
            int fastScrollerWidth =
                    resources.getDimensionPixelSize(R.dimen.fast_scroll_overlay_width);
            int fastScrollerHeight =
                    resources.getDimensionPixelSize(R.dimen.fast_scroll_overlay_height);
            //mListView.setFastScrollerPopupSize(fastScrollerWidth, fastScrollerHeight);
            mListView.invalidate();
        }
    }
    
    public final void showHeaderView(boolean showHeader) {
        if (mHeaderView != null) {
            if (showHeader) {
                mHeaderText.setVisibility(View.VISIBLE);
                mHeaderView.setVisibility(View.VISIBLE);
            } else {
                mHeaderText.setVisibility(View.GONE);
                mHeaderView.setVisibility(View.GONE);
            }
        } else {
            Logger.e(TAG, "showHeaderView, mTextView is null!");
        }
    }

    /**
     * Add an item to the end of the ListView
     * 
     * @param item The item to be added
     */
    public void addMessage(final Object item) {
        Thread currentThread = Thread.currentThread();
        if (THREAD_ID_MAIN == currentThread.getId()) {
            addMessage(item, mItemBinderList.size());
        } else {
            mUiHandler.post(new Runnable() {
                @Override
                public void run() {
                    addMessage(item, mItemBinderList.size());
                }
            });
        }
    }

    public void getSemaphoreLock() {
        try {
			available.acquire();
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}        
      }

    /**
     * Add an item to a specific position of the ListView
     * 
     * @param item The item to be added
     * @param position The position of the newly added item
     */
    public void addMessage(final Object item, final int position) {
        final int size = mItemBinderList.size();
        Logger.d(TAG, "addMessage() item is " + item + " size is " + size
                + " position is " + position);
        if (null != item) {            
                final AbsItemBinder itemBinder = ItemBinderFactory
                        .createItemBinder(item, position);
                Logger.d(TAG, "addMessage() itemBinder is " + itemBinder);
                if (null != itemBinder) {
                    Runnable addItemRunner = new Runnable() {
                        @Override
                        public void run() {
						synchronized (mLock) {
							try {
								if (position > size) {
									Logger.d(TAG, "addMessage() p > s "
											+ itemBinder + "position ="
											+ position);
									mItemBinderList.add(itemBinder);
								} else {
									Logger.d(TAG,
											"addMessage() inside run itemBinder is "
													+ itemBinder + "position ="
													+ position);
                            mItemBinderList.add(position, itemBinder);
								}
                            ChatAdapter.this.notifyDataSetChanged();
							} catch (Exception e) {
								Logger.d(TAG, "addMessage() Exception position ="
										+ position);
								e.printStackTrace();
							}
						}
                         
                        }
                    };
                    Thread currentThread = Thread.currentThread();
                    if (THREAD_ID_MAIN == currentThread.getId()) {
                        Logger.d(TAG, "addMessage() run in a UI thread ");
                        addItemRunner.run();
                    } else {
                        Logger.d(TAG,
                                "addMessage() run in a background thread ");
                        mUiHandler.post(addItemRunner);
                    }
                }
            }
        }
    
    /**
     * Remove message at a specific position
     * @param position The position of the message
     */
    public void removeMessage(final int position) {
        Runnable runnable = new Runnable() {
            @Override
            public void run() {
                mItemBinderList.remove(position);
                ChatAdapter.this.notifyDataSetChanged();
            }
        };
        Logger.d(TAG, "removeMessage() remove position: " + position);
        Thread currentThread = Thread.currentThread();
        if (THREAD_ID_MAIN == currentThread.getId()) {
            Logger.d(TAG, "removeMessage() ran on UI thread position: "
                    + position);
            runnable.run();
        } else {
            Logger.d(TAG, "removeMessage() post to UI thread position: "
                    + position);
            mUiHandler.post(runnable);
        }
    }

    /**
     * Remove all the items on the one-2-one chat fragment when you click clear
     * history.
     */
    public void removeAllItems() {
        mUiHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mItemBinderList != null) {
                    List<AbsItemBinder> activeFileTransferBinders = new ArrayList<AbsItemBinder>();
                    for (int i = 0; i < mItemBinderList.size(); i++) {
                        AbsItemBinder itemBinder = mItemBinderList.get(i);
                        if (itemBinder.getItemType() == ITEM_TYPE_SENT_FILE_TRANSFER
                                || itemBinder.getItemType() == ITEM_TYPE_RECEIVED_FILE_TRANSFER
                                || itemBinder.getItemType() == ITEM_TYPE_RECEIVED_FILE_TRANSFER_ACCEPT
                                || itemBinder.getItemType() == ITEM_TYPE_SENT_FILE_TRANSFER_PENDING) {
                            activeFileTransferBinders.add(itemBinder);
                        } else {
                            Logger
                                    .i(TAG,
                                            "Remove history not include active file transfer status!");
                        }
                    }
                    mItemBinderList.clear();
                    mItemBinderList = activeFileTransferBinders;
                    ChatAdapter.this.notifyDataSetChanged();
                    mListView.setSelection(mItemBinderList.size());
                }
            }
        });
    }

    @Override
    public int getCount() {
        return mItemBinderList.size();
    }

    @Override
    public Object getItem(int position) {
        return mItemBinderList.get(position);
    }

    @Override
    public long getItemId(int position) {
        return 0;
    }

    @Override
    public int getItemViewType(int position) {
        Logger.d(TAG, "getItemViewType() entry, the position is " + position);
        AbsItemBinder itemBinder = mItemBinderList.get(position);
        int type = -1;
        if (null != itemBinder) {
            Logger.d(TAG, "getItemViewType() itemBinder is " + itemBinder);
            type = itemBinder.getItemType();
        } else {
            Logger.e(TAG, "getItemViewType() itemBinder is null");
        }
        Logger.d(TAG, "getItemViewType() the item type of position " + position + " is " + type);
        return type;
    }

    @Override
    public int getViewTypeCount() {
        return ITEM_TYPE_COUNT;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        View view = null;
        AbsItemBinder itemBinder = mItemBinderList.get(position);
        if (null != itemBinder) {
            Logger.d(TAG, "getView() itemBinder is " + itemBinder
                    + ",position = " + position);
        } else {
            Logger.e(TAG, "getView() itemBinder is null" + ",position = "
                    + position);
            return null;
        }
        if (null == convertView) {
            view = inflateItemView(itemBinder, parent);
        } else {
            Logger.v(TAG, "convertView:" + convertView);
            if(compareViewAndItem(convertView,itemBinder)){
                view = convertView;
            }else{
                view = inflateItemView(itemBinder, parent);
            }
        }
        itemBinder.bindView(view);
        return view;
    }
    
    private View inflateItemView(AbsItemBinder itemBinder,ViewGroup parent){
        int layoutResId = itemBinder.getLayoutResId();
        Logger.v(TAG, "layoutResId:" + layoutResId);
        View view = null;
        if (0 != layoutResId) {
            view = mInflater.inflate(layoutResId, parent, false);
            view.setTag(itemBinder.getItemType());
        } else {
            Logger.e(TAG, "getView() Got an invalid layout resource id "
                    + layoutResId + " item type is " + itemBinder.getItemType());
        }
        return view;
    }
    
    /**
     * 
     * @param view The view 
     * @param itemBinder The view's data
     * @return True if itemBinder can be shown in view, otherwise return false.
     */
    private boolean compareViewAndItem(View view, AbsItemBinder itemBinder){
        int itemType = (Integer) view.getTag();
        if(itemBinder != null && itemBinder.getItemType() == itemType){
            return true;
        }else{
            return false;
        }
    }

    @Override
    public int getPositionForSection(int section) {
        Logger.d(TAG, "getPositionForSection() entry ,the section is " + section);
        int position = 0;
        Logger.d(TAG, "getPositionForSection() entry ,the mSectionToPosition is "
                + (mSectionToPosition == null ? "null" : "not null"));
        if (null != mSectionToPosition) {
            boolean isContain = mSectionToPosition.containsKey(section);
            Logger.d(TAG, "getPositionForSection() entry ,the isContain is " + isContain);
            if (isContain) {
                Logger.d(TAG, "getPositionForSection() entry ,the mSectionToPosition size is "
                        + mSectionToPosition.size());
                position = mSectionToPosition.get(section);
            }
        }
        Logger.d(TAG, "getPositionForSection() exit,the position is " + position);
        return position;
    }

    @Override
    public int getSectionForPosition(int position) {
        Logger.d(TAG, "getSectionForPosition() entry,the position is " + position);
        int sectionIndex = 0;
        int size = (mItemBinderList == null) ? 0 : mItemBinderList.size();
        Logger.d(TAG, "getSectionForPosition() entry ,the mItemBinderList size is " + size);
        if (position >= 0 && position < size) {
            AbsItemBinder item = mItemBinderList.get(position);
            if (null != item && item instanceof DateLabelItemBinder) {
                Logger.d(TAG, "getSectionForPosition(), the item is " + item);
                sectionIndex = item.mSectionIndex;
            }
        }
        Logger.d(TAG, "getSectionForPosition() exit, the sectionIndex is " + sectionIndex);
        return sectionIndex;
    }

    @Override
    public Object[] getSections() {
        ArrayList<String> sections = new ArrayList<String>();
        mSectionToPosition = new HashMap<Integer, Integer>();
        int size = mItemBinderList.size();
        AbsItemBinder item = null;
        int sectinIndex = -1;
        for (int i = 0; i < size; i++) {
            item = mItemBinderList.get(i);
            if (item instanceof DateLabelItemBinder) {
                String section = ((DateLabelItemBinder) item).getDate();
                Logger.d(TAG, "getSections() entry, at position " + i + " the section is "
                        + section);
                if (section != null) {
                    sections.add(section);
                } else {
                    Logger.d(TAG, "getSections(), the section is null");
                }

                sectinIndex++;
                item.mSectionIndex = sectinIndex;
                mSectionToPosition.put(item.mSectionIndex, i);
            }
        }
        Logger.d(TAG, "getSections(), the size of sections = " + sections.size() + "\n"
                + "getSections(), the size of mItemBinderList = " + mItemBinderList.size() + "\n"
                + "getSections(), the size of mSectionToPosition = " + mSectionToPosition.size());
        return sections.toArray();
    }
}
