package com.hissage.ui.view;

import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.config.NmsCommonUtils;
import com.hissage.struct.SNmsSimInfo;
import com.hissage.struct.SNmsSimInfo.NmsSimActivateStatus;
import com.hissage.ui.activity.NmsBaseActivity;
import com.hissage.ui.activity.NmsProfileSettingsActivity;
import com.hissage.ui.activity.NmsSystemSettingsActivity;
import com.hissage.util.data.NmsConsts;

import com.hissage.R;

import android.annotation.TargetApi;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.res.TypedArray;
import android.os.Build;
import android.os.Parcel;
import android.os.Parcelable;
import android.preference.Preference;
import android.util.AttributeSet;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.view.ViewGroup;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityManager;
import android.widget.*;

@TargetApi(Build.VERSION_CODES.ICE_CREAM_SANDWICH)
public class NmsPreferenceForProfile_2 extends Preference {

	private final static String TAG = "NmsPreferenceForProfile_2";
	private  CharSequence name;
	private  CharSequence number;
	private boolean mSendAccessibilityEventViewClickedType;
	private AccessibilityManager mAccessibilityManager;
	private boolean mChecked;
	private boolean mDisableDependentsState;
	private Context mContext = null;
	private OnTouchListener myListener; 
	
	

	public NmsPreferenceForProfile_2(Context context) {
		super(context);
		mAccessibilityManager = (AccessibilityManager) getContext()
				.getSystemService(Service.ACCESSIBILITY_SERVICE);
		this.mContext = context;
	}

	public NmsPreferenceForProfile_2(Context context, AttributeSet attrs,
			int defStyle) {
		super(context, attrs, defStyle);
		mAccessibilityManager = (AccessibilityManager) getContext()
				.getSystemService(Service.ACCESSIBILITY_SERVICE);
		this.mContext = context;
	}

	public NmsPreferenceForProfile_2(Context context, AttributeSet attrs) {
		super(context, attrs, 0);
		mAccessibilityManager = (AccessibilityManager) getContext()
				.getSystemService(Service.ACCESSIBILITY_SERVICE);
		this.mContext = context;
	}

	@Override
	protected void onBindView(View view) {
		super.onBindView(view);
		ImageView imageView = (ImageView) view.findViewById(android.R.id.icon);
		TextView title = (TextView) view.findViewById(android.R.id.title);

		TextView nameView = (TextView) view.findViewById(R.id.sim2_name);
		TextView nubView = (TextView) view.findViewById(R.id.sim2_nub);
		Switch checkboxView = (Switch) view.findViewById(R.id.act_switch2);
		
		nameView.setText(getName());
		nubView.setText(getNumber());
		checkboxView.setChecked(mChecked) ;
		checkboxView.setOnTouchListener(myListener) ;
	
		
		
		title.setOnClickListener(new OnClickListener(){

			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				Intent i = new Intent(mContext,
						NmsProfileSettingsActivity.class);
				i.putExtra(NmsConsts.SIM_ID, NmsConsts.SIM_CARD_SLOT_2);
				mContext.startActivity(i);
			}
			
		}) ;

		
		
		if (imageView != null) {
			imageView.setOnClickListener(new OnClickListener() {

				@Override
				public void onClick(View v) {
					Intent i = new Intent(mContext,
							NmsProfileSettingsActivity.class);
					i.putExtra(NmsConsts.SIM_ID, NmsConsts.SIM_CARD_SLOT_2);
					mContext.startActivity(i);
				}
			});

		}



	}
	

	public void setMyListener(View.OnTouchListener _listener) {
		this.myListener = _listener;		
		notifyChanged();
	}

	
	
	public void setName(String _name) {
		if (name != _name) {
			this.name = _name;
			notifyChanged();
		}
	}

	public CharSequence getName() {
		return name;

	}

	public void setNumber(String _number) {
		if (number != _number) {
			this.number = _number;
			notifyChanged();
		}

	}

	public CharSequence getNumber() {
		
		
		return number;

	}

	public boolean isChecked() {
		return mChecked;

	}

	public void setChecked(boolean checked) {

		if (mChecked != checked) {
			this.mChecked = checked;
			persistBoolean(checked);
		}
        notifyDependencyChange(shouldDisableDependents());
        notifyChanged();
	}

	@Override
	public boolean isPersistent() {
		// TODO Auto-generated method stub
		return false;

	}

	@Override
	public boolean shouldDisableDependents() {
		// TODO Auto-generated method stub
		boolean shouldDisable = mDisableDependentsState ? mChecked : !mChecked;
		return shouldDisable || super.shouldDisableDependents();

	}

	@Override
	protected Object onGetDefaultValue(TypedArray a, int index) {
		return a.getBoolean(index, false);
	}

	@Override
	protected void onSetInitialValue(boolean restoreValue, Object defaultValue) {
		setChecked(restoreValue ? getPersistedBoolean(mChecked)
				: (Boolean) defaultValue);
	}

	@Override
	protected Parcelable onSaveInstanceState() {
		final Parcelable superState = super.onSaveInstanceState();
		if (isPersistent()) {
			// No need to save instance state since it's persistent
			return superState;
		}

		final SavedState myState = new SavedState(superState);
		myState.checked = isChecked();
		return myState;
	}

	@Override
	protected void onRestoreInstanceState(Parcelable state) {
		if (state == null || !state.getClass().equals(SavedState.class)) {
			// Didn't save state for us in onSaveInstanceState
			super.onRestoreInstanceState(state);
			return;
		}

		SavedState myState = (SavedState) state;
		super.onRestoreInstanceState(myState.getSuperState());
		setChecked(myState.checked);
	}

	private static class SavedState extends BaseSavedState {
		boolean checked;

		public SavedState(Parcel source) {
			super(source);
			checked = source.readInt() == 1;
		}

		@Override
		public void writeToParcel(Parcel dest, int flags) {
			super.writeToParcel(dest, flags);
			dest.writeInt(checked ? 1 : 0);
		}

		public SavedState(Parcelable superState) {
			super(superState);
		}

		public static final Parcelable.Creator<SavedState> CREATOR = new Parcelable.Creator<SavedState>() {
			public SavedState createFromParcel(Parcel in) {
				return new SavedState(in);
			}

			public SavedState[] newArray(int size) {
				return new SavedState[size];
			}
		};
	}

}
