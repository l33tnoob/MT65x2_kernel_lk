package com.mediatek.common.widget.tests;

import android.os.Bundle;
import android.annotation.SuppressLint;
import android.app.ActionBar;
import android.app.Activity;
import android.app.SearchManager;
import android.content.Context;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.SearchView;

@SuppressLint("NewApi")
public class ActionBarActionViewActivity extends Activity {
	private Menu mMenu;
	MenuItem tmpItem;
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.action_view_main);
        
    }

    @Override
	protected void onStart() {
		// TODO Auto-generated method stub
    	ActionBar actionBar = getActionBar();
        getActionBar().setDisplayHomeAsUpEnabled(true);
        //actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_LIST);
		super.onStart();
	}

	@Override
    public boolean onCreateOptionsMenu(Menu menu) {
		mMenu = menu;
        getMenuInflater().inflate(R.menu.action_view, menu);
    	/*
    	MenuItem addItem = menu.add("Search");
		// addItem.setShowAsAction(MenuItem.SHOW_AS_ACTION_NEVER);
		addItem.setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS|MenuItem.SHOW_AS_ACTION_COLLAPSE_ACTION_VIEW);
		addItem.setIcon(android.R.drawable.ic_menu_search);
		SearchManager searchManager = (SearchManager) getSystemService( Context.SEARCH_SERVICE );
        SearchView searchView = new SearchView( this );
        searchView.setSearchableInfo( searchManager.getSearchableInfo( getComponentName() ) );
        searchView.setIconifiedByDefault( true );
		addItem.setActionView(searchView);
		*/
        MenuItem mSearchMenuItem =menu.findItem(R.id.menu_search) ;
        SearchView mSearchView = (SearchView) mSearchMenuItem.getActionView();
		//mSearchView.setOnQueryTextListener(mOnsearchClickListener);
		mSearchView.setIconifiedByDefault(true);
		/*
		mSearchMenuItem.setOnActionExpandListener(new MenuItem.OnActionExpandListener(){

			public boolean onMenuItemActionExpand(MenuItem item) {
				//getActionBar().setDisplayShowHomeEnabled(false);
				return false;
			}

			public boolean onMenuItemActionCollapse(MenuItem item) {
				// TODO Auto-generated method stub
				return false;
			}});
			*/
		//tmpItem = addItem;
        return true;
    }
	
	@Override
	public boolean onPrepareOptionsMenu(Menu menu) {
		// TODO Auto-generated method stub
		mMenu = menu;
		return super.onPrepareOptionsMenu(menu);
	}
	
	@Override
	public void onOptionsMenuClosed(Menu menu) {
		// TODO Auto-generated method stub
		super.onOptionsMenuClosed(menu);
		mMenu = null;
	}
	
	public Menu getMenu() {
    	return mMenu;
    }
    
    @Override
	public boolean onOptionsItemSelected(MenuItem item) {
		
		switch (item.getItemId()) {
		case android.R.id.home:
			//tmpItem.collapseActionView();
			return false;
		}
		
		return super.onOptionsItemSelected(item);
	}
}
