
package com.mediatek.DataUsageLockScreenClient;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.Drawable;
import android.util.Log;
import android.view.View;

import com.mediatek.ngin3d.Color;
import com.mediatek.ngin3d.Container;
import com.mediatek.ngin3d.Image;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Scale;
import com.mediatek.ngin3d.Text;

public class SIMCardData extends View {
    public static final boolean DEBUG = true;
    private static final String TAG = "DataUsageStageView";
    private int mType;
    private int mColor;
    private int mDisplayW;
    private int mCardCount;
    private int mTitleBarH;
    private boolean mIsLandscape;

    private CircleDrawer mCard_Circle;
    private Image mCard_Bubble;
    private float mCard_Bubble_ResWidth = 1.0f;
    static final float MIN_THRESHOLD = 2.5f;

    private Image mCard_Scale;
    private Image mText_BackColor;
    private Text mText;

    private Container mContainerText; // container for text/lable
    private Container mConDataCircle; // container for bubble+circle

    // floating bound X and Y
    private int mBoundBufferX;
    private int mBoundBufferY;

    private int mDataLimitPx;
    private int mDataUsageSize;

    // SIM card bound line : for single SIM, 5 positions
    public int[] mBound_X = new int[5];
    public int[] mBound_Y = new int[5];

    // SIM card 1 bound line : for dual SIM, 6 positions
    public int[] mBound1_X = new int[6];
    public int[] mBound1_Y = new int[6];

    // SIM card 2 bound line : for dual SIM, 6 positions
    public int[] mBound2_X = new int[6];
    public int[] mBound2_Y = new int[6];

    public float mUsageFirstPointX;
    public float mUsageFirstPointY;

    /**
     * The method to print log
     * 
     * @param tag the tag of the class
     * @param msg the log message to print
     */
    private static void log(String tag, String msg) {
        if (SIMCardData.DEBUG) {
            Log.d(tag, msg);
        }
    }

    public SIMCardData(Context context, int cardType, int cardColor, int cycleLimit,
            int bubbleSize, int cardCount, boolean isLandscape) {
        super(context);
        log(TAG, "SIMCardData,cardType:" + cardType + ",cardColor:" + cardColor
                + ",cycleLimit:" + cycleLimit + ",bubbleSize:" + bubbleSize
                + ",isLandscape:" + isLandscape);
        mType = cardType;
        mColor = cardColor;
        mDataLimitPx = cycleLimit;
        mDataUsageSize = bubbleSize;
        mCardCount = cardCount;
        mIsLandscape = isLandscape;

        if (context != null)
            mDisplayW = getResources().getDisplayMetrics().widthPixels;
        else
            mDisplayW = 0;

        mTitleBarH = getResources().getInteger(R.integer.TITLE_BAR_H);

        initActor();
    }

    private void initActor() {
        setFloatingBound(mType);
        setSIMCardColor();
        setActorPosition();
        setBubbleSize(mDataUsageSize);
        addToContainer();
    }

