package com.orangelabs.rcs.service.api.client.media.audio;

import java.io.IOException;
import java.io.InputStream;
import java.util.Iterator;
import java.util.Vector;

import android.media.MediaRecorder;
import android.net.LocalServerSocket;
import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.os.RemoteException;
import android.util.Log;

import com.orangelabs.rcs.core.ims.protocol.rtp.MediaRegistry;
import com.orangelabs.rcs.core.ims.protocol.rtp.MediaRtpSender;
import com.orangelabs.rcs.core.ims.protocol.rtp.format.audio.AudioFormat;
import com.orangelabs.rcs.core.ims.protocol.rtp.media.MediaException;
import com.orangelabs.rcs.core.ims.protocol.rtp.media.MediaInput;
import com.orangelabs.rcs.core.ims.protocol.rtp.media.MediaSample;
import com.orangelabs.rcs.core.ims.protocol.rtp.stream.RtpStreamListener;
import com.orangelabs.rcs.platform.network.DatagramConnection;
import com.orangelabs.rcs.platform.network.NetworkFactory;
import com.orangelabs.rcs.service.api.client.media.IAudioEventListener;
import com.orangelabs.rcs.service.api.client.media.IAudioPlayer;
import com.orangelabs.rcs.service.api.client.media.MediaCodec;
import com.orangelabs.rcs.utils.CodecsUtils;
import com.orangelabs.rcs.utils.FifoBuffer;
import com.orangelabs.rcs.utils.NetworkRessourceManager;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * Audio RTP player. Only the AMR WB format is supported.
 *
 * @author opob7414
 */
public class LiveAudioPlayer extends IAudioPlayer.Stub implements RtpStreamListener {
	
    /**
     * List of supported audio codecs
     */
    private MediaCodec[] supportedAudioCodecs = null; 
    
    /**
     * Selected audio codec
     */
    private AudioCodec selectedAudioCodec = null;
    
    /**
     * Audio format
     */
    private AudioFormat audioFormat;
    
	/**
     * Local MediaRecorder object to capture mic and encode the stream
     */
    private MediaRecorder mediaRecorder;
    
	/**
     * AudioRenderer for RTP stream sharing
     */
    private AudioRenderer audioRenderer = null;        
    
	/**
     * Local RTP port
     */
    private int localRtpPort;
    
    /**
     * RTP sender session
     */
    private MediaRtpSender rtpSender = null;

    /**
     * RTP audio input
     */
    private AudioRtpInput rtpInput = null;
    
    /**
     * Is player opened
     */
    private boolean opened = false;

    /**
     * Is player started
     */
    private boolean started = false;
    
    /**
     * Audio event listeners
     */
    private Vector<IAudioEventListener> listeners = new Vector<IAudioEventListener>();
    
    /**
     * Temporary connection to reserve the port
     */
    private DatagramConnection temporaryConnection = null; 
    
    /**
     * Timestamp increment
     */
    private int timestampInc = 100; // calculate it ?
    
    /***
     * Current time stamp
     */
    private long timeStamp = 0;
    
    /**
     * Local socket sender
     */
    private LocalSocket localSocketSender;
    
    /**
     * Local socket receiver
     */
    private LocalSocket localSocketReceiver;
    
    /**
     * Local socket
     */
    private LocalServerSocket localServerSocket;
    
    /**
     * Local socket endpoint
     */    
    private static final String LOCAL_SOCKET = "com.orangelabs.rcs.service.api.client.media.audio.socket.player";
    
    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

	/**
     * Constructor for standalone mode
     */
	public LiveAudioPlayer() { 	
		
    	// Set the local RTP port
        localRtpPort = NetworkRessourceManager.generateLocalRtpPort();
        reservePort(localRtpPort);
        
        // Init codecs
        supportedAudioCodecs = CodecsUtils.getSupportedAudioCodecList();   

        // Set the default media codec
        if (supportedAudioCodecs.length > 0) {
            setAudioCodec(supportedAudioCodecs[0]);
        }
        
        if (logger.isActivated()) {
        	logger.info("LiveAudioPlayer constructor : reserve local RTP port("+localRtpPort+"), init codec and set audiocodec("+supportedAudioCodecs[0].getCodecName()+")");
        }

    }
	
