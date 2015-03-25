package com.mediatek.hotknotbeam;


import android.content.Context;
import android.database.Cursor;
import android.provider.MediaStore;
import android.net.Uri;
import android.os.AsyncTask;
import android.util.Log;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;
import java.io.IOException;
import java.io.OutputStream;

import java.util.List;
import java.util.Properties;
import java.net.URI;
import java.net.HttpURLConnection;
import java.net.URISyntaxException;
import java.net.URL;


public class FileUploadTask extends AsyncTask<Uri, Integer, Void> {
    private final static String TAG = HotKnotBeamService.TAG;

    private final static String LINE_END    = "\r\n";
    private final static String TWO_HYPHENS = "--";
    private final static String BOUNDARY    = "*****";

    private final static int MAX_BUFFER_SIZE  = 32 * 1024;

    private int                     mPort = HotKnotBeamService.SERVICE_PORT;
    private String                  mUploadServer = "";
    private HttpURLConnection       mConnection = null;
    private Context                 mContext = null;
    private FileUploadTaskListener  mPostExecuteCB = null;
    private Uri                     mUri = null;
    private String                  mFileName = null;

    public interface FileUploadTaskListener {
        public void onPostExecute(Void result, Uri uri);
    }

    private FileUploadTask() {
        super();
    }

    public FileUploadTask(String server, int port, Context context) {
        mContext = context;
        mPort = port;
        mUploadServer = "http://" + server + ":" + mPort;
        Properties pro = System.getProperties();
        pro.remove("http.proxyHost");
        pro.remove("http.proxyPort");
    }

    public void setOnPostExecute(FileUploadTaskListener callback) {
        mPostExecuteCB = callback;
    }

    public void setUploadFileName(String filename){
        mFileName = filename;
    }

    private void sendClientFinishNotify() {
        OutputStream outStream = null;
        InputStream  inStream = null;

        try {
            URL url = new URL(mUploadServer + "/" + HotKnotBeamConstants.BEAM_FINISH_COMMAND);
            mConnection = (HttpURLConnection) url.openConnection();
            mConnection.setDoInput(true);
            mConnection.setDoOutput(true);
            mConnection.setUseCaches(false);
            mConnection.setRequestMethod("POST");

            mConnection.setConnectTimeout(HotKnotBeamConstants.MAX_TIMEOUT_VALUE);
            mConnection.setReadTimeout(HotKnotBeamConstants.MAX_TIMEOUT_VALUE);
            outStream = mConnection.getOutputStream();
            inStream = mConnection.getInputStream();

            String buffer = LINE_END + LINE_END;
            outStream.write(buffer.getBytes(), 0, buffer.length());
            outStream.flush();
            if(outStream  != null) {
                outStream.close();
                outStream = null;
            }
            if(inStream  != null) {
                inStream.close();
                inStream = null;
            }
            mConnection.disconnect();
            mConnection = null;
        } catch(Exception e) {
            Log.e(TAG, "error in sendClientFinishNotify:" + e.getMessage());
        } finally {
            try {
                if(inStream  != null) {
                    inStream.close();
                    inStream = null;
                }
                if(outStream  != null) {
                    outStream.close();
                    outStream = null;
                }
                if(mConnection != null) {
                    mConnection.disconnect();
                    mConnection = null;
                }
            } catch(IOException ioe) {

            }
        }
    }

