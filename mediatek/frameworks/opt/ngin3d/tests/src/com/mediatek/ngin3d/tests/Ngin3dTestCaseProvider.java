package com.mediatek.ngin3d.tests;

import android.app.Activity;
import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Environment;
import android.os.SystemClock;
import android.util.Log;
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
import java.io.FileWriter;
import java.io.IOException;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.FutureTask;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.greaterThanOrEqualTo;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.lessThanOrEqualTo;

public class Ngin3dTestCaseProvider {
    private static final String TAG = "Ngin3dTestCaseProvider";
    // Test criteria
    public static long ENGINE_INITIALIZATION_TIME_CRITERIA = 0;
    public static long IMAGE_LOADING_FROM_RESOURCE_TIME_CRITERIA = 0;
    public static long IMAGE_LOADING_FROM_ASSET_TIME_CRITERIA = 0;
    public static long IMAGE_LOADING_FROM_BITMAP_TIME_CRITERIA = 0;
    public static long IMAGE_LOADING_FROM_FILE_TIME_CRITERIA = 0;
    public static long SYSTEM_TEXT_CONTENT_UPDATE_TIME_CRITERIA = 0;
    public static long BITMAP_FONT_TEXT_CONTENT_UPDATE_TIME_CRITERIA = 0;
    public static long RENDER_50_ACTORS_TIME_CRITERIA = 0;
    public static long RENDER_100_ACTORS_TIME_CRITERIA = 0;
    public static long START_50_ANIMATIONS_TIME_CRITERIA = 0;
    public static long START_100_ANIMATIONS_TIME_CRITERIA = 0;
    public static long SCREEN_SHOT_TIME_CRITERIA = 0;
    public static long RENDER_25_LANDSCAPES_FPS_CRITERIA = 0;

    /**
     * Causes the current thread to sleep for a specified period.
     *
     * @param ms Sleep time in milliseconds
     */
    private static void sleep(int ms) {
        try {
            Thread.sleep(ms);
        } catch (InterruptedException ie) {
            // Do nothing
        }
    }

    public static FutureTask<Boolean> getEngineInitialTask(
        final PresentationEngine engine, final Activity activity) throws ExecutionException, InterruptedException {

        return new FutureTask<Boolean>(new Callable<Boolean>() {
            public Boolean call() {
                engine.uninitialize();
                Log.v(TAG, "Start engine initialize");
                long t1 = SystemClock.uptimeMillis();
                engine.initialize(480, 800);
                long t2 = SystemClock.uptimeMillis() - t1;
                Log.v(TAG, "Engine initialize costs: " + t2);

                writePerformanceData(activity, "ngin3d.initialize-time.txt", t2);
                assertThat(t2, is(lessThanOrEqualTo(ENGINE_INITIALIZATION_TIME_CRITERIA)));
                return true;
            }
        });
    }

    public static FutureTask<Boolean> getImageLoadingFromResourceTask(
        final PresentationEngine engine, final Resources resources) throws ExecutionException, InterruptedException {
        return getImageLoadingFromResourceTask(engine, null, resources);
    }

    public static FutureTask<Boolean> getImageLoadingFromResourceTask(
        final PresentationEngine engine, final Activity activity) throws ExecutionException, InterruptedException {
        return getImageLoadingFromResourceTask(engine, activity, activity.getResources());
    }

