
package com.mediatek.ngin3d.tests;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLSurfaceView;
import android.os.Environment;
import android.os.SystemClock;
import android.util.Log;
import android.view.SurfaceHolder;
import com.mediatek.ngin3d.BitmapFont;
import com.mediatek.ngin3d.BitmapText;
import com.mediatek.ngin3d.Container;
import com.mediatek.ngin3d.Empty;
import com.mediatek.ngin3d.Glo3D;
import com.mediatek.ngin3d.Image;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Rotation;
import com.mediatek.ngin3d.Scale;
import com.mediatek.ngin3d.Stage;
import com.mediatek.ngin3d.Text;
import com.mediatek.ngin3d.android.StageWallpaperService;
import com.mediatek.ngin3d.animation.AnimationGroup;
import com.mediatek.ngin3d.animation.PropertyAnimation;
import com.mediatek.ngin3d.presentation.PresentationEngine;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import java.io.File;
import java.io.IOException;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.FutureTask;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.lessThanOrEqualTo;

public class MyWallpaperService extends StageWallpaperService {
    private static final String TAG = "MyWallpaperService";
    public MyWallpaperEngine mWallpaperEngine;

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "onCreate(): " + this);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "onStartCommand(): " + this);
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "onDestroy(): " + this);
    }

    @Override
    public Engine onCreateEngine() {
        Log.d(TAG, "onCreateEngine(): " + this);
        return new MyWallpaperEngine();
    }

    public Stage getStage() {
        return mWallpaperEngine.getStage();
    }

    class MyWallpaperEngine extends StageEngine {
        private MyWallpaperEngine mEngine;
        private PresentationEngine mPresentationEngine;
        private final Stage mStage;
        private Container mContainer;
        private WallpaperObserver mReceiver;

        class WallpaperObserver extends BroadcastReceiver {
            public void onReceive(Context context, Intent intent) {
                Log.d(TAG, "Receive wallpaper intent");
                waitSurfaceReady();
                mPresentationEngine = getPresentationEngine();
                // use for auto test
                try {
                    if (intent.getAction().equals(MyWallpaperServiceTest.ENGINE_INITIALIZE)) {
                        Log.d(TAG, "Receive engine initialize intent: ");
                        test01_EngineInitialTime();
                    } else if (intent.getAction().equals(MyWallpaperServiceTest.IMAGE_LOADING_FROM_RESOURCE)) {
                        Log.d(TAG, "Receive IMAGE_LOADING_FROM_RESOURCE intent: ");
                        test02_ImageLoadingFromResourceTime();
                    } else if (intent.getAction().equals(MyWallpaperServiceTest.IMAGE_LOADING_FROM_BITMAP)) {
                        Log.d(TAG, "Receive IMAGE_LOADING_FROM_BITMAP intent: ");
                        test03_ImageLoadingFromBitmapTime();
                    } else if (intent.getAction().equals(MyWallpaperServiceTest.IMAGE_LOADING_FROM_FILE)) {
                        Log.d(TAG, "Receive IMAGE_LOADING_FROM_FILE intent: ");
                        test04_ImageLoadingFromFile();
                    } else if (intent.getAction().equals(MyWallpaperServiceTest.UPDATE_SYSTEM_TEXT)) {
                        Log.d(TAG, "Receive UPDATE_SYSTEM_TEXT intent: ");
                        test05_UpdateSystemTextContent();
                    } else if (intent.getAction().equals(MyWallpaperServiceTest.UPDATE_BITMAP_TEXT)) {
                        Log.d(TAG, "Receive UPDATE_BITMAP_TEXT intent: ");
                        test06_UpdateBitmapTextContent();
                    } else if (intent.getAction().equals(MyWallpaperServiceTest.RENDER_50_ACTOR)) {
                        Log.d(TAG, "Receive RENDER_50_ACTOR intent: ");
                        test07_Render50Actor();
                    } else if (intent.getAction().equals(MyWallpaperServiceTest.RENDER_100_ACTOR)) {
                        Log.d(TAG, "Receive RENDER_100_ACTOR intent: ");
                        test08_Render100Actor();
                    } else if (intent.getAction().equals(MyWallpaperServiceTest.START_50_ANIMATION)) {
                        Log.d(TAG, "Receive START_50_ANIMATION intent: ");
                        test09_Start50Animation();
                    } else if (intent.getAction().equals(MyWallpaperServiceTest.START_100_ANIMATION)) {
                        Log.d(TAG, "Receive START_100_ANIMATION intent: ");
                        test10_Start100Animation();
                    } else if (intent.getAction().equals(MyWallpaperServiceTest.TEST_SCREENSHOT)) {
                        Log.d(TAG, "Receive TEST_SCREENSHOT intent: ");
                        test11_ScreenShot();
                    } else if (intent.getAction().equals(MyWallpaperServiceTest.RENDER_25_LANDSCAPE)) {
                        Log.d(TAG, "Receive RENDER_25_LANDSCAPE intent: ");
                        test12_Render25Landscapes();
                    }
                } catch (ExecutionException e) {
                    e.printStackTrace();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }

        @Override
        public void onCreate(SurfaceHolder surfaceHolder) {
            super.onCreate(surfaceHolder);
            Log.d(TAG, "MyWallpaperEngine onCreate(): " + this);

            mReceiver = new WallpaperObserver();
            // register intent filter
            IntentFilter engineInitialize = new IntentFilter(MyWallpaperServiceTest.ENGINE_INITIALIZE);
            registerReceiver(mReceiver, engineInitialize);
            IntentFilter resourceLoading = new IntentFilter(MyWallpaperServiceTest.IMAGE_LOADING_FROM_RESOURCE);
            registerReceiver(mReceiver, resourceLoading);
            IntentFilter bitmapLoading = new IntentFilter(MyWallpaperServiceTest.IMAGE_LOADING_FROM_BITMAP);
            registerReceiver(mReceiver, bitmapLoading);
            IntentFilter fileLoading = new IntentFilter(MyWallpaperServiceTest.IMAGE_LOADING_FROM_FILE);
            registerReceiver(mReceiver, fileLoading);
            IntentFilter systemText = new IntentFilter(MyWallpaperServiceTest.UPDATE_SYSTEM_TEXT);
            registerReceiver(mReceiver, systemText);
            IntentFilter bitmapText = new IntentFilter(MyWallpaperServiceTest.UPDATE_BITMAP_TEXT);
            registerReceiver(mReceiver, bitmapText);
            IntentFilter render50Actor = new IntentFilter(MyWallpaperServiceTest.RENDER_50_ACTOR);
            registerReceiver(mReceiver, render50Actor);
            IntentFilter render100Actor = new IntentFilter(MyWallpaperServiceTest.RENDER_100_ACTOR);
            registerReceiver(mReceiver, render100Actor);
            IntentFilter start50Animation = new IntentFilter(MyWallpaperServiceTest.START_50_ANIMATION);
            registerReceiver(mReceiver, start50Animation);
            IntentFilter start100Animation = new IntentFilter(MyWallpaperServiceTest.START_100_ANIMATION);
            registerReceiver(mReceiver, start100Animation);
            IntentFilter testScreenshot = new IntentFilter(MyWallpaperServiceTest.TEST_SCREENSHOT);
            registerReceiver(mReceiver, testScreenshot);
            IntentFilter render25Landscape = new IntentFilter(MyWallpaperServiceTest.RENDER_25_LANDSCAPE);
            registerReceiver(mReceiver, render25Landscape);

            mContainer = new Container();

            mStage.add(mContainer);
            setTouchEventsEnabled(true);
            setRenderMode(RENDERMODE_CONTINUOUSLY);
        }

        @Override
        public void onDestroy() {
            super.onDestroy();
            Log.d(TAG, "MyWallpaperEngine onDestroy(): " + this);
            super.onVisibilityChanged(false);
            unregisterReceiver(mReceiver);
        }

        @Override
        public void onVisibilityChanged(boolean visible) {
            super.onVisibilityChanged(true);
        }

        public MyWallpaperEngine() {
            super();
            mStage = getStage();
            mEngine = this;
        }

        public void test01_EngineInitialTime() throws ExecutionException, InterruptedException {
            FutureTask<Boolean> task = Ngin3dTestCaseProvider.getEngineInitialTask(mPresentationEngine, null);
            queueEvent(task);
            assertThat(task.get(), is(true));
        }

        public void test02_ImageLoadingFromResourceTime() throws ExecutionException, InterruptedException {
            FutureTask<Boolean> task = Ngin3dTestCaseProvider.getImageLoadingFromResourceTask(mPresentationEngine,
                getResources());
            queueEvent(task);
            assertThat(task.get(), is(true));
        }

        public void test03_ImageLoadingFromBitmapTime() throws ExecutionException, InterruptedException {
            FutureTask<Boolean> task = Ngin3dTestCaseProvider.getImageLoadingFromBitmapTask(mPresentationEngine,
                getResources());
            queueEvent(task);
            assertThat(task.get(), is(true));
        }

        public void test04_ImageLoadingFromFile() throws ExecutionException, InterruptedException {
            FutureTask<Boolean> task = Ngin3dTestCaseProvider.getImageLoadingFromFileTask(mPresentationEngine, null);
            queueEvent(task);
            assertThat(task.get(), is(true));
        }

        public void test05_UpdateSystemTextContent() throws ExecutionException, InterruptedException {
            FutureTask<Boolean> task = Ngin3dTestCaseProvider.getUpdateSystemTextTask(mPresentationEngine, null);
            queueEvent(task);
            assertThat(task.get(), is(true));
        }

        public void test06_UpdateBitmapTextContent() throws ExecutionException, InterruptedException {
            FutureTask<Boolean> task = Ngin3dTestCaseProvider.getUpdateBitmapTextTask(mPresentationEngine, getResources());
            queueEvent(task);
            assertThat(task.get(), is(true));
        }

        private void test07_Render50Actor() throws ExecutionException, InterruptedException {
            FutureTask<Boolean> task = Ngin3dTestCaseProvider.getRender50ActorsTask(mStage, mPresentationEngine, null);
            queueEvent(task);
            assertThat(task.get(), is(true));
        }

        public void test08_Render100Actor() throws ExecutionException, InterruptedException {
            FutureTask<Boolean> task = Ngin3dTestCaseProvider.getRender100ActorsTask(mStage, mPresentationEngine, null);
            queueEvent(task);
            assertThat(task.get(), is(true));
        }

        public void test09_Start50Animation() throws ExecutionException, InterruptedException {
            FutureTask<Boolean> task = Ngin3dTestCaseProvider.getStart50AnimationsTask(null);
            queueEvent(task);
            assertThat(task.get(), is(true));
        }

        public void test10_Start100Animation() throws ExecutionException, InterruptedException {
            FutureTask<Boolean> task = Ngin3dTestCaseProvider.getStart100AnimationsTask(null);
            queueEvent(task);
            assertThat(task.get(), is(true));
        }

        public void test11_ScreenShot() throws ExecutionException, InterruptedException {
            // Todo: Should we do it for WallpaperService?
        }

        public void test12_Render25Landscapes() {
            setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
            Ngin3dTestCaseProvider.render25LandscapesTest(mStage, mPresentationEngine, null);
            setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        }
    }
}
