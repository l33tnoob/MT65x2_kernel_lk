package com.mediatek.common.widget.tests;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.ActionBar;
import android.app.ActionBar.OnNavigationListener;
import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.SpinnerAdapter;
import android.widget.TextView;

@SuppressLint("NewApi")
@TargetApi(14)
public class ActionBarListActivity extends Activity {

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.list_main);

		SpinnerAdapter adapter = ArrayAdapter.createFromResource(this,
				R.array.action_list,
				android.R.layout.simple_spinner_dropdown_item);
		ActionBar actionBar = getActionBar();
		actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_LIST);
		actionBar.setListNavigationCallbacks(adapter, new DropDownListenser());
		//actionBar.setTitle("XD");
		actionBar.setSubtitle("orz");
	}

	class DropDownListenser implements OnNavigationListener {
		String[] listNames = getResources().getStringArray(R.array.action_list);

		public boolean onNavigationItemSelected(int itemPosition, long itemId) {
			ListContentFragment content = new ListContentFragment(listNames[itemPosition]);
			FragmentManager manager = getFragmentManager();
			FragmentTransaction transaction = manager.beginTransaction();

			transaction.replace(R.id.container, content,
					"test");
			transaction.commit();
			return true;
		}
	}
	
	private class ListContentFragment extends Fragment {
		private String mTag;
		
		public ListContentFragment() {
		}

		ListContentFragment(String tag) {
			mTag = tag;
		}

		@Override
		public void onAttach(Activity activity) {
			super.onAttach(activity);
		}

		@Override
		public View onCreateView(LayoutInflater inflater, ViewGroup container,
				Bundle savedInstanceState) {
			TextView textView = new TextView(getActivity());
			textView.setText(mTag);
			return textView;
		}
	}
}