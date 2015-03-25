package com.mediatek.appguide.plugin.contacts;

import android.content.Context;

import com.mediatek.contacts.ext.ContactAccountExtension;
import com.mediatek.contacts.ext.ContactPluginDefault;

public class ContactsPlugin extends ContactPluginDefault {

    private Context mContext;

    public ContactsPlugin(Context context) {
        mContext = context;
    }

    public ContactAccountExtension createContactAccountExtension() {
        return new SwitchSimContactsExt(mContext);
    }
}
