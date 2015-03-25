package com.mediatek.hotknotbeam;

import android.content.Context;
import android.content.Intent;
import android.media.MediaScannerConnection;
import android.media.MediaPlayer;
import android.net.http.Headers;
import android.net.http.Headers.HeaderCallback;
import android.net.Uri;
import android.os.Environment;
import android.util.Log;

import com.mediatek.hotknotbeam.HotKnotBeamConstants.State;
import com.mediatek.storage.StorageManagerEx;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.EOFException;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketTimeoutException;
import java.text.SimpleDateFormat;
import java.util.Collection;
import java.util.Date;
import java.util.LinkedList;
import java.util.Locale;

import org.apache.http.HttpStatus;
import org.apache.http.HttpVersion;
import org.apache.http.ProtocolVersion;
import org.apache.http.HttpResponse;
import org.apache.http.ReasonPhraseCatalog;

import org.apache.http.impl.EnglishReasonPhraseCatalog;
import org.apache.http.message.BasicStatusLine;
import org.apache.http.protocol.BasicHttpContext;



public class HotKnotFileServer {
    private final static String TAG = HotKnotBeamService.TAG;

    private final static String CRLF = "\r\n";

    private final static int MAX_BUFFER_SIZE = 16 * 1024;
    private final static int MAX_TIMEOUT_VALUE = 15 * 1000; //The default timer value is 30 seconds

    private static boolean mIsServerRunning = false;
    private static int mPort = HotKnotBeamService.SERVICE_PORT;

    // LinkList to queue the download request
    private LinkedList<DownloadInfo> mDownloadList = new LinkedList<DownloadInfo>();

    private Context             mContext = null;
    private HotKnotFileServer   mHotKnotFileServer = null;
    private ServerSocket        mServerSocket = null;
    private Thread              mServerThread = null;
    private HotKnotFileServerCb mHotKnotFileServerCb = null;

    public interface HotKnotFileServerCb {
        public void onHotKnotFileServerFinish(int status);
    }


    public HotKnotFileServer(int port, Context context) {
        mPort = port;
        mContext = context;

        mHotKnotFileServer = this;
        mIsServerRunning = false;
    }

    public void execute() {
        mIsServerRunning = true;

        try {
            mServerSocket = new ServerSocket(mPort);
            mServerThread = new Thread(new ServerThread());
            mServerThread.start();
        } catch(IOException e) {
            e.printStackTrace();
            mIsServerRunning = false;
        }
    }

    public void cancel(int id) {
        synchronized (mDownloadList) {
            for(DownloadInfo info : mDownloadList) {
                if(info.mId == id) {
                    if(info.mState != State.COMPLETE) {
                        info.mState = State.COMPLETE;
                        info.setResult(false);
                        CommunicationThread cmThread = info.getClientThread();
                        Log.d(TAG, "interrupt thread");
                        cmThread.close();
                        cmThread.interrupt();
                    }
                    break;
                }
            }
        }
    }

    public void stop() {
        mIsServerRunning = false;
        mServerThread.interrupt();
        try {
            mServerSocket.close();
        } catch(IOException ioe) {
            Log.e(TAG, "stop in server thread:" + ioe.getMessage());
        }
    }

    public void setHotKnotFileServerCb(HotKnotFileServerCb cb) {
        mHotKnotFileServerCb = cb;
    }

    public Collection<DownloadInfo> getDownloadInfos() {

        synchronized (mDownloadList) {
            if(mDownloadList.size() > 0) {
                return mDownloadList;
            }
        }
        return null;
    }

    class ServerThread extends Thread {

        public void run() {
            Socket socket = null;
            while (!Thread.currentThread().isInterrupted()) {
                try {
                    Log.d(TAG, "Server Listen");
                    socket = mServerSocket.accept();
                    CommunicationThread commThread = new CommunicationThread(socket);
                    new Thread(commThread).start();
                } catch (IOException e) {
                    Log.e(TAG, "accept error:" + e.getMessage());
                }
            }
            if(!mIsServerRunning) {
                Log.d(TAG, "Notify server thread is ended");
                mHotKnotFileServerCb.onHotKnotFileServerFinish(0);
            }
        }
    }

    protected class CommunicationThread extends Thread {
        private Socket mClientSocket;
        private InputStream input;
        DownloadInfo mInfo;