    private boolean sendDataFile(File inFile) {
        boolean retry = false;
        byte[] buffer;
        long fileOffset = 0;
        OutputStream outputStream = null;
        FileInputStream fileInputStream = null;
        int bytesRead, bytesAvailable, bufferSize;

        try {
            String filename = inFile.getName();
            URL sUrl = null;

            if(mFileName != null){
               filename = mFileName;
            }
            filename = Uri.encode(filename);

            if(mUri.getQuery() != null) {
                sUrl = new URL(mUploadServer + "/" + filename + "?" + mUri.getEncodedQuery());
            } else {
                sUrl = new URL(mUploadServer + "/" + filename);
            }
            mConnection = (HttpURLConnection) sUrl.openConnection();

            mConnection.setDoInput(true);
            mConnection.setDoOutput(true);
            mConnection.setUseCaches(false);
            mConnection.setFixedLengthStreamingMode((int) inFile.length());

            mConnection.setRequestMethod("POST");
            mConnection.setConnectTimeout(HotKnotBeamConstants.MAX_TIMEOUT_VALUE);
            mConnection.setReadTimeout(HotKnotBeamConstants.MAX_TIMEOUT_VALUE);

            outputStream =mConnection.getOutputStream();
            fileInputStream = new FileInputStream(inFile);

            bytesAvailable = fileInputStream.available();
            bufferSize = Math.min(bytesAvailable, MAX_BUFFER_SIZE);

            Log.d(TAG, "buffer size:"  + bufferSize);
            buffer = new byte[bufferSize];

            //Start to read file
            bytesRead = fileInputStream.read(buffer, 0, bufferSize);
            while (bytesRead > 0) {
                //Log.d(TAG, "[client]write:"  + bytesAvailable + "/" + bufferSize);
                outputStream.write(buffer, 0, bufferSize);
                fileOffset += bufferSize;

                bytesAvailable = fileInputStream.available();
                bufferSize = Math.min(bytesAvailable, MAX_BUFFER_SIZE);
                bytesRead = fileInputStream.read(buffer, 0, bufferSize);
            }
            //outputStream.writeBytes(LINE_END + TWO_HYPHENS + BOUNDARY + LINE_END);
            outputStream.flush();
            if(outputStream  != null) {
                outputStream.close();
                outputStream = null;
            }

            int serverResponseCode = mConnection.getResponseCode();
            String serverResponseMessage = mConnection.getResponseMessage();
            Log.i(TAG, "Http Response :" + serverResponseCode + ":" + serverResponseMessage);
        } catch(IOException ioe) {
            Log.e(TAG, "error in ioe:" + ioe.getMessage());
            if(ioe.getMessage() != null) {
                if(ioe.getMessage().indexOf("ECONNREFUSED") != -1 ) {
                    retry = true;
                }
            } else {
                ioe.printStackTrace();
            }
        } catch(Exception ex) {
            ex.printStackTrace();
        } finally {
            try {
                if(fileInputStream != null) {
                    fileInputStream.close();
                }
                if(outputStream  != null) {
                    outputStream.close();
                }
                if(mConnection != null) {
                    mConnection.disconnect();
                    mConnection = null;
                }
            } catch(IOException e) {

            }
        }

        return retry;
    }

    @Override
    protected void onPreExecute() {

    }

    @Override
    protected Void doInBackground(Uri... uri) {
        File inFile = null;

        mUri = uri[0];

        if(mUri == null) {
            sendClientFinishNotify();
            return null;
        }

        inFile = getFilePathFromUri(mUri);
        if(inFile == null) {
            return null;
        }

        try {
            Log.i(TAG, "Transfer File:" + inFile.getName() + ":" + inFile.length());

            Log.e(TAG, "max file size:" + HotKnotBeamConstants.MAX_FILE_UPLOAD_SIZE);
            /*
            if(inFile.length() > HotKnotBeamConstants.MAX_FILE_UPLOAD_SIZE){
               Log.e(TAG, "The file size is not allowed");
               return null;
            }
            */
        } catch(Exception e) {
            Log.e(TAG, "transfer file:" + e.getMessage());
        }

        boolean retry = false;
        for(int i = 0; i < HotKnotBeamConstants.MAX_RETRY_COUNT; i++) {
            try {
                if(i != 0) {
                    Thread.sleep(HotKnotBeamConstants.RETRY_SLEEP_TIMER * i);
                }
            } catch(Exception e) {

            }
            if(!sendDataFile(inFile)) {
                return null;
            }
        }

        return null;
    }

    @Override
    protected void onProgressUpdate(Integer... progress) {

    }

    @Override
    protected void onPostExecute(Void result) {
        Log.d(TAG, "Finish upload action");
        if(mPostExecuteCB != null) {
            mPostExecuteCB.onPostExecute(result, mUri);
        }
    }

    private File getFilePathFromUri(Uri uri) {
        File inputFile = null;
        String filePath = "";

        if(uri == null) {
            Log.e(TAG, "File Uri must not be null");
            throw new IllegalArgumentException("File Uri must not be null");
        }

        String scheme = uri.getScheme();

        if(scheme != null && scheme.equalsIgnoreCase("content")) {
            Cursor cursor = mContext.getContentResolver().query(uri, null, null, null, null);
            if(cursor!= null) {
                int index = cursor.getColumnIndexOrThrow(MediaStore.Images.Media.DATA);
                cursor.moveToFirst();
                filePath = cursor.getString(index);
                cursor.close();
            } else {
                Log.e(TAG, "Cursor is null");
            }
        } else if(scheme != null && scheme.equalsIgnoreCase("file")) {
            filePath = "/" + uri.getHost() + uri.getPath();
        }

        if(filePath.length() == 0) {
            Log.e(TAG, "File path is empty");
            return null;
        }

        Log.d(TAG, "The sending path is " + filePath);
        inputFile = new File(filePath);

        if(!inputFile.exists() || inputFile.isDirectory()) {
            Log.e(TAG, "File is not existed or inputFile is a directory");
            return null;
        }

        return inputFile;
    }

}