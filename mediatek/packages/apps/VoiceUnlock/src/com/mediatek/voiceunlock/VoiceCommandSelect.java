package com.mediatek.voiceunlock;

import java.util.ArrayList;

import android.app.Activity;
import android.app.PendingIntent;
import android.app.admin.DevicePolicyManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.pm.PackageParser.NewPermissionInfo;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import android.provider.MediaStore;
import android.provider.Settings;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.RadioButton;
import android.widget.TextView;
import android.widget.AdapterView.OnItemClickListener;

import com.android.internal.widget.LockPatternUtils;

import com.mediatek.voiceunlock.R;

import com.mediatek.voiceunlock.VoiceUnlock.VoiceUnlockFragment;
import com.mediatek.xlog.Xlog;
import com.mediatek.common.featureoption.FeatureOption;

public class VoiceCommandSelect extends PreferenceActivity implements OnClickListener{
    
    private Button mCancelButton;
    private Button mContinuebButton;

    // M : tablet has different voice command items
    private static boolean IS_Tablet = ("tablet".equals(SystemProperties.get("ro.build.characteristics")));

    @Override
    public Intent getIntent() {
        Intent modIntent = new Intent(super.getIntent());
        return modIntent;
    }
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Xlog.d(VoiceUnlock.TAG, "VoiceCommandSelect: activity onCreate" );

        CharSequence msg = getText(R.string.voice_command_select_title);
        showBreadCrumbs(msg, msg);
        setContentView(R.layout.voice_command_select);
        