        public CommunicationThread(Socket clientSocket) {
            mClientSocket = clientSocket;
            try {
                mClientSocket.setSoTimeout(MAX_TIMEOUT_VALUE);
                mClientSocket.setSoSndTimeout(MAX_TIMEOUT_VALUE);
                mClientSocket.setReceiveBufferSize(MAX_BUFFER_SIZE);
                mClientSocket.setSoLinger(false, 0);
                input = mClientSocket.getInputStream();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        public void close() {
            try {
                mClientSocket.close();
            } catch(IOException e) {
                Log.i(TAG, "error:" + e.getMessage());
            }
        }

        public void run() {
            try {
                Log.d(TAG, "Client Connect:" + mClientSocket.getRemoteSocketAddress());
                boolean isReadSuccess = doDownload(StorageManagerEx.getDefaultPath() + File.separator + HotKnotBeamConstants.MAX_HOTKNOT_BEAM_FOLDER);
            } catch(IOException e) {
                Log.e(TAG, "error:" + e.getMessage());
            } catch(Exception ex) {
                ex.printStackTrace();
            } finally {
                try {
                    if(input != null) {
                        input.close();
                    }
                    if(mClientSocket != null) {
                        mClientSocket.close();
                    }
                } catch(Exception ex) {

                }
            }
            Log.d(TAG, "CommunicationThread is finished");
        }

        private boolean doDownload(String rootPath) throws IOException {
            String line = "";
            String fileName = "";
            String extInfo = "";
            int    fileSize = 0;
            BufferedOutputStream  outBody = null;

            //Get FileName
            line = readAsciiLine(input);
            fileName = Uri.decode(parseFileName(line));
            extInfo  = parseExtInfo(line);
            Log.d(TAG, "File info:" + fileName + ":" + extInfo);

            if(fileName.length() == 0) {
                Log.e(TAG, "can't get file name");
                return false;
            }

            if(fileName != null && fileName.equals(HotKnotBeamConstants.BEAM_FINISH_COMMAND)) {
                Log.d(TAG, "Terminate server thread");
                try {
                    outBody = new BufferedOutputStream(mClientSocket.getOutputStream());
                    byte[] response = getResponse(HttpStatus.SC_OK);
                    outBody.write(response, 0, response.length);
                    outBody.flush();
                    mHotKnotFileServer.stop();
                } catch(Exception e) {
                    Log.e(TAG, "[mHotKnotFileServer]" + e.getMessage());
                }
                return true;
            }

            do {
                line = readAsciiLine(input);
                line = line.toLowerCase();

                //Get File Size
                if(line.indexOf(Headers.CONTENT_LEN) != -1) {
                    String contentLen = line.substring(line.indexOf(":")+1);
                    fileSize = Integer.parseInt(contentLen.trim());
                } else if(line.indexOf(Headers.CONTENT_TYPE) != -1) {

                }
            } while(line.length() != 0);

            Log.d(TAG, "File info:" + fileName + ":" + fileSize);

            //Create one donwload info
            mInfo = new DownloadInfo(rootPath, fileName, fileSize, this);
            if(extInfo != null) {
                mInfo.setExtInfo(extInfo);
                Log.d(TAG, "set extInfo:" + extInfo);
                rootPath = mInfo.getSaveFolder();
            }

            synchronized (mDownloadList) {
                if(mInfo.isShowNotification()) {
                    mDownloadList.add(mInfo);
                }
            }

            if(mInfo.isCompressed()){
               SimpleDateFormat tmp = new SimpleDateFormat("yyyy-MM-dd_HH-mm-ss-SSS");
               fileName = tmp.format(new Date());
            }
            File outFile = prepareFile(new File(rootPath), fileName, mInfo.isRenameFile());
            FileOutputStream fout = new FileOutputStream(outFile);

            int recvRemainBytes = fileSize;
            int recvBytes = 0, bufferSize = 0;
            byte[] buffer = new byte[MAX_BUFFER_SIZE];

            mInfo.setState(HotKnotBeamConstants.State.RUNNING);
            try {
                outBody = new BufferedOutputStream(mClientSocket.getOutputStream());
                bufferSize = Math.min(fileSize, MAX_BUFFER_SIZE);
                do {
                    recvBytes = input.read(buffer, 0, bufferSize);
                    recvRemainBytes -= recvBytes;
                    fout.write(buffer, 0, recvBytes);
                    bufferSize = Math.min(recvRemainBytes, MAX_BUFFER_SIZE);
                    //Log.d(TAG, "[server]read:" + recvRemainBytes + "/" + recvBytes + ":" + bufferSize);
                    mInfo.setCurrentBytes(recvRemainBytes);
                } while(recvRemainBytes > 0);

                Log.d(TAG, "Transfer done");

                //Succesesfully transfer done
                if(recvRemainBytes == 0) {

                    if(mInfo.isCompressed()) {
                        fout.close();
                        fout = null;
                        ZipFileUtils.unzip(outFile, new File(rootPath), mContext);
                        outFile.delete();
                        Log.d(TAG, "Unzip successfully");
                    } else {
                    //Update the gallery database
                    String[] paths = new String[1];
                    paths[0] = outFile.getCanonicalPath();
                    MediaScannerConnection.scanFile(mContext, paths, null, null);

                    //Demo purpose
                        if(mInfo.isShowNotification() && MimeUtilsEx.isGallerySupport(paths[0])) {
                            try {
                        showActivity();
                            } catch(Exception e) {
                                e.printStackTrace();
                            }
                        }
                    }

                    mInfo.setResult(true);
                    byte[] response = getResponse(HttpStatus.SC_OK);
                    outBody.write(response, 0, response.length);
                    outBody.flush();
                }

            } catch(SocketTimeoutException ex) {

            } finally {
                mInfo.setState(HotKnotBeamConstants.State.COMPLETE);
                try {
                    if(recvRemainBytes > 0) {
                        Log.e(TAG, "Transfer failed");
                        outFile.delete();

                        mInfo.setResult(false);
                    }

                    if(fout!= null) {
                    fout.close();
                    fout = null;
                    }
                    if(outBody != null) {
                        outBody.close();
                    }
                } catch (IOException e) {
                    Log.e(TAG, "Error closing input stream: " + e.getMessage());
                }
            }
            return true;
        }

        private String readAsciiLine(InputStream in) throws IOException {
            StringBuilder result = new StringBuilder(80);
            while (true) {
                int c = in.read();
                if (c == -1) {
                    throw new EOFException();
                } else if (c == '\n') {
                    break;
                }

                result.append((char) c);
            }
            int length = result.length();
            if (length > 0 && result.charAt(length - 1) == '\r') {
                result.setLength(length - 1);
            }
            return result.toString();
        }

        private String parseFileName(String line) {
            try {
                int dotPos = line.indexOf('/');
                int dotPos2 = 0;
                if(line.indexOf('?') != -1) {
                    dotPos2 = line.indexOf('?');
                } else {
                    dotPos2 = line.lastIndexOf("HTTP/1.1")-1;
                }
                return line.substring(dotPos+1, dotPos2).trim();
            } catch(Exception e) {
                e.printStackTrace();
            }
            return "";
        }

        private File prepareFile(File fileDir, String fileName, boolean isRename) {
            int pos = fileName.indexOf('?');
            if(pos != -1) {
                fileName = fileName.substring(0, pos-1);
            }

            if (!fileDir.exists()) {
                fileDir.mkdirs();
            }

            File file = new File(fileDir, fileName);

            if (file.exists() && isRename) {
                String subFileName = "";
                StringBuilder fileNameBuilder = new StringBuilder();

                int dot = fileName.indexOf(".");
                if(dot != -1) {
                    subFileName = fileName.substring(dot);
                    fileNameBuilder.append(fileName.substring(0, dot));
                } else {
                    fileNameBuilder.append(fileName);
                }

                int fileMainNameLen = fileNameBuilder.length();
                int i = 1;
                File newFile = null;
                while (true) {
                    fileNameBuilder.append("(").append(i++).append(")")
                    .append(subFileName);

                    newFile = new File(fileDir, fileNameBuilder.toString());
                    if (!newFile.exists()) {
                        return newFile;
                    }

                    fileNameBuilder.setLength(fileMainNameLen);
                }
            } else {
                return file;
            }
        }

        private byte[] getResponse(int status) {
            String response = "";

            final ReasonPhraseCatalog reasonCatalog = EnglishReasonPhraseCatalog.INSTANCE;
            final String reason = reasonCatalog.getReason(status, Locale.getDefault());
            BasicStatusLine statusLine = new BasicStatusLine(HttpVersion.HTTP_1_1, status, reason);
            response = statusLine.toString() + CRLF + CRLF;

            return response.getBytes();
        }

        private void showActivity() {
            String mimeType = mInfo.getMimeType();

            try {
                MediaPlayer magicMp = MediaPlayer.create(mContext, R.raw.magic);
                Log.d(TAG, "Play magic sound");

                if(!magicMp.isPlaying())
                {
                    magicMp.start();
                }
            } catch(Exception e) {
                e.printStackTrace();
            }

            Uri uri = mInfo.getUri();
            if(mimeType != null && uri != null) {
                final Intent intent = new Intent(Intent.ACTION_VIEW);
                intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
                intent.setDataAndTypeAndNormalize(uri, mimeType);
                mContext.startActivity(intent);
            } else {
                Log.e(TAG, "mimeType is null or uri is null");
            }
        }

        private String parseExtInfo(String line) {
            String extInfo = null;
            int pos = line.indexOf('?');
            int pos2 = line.lastIndexOf("HTTP/1.1")-1;
            if(pos == -1) return extInfo; //No support query string

            extInfo = line.substring(pos+1, pos2);
            try {
                Uri.decode(extInfo);
            } catch(Exception e) {
                Log.e(TAG, "decode:" + e.getMessage());
            }

            return extInfo;
        }
    }
}