	/**
     * Constructor for RTP stream sharing with audio renderer
     */
	public LiveAudioPlayer(AudioRenderer ar) { 	
		
    	// Set the local RTP port
        localRtpPort = NetworkRessourceManager.generateLocalRtpPort();
        reservePort(localRtpPort);
        
        // Get and set locally the audio renderer reference
        audioRenderer = ar;
        
        // Init codecs
        supportedAudioCodecs = CodecsUtils.getSupportedAudioCodecList();   

        // Set the default media codec
        if (supportedAudioCodecs.length > 0) {
            setAudioCodec(supportedAudioCodecs[0]);
        }
        
        if (logger.isActivated()) {
        	logger.info("LiveAudioPlayer constructor with renderer : reserve local RTP port("+localRtpPort+"), init codec and set audiocodec("+supportedAudioCodecs[0].getCodecName()+")");        	
        }

    }	
	
	@Override
	public void rtpStreamAborted() {
		notifyPlayerEventError("RTP session aborted");
	}

	 /**
     * Reserve a port.
     *
     * @param port Port to reserve
     */
    private void reservePort(int port) {
        if (temporaryConnection == null) {
            try {
                temporaryConnection = NetworkFactory.getFactory().createDatagramConnection();
                temporaryConnection.open(port);
            } catch (IOException e) {
                temporaryConnection = null;
            }
        }
    }
    
    /**
     * Release the reserved port.
     */
    private void releasePort() {
        if (temporaryConnection != null) {
            try {
                temporaryConnection.close();
            } catch (IOException e) {
                temporaryConnection = null;
            }
        }
    }
    
    /**
     * Is player opened
     *
     * @return Boolean
     */
    public boolean isOpened() {
        return opened;
    }

    /**
     * Is player started
     *
     * @return Boolean
     */
    public boolean isStarted() {
        return started;
    }

	@Override
	public void open(String remoteHost, int remotePort) throws RemoteException {

        if (opened) {
            // Already opened
        	if (logger.isActivated()) {
        		logger.info("audioplayer open : already opened");
        	}
            return;
        }		
        
		if (logger.isActivated()) {
			logger.info("open the audioplayer");
		}
        
        // Init the socket listener thread
        new LiveAudioPlayerSocketListener().start();
        
        // Init the RTP layer
        try {
            releasePort();
            rtpSender = new MediaRtpSender(audioFormat, localRtpPort);
            rtpInput = new AudioRtpInput();
            rtpInput.open();
            if ( audioRenderer != null ) {
            	// The audio renderer is supposed to be opened and so we used its RTP stream
            	if (logger.isActivated()) {
            		logger.info("audioplayer share the audio renderer rtp stream on same port");
            	}
            	rtpSender.prepareSession(rtpInput, remoteHost, remotePort, audioRenderer.getRtpInputStream(), this);
            } else { 
            	// The audio renderer doesn't exist and so we create a new RTP stream
            	rtpSender.prepareSession(rtpInput, remoteHost, remotePort, this);
            }
        } catch (Exception e) {
            notifyPlayerEventError(e.getMessage());
            return;
        }
        
        // Player is opened
        opened = true;
        notifyPlayerEventOpened();       
	}

	@Override
	public void close() throws RemoteException {		
        if (!opened) {
            // Already closed
        	if (logger.isActivated()) {
        		logger.info("audioplayer close : already closed");
        	}
            return;
        }
        
		if (logger.isActivated()) {
			logger.info("close the audioplayer");
		}
        
        // Close the RTP layer
        rtpInput.close();
        rtpSender.stopSession();

        // Player is closed
        opened = false;
        notifyPlayerEventClosed();
        listeners.clear();		
	}

	@Override
	public void start() throws RemoteException {    		
        if (!opened) {
            // Player not opened
        	if (logger.isActivated()) {
        		logger.info("audioplayer start : not opened");
        	}
            return;
        }

        if (started) {
            // Already started
        	if (logger.isActivated()) {
        		logger.info("audioplayer start : already started");
        	}
            return;
        }
        
		if (logger.isActivated()) {
			logger.info("start the LiveAudioPlayer");
		}
        
        // Set and start the media recorder
        startMediaRecorder();
    	
    	// Start the RTP sender
    	rtpSender.startSession();
    	
    	// Start the media recorder
		mediaRecorder.start();
		if (logger.isActivated()) {
			logger.info("start MediaRecorder");
		}
		
        // Player is started
        started = true;
        notifyPlayerEventStarted();
	}
	
