/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.hissage.ui.view;

import android.app.Activity;
import android.app.DialogFragment;
import android.app.Fragment;
import android.os.Bundle;
import android.view.MenuItem;

import com.hissage.R;

public class NmsWallpaperChooser extends Activity {

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        setContentView(R.layout.wallpaper_chooser_base);
        getActionBar().setDisplayHomeAsUpEnabled(true);
        getActionBar().setDisplayShowHomeEnabled(true);
        getActionBar().setLogo(R.drawable.isms);

        Fragment fragmentView = getFragmentManager().findFragmentById(
                R.id.wallpaper_chooser_fragment);

        if (fragmentView == null) {
            DialogFragment fragment = NmsWallpaperChooserDialogFragment.newInstance();
            fragment.show(getFragmentManager(), "dialog");
        }
    }

    @Override
    public boolean onMenuItemSelected(int featureId, MenuItem item) {
        int id = item.getItemId();
        switch (id) {
        case android.R.id.home: {
            finish();
            break;
        }
        }
        return super.onMenuItemSelected(featureId, item);
    }
}
