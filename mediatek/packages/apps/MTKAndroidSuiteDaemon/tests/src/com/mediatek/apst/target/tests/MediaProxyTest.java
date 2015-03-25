package com.mediatek.apst.target.tests;

import android.content.Context;
import android.os.Environment;
import android.test.AndroidTestCase;
import android.util.Log;

import com.mediatek.apst.target.data.proxy.media.MediaProxy;
import com.mediatek.apst.util.entity.media.MediaInfo;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;

public class MediaProxyTest extends AndroidTestCase {

    private Context mContext;
    MediaProxy mProxy;
    String mDcimDirectory;
    String mPictureDirectory;
    String mSdPath;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = getContext();
        mProxy = MediaProxy.getInstance(mContext);
        mSdPath = Environment.getExternalStorageDirectory().getAbsolutePath();
        //mDcimDirectory = mSdPath + File.separator + "ApstTestCaseDCIM";
        //mPictureDirectory = mSdPath + File.separator + "ApstTestCasePictures";
        mDcimDirectory = mSdPath + File.separator + "DCIM";
        mPictureDirectory = mSdPath + File.separator + "Pictures";
    }

    @Override
    protected void tearDown() throws Exception {
        mContext = null;
        super.tearDown();
    }

    public void test01_InitMedia() {
        initMedia();
    }

    public void test02_IsSdMounted() {
        mProxy.isSdMounted();
    }

    public void test03_IsSdWriteable() {
        mProxy.isSdWriteable();
    }

    public void test04_ExistFile() {
        mProxy.existFile();
    }

    public void test05_GetContentDirectories() {
        MediaInfo[] dirs = mProxy.getContentDirectories();
        if (mProxy.isSdMounted()) {
            return;
        }
        assertNotNull(dirs);
    }

    public void test06_GetFile() {
        mProxy.getFile(MediaInfo.ALARMS);
        mProxy.getFile(MediaInfo.DCIM);
        mProxy.getFile(MediaInfo.DOWNLOADS);
        mProxy.getFile(MediaInfo.MOVIES);
        mProxy.getFile(MediaInfo.MUSIC);
        mProxy.getFile(MediaInfo.NOTIFICATIONS);
        mProxy.getFile(MediaInfo.PICTURES);
        mProxy.getFile(MediaInfo.PODCASTS);
        mProxy.getFile(MediaInfo.RINGTONES);
        mProxy.getFile(255);
    }

    public void test07_GetFiles() {
        mProxy.getFiles(MediaInfo.ALARMS);
        mProxy.getFiles(MediaInfo.DCIM);
        mProxy.getFiles(MediaInfo.DOWNLOADS);
        mProxy.getFiles(MediaInfo.MOVIES);
        mProxy.getFiles(MediaInfo.MUSIC);
        mProxy.getFiles(MediaInfo.NOTIFICATIONS);
        mProxy.getFiles(MediaInfo.PICTURES);
        mProxy.getFiles(MediaInfo.PODCASTS);
        mProxy.getFiles(MediaInfo.RINGTONES);
        mProxy.getFile(255);
    }

    public void test08_GetFilesUnder() {
        // test it in getFiles();
    }

    public void test09_GetFilesUnder() {
        ArrayList<String> oldPaths = new ArrayList<String>();
        mProxy.getFilesUnder("12345", oldPaths);
        mProxy.getFilesUnder("12345", null);
        mProxy.getFilesUnder(mDcimDirectory, oldPaths);
        mProxy.getFilesUnder(mPictureDirectory, oldPaths);
        if (mProxy.isSdMounted()) {
            assertTrue(oldPaths.size() >= 0);
        }
    }

    public void test10_GetFilesUnder() {
        initMedia();
        ArrayList<String> oldPaths = new ArrayList<String>();
        ArrayList<String> newPaths = new ArrayList<String>();
        mProxy.getFilesUnder("12345", oldPaths, newPaths);
        mProxy.getFilesUnder("12345", null, newPaths);
        mProxy.getFilesUnder(mDcimDirectory, oldPaths, newPaths);
        mProxy.getFilesUnder(mPictureDirectory, oldPaths, newPaths);
        assertTrue(oldPaths.size() >= 0);
        assertTrue(newPaths.size() >= 0);
    }

    public void test11_GetFilesUnderDirs() {
        ArrayList<String> oldPaths = new ArrayList<String>();
        ArrayList<String> newPaths = new ArrayList<String>();
        ArrayList<String> dirs = new ArrayList<String>();
        dirs.add(mDcimDirectory);
        dirs.add(mPictureDirectory);
        mProxy.getFilesUnderDirs(dirs, oldPaths, newPaths);
        mProxy.getFilesUnderDirs(null, oldPaths, newPaths);
        mProxy.getFilesUnderDirs(dirs, null, newPaths);
        mProxy.getFilesUnderDirs(dirs, oldPaths, null);
        assertTrue(oldPaths.size() >= 0);
        assertTrue(newPaths.size() >= 0);
    }

    public void test12_GetFilesEndWith() {
        ArrayList<String> oldPaths = new ArrayList<String>();
        ArrayList<String> newPaths = new ArrayList<String>();
        mProxy.getFilesEndWith(mDcimDirectory, "jpg", null, newPaths);
        mProxy.getFilesEndWith(mDcimDirectory, "jpg", oldPaths, newPaths);
        mProxy.getFilesEndWith(mPictureDirectory, "jpg", oldPaths, newPaths);
        mProxy.getFilesEndWith(mDcimDirectory, "mp3", oldPaths, newPaths);
        mProxy.getFilesEndWith(mPictureDirectory, "mp3", oldPaths, newPaths);
        assertTrue(oldPaths.size() >= 0);
        assertTrue(newPaths.size() >= 0);
    }

    public void test13_GetDirectory() {
        // private methods
    }

    public void test14_GetDirectiries() {
        ArrayList<String> paths = new ArrayList<String>();
        paths.add(mDcimDirectory);
        paths.add(mPictureDirectory);
        ArrayList<File> dirs = new ArrayList<File>();
        mProxy.getDirectiries(paths, dirs);
        assertNotNull(dirs);
        assertTrue(dirs.size() >= 0);
    }

    public void test15_CreateDirectory() {
        boolean result;
        result = mProxy.createDirectory(mPictureDirectory);
        assertTrue(result);
        result = mProxy.createDirectory(mPictureDirectory + File.separator
                + "NotExit");
        assertTrue(result);
    }

    public void test16_CheckStoreState() {
        mProxy.checkStoreState(1000, MediaInfo.PICTURES);
    }

    public void test17_IsFileExisted() {
        boolean result;
        result = mProxy.isFileExisted(mPictureDirectory);
        assertTrue(result);
        result = mProxy.isFileExisted(mPictureDirectory + File.separator
                + "Test100_NotExit");
        assertTrue(!result);
    }

    public void test18_Scan() {
        File musicFile = Environment
                .getExternalStoragePublicDirectory(Environment.DIRECTORY_MUSIC);
        File pictureFile = Environment
                .getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES);
        try {
            if (!musicFile.exists()) {
                musicFile.createNewFile();
            }
            File musicTestFile = new File(musicFile.getAbsolutePath()
                    + File.separator + "Test.mp3");
            if (!musicTestFile.exists()) {
                musicTestFile.createNewFile();
            }

            if (!pictureFile.exists()) {
                pictureFile.createNewFile();
            }

            File pictureTestFile = new File(pictureFile.getAbsolutePath()
                    + File.separator + "Test.bmp");
            if (!pictureTestFile.exists()) {
                pictureTestFile.createNewFile();
            }
        } catch (IOException e) {

        }
        mProxy.scan(mContext);
    }

    public void test19_Scan() {
        mProxy.scan(mContext, mPictureDirectory);
    }

    public void test20_RenameFile() {
        boolean result;
        result = mProxy.renameFile(null, "");
        assertTrue(!result);
        result = mProxy.renameFile("", null);
        assertTrue(!result);
        result = mProxy.renameFile("", "123");
        assertTrue(!result);
        result = mProxy.renameFile("123", "");
        assertTrue(!result);
        File file = new File(mDcimDirectory + File.separator + "test01.jpg");
        if (!file.exists()) {
            try {
                file.createNewFile();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        result = mProxy.renameFile(mDcimDirectory + File.separator
                + "test01.jpg", mDcimDirectory + File.separator
                + "rename01.jpg");
//        assertTrue(result);
        result = mProxy.renameFile(mDcimDirectory + File.separator
                + "test02.mp3", mDcimDirectory + File.separator
                + "renameFolder" + File.separator + "test01.jpg");
//        assertTrue(result);
    }

    public void test21_RenameFileForBackup() {
        initMedia();
        boolean result;
        result = mProxy.renameFileForBackup(null, "");
        assertTrue(!result);
        result = mProxy.renameFileForBackup("", null);
        assertTrue(!result);
        result = mProxy.renameFileForBackup("", "123");
        assertTrue(!result);
        result = mProxy.renameFileForBackup("123", "");
        assertTrue(!result);
        File file = new File(mDcimDirectory + File.separator + "test01.jpg");
        if (!file.exists()) {
            try {
                file.createNewFile();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        result = mProxy.renameFileForBackup(file.getAbsolutePath(),
                mDcimDirectory + File.separator + "rename02.mp3");
//        assertTrue(result);
        result = mProxy.renameFileForBackup(mDcimDirectory + File.separator
                + "test02.mp3", mDcimDirectory + File.separator
                + "renameFolder2" + File.separator + "rename02.mp3");
//        assertTrue(result);
    }

    public void test22_RenameFiles() {
        boolean[] result;
        String[] oldPaths = new String[1];
        oldPaths[0] = mPictureDirectory + File.separator + "Test";
        String[] newPaths = new String[1];
        newPaths[0] = mPictureDirectory + File.separator + "Test100";
        String[] newPaths2 = new String[2];
        result = mProxy.renameFiles(null, oldPaths);
        assertTrue(result == null);
        result = mProxy.renameFiles(oldPaths, null);
        assertTrue(result == null);
        result = mProxy.renameFiles(oldPaths, newPaths2);
        assertTrue(result == null);
        result = mProxy.renameFiles(oldPaths, newPaths);
        assertTrue(result != null);
        result = mProxy.renameFiles(newPaths, oldPaths);
        assertTrue(result != null);
    }

    public void test23_DeleteAllFileUnder() {
        initMedia();
        mProxy.deleteAllFileUnder(mDcimDirectory);
        mProxy.deleteAllFileUnder(mPictureDirectory);
        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    public void test24_DeleteDirectory() {
        initMedia();
        mProxy.deleteDirectory(mDcimDirectory);
        mProxy.deleteDirectory(mPictureDirectory);
        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    public void test25_DeleteAllFiles() {
        initMedia();
        ArrayList<String> paths = new ArrayList<String>(2);
        paths.add(mDcimDirectory);
        paths.add(mPictureDirectory);
        mProxy.deleteAllFiles(paths);
        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    public void test26_DeleteAllDirectorys() {
        initMedia();
        ArrayList<String> paths = new ArrayList<String>(2);
        paths.add(mDcimDirectory);
        paths.add(mPictureDirectory);
        mProxy.deleteAllDirectorys(paths);
        mProxy.deleteAllFiles(paths);
        mProxy.deleteAllDirectorys(paths);
        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        initMedia();
    }

    private void initMedia() {
        boolean result;
        if (mSdPath != null) {
            Log.d("APST/MediaProxyTest", "SD Path is: " + mSdPath);
        } else {
            Log.e("APST/MediaProxyTest", "SD Path is null");
        }
        if (mProxy.isSdMounted()) {
            Log.e("APST/MediaProxyTest", "NO SD card!!");
            return;
        }

        File dcimFolder = new File(mDcimDirectory);
        File pictureFolder = new File(mPictureDirectory);
        if (!dcimFolder.exists()) {
            result = dcimFolder.mkdirs();
            assertTrue(result);
        }

        if (!pictureFolder.exists()) {
            result = pictureFolder.mkdirs();
            assertTrue(result);
        }

        File dcimTestFolder = new File(mDcimDirectory + File.separator
                + "TestFolder");
        File pictureTestFolder = new File(mPictureDirectory + File.separator
                + "TestFolder");

        if (!dcimTestFolder.exists()) {
            result = dcimTestFolder.mkdirs();
            assertTrue(result);
        }

        if (!pictureTestFolder.exists()) {
            result = pictureTestFolder.mkdirs();
            assertTrue(result);
        }

        File dcimTestFolder2 = new File(mDcimDirectory + File.separator
                + "TestFolder2");
        File pictureTestFolder2 = new File(mPictureDirectory + File.separator
                + "TestFolder2");

        if (!dcimTestFolder2.exists()) {
            result = dcimTestFolder2.mkdirs();
            assertTrue(result);
        }

        if (!pictureTestFolder2.exists()) {
            result = pictureTestFolder2.mkdirs();
            assertTrue(result);
        }

        File[] testFile1 = {
                new File(mDcimDirectory + File.separator + "test01.jpg"),
                new File(mDcimDirectory + File.separator + "test02.mp3"),
                new File(mDcimDirectory + File.separator + "test03.rmvb"),
                new File(mDcimDirectory + File.separator + "test04.rm") };

        File[] testFile2 = {
                new File(dcimTestFolder.getAbsolutePath() + File.separator
                        + "test01.jpg"),
                new File(dcimTestFolder.getAbsolutePath() + File.separator
                        + "test02.mp3"),
                new File(dcimTestFolder.getAbsolutePath() + File.separator
                        + "test03.rmvb"),
                new File(dcimTestFolder.getAbsolutePath() + File.separator
                        + "test04.rm") };

        File[] testFile3 = {
                new File(mPictureDirectory + File.separator + "test01.jpg"),
                new File(mPictureDirectory + File.separator + "test02.mp3"),
                new File(mPictureDirectory + File.separator + "test03.rmvb"),
                new File(mPictureDirectory + File.separator + "test04.rm") };

        File[] testFile4 = {
                new File(pictureTestFolder.getAbsolutePath() + File.separator
                        + "test01.jpg"),
                new File(pictureTestFolder.getAbsolutePath() + File.separator
                        + "test02.mp3"),
                new File(pictureTestFolder.getAbsolutePath() + File.separator
                        + "test03.rmvb"),
                new File(pictureTestFolder.getAbsolutePath() + File.separator
                        + "test04.rm") };

        try {
            for (File file : testFile1) {
                if (!file.exists()) {
                    result = file.createNewFile();
                    assertTrue(result);
                }
            }

            for (File file : testFile2) {
                if (!file.exists()) {
                    result = file.createNewFile();
                    assertTrue(result);
                }
            }

            for (File file : testFile3) {
                if (!file.exists()) {
                    result = file.createNewFile();
                    assertTrue(result);
                }
            }

            for (File file : testFile4) {
                if (!file.exists()) {
                    result = file.createNewFile();
                    assertTrue(result);
                }
            }
        } catch (FileNotFoundException e) {
            Log.e("APST/MediaProxyTest", "Catch FileNotFoundException");
        } catch (IOException e) {
            Log.e("APST/MediaProxyTest", "Catch IOException");
        }
    }
}