    private void setSIMCardColor() {
        // new circle & bubble & TextBgColor
        Drawable draw = null;
        switch (mColor) {
            case Constants.CIRCLE_BULE:
                mCard_Circle = new CircleDrawer(mDataLimitPx, Constants.IN_CIRCLE_BULE);
                mCard_Bubble = Image.createFromResource(getResources(), R.drawable.circle_blue);
                draw = getResources().getDrawable(R.drawable.circle_blue);
                mText_BackColor = Image.createFromResource(getResources(), R.drawable.label_blue);
                break;
            case Constants.CIRCLE_ORANGE:
                mCard_Circle = new CircleDrawer(mDataLimitPx, Constants.IN_CIRCLE_ORANGE);
                mCard_Bubble = Image.createFromResource(getResources(), R.drawable.circle_orange);
                draw = getResources().getDrawable(R.drawable.circle_orange);
                mText_BackColor = Image.createFromResource(getResources(), R.drawable.label_orange);
                break;
            case Constants.CIRCLE_GREEN:
                mCard_Circle = new CircleDrawer(mDataLimitPx, Constants.IN_CIRCLE_GREEN);
                mCard_Bubble = Image.createFromResource(getResources(), R.drawable.circle_green);
                draw = getResources().getDrawable(R.drawable.circle_green);
                mText_BackColor = Image.createFromResource(getResources(), R.drawable.label_green);
                break;
            case Constants.CIRCLE_PURPLE:
                mCard_Circle = new CircleDrawer(mDataLimitPx, Constants.IN_CIRCLE_PURPLE);
                mCard_Bubble = Image.createFromResource(getResources(), R.drawable.circle_purple);
                draw = getResources().getDrawable(R.drawable.circle_purple);
                mText_BackColor = Image.createFromResource(getResources(), R.drawable.label_purple);
                break;
            default:
                break;
        }

        if (null != draw) {
            mCard_Bubble_ResWidth = (float)draw.getIntrinsicWidth();
        }

        // new scale
        mCard_Scale = Image.createFromResource(getResources(), R.drawable.data_scale);

        // new text & set textSize/bgColor/color
        mText = new Text();

        if (mIsLandscape) {
            mText.setTextSize(Constants.TEXT_SIZE_LANDSCAPE_SMALL);
        } else {
            if (mDisplayW >= Constants.XXHDPI_WIDTH) 
                mText.setTextSize(Constants.TEXT_SIZE_LARGE);
            else if (mDisplayW > Constants.HVGA_WIDTH)
                mText.setTextSize(Constants.TEXT_SIZE_MIDDLE);
            else
                mText.setTextSize(Constants.TEXT_SIZE_SMALL);
        }

        mText.setBackgroundColor(Color.TRANSPARENT);
        mText.setColor(Color.WHITE);
    }

    private void setActorPosition() {
        // data_scale image's width/height
        int dataScaleW = BitmapFactory.decodeResource(getResources(), R.drawable.data_scale)
                .getWidth();
        int dataScaleH = BitmapFactory.decodeResource(getResources(), R.drawable.data_scale)
                .getHeight();

        // data_position width/height
        float scalePositionW = 0;

        mConDataCircle = new Container();

        switch (mType) {
            case Constants.SINGLE_CARD:
                // Single-SIM
                scalePositionW = getResources().getInteger(R.integer.SCALE_ONE);
                mUsageFirstPointX = mBound_X[0] + (mBoundBufferX / 2);
                mUsageFirstPointY = mBound_Y[0] + (mBoundBufferY / 2) - mTitleBarH;
                break;
            case Constants.GEMINI_CARD_ONE:
                // Dual-SIM, SIM1
                if (mCardCount == 1) { // mCardCount=1,GEMINI_CARD_ONE,left
                    scalePositionW = getResources().getInteger(R.integer.SCALE_ONE);
                    mUsageFirstPointX = mBound_X[0] + (mBoundBufferX / 2);
                    mUsageFirstPointY = mBound_Y[0] + (mBoundBufferY / 2) - mTitleBarH;
                } else {
                    scalePositionW = getResources().getInteger(R.integer.SCALE_ONE);

                    mUsageFirstPointX = mBound1_X[0] + (mBoundBufferX / 2);
                    if (mIsLandscape)
                        mUsageFirstPointY = mBound1_Y[1] + (mBoundBufferY / 2) - mTitleBarH;
                    else
                        mUsageFirstPointY = mBound1_Y[2] + (mBoundBufferY / 2) - mTitleBarH;
                }
                break;
            case Constants.GEMINI_CARD_TWO:
                // Dual-SIM, SIM2
                if (mCardCount == 1) { // mCardCount=1,GEMINI_CARD_TWOscale,right
                    scalePositionW = getResources().getInteger(R.integer.SCALE_TWO);
                    mUsageFirstPointX = mBound_X[0] + (mBoundBufferX / 2);
                    mUsageFirstPointY = mBound_Y[0] + (mBoundBufferY / 2) - mTitleBarH;
                } else {
                    scalePositionW = getResources().getInteger(R.integer.SCALE_TWO);

                    mUsageFirstPointX = mBound2_X[0] + (mBoundBufferX / 2);
                    if (mIsLandscape)
                        mUsageFirstPointY = mBound2_Y[2] + (mBoundBufferY / 2) - mTitleBarH;
                    else
                        mUsageFirstPointY = mBound2_Y[1] + (mBoundBufferY / 2) - mTitleBarH;
                }
                break;
            default:
                break;
        }

        mConDataCircle.setPosition(new Point(mUsageFirstPointX, mUsageFirstPointY));
        mCard_Scale.setPosition(new Point(scalePositionW + (dataScaleW / (float) 2),
                getResources().getInteger(R.integer.SCALE_BUFFER_PX) + (dataScaleH / (float) 2)));
    }

