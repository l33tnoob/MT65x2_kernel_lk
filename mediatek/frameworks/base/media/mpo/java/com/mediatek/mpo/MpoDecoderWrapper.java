package com.mediatek.mpo;

import com.mediatek.common.mpodecoder.IMpoDecoder;
import com.mediatek.mpo.MpoDecoder;

import android.util.Log;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory.Options;
import android.net.Uri;
import android.os.SystemClock;
import android.content.ContentResolver;
import android.content.Context;

public class MpoDecoderWrapper implements IMpoDecoder{
    private static final String TAG = "MpoDecoWrapper";
    private MpoDecoder obj = null;

    public MpoDecoderWrapper() {
        Log.d(TAG, "Default constructor is called");
    }

    public MpoDecoderWrapper(String pathName) {
        Log.d(TAG, "DecodeFile constructor is called");
        obj = MpoDecoder.decodeFile(pathName);
    }

    public MpoDecoderWrapper(ContentResolver cr, Uri mpoUri) {
        Log.d(TAG, "DecodeUri constructor is called");
        obj= MpoDecoder.decodeUri(cr,mpoUri);
    }

    /// M: for certain mpo file, MpoDecoder.decodeFile() will return null
    // then it will cause JE when reference the returned instance.
    public boolean isMpoDecoderValid() {
        if (obj != null) {
            return true;
        }
        return false;
    }

    public int width(){
        if (obj != null) {
            return obj.width();
        }
        return -1;
    }

    public int height(){
        if (obj != null) {
            return obj.height();
        }
        return -1;
    }

    public int frameCount(){
        if (obj != null) {
            return obj.frameCount();
        }
        return -1;
    }

    public int getMtkMpoType(){
        if (obj != null) {
            return obj.getMtkMpoType();
        }
        return -1;
    }

    public int suggestMtkMpoType(){
        if (obj != null) {
            return obj.suggestMtkMpoType();
        }
        return -1;
    }

    public Bitmap frameBitmap(int frameIndex, Options options){
        if (obj != null) {
            return obj.frameBitmap(frameIndex,options);
        }
        return null;
    }

    public void close(){
        if (obj != null) {
            obj.close();
        }
    }
}
