package com.hissage.ui.view;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.ThumbnailUtils;
import android.os.Handler;
import android.os.Message;
import android.provider.MediaStore.Video.Thumbnails;
import android.text.Editable;
import android.text.InputFilter;
import android.text.Spanned;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.text.style.ImageSpan;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.Toast;

import com.hissage.R;
import com.hissage.config.NmsCustomUIConfig;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.util.log.NmsLog;
import com.hissage.util.message.SmileyParser;

public class NmsCaptionEditor extends LinearLayout implements TextWatcher {

    private Context mContext;
    private View mConvertView;
    private ImageView mAttachThumbnail;
    private ImageView mAttachThumbnailDelete;
    private EditText mTextEditor;
    private ActionListener mListener;
    private int mEmoticonCount;
    private boolean mIsGroupChat = false;
    private static Toast mToast = null;
    private static int mToastId = 0;
    public final static int ACTION_TOUCH = 0;
    public final static int ACTION_TEXT_CHANGE = 1;
    public final static int ACTION_DELETE_ATTACH = 2;
    
    private Handler mHandler = new Handler();
    private boolean mIsHandling = false ;
    private boolean mIsHandleNow = true ;
    
    public NmsCaptionEditor(Context context) {
        super(context);
        mContext = context;
    }

    public NmsCaptionEditor(Context context, AttributeSet attrs) {
        super(context, attrs);
        mConvertView = LayoutInflater.from(context).inflate(R.layout.caption_editor, this, true);
        mContext = context;

        mAttachThumbnail = (ImageView) mConvertView.findViewById(R.id.iv_attach_thumbnail);
        mAttachThumbnailDelete = (ImageView) mConvertView.findViewById(R.id.iv_attach_thumbnail_delete);
        mAttachThumbnail.setOnClickListener(new android.view.View.OnClickListener() {
            
            @Override
            public void onClick(View v) {
                mListener.doAction(ACTION_DELETE_ATTACH);
            }
        });
        mTextEditor = (EditText) mConvertView.findViewById(R.id.et_embedded_text_editor);
        mTextEditor.requestFocus();
        mTextEditor.addTextChangedListener(this);
        mTextEditor.setOnKeyListener(new OnKeyListener() {

            @Override
            public boolean onKey(View v, int keyCode, KeyEvent event) {
                if (keyCode == KeyEvent.KEYCODE_DEL && event.getAction() == KeyEvent.ACTION_DOWN) {
                    String s = ((EditText) v).getText().toString();
                    int index = ((EditText) v).getSelectionStart();
                    if (TextUtils.isEmpty(s) || index == 0) {
                        mListener.doAction(ACTION_DELETE_ATTACH);
                    }
                    return false;
                }
                return false;
            }
        });
        mTextEditor.setOnTouchListener(new OnTouchListener() {

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                mListener.doAction(ACTION_TOUCH);
                return false;
            }
        });
        
    }

    public void setMaxLength(final int length) {
        // mTextEditor.setFilters(new InputFilter[] { new
        // InputFilter.LengthFilter(length) });
        InputFilter[] filter = new InputFilter[1];

        filter[0] = new InputFilter.LengthFilter(length) {
            @Override
            public CharSequence filter(CharSequence source, int start, int end, Spanned dest,
                    int dstart, int dend) {
                // TODO Auto-generated method stub

                CharSequence destContent = super.filter(source, start, end, dest, dstart, dend);

                if (destContent != null && (source != null && source.length() > 0)) {
                    if (destContent.length() == 0 || destContent.length() != source.length()) {
                        showToast(mContext, R.string.STR_NMS_MAX_CHARACTERS_REACHED);
                        // } else if (destContent.length() != source.length()) {
                        // Toast.makeText(mContext,
                        // R.string.STR_NMS_TRUNC_CAPTION, Toast.LENGTH_SHORT)
                        // .show();
                    }
                }
                return destContent;
            }
        };
        mTextEditor.setFilters(filter);

        int len = getTextContent().length();
        if (len > length) {
            Editable text = mTextEditor.getEditableText();
            text.delete(length, len);
            mListener.doAction(ACTION_TEXT_CHANGE);
            Toast.makeText(mContext, R.string.STR_NMS_TRUNC_CAPTION, Toast.LENGTH_SHORT).show();
        }
    }

    public void setForbidSend(boolean isDefultMms) {
        if(isDefultMms){
            mTextEditor.setFocusable(true);
            mTextEditor.setFocusableInTouchMode(true);
            mTextEditor.setEnabled(true);
            if (mIsGroupChat) {
                setEditHint(R.string.STR_NMS_INPUT_HINT_ISMS);
            } else {
                setEditHint(R.string.STR_NMS_INPUT_HINT);
            }
        }else{
            mTextEditor.setEnabled(false);
            mTextEditor.setFocusable(false);
            mTextEditor.setFocusableInTouchMode(false);
            setEditHint(R.string.STR_NMS_INPUT_HINT_FORBIT);
        }
    }

    private void setEditHint(int resid){
        mTextEditor.setHint(resid);
    }

    public void deleteAttach() {
        mAttachThumbnail.setVisibility(View.GONE);
        mAttachThumbnailDelete.setVisibility(View.GONE);
        setMaxLength(NmsCustomUIConfig.MESSAGE_MAX_LENGTH);
        if (mIsGroupChat) {
            setEditHint(R.string.STR_NMS_INPUT_HINT_ISMS);
        } else {
            setEditHint(R.string.STR_NMS_INPUT_HINT);
        }
    }

    public String getTextContent() {
        return mTextEditor.getText().toString();
    }

    public void setTextContent(String text) {
        mTextEditor.setText(text);
    }

    public void setSection(int index) {
        mTextEditor.setSelection(index);
    }

    public void setTextSize(float textSize) {
        mTextEditor.setTextSize(textSize);
    }

    public void insertEmoticon(String text) {
        if (mEmoticonCount < NmsCustomUIConfig.EMOTICONS_MAX_COUNT)
            mIsHandleNow = true ;
        
        int index = mTextEditor.getSelectionStart();
        Editable edit = mTextEditor.getEditableText();
        if (text.length() > NmsCustomUIConfig.MESSAGE_MAX_LENGTH - edit.length()) {
            showToast(mContext, R.string.STR_NMS_MAX_CHARACTERS_REACHED);
            return;
        }

        if (mEmoticonCount >= NmsCustomUIConfig.EMOTICONS_MAX_COUNT) {
            showToast(mContext, R.string.STR_NMS_MAX_EMOTICON_REACHED);
            return;
        }

        if (index < 0 || index >= edit.length()) {
            edit.append(text);
        } else {
            edit.insert(index, text);
        }
    }

    public void insertQuick(String text) {
        int index = mTextEditor.getSelectionStart();
        Editable edit = mTextEditor.getEditableText();

        if (index < 0 || index >= edit.length()) {
            edit.append(text);
        } else {
            edit.insert(index, text);
        }
    }

    public synchronized void deleteEmoticon() {
        Editable edit = mTextEditor.getEditableText();
        int length = edit.length();
        int cursor = mTextEditor.getSelectionStart();
        if (length == 0 || cursor == 0) {
            return;
        }
        ImageSpan[] spans = edit.getSpans(0, cursor, ImageSpan.class);
        ImageSpan span = null;
        int index = 0;
        if (null != spans && spans.length != 0) {
            span = spans[spans.length - 1];
            index = edit.getSpanEnd(span);
        }

        if (index == cursor) {
            int start = edit.getSpanStart(span);
            edit.delete(start, cursor);
        } else {
            edit.delete(cursor - 1, cursor);
        }
    }

    public void seteAttach(String path, int type) {
        if (type == NmsIpMessageConsts.NmsIpMessageType.PICTURE) {
            BitmapFactory.Options options = new BitmapFactory.Options();
            options.inJustDecodeBounds = true;
            Bitmap bitmap = BitmapFactory.decodeFile(path, options);
            options.inJustDecodeBounds = false;
            int l = Math.max(options.outHeight, options.outWidth);
            int be = (int) (l / 100);
            if (be <= 0)
                be = 1;
            options.inSampleSize = be;
            bitmap = BitmapFactory.decodeFile(path, options);
            if (null != bitmap) {
                mAttachThumbnail.setImageBitmap(bitmap);
            } else {
                mAttachThumbnail.setImageResource(R.drawable.isms_choose_a_photo);
            }
        } else if (type == NmsIpMessageConsts.NmsIpMessageType.VIDEO) {
            Bitmap bitmap = ThumbnailUtils.createVideoThumbnail(path, Thumbnails.MINI_KIND);
            if (null != bitmap) {
                mAttachThumbnail.setImageBitmap(bitmap);
            } else {
                mAttachThumbnail.setImageResource(R.drawable.isms_choose_a_video);
            }
        } else if (type == NmsIpMessageConsts.NmsIpMessageType.VOICE) {
            mAttachThumbnail.setImageResource(R.drawable.ic_soundrecorder);
        }
        mAttachThumbnail.setVisibility(View.VISIBLE);
        mAttachThumbnailDelete.setVisibility(View.VISIBLE);
        setMaxLength(NmsCustomUIConfig.CAPTION_MAX_LENGTH);
        setEditHint(R.string.STR_NMS_CAPTION_HINT);
    }

    public int getLineCount() {
        return mTextEditor.getLineCount();
    }

    public void requestEditFocus() {
        mTextEditor.requestFocus();
    }

    public void setEditEnabled(boolean enabled) {
        mTextEditor.setEnabled(enabled);
    }

    public void showKeyBoard(boolean show) {
        InputMethodManager imm = (InputMethodManager) getContext().getSystemService(
                Context.INPUT_METHOD_SERVICE);
        if (show) {
            mTextEditor.requestFocus();
            imm.showSoftInput(mTextEditor, 0);
        } else {
            imm.hideSoftInputFromWindow(mTextEditor.getWindowToken(), 0);
        }
    }

    @Override
    public void beforeTextChanged(CharSequence s, int start, int count, int after) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onTextChanged(CharSequence s, int start, int before, int count) {
        // TODO Auto-generated method stub
        // NmsLog.error("captionEditor", String.format("on text changed is called start:%d, before:%d, count:%d", start, before, count)) ;
    }

    @Override
    public void afterTextChanged(Editable s) {
        // NmsLog.error("captionEditor", "after text changed is called") ;
        if (mIsHandleNow) {
            mIsHandleNow = false ;
            mIsHandling = false ;
            setEmoticon(s);
            mListener.doAction(ACTION_TEXT_CHANGE);
            return ;
        }
        
        if (!mIsHandling) {
            mIsHandling = true ;
            final Editable tmpS = s ;
            mHandler.postDelayed(new Runnable() {
                public void run() {
                    if (!mIsHandling) 
                        return ;
                    try {
                        setEmoticon(tmpS);
                        mIsHandling = false ;
                    } catch (Exception e) {
                        NmsLog.error("captionEditor", "got execption in setEmoticon: " + e.toString()) ;
                        mIsHandling = false ;
                    }
                }
            }, (s.length() < 50) ? 500 : 2500) ;
        }
        
        mListener.doAction(ACTION_TEXT_CHANGE);
    }

    private void setEmoticon(Editable s) {
        SmileyParser parser = SmileyParser.getInstance();
        mEmoticonCount = parser.addSmileySpans(s);
    }

    public void addActionListener(ActionListener l) {
        mListener = l;
    }

    public interface ActionListener {
        public void doAction(int type);
    }

    public void setIsGroupChat(boolean isGroupChat) {
        if (true == isGroupChat) {
            mIsGroupChat = true;
            if (View.VISIBLE == mAttachThumbnail.getVisibility()) {
                setEditHint(R.string.STR_NMS_CAPTION_HINT);
            } else {
                setEditHint(R.string.STR_NMS_INPUT_HINT_ISMS);
            }
        } else {
            mIsGroupChat = false;
            if (View.VISIBLE == mAttachThumbnail.getVisibility()) {
                setEditHint(R.string.STR_NMS_CAPTION_HINT);
            } else {
                setEditHint(R.string.STR_NMS_INPUT_HINT);
            }
        }
    }

    private static void showToast(Context m, int resId) {
        if (mToast != null && mToastId == resId) {
            mToast.cancel();
        }
        mToastId = resId;
        mToast = Toast.makeText(m, resId, Toast.LENGTH_SHORT);
        mToast.show();
    }
}