    private void addToContainer() {
        mConDataCircle.add(mCard_Bubble);
        mConDataCircle.add(mCard_Circle);

        mContainerText = new Container();
        mContainerText.add(mText_BackColor);
        mContainerText.add(mText);
    }

    private void printBoundArray(String name, int x[], int y[]) {
        int length = (x.length < y.length) ? x.length : y.length;
        for (int i = 0; i < length; i++) {
            log(TAG, name + "[" + i + "] = (" + x[i] + "," + y[i] + ")");
        }
    }

    private void setFloatingBound(int type) {
        log(TAG, "setFloatingBound:" + type);

        // read pre-defined position of floating points from resource
        switch (type) {
            case Constants.SINGLE_CARD:
                // Single-SIM
                mBound_X[0] = getResources().getInteger(R.integer.SC_X1);
                mBound_Y[0] = getResources().getInteger(R.integer.SC_Y1);

                mBound_X[1] = getResources().getInteger(R.integer.SC_X2);
                mBound_Y[1] = getResources().getInteger(R.integer.SC_Y2);

                mBound_X[2] = getResources().getInteger(R.integer.SC_X3);
                mBound_Y[2] = getResources().getInteger(R.integer.SC_Y3);

                mBound_X[3] = getResources().getInteger(R.integer.SC_X4);
                mBound_Y[3] = getResources().getInteger(R.integer.SC_Y4);

                mBound_X[4] = getResources().getInteger(R.integer.SC_X5);
                mBound_Y[4] = getResources().getInteger(R.integer.SC_Y5);

                mBoundBufferX = mBound_X[2] - mBound_X[0];
                mBoundBufferY = mBound_Y[3] - mBound_Y[2];

                printBoundArray("mBound_XY", mBound_X, mBound_Y);
                break;
            case Constants.GEMINI_CARD_ONE:
                // Dual-SIM ,SIM 1
                if (mCardCount == 1) {
                    setFloatingBound(Constants.SINGLE_CARD);
                    break;
                }
                mBound1_X[0] = getResources().getInteger(R.integer.SC1_X1);
                mBound1_Y[0] = getResources().getInteger(R.integer.SC1_Y1);

                mBound1_X[1] = getResources().getInteger(R.integer.SC1_X2);
                mBound1_Y[1] = getResources().getInteger(R.integer.SC1_Y2);

                mBound1_X[2] = getResources().getInteger(R.integer.SC1_X3);
                mBound1_Y[2] = getResources().getInteger(R.integer.SC1_Y3);

                mBound1_X[3] = getResources().getInteger(R.integer.SC1_X4);
                mBound1_Y[3] = getResources().getInteger(R.integer.SC1_Y4);

                mBound1_X[4] = getResources().getInteger(R.integer.SC1_X5);
                mBound1_Y[4] = getResources().getInteger(R.integer.SC1_Y5);

                mBound1_X[5] = getResources().getInteger(R.integer.SC1_X6);
                mBound1_Y[5] = getResources().getInteger(R.integer.SC1_Y6);

                mBoundBufferX = mBound1_X[3] - mBound1_X[0];
                if (mIsLandscape) {
                    mBoundBufferY = mBound1_Y[2] - mBound1_Y[1];
                } else {
                    mBoundBufferY = mBound1_Y[1] - mBound1_Y[2];
                }

                printBoundArray("mBound1_XY", mBound1_X, mBound1_Y);
                break;
            case Constants.GEMINI_CARD_TWO:
                if (mCardCount == 1) {
                    setFloatingBound(Constants.SINGLE_CARD);
                    break;
                }
                // Dual-SIM ,SIM 2
                mBound2_X[0] = getResources().getInteger(R.integer.SC2_X1);
                mBound2_Y[0] = getResources().getInteger(R.integer.SC2_Y1);

                mBound2_X[1] = getResources().getInteger(R.integer.SC2_X2);
                mBound2_Y[1] = getResources().getInteger(R.integer.SC2_Y2);

                mBound2_X[2] = getResources().getInteger(R.integer.SC2_X3);
                mBound2_Y[2] = getResources().getInteger(R.integer.SC2_Y3);

                mBound2_X[3] = getResources().getInteger(R.integer.SC2_X4);
                mBound2_Y[3] = getResources().getInteger(R.integer.SC2_Y4);

                mBound2_X[4] = getResources().getInteger(R.integer.SC2_X5);
                mBound2_Y[4] = getResources().getInteger(R.integer.SC2_Y5);

                mBound2_X[5] = getResources().getInteger(R.integer.SC2_X6);
                mBound2_Y[5] = getResources().getInteger(R.integer.SC2_Y6);

                mBoundBufferX = mBound2_X[3] - mBound2_X[0];
                if (mIsLandscape)
                    mBoundBufferY = mBound2_Y[1] - mBound2_Y[2];
                else
                    mBoundBufferY = mBound2_Y[2] - mBound2_Y[1];

                printBoundArray("mBound2_XY", mBound2_X, mBound2_Y);
                break;
            default:
                break;
        }
    }