    /**
     * Create, prepare and start the media recorder
     */
	public void startMediaRecorder() {
		// Create media recorder
		mediaRecorder = new MediaRecorder();
		if (logger.isActivated()) {
			logger.info("create MediaRecorder");
		}
		
		// Set media recorder listener 
		mediaRecorder.setOnErrorListener(new MediaRecorder.OnErrorListener() {

			@Override
			public void onError(MediaRecorder mr, int what, int extra) {
				if (logger.isActivated()) {
					logger.error("mediaRecorder error : reason=" + what);
				}
			}
		});
		
		// Set media recorder audio source, output format and audio encoder
    	mediaRecorder.setAudioSource(MediaRecorder.AudioSource.MIC);    	
    	mediaRecorder.setOutputFormat(MediaRecorder.OutputFormat.AMR_WB);
    	mediaRecorder.setAudioEncoder(MediaRecorder.AudioEncoder.AMR_WB);
    	if (logger.isActivated()) {
    		logger.info("set mediaRecorder source=MIC outputformat=AMR_WB audioencoder=AMR_WB");
    	}
		
		// Set output in a local socket
		localSocketSender = new LocalSocket(); 
		if (logger.isActivated()) {
			logger.info("new localSenderSocket");
		}

		try {
			 localSocketSender.connect(new LocalSocketAddress(LOCAL_SOCKET));
			 if (logger.isActivated()) {
				 logger.info("localSenderSocket connect locally to the thread");
			 }
		} catch (IOException e) {
			e.printStackTrace();
		}

		mediaRecorder.setOutputFile(localSocketSender.getFileDescriptor());
		if (logger.isActivated()) {
			logger.info("mediaRecorder local socket sender endpoint = " + LOCAL_SOCKET);
		}
    	
    	// Prepare the media recorder
    	try {
			mediaRecorder.prepare();
			if (logger.isActivated()) {
				logger.info("prepare mediaRecorder");
			}
		} catch (IllegalStateException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	@Override
	public void stop() throws RemoteException {		
        if (!opened) {
            // Player not opened
        	if (logger.isActivated()) {
        		logger.info("audioplayer stop : not opened");
        	}
            return;
        }

        if (!started) {
            // Already stopped
        	if (logger.isActivated()) {
        		logger.info("audioplayer stop : not started");
        	}
            return;
        }
        
		if (logger.isActivated()) {
			logger.info("stop the audioplayer");
		}
        
		// Stop the audio recorder
        stopMediaRecorder();
   	 	
        notifyPlayerEventStopped();		
	}
	
	public void stopMediaRecorder() {
		// Stop audio recorder
   	 	mediaRecorder.stop();
   	 	if (logger.isActivated()) {
   	 		logger.info("stop MediaRecorder");
   	 	}
		try {
			localSocketSender.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
		if (logger.isActivated()) {
			logger.info("close localSocketSender");
		}
   	 	mediaRecorder.reset();
   	 	// Release the recorder
   	 	mediaRecorder.release();
   	 	if (logger.isActivated()) {
   	 		logger.info("release MediaRecorder");
   	 	}	
	}

	@Override
	public int getLocalRtpPort() throws RemoteException {
		return localRtpPort;
	}

	@Override
	public void addListener(IAudioEventListener listener)
			throws RemoteException {		
		listeners.addElement(listener);
	}

	@Override
	public void removeAllListeners() throws RemoteException {
		listeners.removeAllElements();
	}

    /**
     * Get supported media codecs
     *
     * @return media Codecs list
     */
    public MediaCodec[] getSupportedAudioCodecs() {
        return supportedAudioCodecs;
    }

	@Override
	public MediaCodec getAudioCodec() {
        if (selectedAudioCodec == null)
            return null;
        else
            return selectedAudioCodec.getMediaCodec();
	}

    /**
     * Set audio codec
     *
     * @param mediaCodec Audio codec
     */
	public void setAudioCodec(MediaCodec mediaCodec) {
        if (AudioCodec.checkAudioCodec(supportedAudioCodecs, new AudioCodec(mediaCodec))) {
            selectedAudioCodec = new AudioCodec(mediaCodec);
            audioFormat = (AudioFormat) MediaRegistry.generateFormat(mediaCodec.getCodecName());
        } else {
            notifyPlayerEventError("Codec not supported");
        }           
	}
	
    /**
     * Notify player event started
     */
    private void notifyPlayerEventStarted() {
        if (logger.isActivated()) {
            logger.info("Player is started");
        }
        Iterator<IAudioEventListener> ite = listeners.iterator();
        while (ite.hasNext()) {
            try {
                ((IAudioEventListener)ite.next()).audioStarted();
            } catch (RemoteException e) {
                if (logger.isActivated()) {
                    logger.error("Can't notify listener", e);
                }
            }
        }
    }

    /**
     * Notify player event stopped
     */
    private void notifyPlayerEventStopped() {
        if (logger.isActivated()) {
            logger.info("Player is stopped");
        }
        Iterator<IAudioEventListener> ite = listeners.iterator();
        while (ite.hasNext()) {
            try {
                ((IAudioEventListener)ite.next()).audioStopped();
            } catch (RemoteException e) {
                if (logger.isActivated()) {
                    logger.error("Can't notify listener", e);
                }
            }
        }
    }

    /**
     * Notify player event opened
     */
    private void notifyPlayerEventOpened() {
        if (logger.isActivated()) {
            logger.info("Player is opened");
        }
        Iterator<IAudioEventListener> ite = listeners.iterator();
        while (ite.hasNext()) {
            try {
                ((IAudioEventListener)ite.next()).audioOpened();
            } catch (RemoteException e) {
                if (logger.isActivated()) {
                    logger.error("Can't notify listener", e);
                }
            }
        }
    }

    /**
     * Notify player event closed
     */
    private void notifyPlayerEventClosed() {
        if (logger.isActivated()) {
            logger.info("Player is closed");
        }
        Iterator<IAudioEventListener> ite = listeners.iterator();
        while (ite.hasNext()) {
            try {
                ((IAudioEventListener)ite.next()).audioClosed();
            } catch (RemoteException e) {
                if (logger.isActivated()) {
                    logger.error("Can't notify listener", e);
                }
            }
        }
    }

    /**
     * Notify player event error
     */
    private void notifyPlayerEventError(String error) {
        if (logger.isActivated()) {
            logger.info("Player error: " + error);
        }

        Iterator<IAudioEventListener> ite = listeners.iterator();
        while (ite.hasNext()) {
            try {
                ((IAudioEventListener)ite.next()).audioError(error);
            } catch (RemoteException e) {
                if (logger.isActivated()) {
                    logger.error("Can't notify listener", e);
                }
            }
        }
    }
	
    /**
     * Audio RTP input
     */
    private static class AudioRtpInput implements MediaInput {
        /**
         * Received frames
         */
        private FifoBuffer fifo = null;
        /**
         * Constructor
         */
        public AudioRtpInput() {
        }

        /**
         * Add a new audio sample
         *
         * @param data Data
         * @param timestamp Timestamp
         * @param marker Marker bit 
         */
        public void addSample(byte[] data, long timestamp) {
            if (fifo != null) {
                MediaSample sample = new MediaSample(data, timestamp);
                fifo.addObject(sample);
            }
        }

        /**
         * Open the player
         */
        public void open() {
            fifo = new FifoBuffer();
        }

        /**
         * Close the player
         */
        public void close() {
            if (fifo != null) {
                fifo.close();
                fifo = null;
            }
        }

        /**
         * Read an media sample (blocking method)
         *
         * @return Media sample
         * @throws MediaException
         */
        public MediaSample readSample() throws MediaException {
            try {
                if (fifo != null) {
                    return (MediaSample)fifo.getObject();
                } else {
                    throw new MediaException("Media audio input not opened");
                }
            } catch (Exception e) {
                throw new MediaException("Can't read media audio sample");
            }
        }    
    }
    
    /**
     * Thread that listen from local socket connection and read bytes from it to add in RTP stream
     *
     */
    class LiveAudioPlayerSocketListener extends Thread {
        
        public LiveAudioPlayerSocketListener() {
        }
        
        @Override
        public void run() {

            try {
                localServerSocket = new LocalServerSocket(LOCAL_SOCKET);
                
                while (true) {
                	
                    localSocketReceiver = localServerSocket.accept();
 
                    if (localSocketReceiver != null) { 
                        
                        // Reading bytes from the socket
                    	
            	 		InputStream in = localSocketReceiver.getInputStream();
            	        int len = 0; 
            	        byte[] b = new byte[1024];
            	        byte[] buffer;
            	        
            	        if (logger.isActivated()) {
            	        	logger.info("start reading inputstream in localsocket server");
            	        }
            	        
            	        while ((len = in.read(b)) >= 0) {
            	        	buffer = new byte[len];
            	        	for (int j = 0; j < len; j++) {
            	            	buffer[j] = b[j];
            	            }
            	            rtpInput.addSample(buffer, timeStamp);          
//                	        if (logger.isActivated()) {
//                	        	logger.info("addSample to rtp input: " + buffer.length);
//                	            StringBuilder sb = new StringBuilder();
//                	            for (int y = 0; y < buffer.length; y++) {
//                	            	sb.append(" "+Byte.valueOf(buffer[y]).toString());        	            	
//                	            }
//                	        	logger.info("addSample to rtp input: " + sb.toString());
//                	        }
            	            timeStamp += timestampInc; // needed ?
            	            
            	        }
            	        if (logger.isActivated()) {
            	        	logger.info("stop reading inputstream in localsocket server");
            	        }
                    }
                }
            } catch (IOException e) {
                Log.e(getClass().getName(), e.getMessage());
            }
        }
    }

}
