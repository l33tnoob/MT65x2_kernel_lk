package com.mediatek.common.voicecommand;

import android.os.Bundle;
import com.mediatek.common.voicecommand.IVoiceCommandListener;

interface IVoiceCommandManagerService {

    int registerListener(String pkgName, IVoiceCommandListener listener);

    int unregisterListener(String pkgName, IVoiceCommandListener listener);

    int sendCommand(String pkgName, int mainAction ,int subAction , in Bundle extraData);

}