    public void setBubbleSize(int usageSize) {
        log(TAG, "setBubbleSize(), usageSize:" + usageSize + ",mDataLimitPx:" + mDataLimitPx);

        double bubblePX = (double) (BitmapFactory.decodeResource(getResources()
                , R.drawable.circle_blue).getWidth());
        // Formula -> Bubble PX : Out Circle PX(mDataLimitPx * 2 = Diameter) = 1:X
        double pxProportion = (double) (((double) (mDataLimitPx * 2)) / bubblePX);
        log(TAG, "pxProportion:" + pxProportion);

        // set bubble scale according to the dataUsageSize
        if (usageSize == 0) {
            mCard_Bubble.setScale(new Scale((float) 0, (float) 0));
        } else { // usageSize > 10
            float scaleSize = (float) (pxProportion * usageSize / 100);
            log(TAG, "scaleSize = pxProportion * usageSize/100 = " + scaleSize);

            // spec: set out circle alpha > 105%
            if (usageSize > 105) {
                mCard_Circle.setOutCircleColor(Constants.OUT_CIRCLE_COLOR_ALPHA_MORE);
            } else {
                mCard_Circle.setOutCircleColor(Constants.OUT_CIRCLE_COLOR_ALPHA_NORMAL);
            }

            // spec: set max limit 200%
            if (usageSize > 200) {
                mCard_Bubble.setScale(new Scale((float)pxProportion * 2, (float)pxProportion * 2));
            } else {
                mCard_Bubble.setScale(new Scale((float)scaleSize, (float)scaleSize));
            }

            Scale ratio = mCard_Bubble.getScale();
            log(TAG, "mCard_Bubble_ResWidth = " + mCard_Bubble_ResWidth + ", ratio.x = " + ratio.x);

            if (MIN_THRESHOLD >= mCard_Bubble_ResWidth * ratio.x) {
                float fixScale = MIN_THRESHOLD / mCard_Bubble_ResWidth;
                mCard_Bubble.setScale(new Scale((float) fixScale, (float) fixScale));
            }
        }
    }

    public Image getScale() {
        return mCard_Scale;
    }

    public Text getText() {
        return mText;
    }

    public Container getContainerText() {
        return mContainerText;
    }

    public Container getConDataCircle() {
        return mConDataCircle;
    }

    public int getBoundBufferX() {
        return mBoundBufferX;
    }

    public int getBoundBufferY() {
        return mBoundBufferY;
    }
}