    private static FutureTask<Boolean> getImageLoadingFromResourceTask(
        final PresentationEngine engine, final Activity activity,
        final Resources resources) throws ExecutionException, InterruptedException {

        return new FutureTask<Boolean>(new Callable<Boolean>() {
            public Boolean call() {
                Log.v(TAG, "Start loading images from png resource png");

                long png1 = SystemClock.uptimeMillis();
                Image image = Image.createFromResource(resources, R.drawable.earthpng);
                image.realize(engine);
                long png2 = SystemClock.uptimeMillis() - png1;
                Log.v(TAG, "Loading images from png resource costs: " + png2);
                writePerformanceData(activity, "ngin3d.image_loading_from_png_resource-time.txt", png2);
                assertThat(png2, is(lessThanOrEqualTo(IMAGE_LOADING_FROM_RESOURCE_TIME_CRITERIA)));

                Log.v(TAG, "Start loading images from jpg resource");
                long jpg1 = SystemClock.uptimeMillis();
                image = Image.createFromResource(resources, R.drawable.earthjpg);
                image.realize(engine);
                long jpg2 = SystemClock.uptimeMillis() - jpg1;
                Log.v(TAG, "Loading images from jpg resource costs: " + jpg2);
                writePerformanceData(activity, "ngin3d.image_loading_from_jpg_resource-time.txt", jpg2);
                assertThat(jpg2, is(lessThanOrEqualTo(IMAGE_LOADING_FROM_RESOURCE_TIME_CRITERIA)));

                Log.v(TAG, "Start loading images from bmp resource");
                long bmp1 = SystemClock.uptimeMillis();
                image = Image.createFromResource(resources, R.drawable.earthbmp);
                image.realize(engine);
                long bmp2 = SystemClock.uptimeMillis() - bmp1;
                Log.v(TAG, "Loading images from bmp resource costs: " + bmp2);
                writePerformanceData(activity, "ngin3d.image_loading_from_bmp_resource-time.txt", bmp2);
                assertThat(bmp2, is(lessThanOrEqualTo(IMAGE_LOADING_FROM_RESOURCE_TIME_CRITERIA)));

                return true;
            }
        });

    }

    public static FutureTask<Boolean> getImageLoadingFromAssetTask(
        final PresentationEngine engine, final Activity activity) throws ExecutionException, InterruptedException {

        return new FutureTask<Boolean>(new Callable<Boolean>() {
            public Boolean call() {
                Log.v(TAG, "Start loading images from png asset");
                long png1 = SystemClock.uptimeMillis();
                Image image = Image.createFromAsset("earth.png");
                image.realize(engine);
                long png2 = SystemClock.uptimeMillis() - png1;
                Log.v(TAG, "Loading images from png asset costs: " + png2);
                writePerformanceData(activity, "ngin3d.image_loading_from_jpg_asset-time.txt", png2);
                assertThat(png2, is(lessThanOrEqualTo(IMAGE_LOADING_FROM_ASSET_TIME_CRITERIA)));

                Log.v(TAG, "Start loading images from jpg asset");
                long jpg1 = SystemClock.uptimeMillis();
                image = Image.createFromAsset("earth.jpg");
                image.realize(engine);
                long jpg2 = SystemClock.uptimeMillis() - jpg1;
                Log.v(TAG, "Loading images from jpg asset costs: " + jpg2);
                writePerformanceData(activity, "ngin3d.image_loading_from_jpg_asset-time.txt", jpg2);
                assertThat(jpg2, is(lessThanOrEqualTo(IMAGE_LOADING_FROM_ASSET_TIME_CRITERIA)));

                Log.v(TAG, "Start loading images from bmp asset");
                long bmp1 = SystemClock.uptimeMillis();
                image = Image.createFromAsset("earth.bmp");
                image.realize(engine);
                long bmp2 = SystemClock.uptimeMillis() - bmp1;
                Log.v(TAG, "Loading images from bmp asset costs: " + bmp2);
                writePerformanceData(activity, "ngin3d.image_loading_from_bmp_asset-time.txt", bmp2);
                assertThat(bmp2, is(lessThanOrEqualTo(IMAGE_LOADING_FROM_ASSET_TIME_CRITERIA)));

                return true;
            }
        });
    }

    public static FutureTask<Boolean> getImageLoadingFromBitmapTask(
        final PresentationEngine engine, final Resources resources) throws ExecutionException, InterruptedException {
        return getImageLoadingFromBitmapTask(engine, null, resources);
    }

