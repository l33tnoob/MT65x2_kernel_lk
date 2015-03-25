package com.hissage.ui.activity;

import java.io.File;
import java.io.FileOutputStream;

import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ActivityInfo;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.provider.MediaStore;
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.hissage.R;
import com.hissage.api.NmsIpMessageApi;
import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.config.NmsBitmapUtils;
import com.hissage.config.NmsCommonUtils;
import com.hissage.config.NmsProfileSettings;
import com.hissage.jni.engineadapter;
//M: Activation Statistics
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.smsmms.NmsSMSMMSManager;
import com.hissage.platfrom.NmsPlatformAdapter;
import com.hissage.service.NmsService;
import com.hissage.struct.SNmsMsgType;
import com.hissage.struct.SNmsSimInfo;
import com.hissage.struct.SNmsSimInfo.NmsSimActivateStatus;
import com.hissage.util.data.NmsConsts;
import com.hissage.util.data.NmsConsts.NmsIntentStrId;
import com.hissage.util.log.NmsLog;

public class NmsProfileSettingsActivity extends NmsBaseActivity {

    private static final String TAG = "profile_settings";

    private Context mContext;
	private int currentSIMID;

    private static final int PHOTO_PICKED_WITH_DATA = 3021;
    private static final int CAMERA_WITH_DATA = 3023;
    private static final int PHOTO_WITH_DATA = 3025;

    private class SNmsSimInfoMTK {
        long sim_id = NmsConsts.INVALID_SIM_ID;
        String simName = null;
        String phone = null;
        int color = -1;
        boolean simEnable = false;
    }

    private NmsProfileSettings mUserConfig;

    private ImageView mIvAvatar = null;
    private TextView mTvUserName = null;
    private SNmsSimInfoMTK[] mSimInfo = new SNmsSimInfoMTK[NmsConsts.SIM_CARD_COUNT];
    private NmsSetProfileResultRecver mResultRecver = null;

