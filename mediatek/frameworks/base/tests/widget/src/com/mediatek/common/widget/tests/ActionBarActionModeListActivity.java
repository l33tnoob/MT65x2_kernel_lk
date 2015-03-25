package com.mediatek.common.widget.tests;

import android.os.Bundle;
import android.app.Activity;
import android.app.ListActivity;
import android.util.SparseBooleanArray;
import android.view.ActionMode;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.AbsListView.MultiChoiceModeListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Toast;

public class ActionBarActionModeListActivity extends ListActivity {

	String[] rivers = new String[] { "Indus", "Ganges", "Brahmaputra",
			"Jamuna", "Sutlej", "Kaveri", "Godavari" };

	MultiChoiceModeListener mMultiChoiceModeListener;

	@Override
	protected void onStart() {
		super.onStart();

		/**
		 * For contextual action mode, the choice mode should be
		 * CHOICE_MODE_MULTIPLE_MODAL
		 */
		getListView().setChoiceMode(ListView.CHOICE_MODE_MULTIPLE_MODAL);

		/** Setting multichoicemode listener for the listview */
		getListView().setMultiChoiceModeListener(mMultiChoiceModeListener);
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		/** Defining a multichoicemodelistener for the listview. */
		mMultiChoiceModeListener = new MultiChoiceModeListener() {

			public boolean onPrepareActionMode(ActionMode mode, Menu menu) {
				return false;
			}

			public void onDestroyActionMode(ActionMode mode) {
			}

			/**
			 * This will be invoked when action mode is created. In our case ,
			 * it is on long clicking a menu item
			 */
			public boolean onCreateActionMode(ActionMode mode, Menu menu) {
				getMenuInflater().inflate(R.menu.action_mode_list, menu);
				return true;
			}

			/** Invoked when an action in the action mode is clicked */
			public boolean onActionItemClicked(ActionMode mode, MenuItem item) {
				Toast.makeText(
						getBaseContext(),
						"Applying " + item.getTitle() + " on "
								+ getListView().getCheckedItemCount()
								+ " Rivers \n" + getCheckedItems(),
						Toast.LENGTH_LONG).show();
				return false;
			}

			public void onItemCheckedStateChanged(ActionMode mode,
					int position, long id, boolean checked) {
			}
		};

		/** Defining array adapter to host the list of items */
		ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
				android.R.layout.simple_list_item_multiple_choice, rivers);

		/** Setting array adapter for the listview */
		getListView().setAdapter(adapter);

	}

	/** Returning the selected rivers */
	public String getCheckedItems() {
		String selected_rivers = "";
		SparseBooleanArray checkedItems = getListView()
				.getCheckedItemPositions();
		for (int i = 0, j = 0; i < rivers.length; i++) {
			if (checkedItems.get(i)) {
				j++;
				selected_rivers += "\n" + j + "." + rivers[i];
			}

		}
		return selected_rivers;
	}
}