    public static FutureTask<Boolean> getImageLoadingFromBitmapTask(
        final PresentationEngine engine, final Activity activity) throws ExecutionException, InterruptedException {
        return getImageLoadingFromBitmapTask(engine, activity, activity.getResources());
    }

    private static FutureTask<Boolean> getImageLoadingFromBitmapTask(
        final PresentationEngine engine, final Activity activity, final Resources resources)
        throws ExecutionException, InterruptedException {

        return new FutureTask<Boolean>(new Callable<Boolean>() {
            public Boolean call() {
                Log.v(TAG, "Start loading images from bitmap");
                long t1 = SystemClock.uptimeMillis();
                Bitmap bitmap = BitmapFactory.decodeResource(resources, R.drawable.earth);
                Image image = Image.createFromBitmap(bitmap);
                image.realize(engine);
                long t2 = SystemClock.uptimeMillis() - t1;
                Log.v(TAG, "Loading images from bitmap costs: " + t2);

                writePerformanceData(activity, "ngin3d.image_loading_from_bitmap-time.txt", t2);
                assertThat(t2, is(lessThanOrEqualTo(IMAGE_LOADING_FROM_BITMAP_TIME_CRITERIA)));
                return true;
            }
        });
    }

    public static FutureTask<Boolean> getImageLoadingFromFileTask(
        final PresentationEngine engine, final Activity activity) throws ExecutionException, InterruptedException {

        return new FutureTask<Boolean>(new Callable<Boolean>() {
            public Boolean call() {
                Log.v(TAG, "Start loading images fro file");
                long t1 = SystemClock.uptimeMillis();
                Image image = Image.createFromFile("/sdcard/a3d/earth.bmp");
                image.realize(engine);
                long t2 = SystemClock.uptimeMillis() - t1;
                Log.v(TAG, "Loading images fro file costs: " + t2);

                writePerformanceData(activity, "ngin3d.image_loading_from_file-time.txt", t2);
                assertThat(t2, is(lessThanOrEqualTo(IMAGE_LOADING_FROM_FILE_TIME_CRITERIA)));
                return true;
            }
        });
    }

    public static FutureTask<Boolean> getUpdateSystemTextTask(
        final PresentationEngine engine, final Activity activity) throws ExecutionException, InterruptedException {
        return new FutureTask<Boolean>(new Callable<Boolean>() {
            public Boolean call() {
                Text prepareShader = new Text("compile shader for text in advance");
                prepareShader.realize(engine);

                Log.v(TAG, "Start update system text");
                long t1 = SystemClock.uptimeMillis();
                Text sysText = new Text("MediaTek");
                sysText.realize(engine);
                long t2 = SystemClock.uptimeMillis() - t1;
                Log.v(TAG, "Update system text costs: " + t2);

                writePerformanceData(activity, "ngin3d.update_system_text-time.txt", t2);
                assertThat(t2, is(lessThanOrEqualTo(SYSTEM_TEXT_CONTENT_UPDATE_TIME_CRITERIA)));
                return true;
            }
        });
    }

    public static FutureTask<Boolean> getUpdateBitmapTextTask(
        final PresentationEngine engine, final Resources resources) throws ExecutionException, InterruptedException {
        return getUpdateBitmapTextTask(engine, null, resources);
    }

    public static FutureTask<Boolean> getUpdateBitmapTextTask(
        final PresentationEngine engine, final Activity activity) throws ExecutionException, InterruptedException {
        return getUpdateBitmapTextTask(engine, activity, activity.getResources());
    }

    private static FutureTask<Boolean> getUpdateBitmapTextTask(
        final PresentationEngine engine, final Activity activity, final Resources resources)
        throws ExecutionException, InterruptedException {

        return new FutureTask<Boolean>(new Callable<Boolean>() {
            public Boolean call() {
                Log.v(TAG, "Start update bitmap text");
                long t1 = SystemClock.uptimeMillis();
                BitmapFont font = new BitmapFont(resources, R.raw.bmfont1, R.drawable.bmfont1);
                BitmapText fontText = new BitmapText("MediaTek", font);
                fontText.realize(engine);
                long t2 = SystemClock.uptimeMillis() - t1;
                Log.v(TAG, "Update bitmap text costs: " + t2);

                writePerformanceData(activity, "ngin3d.update_bitmap_text-time.txt", t2);
                assertThat(t2, is(lessThanOrEqualTo(BITMAP_FONT_TEXT_CONTENT_UPDATE_TIME_CRITERIA)));
                return true;
            }
        });
    }