        mCancelButton = (Button)findViewById(R.id.cancel_button);
        mContinuebButton = (Button)findViewById(R.id.continue_button);
        mCancelButton.setOnClickListener(this);
        mContinuebButton.setOnClickListener(this);
    }

    @Override
    protected boolean isValidFragment(String fragmentName) {
        if (VoiceCommandSelectFragment.class.getName().equals(fragmentName)) return true;
        return false;
    }

    @Override
    public void onClick(View v) {
        if (v == mContinuebButton) {
            String commandValue = VoiceCommandSelectFragment.getCommandValue();
            if (commandValue == null) {
                return;
            } else {
                Intent intent = getIntent();
                intent.setClass(this, VoiceUnlockSetupIntro.class);
                intent.putExtra(LockPatternUtils.SETTINGS_COMMAND_VALUE, commandValue);
                startActivity(intent);
            }
        }
        finish();
    }

    public static class VoiceCommandSelectFragment extends SettingsPreferenceFragment implements
            OnItemClickListener{
        private static final int RESULT_LAUNCH_APP = 1000;

        private ListView mCommandList;
        private VoiceCommandAdapter mAdapter;
        private long mChecked;
//        private CommandApp mCustomApp;
        private static String mCommandVaule;
        static ArrayList<ComponentName> sDefaultApps = new ArrayList<ComponentName>();

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            log("VoiceCommandSelectFragment onCreate");
            mAdapter = new VoiceCommandAdapter(getActivity());
            mChecked = 0;
            mCommandVaule = null;
//            mCustomApp = null;
        }
        
        private void initDefaultApp() {
            sDefaultApps.clear();

            boolean addPhoneApps = true;
            int userId = UserHandle.myUserId();
            if (FeatureOption.MTK_ONLY_OWNER_SIM_SUPPORT && userId != UserHandle.USER_OWNER) {
                addPhoneApps = false;
            }

            if (addPhoneApps) {
                sDefaultApps.add(new ComponentName("com.android.dialer", 
                        "com.android.dialer.DialtactsActivity"));
                sDefaultApps.add(new ComponentName("com.android.mms",
                        "com.android.mms.ui.BootActivity"));
            }
            sDefaultApps.add(new ComponentName("com.android.gallery3d",
                    "com.android.camera.CameraLauncher"));

            if(IS_Tablet) {
                sDefaultApps.add(new ComponentName("com.android.music",
                    "com.android.music.MusicBrowserActivity"));
                sDefaultApps.add(new ComponentName("com.android.email"
                    ,"com.android.email.activity.Welcome"));
            }
        }
        
        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container,
                Bundle savedInstanceState) {
            View v = super.onCreateView(inflater, container, savedInstanceState);
            return v;
        }
        @Override
        public void onStart() {
            super.onStart();
            mCommandList = getListView();
            mCommandList.setAdapter(mAdapter);
            mCommandList.setOnItemClickListener(this);
        }
        
        @Override
        public void onItemClick(AdapterView<?> parent, View view, int position,
                long id) {
//            if (id != (mAdapter.getCount() -1)) {
//                //the last one is customize, skip it
                if (id != mChecked) {
                    mChecked = id;
                    mAdapter.notifyDataSetChanged();
                }
//            } else {
//                Intent intent = new Intent(getActivity(), AppChooser.class);
//                startActivityForResult(intent, RESULT_LAUNCH_APP);
//            }
            
        }
        
        class VoiceCommandAdapter extends ArrayAdapter<CommandApp> {
            private Context mContext;
            private ArrayList<CommandApp> mCommands = new ArrayList<CommandApp>();
            private final LayoutInflater mInflater;

            public VoiceCommandAdapter(Context context) {
                super(context, 0);
                mInflater = (LayoutInflater) getSystemService(
                        Context.LAYOUT_INFLATER_SERVICE);
                mContext = context;
                initDefaultApp();
                for (ComponentName app : sDefaultApps) {
                    String label = getAppLabel(app);
                    if (label != null) {
                        mCommands.add(new CommandApp(label, app));
                    }
                }
                //mCommands.add(new CommandApp(null, null));
            }

            @Override
            public View getView(int position, View convertView, ViewGroup parent) {
                VoiceCommandViewHold holder = null;
                if (convertView == null) {
                    convertView = mInflater.inflate(R.layout.voice_command_select_item, null);
                    holder = new VoiceCommandViewHold();
                    holder.mPrimary = (TextView) convertView
                            .findViewById(R.id.primary);
                    holder.mSecondary = (TextView) convertView
                        .findViewById(R.id.secondary);
                    holder.mRadio = (RadioButton) convertView
                        .findViewById(R.id.radio);
                    convertView.setTag(holder);
                } else {
                    holder = (VoiceCommandViewHold) convertView.getTag();
                }

                CommandApp app = getItem(position);
                if (app.mName != null) {
                    String primary = mContext.getResources().getString(R.string.voice_command_primary, app.mName);
                    String secondary = mContext.getResources().getString(R.string.voice_command_secondary, app.mName);
                    holder.mPrimary.setText(primary);
                    holder.mSecondary.setText(secondary);
                } /*else {
                    holder.mPrimary.setText(R.string.voice_command_customize);
                    if (mCustomApp != null) {
                        String secondary = mContext.getResources().getString(R.string.voice_command_secondary, mCustomApp.mName);
                        holder.mSecondary.setVisibility(View.VISIBLE);
                        holder.mSecondary.setText(secondary);
                    } else {
                        holder.mSecondary.setVisibility(View.GONE);
                    }
                }*/
                
                if (mChecked != -1) {
                    log("mChecked=" + mChecked + " position=" + " position + getItemId(position)=" + getItemId(position));
                    if (getItemId(position) == mChecked) {
                        holder.mRadio.setChecked(true);
//                        if (mCustomApp != null) {
//                            mCommandVaule = mCustomApp.mComponentName.flattenToShortString();
//                            mCustomApp = null;
//                        } else {
                            log("getItem(position)=" + getItem(position) + " getItem(position).mComponentName = " + getItem(position).mComponentName);
                            mCommandVaule = getItem(position).mComponentName.flattenToShortString();
//                        }
                    } else {
                        holder.mRadio.setChecked(false);
                    }
                }
                
                return convertView;
            }
            
            @Override
            public int getCount() {
                return mCommands.size();
            }

            @Override
            public CommandApp getItem(int position) {
                return mCommands.get(position);
            }

        }
        
        class VoiceCommandViewHold {
            TextView mPrimary;
            TextView mSecondary;
            RadioButton mRadio;
        }
        
        class CommandApp {
            public CommandApp(String name, ComponentName componentName) {
                mName = name;
                mComponentName = componentName;
            }
            String mName;
            ComponentName mComponentName;
        }
        
        public static String getCommandValue() {
            return mCommandVaule;
        }

//        @Override
//        public void onActivityResult(int requestCode, int resultCode, Intent data) {
//            super.onActivityResult(requestCode, resultCode, data);
//            if (requestCode == RESULT_LAUNCH_APP && resultCode == Activity.RESULT_OK) {
//                ComponentName cn = ComponentName.unflattenFromString(data.getAction());
//                mCustomApp = new CommandApp(getAppLabel(cn), cn);
//                mChecked = mAdapter.getCount() -1;
//                log("after choose app mChecked = " + mChecked + "mCustomApp = " + mCustomApp.toString());
//                mAdapter.notifyDataSetChanged();
//            }
//        }
        
        private String getAppLabel(ComponentName componentName) {
            ActivityInfo info;
            try {
                info = getPackageManager().getActivityInfo(componentName, 
                        PackageManager.GET_SHARED_LIBRARY_FILES);
            } catch (NameNotFoundException e) {
                return null;
            }
            
            CharSequence name = info.loadLabel(getPackageManager());
            return name.toString();
        }
        
        private void log(String msg) {
            if (VoiceUnlock.DEBUG) {
                Xlog.d(VoiceUnlock.TAG, "VoiceCommandSelect: " + msg);
            }
        }

    }
}
