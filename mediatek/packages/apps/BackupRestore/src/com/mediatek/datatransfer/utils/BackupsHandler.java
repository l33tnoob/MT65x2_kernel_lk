package com.mediatek.datatransfer.utils;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

public interface BackupsHandler {
    List<File> result = new ArrayList<File>();

    public boolean init();

    public void onStart();

    public void reset();

    public void cancel();

    public List<File> onEnd();

    public String getBackupType();
}