    public static FutureTask<Boolean> getRender50ActorsTask(
        final Stage stage, final PresentationEngine engine, final Activity activity) throws ExecutionException, InterruptedException {

        return new FutureTask<Boolean>(new Callable<Boolean>() {
            public Boolean call() {
                for (int i = 0; i < 50; i++) {
                    Container c = new Container();
                    stage.add(c);
                }
                Log.v(TAG, "Start render 50 Actors");
                long t1 = SystemClock.uptimeMillis();
                stage.realize(engine);
                long t2 = SystemClock.uptimeMillis() - t1;
                Log.v(TAG, "Render 50 Actors costs: " + t2);

                writePerformanceData(activity, "ngin3d.render_50_actors-time.txt", t2);
                assertThat(t2, is(lessThanOrEqualTo(RENDER_50_ACTORS_TIME_CRITERIA)));
                stage.removeAll();
                stage.realize(engine);
                return true;
            }
        });
    }

    public static FutureTask<Boolean> getRender100ActorsTask(
        final Stage stage, final PresentationEngine engine, final Activity activity) throws ExecutionException, InterruptedException {

        return new FutureTask<Boolean>(new Callable<Boolean>() {
            public Boolean call() {
                for (int i = 0; i < 100; i++) {
                    Container c = new Container();
                    stage.add(c);
                }
                Log.v(TAG, "Start render 100 Actors");
                long t1 = SystemClock.uptimeMillis();
                stage.realize(engine);
                long t2 = SystemClock.uptimeMillis() - t1;
                Log.v(TAG, "Render 100 Actors costs: " + t2);

                writePerformanceData(activity, "ngin3d.render_100_actors-time.txt", t2);
                assertThat(t2, is(lessThanOrEqualTo(RENDER_100_ACTORS_TIME_CRITERIA)));
                stage.removeAll();
                stage.realize(engine);
                return true;
            }
        });
    }

    public static FutureTask<Boolean> getStart50AnimationsTask(final Activity activity) throws ExecutionException, InterruptedException {
        return new FutureTask<Boolean>(new Callable<Boolean>() {
            public Boolean call() {
                Empty empty1 = new Empty();
                AnimationGroup group = new AnimationGroup();
                Rotation start = new Rotation(0, 0, 0);
                Rotation end = new Rotation(0, 0, 360);

                for (int i = 0; i < 50; i++) {
                    PropertyAnimation ani = new PropertyAnimation(empty1, "rotation", start, end);
                    ani.setDuration(1000);
                    group.add(ani);
                }
                Log.v(TAG, "start 50 animations");
                long t1 = SystemClock.uptimeMillis();
                group.start();
                long t2 = SystemClock.uptimeMillis() - t1;
                Log.v(TAG, "start 50 animations costs: " + t2);

                writePerformanceData(activity, "ngin3d.start_50_animations-time.txt", t2);
                assertThat(t2, is(lessThanOrEqualTo(START_50_ANIMATIONS_TIME_CRITERIA)));
                group.stop();
                return true;
            }
        });
    }

