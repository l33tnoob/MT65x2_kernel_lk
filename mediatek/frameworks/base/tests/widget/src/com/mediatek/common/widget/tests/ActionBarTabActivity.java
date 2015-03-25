package com.mediatek.common.widget.tests;

import android.os.Bundle;
import android.app.ActionBar;
import android.app.ActionBar.LayoutParams;
import android.app.ActionBar.Tab;
import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentTransaction;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import android.widget.Toast;

public class ActionBarTabActivity extends Activity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.action_bar_tabs);
		
		final ActionBar bar = getActionBar();
		int flags = ActionBar.DISPLAY_HOME_AS_UP | ActionBar.DISPLAY_SHOW_HOME | ActionBar.DISPLAY_SHOW_TITLE | ActionBar.DISPLAY_SHOW_CUSTOM;
        bar.setDisplayOptions(flags);
        
        View mCustomView = null;
        mCustomView = getLayoutInflater().inflate(R.layout.action_bar_display_options_custom, null);
        bar.setCustomView(mCustomView,
                new ActionBar.LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT));
	}

	public void onAddTab(View v) {
		final ActionBar bar = getActionBar();
		final int tabCount = bar.getTabCount();
		final String text = "Tab " + tabCount;
		bar.addTab(bar.newTab().setText(text)
				.setTabListener(new TabListener(new TabContentFragment(text))));
	}

	public void onRemoveTab(View v) {
		final ActionBar bar = getActionBar();
		if(bar.getTabCount() > 0) {
			bar.removeTabAt(bar.getTabCount() - 1);
		}
	}

	public void onToggleTabs(View v) {
		final ActionBar bar = getActionBar();
		if (bar.getNavigationMode() == ActionBar.NAVIGATION_MODE_TABS) {
			bar.setNavigationMode(ActionBar.NAVIGATION_MODE_STANDARD);
			bar.setDisplayOptions(ActionBar.DISPLAY_SHOW_TITLE,
					ActionBar.DISPLAY_SHOW_TITLE);
		} else {
			bar.setNavigationMode(ActionBar.NAVIGATION_MODE_TABS);
			bar.setDisplayOptions(bar.getDisplayOptions(), ActionBar.DISPLAY_SHOW_TITLE);
		}
	}

	public void onRemoveAllTabs(View v) {
		getActionBar().removeAllTabs();
	}
	
	@Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.display_options_actions, menu);
        return true;
    }

	private class TabListener implements ActionBar.TabListener {
		private TabContentFragment mFragment;

		public TabListener(TabContentFragment fragment) {
			mFragment = fragment;
		}

		public void onTabSelected(Tab tab, FragmentTransaction ft) {
			ft.add(R.id.fragment_content, mFragment, mFragment.getText());
		}

		public void onTabUnselected(Tab tab, FragmentTransaction ft) {
			ft.remove(mFragment);
		}

		public void onTabReselected(Tab tab, FragmentTransaction ft) {
			Toast.makeText(ActionBarTabActivity.this, "Reselected!",
					Toast.LENGTH_SHORT).show();
		}
	}

	private class TabContentFragment extends Fragment {

		private String mText;

		public TabContentFragment(String text) {
			mText = text;
		}

		public String getText() {
			return mText;
		}

		@Override
		public View onCreateView(LayoutInflater inflater, ViewGroup container,
				Bundle savedInstanceState) {
			View fragView = inflater.inflate(R.layout.action_bar_tab_content,
					container, false);
			TextView text = (TextView) fragView.findViewById(R.id.text);
			text.setText(mText);
			return fragView;
		}
	}
}
