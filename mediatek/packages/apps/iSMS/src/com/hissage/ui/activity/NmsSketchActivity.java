package com.hissage.ui.activity;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.RectF;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.ImageButton;
import android.widget.SeekBar;
import android.widget.Toast;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;

import com.hissage.R;
import com.hissage.config.NmsCommonUtils;
import com.hissage.config.NmsConfig;
import com.hissage.config.NmsCustomUIConfig;
import com.hissage.ui.view.NmsColorPickerPanelView;
import com.hissage.ui.view.NmsPaintView;
import com.hissage.ui.view.NmsToolColorView;
import com.hissage.ui.view.NmsToolSizeView;
import com.hissage.util.data.NmsAlertDialogUtils;
import com.hissage.util.log.NmsLog;

public class NmsSketchActivity extends Activity {

    public String Tag = "SketchActivity";

    public enum Tools {
        PEN, ERASER
    }

    public int MAX_PIX;

    private static int DEFATUL_SIZE = 24;
    private static int DEFATUL_COLOR = Color.parseColor("#f26522");
    private Context mContext;
    private int mSize = DEFATUL_SIZE;
    private int tempSize = 2;
    private int mColor = DEFATUL_COLOR;
    private NmsPaintView mPaintView;
    private String mPath;

    private MenuItem mItemSize;
    private MenuItem mItemColor;
    private MenuItem mItemTool;

