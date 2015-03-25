package com.hissage.util.message;

import android.content.Context;
import android.content.Intent;

import com.hissage.contact.NmsContactManager;
import com.hissage.ui.activity.NmsContactSelectionActivity;
import com.hissage.ui.activity.NmsGroupChatInfoActivity;
import com.hissage.ui.activity.NmsNewEditGroupChatActivity;
import com.hissage.util.log.NmsLog;

public class NmsUIApiUtils {

    public final static String Tag = "NmsUIApiUtils";

    public static void createGroupChat(Context context) {
        Intent i = new Intent(context, NmsContactSelectionActivity.class);
        i.putExtra(NmsContactSelectionActivity.LOADTYPE, NmsContactManager.TYPE_HISSAGE);
        i.putExtra(NmsContactSelectionActivity.SELECTMAX, 10);
        context.startActivity(i);
    }

    public static void editGroupChat(Context context, short groupId) {
        Intent i = new Intent(context, NmsNewEditGroupChatActivity.class);
        i.putExtra("groupId", groupId);

        NmsLog.trace(Tag, "edit group chat, the groupId:" + groupId);
        context.startActivity(i);
    }

    public static void viewGroupChatInfo(Context context, short groupId) {
        Intent i = new Intent(context, NmsGroupChatInfoActivity.class);
        i.putExtra("groupId", groupId);
        
        NmsLog.trace(Tag, "view group chat information, the groupId:" + groupId);
        context.startActivity(i);
    }
}