    private Bitmap getAvatar(String fileName) {

        Bitmap srcAvatar = null;
        if (!TextUtils.isEmpty(mUserConfig.fileName)
                && NmsCommonUtils.isExistsFile(mUserConfig.fileName)) {
            srcAvatar = BitmapFactory.decodeFile(fileName);
        } else {
            srcAvatar = BitmapFactory.decodeResource(this.getResources(),
                    R.drawable.ic_contact_picture);
        }

        if (null == srcAvatar) {
            NmsLog.error(TAG, "can not parse avatar file: " + fileName);
            return null;
        }

        return srcAvatar;
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

    private void getAllSimInfo() {

        for (int i = 0; i < NmsConsts.SIM_CARD_COUNT; ++i) {
            if (null == mSimInfo[i]) {
                mSimInfo[i] = new SNmsSimInfoMTK();
            }
            mSimInfo[i].sim_id = NmsPlatformAdapter.getInstance(this).getSimIdBySlotId(i);
            if (mSimInfo[i].sim_id > 0) {
                mSimInfo[i].simName = NmsPlatformAdapter.getInstance(this).getSimName(
                        mSimInfo[i].sim_id);
                mSimInfo[i].color = NmsPlatformAdapter.getInstance(this).getSimColor(
                        mSimInfo[i].sim_id);
                SNmsSimInfo sim = NmsIpMessageApiNative
                        .nmsGetSimInfoViaSimId((int) mSimInfo[i].sim_id);
                if (null != sim) {
                    mSimInfo[i].simEnable = NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED == sim.status;
                    mSimInfo[i].phone = sim.number;
                } else {
                    // do nothing
                }
            } else {
                // do nothing
            }
        }
    }

    private void updateStatus() {

        getAllSimInfo();

        SNmsSimInfo info = NmsIpMessageApiNative.nmsGetSimInfoViaSimId((int)NmsPlatformAdapter.getInstance(this).getSimIdBySlotId(currentSIMID));		
        mUserConfig = engineadapter.get().nmsUIGetUserInfoViaImsi(info.imsi);
		
        if (null == mUserConfig) {
            NmsLog.error(TAG, "can't get user profile.");
            finish();
            return;
        }

        if (!TextUtils.isEmpty(mUserConfig.fileName)
                && NmsCommonUtils.isExistsFile(mUserConfig.fileName)) {

            Bitmap avatar = getAvatar(mUserConfig.fileName);
            if (avatar != null) {
                mIvAvatar.setImageBitmap(avatar);
            }
        }

        if (TextUtils.isEmpty(mUserConfig.name)) {
            mTvUserName.setText(R.string.STR_NMS_NO_NAME);
        } else {
            mTvUserName.setText(mUserConfig.name);
        }

    }

    private void init() {

        mIvAvatar = (ImageView) findViewById(R.id.avatar);
        mIvAvatar.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                showPhotoSettingDialog();
            }
        });

        mTvUserName = (TextView) findViewById(R.id.user_name);

        mTvUserName.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                showEditNameDialog();
            }
        });
    }

    protected void doCropPhoto(Bitmap data) {
        Intent intent = getCropImageIntent(data);
        try {
            startActivityForResult(intent, PHOTO_WITH_DATA);
        } catch (Exception e) {
            Toast.makeText(this, R.string.STR_NMS_NO_APP, Toast.LENGTH_SHORT).show();
            NmsLog.trace(TAG, NmsLog.nmsGetStactTrace(e));
        }
    }

    public static Intent getCropImageIntent(Bitmap data) {
        Intent intent = new Intent("com.android.camera.action.CROP");
        intent.setType("image/*");
        intent.putExtra("data", data);
        intent.putExtra("crop", "true");
        intent.putExtra("aspectX", 1);
        intent.putExtra("aspectY", 1);
        intent.putExtra("outputX", 128);
        intent.putExtra("outputY", 128);
        intent.putExtra("scaleUpIfNeeded", true);
        intent.putExtra("scale", true);
        intent.putExtra("noFaceDetection", true);
        intent.putExtra("return-data", true);
        return intent;
    }

    private void activityResultCam(Intent data) {
        final Bitmap photo = data.getParcelableExtra("data");
        if (photo != null) {
            doCropPhoto(photo);
        } else {
            NmsLog.trace(TAG, "CAMERA_WITH_DATA is null.");
        }
    }

    private void activityResultPhotoPick(Intent data) {
        Bitmap photo1 = data.getParcelableExtra("data");
        if (photo1 != null) {
            saveAvatar(photo1);
            photo1.recycle();
        } else {
            NmsLog.trace(TAG, "PHOTO_PICKED_WITH_DATA is null.");
        }
    }

    private void activityResultPhoto(Intent data) {
        Bitmap photoa = data.getParcelableExtra("data");
        if (photoa != null) {
            saveAvatar(photoa);
            photoa.recycle();
        } else {
            NmsLog.trace(TAG, "PHOTO_WITH_DATA is null.");
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (RESULT_OK != resultCode || null == data) {
            NmsLog.trace(TAG, "activity result not ok, requestCode: " + requestCode
                    + ", resultCode " + requestCode);
            return;
        }

        switch (requestCode) {
        case CAMERA_WITH_DATA:
            activityResultCam(data);
            break;

        case PHOTO_PICKED_WITH_DATA:
            activityResultPhotoPick(data);
            break;

        case PHOTO_WITH_DATA:
            activityResultPhoto(data);
            break;

        default:
            NmsLog.error(TAG, "not handle request code: " + requestCode);
            break;
        }
    }

    private void saveAvatar(Bitmap bitmap) {
        String mCachePath = NmsCommonUtils.getSDCardPath(this) + "/";
        String mPhotoFilePath = mCachePath + "avatar.jpg";
        File file = new File(mPhotoFilePath);
        boolean isSaveAvatarOK = false;
        try {
            FileOutputStream out = new FileOutputStream(file);
            bitmap = NmsCommonUtils.resizeImage(bitmap, 128, 128, false);
            if (bitmap.compress(Bitmap.CompressFormat.JPEG, 100, out)) {
                out.flush();
                out.close();

                int fileSize = (int) file.length();
                if (fileSize >= 10 * 1024) {
                    byte[] data = NmsBitmapUtils.resizeImgBySize(mPhotoFilePath, 10 * 1024, false);
                    if (null == data) {
                        NmsLog.error(TAG, "avatar resize failed ");
                        Toast.makeText(this, R.string.STR_NMS_SAVE_AVATAR_FAILD, Toast.LENGTH_SHORT)
                                .show();
                        return;
                    }
                    try {
                        NmsCommonUtils.nmsStream2File(data, mPhotoFilePath);
                        isSaveAvatarOK = true ;
                    } catch (Exception e) {
                        NmsLog.nmsPrintStackTrace(e);
                    }
                } else {
                    isSaveAvatarOK = true ;
                }
            } else {
                out.close() ;
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
            Toast.makeText(this, R.string.STR_NMS_SAVE_AVATAR_FAILD, Toast.LENGTH_SHORT).show();
            return;
        }
        
        File avatarFile = new File(mPhotoFilePath);
        
        if (!isSaveAvatarOK || !avatarFile.exists() || avatarFile.length() <= 0) {
            NmsLog.error(TAG, "avatarFile is not exist or empty") ;
            Toast.makeText(this, R.string.STR_NMS_SAVE_AVATAR_FAILD, Toast.LENGTH_SHORT).show();
            return ;
        }

		SNmsSimInfo info = NmsIpMessageApiNative
				.nmsGetSimInfoViaSimId((int) NmsPlatformAdapter.getInstance(
						this).getSimIdBySlotId(currentSIMID));
		if (null == info) {
			NmsLog.error(TAG, "get simInfo error! SNmsSimInfo info:" + info);

		} else {
			NmsLog.trace(TAG, "get simInfo succeed! SNmsSimInfo info:" + info);
			mUserConfig.imsi = info.imsi;
			mUserConfig.fileName = mPhotoFilePath;
		}
		

        if (engineadapter.get().nmsUISetUserInfo(mUserConfig) < 0) {
            Toast.makeText(mContext, R.string.STR_NMS_SET_PROFILE_FAIL, Toast.LENGTH_SHORT).show();
        }
        
        updateStatus() ;
    }

    private void getAvatarFromCam() {

        Intent intent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
        try {
            startActivityForResult(intent, CAMERA_WITH_DATA);
        } catch (Exception e) {
            Toast.makeText(this, R.string.STR_NMS_NO_APP, Toast.LENGTH_SHORT).show();
            NmsLog.trace(TAG, NmsLog.nmsGetStactTrace(e));
        }

    }

    private void getAvatarFromGallery() {

        Intent intent = new Intent(Intent.ACTION_PICK);
        intent.setType("image/*");
        intent.putExtra("crop", "true");
        intent.putExtra("aspectX", 1);
        intent.putExtra("aspectY", 1);
        intent.putExtra("outputX", 128);
        intent.putExtra("outputY", 128);
        intent.putExtra("scale", true);
        intent.putExtra("noFaceDetection", true);
        intent.putExtra("return-data", true);

        try {
            startActivityForResult(
                    Intent.createChooser(intent,
                            this.getResources().getText(R.string.STR_NMS_PROFILE_TITLE)),
                    PHOTO_PICKED_WITH_DATA);
        } catch (Exception e) {
            Toast.makeText(this, R.string.STR_NMS_NO_APP, Toast.LENGTH_SHORT).show();
            NmsLog.trace(TAG, NmsLog.nmsGetStactTrace(e));
        }
    }

    private boolean isHasActivitedSim(boolean mustEnable) {
        
            if (mSimInfo[currentSIMID].sim_id > 0) {
                SNmsSimInfo simInfo = NmsIpMessageApiNative
                        .nmsGetSimInfoViaSimId((int) mSimInfo[currentSIMID].sim_id);
                if (mustEnable) {
                    if (null != simInfo
                            && simInfo.status == NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED) {
                        return true;
                    }
                } else {
                    if (null != simInfo
                            && simInfo.status >= NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED) {
                        return true;
                    }
                }
            }
        
        return false;

    }

    private void showEditNameDialog() {
        
        if (!NmsSMSMMSManager.isDefaultSmsApp()) {
            Toast.makeText(this, R.string.STR_UNABLE_EDIT_FOR_NOT_DEFAULT_SMS_APP, Toast.LENGTH_LONG).show();
            return ;
        }

        if (!checkSdSimNetworkStatus(false)) {
            return;
        }

        if (!isHasActivitedSim(false)) {
            Toast.makeText(this, R.string.STR_NMS_NO_ACTIVITED, Toast.LENGTH_SHORT).show();
//            showActivitionDlg(NmsConsts.ALL_SIM_CARD, WELCOME);
			//M: Activation Statistics
            showActivitionDlg(mSimInfo[currentSIMID].sim_id, WELCOME, NmsIpMessageConsts.NmsUIActivateType.SETTING);
            return;
        }

        if (!isHasActivitedSim(true)) {
            Toast.makeText(this, R.string.STR_NMS_NO_ACTIVITED, Toast.LENGTH_SHORT).show();
            return;
        }

        LayoutInflater factory = LayoutInflater.from(this);
        final View textEntryView = factory.inflate(R.layout.alert_dialog_text_entry, null);
        EditText editText = (EditText) textEntryView.findViewById(R.id.username_edit);
        editText.setText(mUserConfig.name);
        new AlertDialog.Builder(this).setTitle(R.string.STR_NMS_INPUT_NAME).setView(textEntryView)
                .setPositiveButton(R.string.STR_NMS_OK, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        EditText text = (EditText) textEntryView.findViewById(R.id.username_edit);
                        String value = text.getText().toString();
                        if (!TextUtils.isEmpty(value)) {
                            saveUserName(value);
                        }
                    }
                }).setNegativeButton(R.string.STR_NMS_CANCEL, null).show();
    }

    private void saveUserName(String name) {
		SNmsSimInfo info = NmsIpMessageApiNative
				.nmsGetSimInfoViaSimId((int) NmsPlatformAdapter.getInstance(
						this).getSimIdBySlotId(currentSIMID));
		if (null == info) {
			NmsLog.error(TAG, "get simInfo error! SNmsSimInfo info:" + info);

		} else {
			NmsLog.trace(TAG, "get simInfo succeed! SNmsSimInfo info:" + info);
			mUserConfig.imsi = info.imsi;
			mUserConfig.name = name;
		}
        
        if (name.contains("\"") || name.contains("<") || name.contains(">") || name.contains(",") || name.contains("~")) {
            Toast.makeText(mContext, R.string.STR_NMS_ILLEGAL_PROFILE_NAME, Toast.LENGTH_SHORT).show();
            return ;
        }
        
        if (engineadapter.get().nmsUISetUserInfo(mUserConfig) < 0) {
            Toast.makeText(mContext, R.string.STR_NMS_SET_PROFILE_FAIL,
                    Toast.LENGTH_SHORT).show();
        }else{
            mTvUserName.setText(name);
        }
    }

    private void showPhotoSettingDialog() {
        
        if (!NmsSMSMMSManager.isDefaultSmsApp()) {
            Toast.makeText(this, R.string.STR_UNABLE_EDIT_FOR_NOT_DEFAULT_SMS_APP, Toast.LENGTH_LONG).show();
            return ;
        }

        if (!checkSdSimNetworkStatus(true)) {
            return;
        }

        if (!isHasActivitedSim(false)) {
            Toast.makeText(this, R.string.STR_NMS_NO_ACTIVITED, Toast.LENGTH_LONG).show();
//            showActivitionDlg(NmsConsts.ALL_SIM_CARD, WELCOME);
			//M: Activation Statistics
            showActivitionDlg(mSimInfo[currentSIMID].sim_id, WELCOME, NmsIpMessageConsts.NmsUIActivateType.SETTING);
            return;
        }

        if (!isHasActivitedSim(true)) {
            Toast.makeText(this, R.string.STR_NMS_NO_ACTIVITED, Toast.LENGTH_SHORT).show();
            return;
        }

        new AlertDialog.Builder(this).setTitle(R.string.STR_NMS_PROFILE_TITLE)
                .setItems(R.array.profile_avatar, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        if (which == 0) {
                            getAvatarFromCam();
                        } else {
                            getAvatarFromGallery();
                        }
                    }
                }).create().show();
    }

    private void regRecver() {
        mResultRecver = new NmsSetProfileResultRecver();
        IntentFilter filter = new IntentFilter();
        filter.addAction(NmsProfileSettings.CMD_RESULT_KEY);
        filter.addAction(NmsIntentStrId.NMS_REG_STATUS);
        registerReceiver(mResultRecver, filter);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.profile_settings);
		Intent _intent = this.getIntent();
		currentSIMID = _intent.getIntExtra(NmsConsts.SIM_ID, -1);

        mContext = this;

        mFollowSysScrOri = false;
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);

        getActionBar().setDisplayHomeAsUpEnabled(true);
        getActionBar().setDisplayShowHomeEnabled(true);
        // getActionBar().setLogo(R.drawable.isms);

        getAllSimInfo();
        init();
        regRecver();
        updateStatus();
    }

    @Override
    public void onResume() {
        super.onResume();
        updateStatus();

    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    public void onDestroy() {
        if (mResultRecver != null) {
            unregisterReceiver(mResultRecver);
        }
        super.onDestroy();
        this.finish();
    }

    public class NmsSetProfileResultRecver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {           
            
            //add for jira-557
            if(NmsIntentStrId.NMS_REG_STATUS.equals(intent.getAction())){
                int regStatus = intent.getIntExtra("regStatus", -1);
                switch (regStatus) {
                case SNmsMsgType.NMS_UI_MSG_REGISTRATION_OVER:
                    Toast.makeText(getApplicationContext(),R.string.STR_NMS_ENABLE_SUCCESS, Toast.LENGTH_SHORT).show();
                    updateStatus();
                    break;
                }
                return;
            }
            //add end            
            
            if (null == intent || !NmsProfileSettings.CMD_RESULT_KEY.equals(intent.getAction())) {
                NmsLog.error(TAG, "recv error intent: " + intent);
                return;
            }

            int cmdCode = intent.getIntExtra(NmsProfileSettings.CMD_CODE, -1);
            if (NmsProfileSettings.CMD_SET_HESINE_INFO_ACK == cmdCode) {
                int errorCode = intent.getIntExtra(NmsProfileSettings.CMD_RESULT_KEY, -1000);
                if (0 == errorCode) {
                    Toast.makeText(mContext, R.string.STR_NMS_SET_PROFILE_OK,
                            Toast.LENGTH_SHORT).show();
                } else {
                    NmsLog.error(TAG, "set profile failed");
                    Toast.makeText(mContext, R.string.STR_NMS_SET_PROFILE_FAIL,
                            Toast.LENGTH_SHORT).show();
                }
            }

            updateStatus();

        }

    }
}
