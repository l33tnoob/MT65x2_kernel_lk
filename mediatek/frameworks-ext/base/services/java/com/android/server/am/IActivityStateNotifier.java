package com.android.server.am;

import java.util.HashSet;

public interface IActivityStateNotifier {

    public enum ActivityState {
        Paused,
        Resumed,
        Destroyed
    }

    /**
     * Notify activity state change
     */
    public void notifyActivityState(String packageName, String className, ActivityState actState);

    /**
     * Notify the process of activity has died
     */
    public void notifyAppDied(HashSet<String> packageList);
}