    public static FutureTask<Boolean> getStart100AnimationsTask(final Activity activity) throws ExecutionException, InterruptedException {
        return new FutureTask<Boolean>(new Callable<Boolean>() {
            public Boolean call() {
                Empty empty1 = new Empty();
                AnimationGroup group = new AnimationGroup();
                Rotation start = new Rotation(0, 0, 0);
                Rotation end = new Rotation(0, 0, 360);

                for (int i = 0; i < 100; i++) {
                    PropertyAnimation ani = new PropertyAnimation(empty1, "rotation", start, end);
                    ani.setDuration(1000);
                    group.add(ani);
                }
                Log.v(TAG, "start 100 animations");
                long t1 = SystemClock.uptimeMillis();
                group.start();
                long t2 = SystemClock.uptimeMillis() - t1;
                Log.v(TAG, "start 100 animations costs: " + t2);

                writePerformanceData(activity, "ngin3d.start_100_animations-time.txt", t2);
                assertThat(t2, is(lessThanOrEqualTo(START_100_ANIMATIONS_TIME_CRITERIA)));
                group.stop();
                return true;
            }
        });
    }

    public static void render25LandscapesTest(final Stage stage, final PresentationEngine engine, final Activity activity) {
        int width = stage.getWidth();
        int height = stage.getHeight();

        Container scene = new Container();
        stage.add(scene);
        scene.setPosition(new Point(0, height, 0));
        scene.setScale(new Scale(1, -1, 1));// UI is Y-down, model is Y-up

        int rows = 5;
        int columns = 5;
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < columns; j++) {
                Glo3D landscape = Glo3D.createFromAsset("landscape.glo");
                scene.add(landscape);
                landscape.setPosition(new Point(
                    width / columns * (i + 0.5f),
                    height / rows * (j + 0.5f)));
                landscape.setRotation(new Rotation(
                    360 / columns * i,
                    360 / rows * j,
                    0));
                landscape.setScale(new Scale(10, 10, 10));
            }
        }

        // Sleep for 5 seconds to give plenty of time for the render thread to
        // realize the Actors in the presentation layer.
        sleep(5000);
        Log.v(TAG, "Start render 25 landscapes");
        float fps = measureFps(engine, 5000, 1000);
        Log.v(TAG, "25 landscapes render at: " + fps + " fps");

        stage.removeAll();

        writePerformanceData(activity, "ngin3d.render_25_landscapes-fps.txt",
            fps);
        assertThat(fps, is(greaterThanOrEqualTo(
            (float) RENDER_25_LANDSCAPES_FPS_CRITERIA)));
    }

    /**
     * To detect slow calls such as disk read/write and network access during engine
     * initialization and rendering.
     */
    public static FutureTask<Boolean> getDetectSlowCallsTask(
        final Stage stage, final PresentationEngine engine) throws ExecutionException, InterruptedException {
        return new FutureTask<Boolean>(new Callable<Boolean>() {
            public Boolean call() {
                engine.uninitialize();
                engine.initialize(480, 800);

                for (int i = 0; i < 100; i++) {
                    Container c = new Container();
                    stage.add(c);
                }
                stage.realize(engine);
                return true;
            }
        });
    }

    private static void writePerformanceData(Activity activity, String name, Object data) {
        if (activity == null) {
            return;
        }
        File dataFile = new File(activity.getDir("perf", Context.MODE_PRIVATE), name);
        dataFile.delete();
        try {
            FileWriter writer = new FileWriter(dataFile);
            writer.write("YVALUE=" + data);
            writer.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /**
     * Measures the average frame rate over a given duration.
     *
     * @param duration Duration over which to measure in milliseconds
     * @param interval Interval at which to sample frame rate in milliseconds
     * @return Average frame rate in frames per second
     */
    private static float measureFps(PresentationEngine mPresentationEngine, int duration, int interval) {
        if (duration <= 0 || interval <= 0 || interval > duration) {
            return -1;
        }
        float fpsSum = 0;
        int intervalCount = duration / interval;

        for (int i = 0; i < intervalCount; ++i) {
            sleep(interval);
            fpsSum += mPresentationEngine.getFPS();
        }
        return fpsSum / intervalCount;
    }

    public static void setupCriteriaValue(Resources res) {
        File dir = Environment.getExternalStorageDirectory();
        File file = new File(dir, "/data/ngin3d-performance.xml");
        if (file.exists()) {
            try {
                DocumentBuilder documentBuilder = DocumentBuilderFactory.newInstance().newDocumentBuilder();
                Document document = documentBuilder.parse(file);
                NodeList nodeList = document.getElementsByTagName("integer");
                ENGINE_INITIALIZATION_TIME_CRITERIA = Integer.parseInt((nodeList.item(0)).getTextContent());
                IMAGE_LOADING_FROM_RESOURCE_TIME_CRITERIA = Integer.parseInt((nodeList.item(1)).getTextContent());
                IMAGE_LOADING_FROM_ASSET_TIME_CRITERIA = Integer.parseInt((nodeList.item(2)).getTextContent());
                IMAGE_LOADING_FROM_BITMAP_TIME_CRITERIA = Integer.parseInt((nodeList.item(3)).getTextContent());
                IMAGE_LOADING_FROM_FILE_TIME_CRITERIA = Integer.parseInt((nodeList.item(4)).getTextContent());
                SYSTEM_TEXT_CONTENT_UPDATE_TIME_CRITERIA = Integer.parseInt((nodeList.item(5)).getTextContent());
                BITMAP_FONT_TEXT_CONTENT_UPDATE_TIME_CRITERIA = Integer.parseInt((nodeList.item(6)).getTextContent());
                RENDER_50_ACTORS_TIME_CRITERIA = Integer.parseInt((nodeList.item(7)).getTextContent());
                RENDER_100_ACTORS_TIME_CRITERIA = Integer.parseInt((nodeList.item(8)).getTextContent());
                START_50_ANIMATIONS_TIME_CRITERIA = Integer.parseInt((nodeList.item(9)).getTextContent());
                START_100_ANIMATIONS_TIME_CRITERIA = Integer.parseInt((nodeList.item(10)).getTextContent());
                SCREEN_SHOT_TIME_CRITERIA = Integer.parseInt((nodeList.item(11)).getTextContent());
                RENDER_25_LANDSCAPES_FPS_CRITERIA = Integer.parseInt((nodeList.item(12)).getTextContent());
            } catch (ParserConfigurationException e) {
                e.printStackTrace();
            } catch (IOException e) {
                e.printStackTrace();
            } catch (SAXException e) {
                e.printStackTrace();
            }
        } else {
            ENGINE_INITIALIZATION_TIME_CRITERIA = res.getInteger(R.integer.initializtion);
            IMAGE_LOADING_FROM_RESOURCE_TIME_CRITERIA = res.getInteger(R.integer.loading_from_resource);
            IMAGE_LOADING_FROM_ASSET_TIME_CRITERIA = res.getInteger(R.integer.loading_from_asset);
            IMAGE_LOADING_FROM_BITMAP_TIME_CRITERIA = res.getInteger(R.integer.loading_from_bitmap);
            IMAGE_LOADING_FROM_FILE_TIME_CRITERIA = res.getInteger(R.integer.loading_from_file);
            SYSTEM_TEXT_CONTENT_UPDATE_TIME_CRITERIA = res.getInteger(R.integer.system_text_update);
            BITMAP_FONT_TEXT_CONTENT_UPDATE_TIME_CRITERIA = res.getInteger(R.integer.bitmap_font_text_update);
            RENDER_50_ACTORS_TIME_CRITERIA = res.getInteger(R.integer.render_50_actors);
            RENDER_100_ACTORS_TIME_CRITERIA = res.getInteger(R.integer.render_100_actors);
            START_50_ANIMATIONS_TIME_CRITERIA = res.getInteger(R.integer.start_50_animations);
            START_100_ANIMATIONS_TIME_CRITERIA = res.getInteger(R.integer.start_100_animations);
            SCREEN_SHOT_TIME_CRITERIA = res.getInteger(R.integer.screenshot);
            RENDER_25_LANDSCAPES_FPS_CRITERIA = res.getInteger(R.integer.render_25_landscapes);
        }
    }
}