    private int[] mCustomColor = { 0xffff0000, 0xffffff00, 0xff00ff00, 0xff00ffff, 0xff0000ff,
            0xffff00ff };
    private AlertDialog colorDialog;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.sketch);
        NmsLog.trace(Tag, "Start SketchActivity...");
        
        mPaintView = (NmsPaintView) findViewById(R.id.pv_sketch);
        mContext = this;

        MAX_PIX = this.getResources().getDimensionPixelOffset(R.dimen.action_menu_size);

        if (NmsCommonUtils.getSDCardStatus()) {
            String mSketchPath = NmsCommonUtils.getSDCardPath(this) + File.separator
                    + NmsCustomUIConfig.ROOTDIRECTORY + File.separator + "sketch";
            File f = new File(mSketchPath);
            if (!f.exists()) {
                f.mkdirs();
            }
        }

        mSize = NmsConfig.getSketchSize();
        mSize = mSize <= 0 ? DEFATUL_SIZE : mSize;

        mColor = NmsConfig.getSketchColor();
        mColor = mColor == 0 ? DEFATUL_COLOR : mColor;
        mPaintView.setColor(mColor);
        mPaintView.setSize(mSize);

        initActionBar();
        
        if (NmsCommonUtils.getSDcardAvailableSpace() < NmsCustomUIConfig.MIN_SEND_ATTACH_SD_CARD_SIZE) {
            NmsLog.error(Tag, "sd card available space is not enough: " + NmsCommonUtils.getSDcardAvailableSpace()) ;
            Toast.makeText(this, R.string.STR_NMS_SD_CARD_FULL,
                    Toast.LENGTH_SHORT).show();
            setResult(RESULT_CANCELED);
            finish();
            return;
        }
    }

    private void initCustomColor() {
        String colorString = NmsConfig.getSketchCustomColor();
        if (!TextUtils.isEmpty(colorString)) {
            String[] color = colorString.split(",");
            for (int i = 0; i < color.length; i++) {
                mCustomColor[i] = Integer.parseInt(color[i]);
            }
        }
    }

    private void saveCustomColor() {
        String s = String.valueOf(mColor);
        int j = 0;
        for (int i = 0; i < mCustomColor.length; i++) {
            if (mColor != mCustomColor[i]) {
                s += "," + String.valueOf(mCustomColor[i]);
                j++;
            }
            if (j == 5) {
                break;
            }
        }

        NmsConfig.setSketchCustomColor(s);
    }

    private void initActionBar() {
        ActionBar actionBar = getActionBar();
        actionBar.setDisplayShowTitleEnabled(false);
        actionBar.setDisplayShowHomeEnabled(false);
        actionBar.setDisplayHomeAsUpEnabled(false);

        ViewGroup v = (ViewGroup) LayoutInflater.from(this).inflate(R.layout.action_bar_sketch,
                null);
        actionBar.setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM, ActionBar.DISPLAY_SHOW_CUSTOM);
        actionBar.setCustomView(v, new ActionBar.LayoutParams(ActionBar.LayoutParams.WRAP_CONTENT,
                ActionBar.LayoutParams.WRAP_CONTENT, Gravity.CENTER_VERTICAL | Gravity.LEFT));

        TextView tvTitle = (TextView) v.findViewById(R.id.tv_title);
        tvTitle.setText(R.string.STR_NMS_SKETCH_TITLE);

        ImageButton ivDone = (ImageButton) v.findViewById(R.id.iv_done);
        ivDone.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                createPaint();
            }
        });
    }

    protected void onResume() {
        super.onResume();
        NmsLog.trace("Tag", "onResume SketchActivity...");
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.sketch_menu, menu);
        mItemTool = menu.findItem(R.id.menu_sketch_tools);
        mItemSize = menu.findItem(R.id.menu_sketch_size);
        mItemColor = menu.findItem(R.id.menu_sketch_color);

        makeSizeBitmap(mSize);
        makeColorBitmap(mColor);

        return true;
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        return mPaintView.onTouchEvent(event);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {

        if (item.getItemId() == R.id.menu_sketch_tools_pen) {
            onToolChange(Tools.PEN);
            mItemTool.setIcon(R.drawable.ic_action_pencil);
            mItemTool.setTitle(R.string.STR_NMS_SKETCH_TOOLS_PEN);
        } else if (item.getItemId() == R.id.menu_sketch_tools_eraser) {
            onToolChange(Tools.ERASER);
            mItemTool.setIcon(R.drawable.ic_menu_eraser);
            mItemTool.setTitle(R.string.STR_NMS_SKETCH_TOOLS_ERASER);
        } else if (item.getItemId() == R.id.menu_sketch_size) {
            onSizeChanged();
        } else if (item.getItemId() == R.id.menu_sketch_color) {
            onColorChanged();
        } else if (item.getItemId() == R.id.menu_sketch_delete) {
            clearPaint();
        } else if (item.getItemId() == R.id.menu_sketch_undo) {
            if (mPaintView.undo() < 0) {
                Toast.makeText(this, R.string.STR_NMS_NOTUNDO, Toast.LENGTH_SHORT).show();
            }
        } else if (item.getItemId() == R.id.menu_sketch_redo) {
            if (mPaintView.redo() < 0) {
                Toast.makeText(this, R.string.STR_NMS_NOTREDO, Toast.LENGTH_SHORT).show();
            }
        } else if (item.getItemId() == R.id.menu_sketch_discard) {
            if (mPaintView.mIsDraw) {
                dropDailog();
            } else {
                this.finish();
            }
        } else {
            return true;
        }

        return false;
    }

    private void onToolChange(Tools tool) {
        mPaintView.setTool(tool);

    }

    private void setSize(int size) {
        makeSizeBitmap(size);
        NmsConfig.setSketchSize(size);
        mPaintView.setSize(size);
    }

    private void makeSizeBitmap(int size) {

        String s = String.valueOf(size);
        String mstrTitle = this.getText(R.string.STR_NMS_PEN_SIZE).toString();
        Bitmap mbmpTest = Bitmap.createBitmap(MAX_PIX, MAX_PIX, Config.ARGB_8888);
        Canvas canvasTemp = new Canvas(mbmpTest);
        canvasTemp.drawColor(Color.TRANSPARENT);
        Paint p = new Paint();
        p.setColor(0xccffffff);
        p.setFlags(Paint.FAKE_BOLD_TEXT_FLAG);
        p.setAntiAlias(true);

        // p.setFlags(Paint.FAKE_BOLD_TEXT_FLAG);
        p.setTextSize(this.getResources().getDimensionPixelOffset(R.dimen.action_bar_size));

        // canvasTemp.drawText(mstrTitle, (MAX_PIX - p.measureText(mstrTitle)) /
        // 2, this
        // .getResources().getDimensionPixelOffset(R.dimen.sketch_bound_size),
        // p);
        p.setTextSize(this.getResources().getDimensionPixelOffset(R.dimen.action_bar_big_size));

        canvasTemp.drawText(s, (MAX_PIX - p.measureText(s)) / 2, this.getResources()
                .getDimensionPixelOffset(R.dimen.sketch_bound_big_size), p);

        int lineSize = Integer.parseInt(s);
        int offSet = 0;
        if (lineSize <= 19) {
            offSet = 5;
        } else if (lineSize <= 36) {
            offSet = 8;
        } else if (lineSize <= 53) {
            offSet = 11;
        } else {
            offSet = 14;
        }

        int lineTop = this.getResources().getDimensionPixelOffset(R.dimen.sketch_tool_line);
        canvasTemp.drawRoundRect(new RectF(0, lineTop, MAX_PIX, lineTop + offSet), 5f, 5f, p);

        Drawable drawable = new BitmapDrawable(mbmpTest);
        mItemSize.setIcon(drawable);
    }

    private void makeColorBitmap(long color) {
        Bitmap bmp = Bitmap.createBitmap(MAX_PIX, MAX_PIX, Config.ARGB_8888);
        Canvas canvasTemp = new Canvas(bmp);
        canvasTemp.drawColor((int) color);
        Paint paint = new Paint();
        paint.setColor(0xff777777);
        paint.setStyle(Paint.Style.STROKE);
        paint.setStrokeWidth(5);
        canvasTemp.drawRect(0, 0, MAX_PIX, MAX_PIX, paint);

        Drawable drawable = new BitmapDrawable(bmp);
        mItemColor.setIcon(drawable);
    }

    private void setColor(int color) {
        makeColorBitmap(color);
        NmsConfig.setSketchColor(color);
        mPaintView.setColor(color);
        saveCustomColor();
    }

    private void clearPaint() {
        if (mPaintView.mIsDraw) {
            NmsAlertDialogUtils.showDialog(mContext, R.string.STR_NMS_CLEAR_SKETCH_TITLE, 0,
                    R.string.STR_NMS_CLEAR_SKETCH_CONTENT, R.string.STR_NMS_OK,
                    R.string.STR_NMS_CANCEL, new DialogInterface.OnClickListener() {

                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            // TODO Auto-generated method stub
                            mPaintView.clear();
                        }
                    }, null);
        }
    }

    private void createPaint() {
        finishPaint();
    }

    private void finishPaint() {
        Bitmap bitmap = mPaintView.buildSketchBitmap();
        NmsLog.trace("Tag", "onResume SketchActivity...");
        if (mPaintView.mIsDraw && bitmap != null) {
            if (NmsCommonUtils.getSDCardStatus()) {
                mPath = NmsCommonUtils.getSDCardPath(this) + File.separator
                        + NmsCustomUIConfig.ROOTDIRECTORY + File.separator + "sketch"
                        + File.separator + System.currentTimeMillis() + ".ske.png";
                File file = new File(mPath);
                try {
                    FileOutputStream out = new FileOutputStream(file);
                    if (bitmap.compress(Bitmap.CompressFormat.PNG, 10, out)) {
                        out.flush();
                        out.close();
                    }
                } catch (FileNotFoundException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                } catch (IOException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }

                Intent intent = new Intent();
                Uri fileUri = Uri.fromFile(file);
                intent.setData(fileUri);
                // intent.putExtra("data", fileUri);
                setResult(RESULT_OK, intent);
            }
        } else {
            // Toast.makeText(this, R.string.STR_NMS_SKETCH_ERROR,
            // Toast.LENGTH_SHORT).show();
        }

        this.finish();
    }

    public void onColorChanged() {
        initCustomColor();
        AlertDialog.Builder builder;
        LayoutInflater inflater = (LayoutInflater) mContext
                .getSystemService(LAYOUT_INFLATER_SERVICE);

        View layout = inflater.inflate(R.layout.sketch_dialog_color, null);
        final NmsToolColorView tcView = (NmsToolColorView) layout.findViewById(R.id.tc_color);
        tcView.setCurrentColor(mColor);
        tcView.setCustomColorCell(mCustomColor);

        builder = new AlertDialog.Builder(mContext);
        builder.setTitle(R.string.STR_NMS_COLOR_SKETCH_TITLE);
        builder.setView(layout);
        builder.setPositiveButton(R.string.STR_NMS_OK, new DialogInterface.OnClickListener() {

            @Override
            public void onClick(DialogInterface dialog, int which) {
                // TODO Auto-generated method stub
                dialog.dismiss();
                mColor = tcView.getCurrentColor();
                setColor(mColor);
            }
        });
        builder.setNegativeButton(R.string.STR_NMS_SKETCH_CUSTOM,
                new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        // TODO Auto-generated method stub
                        dialog.dismiss();
                        onCustomColor();
                    }
                });
        colorDialog = builder.create();
        colorDialog.show();
    }

    public void onSizeChanged() {

        AlertDialog.Builder builder;
        AlertDialog alertDialog;

        LayoutInflater inflater = (LayoutInflater) mContext
                .getSystemService(LAYOUT_INFLATER_SERVICE);
        View layout = inflater.inflate(R.layout.dialog_size, null);

        final TextView textView = (TextView) layout.findViewById(R.id.tv_size);
        textView.setText(String.valueOf(mSize));
        final NmsToolSizeView tsView = (NmsToolSizeView) layout.findViewById(R.id.ts_size);
        tsView.setRadius(mSize);
        SeekBar sbView = (SeekBar) layout.findViewById(R.id.sb_size);
        sbView.setProgress(mSize);
        sbView.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                // TODO Auto-generated method stub
                tempSize = progress + 2;
                tsView.setRadius(tempSize);
                textView.setText(String.valueOf(tempSize));
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                // TODO Auto-generated method stub

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                // TODO Auto-generated method stub

            }
        });

        builder = new AlertDialog.Builder(mContext);
        builder.setTitle(R.string.STR_NMS_SIZE_SKETCH_TITLE);
        builder.setView(layout);
        builder.setPositiveButton(R.string.STR_NMS_OK, new DialogInterface.OnClickListener() {

            @Override
            public void onClick(DialogInterface dialog, int which) {
                // TODO Auto-generated method stub
                mSize = tempSize;
                setSize(mSize);
            }
        });
        builder.setNegativeButton(R.string.STR_NMS_CANCEL, null);
        alertDialog = builder.create();
        alertDialog.show();

    }

    @Override
    public void onBackPressed() {
        if (mPaintView.mIsDraw) {
            dropDailog();
        } else {
            super.onBackPressed();
        }
    }

    private void dropDailog() {
        NmsAlertDialogUtils.showDialog(mContext, R.string.STR_NMS_BACK_TITLE, 0,
                R.string.STR_NMS_LEAVE_SKETCH_CONTENT, R.string.STR_NMS_OK,
                R.string.STR_NMS_CANCEL, new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        // TODO Auto-generated method stub
                        NmsSketchActivity.this.finish();
                    }
                }, null);
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
    }

    public void onCustomColor() {
        // TODO Auto-generated method stub

        AlertDialog.Builder builder;
        AlertDialog customDialog;

        LayoutInflater inflater = (LayoutInflater) mContext
                .getSystemService(LAYOUT_INFLATER_SERVICE);
        View layout = inflater.inflate(R.layout.sketch_custom_color, null);

        final NmsColorPickerPanelView colorView = (NmsColorPickerPanelView) layout
                .findViewById(R.id.pick_color);

        colorView.setInitialColor(0xffe01f26);

        builder = new AlertDialog.Builder(mContext);
        builder.setTitle(R.string.STR_NMS_SKETCH_CUSTOM_COLOR);
        builder.setView(layout);
        builder.setPositiveButton(R.string.STR_NMS_OK, new DialogInterface.OnClickListener() {

            @Override
            public void onClick(DialogInterface dialog, int which) {
                // TODO Auto-generated method stub

                mColor = colorView.getCurrentColor();
                setColor(mColor);
            }
        });
        builder.setNegativeButton(R.string.STR_NMS_CANCEL, null);
        customDialog = builder.create();
        customDialog.show();

    }

}