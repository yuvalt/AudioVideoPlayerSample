package com.samsungnext.media;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.nio.ByteBuffer;

import android.media.MediaCodec;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.media.MediaMetadataRetriever;
import android.text.TextUtils;
import android.util.Log;
import android.view.Surface;

public class MediaVideoPlayer {
    private static final boolean DEBUG = true;
    private static final String TAG_STATIC = "MediaMoviePlayer:";
    private final String TAG = TAG_STATIC + getClass().getSimpleName();

	private final IFrameCallback mCallback;

	public MediaVideoPlayer(final Surface outputSurface, final IFrameCallback callback) throws NullPointerException {
    	if (DEBUG) Log.v(TAG, "Constructor:");
    	if ((outputSurface == null) || (callback == null))
    		throw new NullPointerException("outputSurface and callback should not be null");

		mOutputSurface = outputSurface;
		mCallback = callback;
		new Thread(mMoviePlayerTask, TAG).start();
    	synchronized (mSync) {
    		try {
    			if (!mIsRunning)
    				mSync.wait();
			} catch (final InterruptedException e) {
				// ignore
			}
    	}
    }

    public final int getWidth() {
        return mVideoWidth;
    }

    public final int getHeight() {
        return mVideoHeight;
    }

    public final int getBitRate() {
    	return mBitrate;
    }

    public final float getFramerate() {
    	return mFrameRate;
    }

    /**
     * @return 0, 90, 180, 270
     */
    public final int getRotation() {
    	return mRotation;
    }

    /**
     * get duration time as micro seconds
     * @return
     */
    public final long getDurationUs() {
    	return mDuration;
    }

    /**
     * request to prepare movie playing
     * @param src_movie
     */
    public final void prepare(final String src_movie) {
    	if (DEBUG) Log.v(TAG, "prepare:");
    	synchronized (mSync) {
    		mSourcePath = src_movie;
    		mRequest = REQ_PREPARE;
    		mSync.notifyAll();
    	}
    }

    /**
     * request to start playing movie
     * this method can be called after prepare
     */
    public final void play() {
    	if (DEBUG) Log.v(TAG, "play:");
    	synchronized (mSync) {
    		if (mState == STATE_PLAYING) return;
    		mRequest = REQ_START;
    		mSync.notifyAll();
    	}
	}

    /**
     * request to seek to specifc timed frame<br>
     * if the frame is not a key frame, frame image will be broken
     * @param newTime seek to new time[usec]
     */
    public final void seek(final long newTime) {
    	if (DEBUG) Log.v(TAG, "seek");
    	synchronized (mSync) {
    		mRequest = REQ_SEEK;
    		mRequestTime = newTime;
    		mSync.notifyAll();
    	}
    }

    /**
     * request stop playing
     */
    public final void stop() {
    	if (DEBUG) Log.v(TAG, "stop:");
    	synchronized (mSync) {
    		if (mState != STATE_STOP) {
	    		mRequest = REQ_STOP;
	    		mSync.notifyAll();
	        	try {
	    			mSync.wait(50);
	    		} catch (final InterruptedException e) {
	    			// ignore
	    		}
    		}
    	}
    }

    /**
     * request pause playing<br>
     * this function is un-implemented yet
     */
    public final void pause() {
    	if (DEBUG) Log.v(TAG, "pause:");
    	synchronized (mSync) {
    		mRequest = REQ_PAUSE;
    		mSync.notifyAll();
    	}
    }

    /**
     * request resume from pausing<br>
     * this function is un-implemented yet
     */
    public final void resume() {
    	if (DEBUG) Log.v(TAG, "resume:");
    	synchronized (mSync) {
    		mRequest = REQ_RESUME;
    		mSync.notifyAll();
    	}
    }

    /**
     * release releated resources
     */
    public final void release() {
    	if (DEBUG) Log.v(TAG, "release:");
    	stop();
    	synchronized (mSync) {
    		mRequest = REQ_QUIT;
    		mSync.notifyAll();
    	}
    }

//================================================================================
    private static final int TIMEOUT_USEC = 10000;	// 10msec

    /*
     * STATE_CLOSED => [prepare] => STATE_PREPARED [start]
     * 	=> STATE_PLAYING => [seek] => STATE_PLAYING
     * 		=> [pause] => STATE_PAUSED => [resume] => STATE_PLAYING
     * 		=> [stop] => STATE_CLOSED
     */
    private static final int STATE_STOP = 0;
    private static final int STATE_PREPARED = 1;
    private static final int STATE_PLAYING = 2;
    private static final int STATE_PAUSED = 3;

    // request code
    private static final int REQ_NON = 0;
    private static final int REQ_PREPARE = 1;
    private static final int REQ_START = 2;
    private static final int REQ_SEEK = 3;
    private static final int REQ_STOP = 4;
    private static final int REQ_PAUSE = 5;
    private static final int REQ_RESUME = 6;
    private static final int REQ_QUIT = 9;

//	private static final long EPS = (long)(1 / 240.0f * 1000000);	// 1/240 seconds[マイクロ秒]

	protected MediaMetadataRetriever mMetadata;
	private final Object mSync = new Object();
	private volatile boolean mIsRunning;
	private int mState;
	private String mSourcePath;
	private long mDuration;
	private int mRequest;
	private long mRequestTime;
    // for video playback
	private final Object mVideoSync = new Object();
	private final Surface mOutputSurface;
	protected MediaExtractor mVideoMediaExtractor;
	private MediaCodec mVideoMediaCodec;
	private MediaCodec.BufferInfo mVideoBufferInfo;
	private ByteBuffer[] mVideoInputBuffers;
	private ByteBuffer[] mVideoOutputBuffers;
	private long mVideoStartTime;
	private long previousVideoPresentationTimeUs = -1;
	private volatile int mVideoTrackIndex;
	private boolean mVideoInputDone;
	private boolean mVideoOutputDone;
	private int mVideoWidth, mVideoHeight;
	private int mBitrate;
	private float mFrameRate;
	private int mRotation;

//--------------------------------------------------------------------------------
	/**
	 * playback control task
	 */
	private final Runnable mMoviePlayerTask = new Runnable() {
		@Override
		public final void run() {
			boolean local_isRunning = false;
			int local_req;
			try {
		    	synchronized (mSync) {
					local_isRunning = mIsRunning = true;
					mState = STATE_STOP;
					mRequest = REQ_NON;
					mRequestTime = -1;
		    		mSync.notifyAll();
		    	}
				for ( ; local_isRunning ; ) {
					try {
						synchronized (mSync) {
							local_isRunning = mIsRunning;
							local_req = mRequest;
							mRequest = REQ_NON;
						}
						switch (mState) {
						case STATE_STOP:
							local_isRunning = processStop(local_req);
							break;
						case STATE_PREPARED:
							local_isRunning = processPrepared(local_req);
							break;
						case STATE_PLAYING:
							local_isRunning = processPlaying(local_req);
							break;
						case STATE_PAUSED:
							local_isRunning = processPaused(local_req);
							break;
						}
					} catch (final InterruptedException e) {
						break;
					} catch (final Exception e) {
						Log.e(TAG, "MoviePlayerTask:", e);
						break;
					}
				} // for (;local_isRunning;)
			} finally {
				if (DEBUG) Log.v(TAG, "player task finished:local_isRunning=" + local_isRunning);
				handleStop();
			}
		}
	};

//--------------------------------------------------------------------------------
	/**
	 * video playback task
	 */
	private final Runnable mVideoTask = new Runnable() {
		@Override
		public void run() {
			if (DEBUG) Log.v(TAG, "VideoTask:start");
			for (; mIsRunning && !mVideoInputDone && !mVideoOutputDone ;) {
				try {
			        if (!mVideoInputDone) {
			        	handleInputVideo();
			        }
			        if (!mVideoOutputDone) {
						handleOutputVideo(mCallback);
			        }
				} catch (final Exception e) {
					Log.e(TAG, "VideoTask:", e);
					break;
				}
			} // end of for
			if (DEBUG) Log.v(TAG, "VideoTask:finished");
			synchronized (mSync) {
				mVideoInputDone = mVideoOutputDone = true;
				mSync.notifyAll();
			}
		}
	};

//--------------------------------------------------------------------------------

	/**
	 * @param req
	 * @return
	 * @throws InterruptedException
	 * @throws IOException
	 */
	private final boolean processStop(final int req) throws InterruptedException, IOException {
		boolean local_isRunning = true;
		switch (req) {
		case REQ_PREPARE:
			handlePrepare(mSourcePath);
			break;
		case REQ_START:
		case REQ_PAUSE:
		case REQ_RESUME:
			throw new IllegalStateException("invalid state:" + mState);
		case REQ_QUIT:
			local_isRunning = false;
			break;
//		case REQ_SEEK:
//		case REQ_STOP:
		default:
			synchronized (mSync) {
				mSync.wait();
			}
			break;
		}
		synchronized (mSync) {
			local_isRunning &= mIsRunning;
		}
		return local_isRunning;
	}

	/**
	 * @param req
	 * @return
	 * @throws InterruptedException
	 */
	private final boolean processPrepared(final int req) throws InterruptedException {
		boolean local_isRunning = true;
		switch (req) {
		case REQ_START:
			handleStart();
			break;
		case REQ_PAUSE:
		case REQ_RESUME:
			throw new IllegalStateException("invalid state:" + mState);
		case REQ_STOP:
			handleStop();
			break;
		case REQ_QUIT:
			local_isRunning = false;
			break;
//		case REQ_PREPARE:
//		case REQ_SEEK:
		default:
			synchronized (mSync) {
				mSync.wait();
			}
			break;
		} // end of switch (req)
		synchronized (mSync) {
			local_isRunning &= mIsRunning;
		}
		return local_isRunning;
	}

	/**
	 * @param req
	 * @return
	 */
	private final boolean processPlaying(final int req) {
		boolean local_isRunning = true;
		switch (req) {
		case REQ_PREPARE:
		case REQ_START:
		case REQ_RESUME:
			throw new IllegalStateException("invalid state:" + mState);
		case REQ_SEEK:
			handleSeek(mRequestTime);
			break;
		case REQ_STOP:
			handleStop();
			break;
		case REQ_PAUSE:
			handlePause();
			break;
		case REQ_QUIT:
			local_isRunning = false;
			break;
		default:
			handleLoop(mCallback);
			break;
		} // end of switch (req)
		synchronized (mSync) {
			local_isRunning &= mIsRunning;
		}
		return local_isRunning;
	}

	/**
	 * @param req
	 * @return
	 * @throws InterruptedException
	 */
	private final boolean processPaused(final int req) throws InterruptedException {
		boolean local_isRunning = true;
		switch (req) {
		case REQ_PREPARE:
		case REQ_START:
			throw new IllegalStateException("invalid state:" + mState);
		case REQ_SEEK:
			handleSeek(mRequestTime);
			break;
		case REQ_STOP:
			handleStop();
			break;
		case REQ_RESUME:
			handleResume();
			break;
		case REQ_QUIT:
			local_isRunning = false;
			break;
//		case REQ_PAUSE:
		default:
			synchronized (mSync) {
				mSync.wait();
			}
			break;
		} // end of switch (req)
		synchronized (mSync) {
			local_isRunning &= mIsRunning;
		}
		return local_isRunning;
	}

//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
	/**
	 * @param sourceFile
	 * @throws IOException
	 */
	private final void handlePrepare(final String sourceFile) throws IOException {
		if (DEBUG) Log.v(TAG, "handlePrepare:");
        synchronized (mSync) {
			if (mState != STATE_STOP) {
				throw new RuntimeException("invalid state:" + mState);
			}
		}
        final File src = new File(sourceFile);
        if (TextUtils.isEmpty(sourceFile) || !src.canRead()) {
            throw new FileNotFoundException("Unable to read " + sourceFile);
        }
        mVideoTrackIndex = -1;
		mMetadata = new MediaMetadataRetriever();
		mMetadata.setDataSource(sourceFile);
		updateMovieInfo();
		// preparation for video playback
		mVideoTrackIndex = internalPrepareVideo(sourceFile);
		if (mVideoTrackIndex < 0) {
			throw new RuntimeException("No video track found in " + sourceFile);
		}
		synchronized (mSync) {
			mState = STATE_PREPARED;
		}
		mCallback.onPrepared();
	}

	/**
	 * @param sourceFile
	 * @return first video track index, -1 if not found
	 */
	protected int internalPrepareVideo(final String sourceFile) {
		int trackIndex = -1;
		mVideoMediaExtractor = new MediaExtractor();
		try {
			mVideoMediaExtractor.setDataSource(sourceFile);
			trackIndex = selectTrack(mVideoMediaExtractor, "video/");
			if (trackIndex >= 0) {
				mVideoMediaExtractor.selectTrack(trackIndex);
		        final MediaFormat format = mVideoMediaExtractor.getTrackFormat(trackIndex);
	        	mVideoWidth = format.getInteger(MediaFormat.KEY_WIDTH);
	        	mVideoHeight = format.getInteger(MediaFormat.KEY_HEIGHT);
	        	mDuration = format.getLong(MediaFormat.KEY_DURATION);

				if (DEBUG) Log.v(TAG, String.format("format:size(%d,%d),duration=%d,bps=%d,framerate=%f,rotation=%d",
					mVideoWidth, mVideoHeight, mDuration, mBitrate, mFrameRate, mRotation));
			}
		} catch (final IOException e) {
			Log.w(TAG, e);
		}
		return trackIndex;
	}

	protected void updateMovieInfo() {
		mVideoWidth = mVideoHeight = mRotation = mBitrate = 0;
		mDuration = 0;
		mFrameRate = 0;
		String value = mMetadata.extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_WIDTH);
		if (!TextUtils.isEmpty(value)) {
			mVideoWidth = Integer.parseInt(value);
		}
		value = mMetadata.extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_HEIGHT);
		if (!TextUtils.isEmpty(value)) {
			mVideoHeight = Integer.parseInt(value);
		}
		value = mMetadata.extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_ROTATION);
		if (!TextUtils.isEmpty(value)) {
			mRotation = Integer.parseInt(value);
		}
		value = mMetadata.extractMetadata(MediaMetadataRetriever.METADATA_KEY_BITRATE);
		if (!TextUtils.isEmpty(value)) {
			mBitrate = Integer.parseInt(value);
		}
		value = mMetadata.extractMetadata(MediaMetadataRetriever.METADATA_KEY_DURATION);
		if (!TextUtils.isEmpty(value)) {
			mDuration = Long.parseLong(value) * 1000;
		}
	}

	private final void handleStart() {
    	if (DEBUG) Log.v(TAG, "handleStart:");
		synchronized (mSync) {
			if (mState != STATE_PREPARED)
				throw new RuntimeException("invalid state:" + mState);
			mState = STATE_PLAYING;
		}
        if (mRequestTime > 0) {
        	handleSeek(mRequestTime);
        }
        previousVideoPresentationTimeUs = -1;
		mVideoInputDone = mVideoOutputDone = true;
		Thread videoThread = null;
		if (mVideoTrackIndex >= 0) {
			final MediaCodec codec = internalStartVideo(mVideoMediaExtractor, mVideoTrackIndex);
			if (codec != null) {
				mVideoMediaCodec = codec;
		        mVideoBufferInfo = new MediaCodec.BufferInfo();
		        mVideoInputBuffers = codec.getInputBuffers();
		        mVideoOutputBuffers = codec.getOutputBuffers();
			}
			mVideoInputDone = mVideoOutputDone = false;
			videoThread = new Thread(mVideoTask, "VideoTask");
		}
		if (videoThread != null) videoThread.start();
	}

	/**
	 * @param media_extractor
	 * @param trackIndex
	 * @return
	 */
	protected MediaCodec internalStartVideo(final MediaExtractor media_extractor, final int trackIndex) {
		if (DEBUG) Log.v(TAG, "internalStartVideo:");
		MediaCodec codec = null;
		if (trackIndex >= 0) {
	        final MediaFormat format = media_extractor.getTrackFormat(trackIndex);
	        final String mime = format.getString(MediaFormat.KEY_MIME);
			try {
				codec = MediaCodec.createDecoderByType(mime);
				codec.configure(format, mOutputSurface, null, 0);
		        codec.start();
			} catch (final IOException e) {
				codec = null;
				Log.w(TAG, e);
			}
	    	if (DEBUG) Log.v(TAG, "internalStartVideo:codec started");
		}
		return codec;
	}

	private final void handleSeek(final long newTime) {
        if (DEBUG) Log.d(TAG, "handleSeek");
		if (newTime < 0) return;

		if (mVideoTrackIndex >= 0) {
			mVideoMediaExtractor.seekTo(newTime, MediaExtractor.SEEK_TO_CLOSEST_SYNC);
	        mVideoMediaExtractor.advance();
		}
        mRequestTime = -1;
	}

	private final void handleLoop(final IFrameCallback frameCallback) {
//		if (DEBUG) Log.d(TAG, "handleLoop");

		synchronized (mSync) {
			try {
				mSync.wait();
			} catch (final InterruptedException e) {
				// ignore
			}
		}
        if (mVideoInputDone && mVideoOutputDone) {
            if (DEBUG) Log.d(TAG, "Reached EOS, looping check");
        	handleStop();
        }
	}

	/**
	 * @param codec
	 * @param extractor
	 * @param inputBuffers
	 * @param presentationTimeUs
	 * @param isAudio
	 */
	protected boolean internalProcessInput(final MediaCodec codec,
		final MediaExtractor extractor,
		final ByteBuffer[] inputBuffers,
		final long presentationTimeUs, final boolean isAudio) {

//		if (DEBUG) Log.v(TAG, "internalProcessInput:presentationTimeUs=" + presentationTimeUs);
		boolean result = true;
		while (mIsRunning) {
            final int inputBufIndex = codec.dequeueInputBuffer(TIMEOUT_USEC);
            if (inputBufIndex == MediaCodec.INFO_TRY_AGAIN_LATER)
            	break;
            if (inputBufIndex >= 0) {
                final int size = extractor.readSampleData(inputBuffers[inputBufIndex], 0);
                if (size > 0) {
                	codec.queueInputBuffer(inputBufIndex, 0, size, presentationTimeUs, 0);
                }
            	result = extractor.advance();	// return false if no data is available
                break;
            }
		}
		return result;
	}

	private final void handleInputVideo() {
    	long presentationTimeUs = mVideoMediaExtractor.getSampleTime();
		if (presentationTimeUs < previousVideoPresentationTimeUs) {
    		presentationTimeUs += previousVideoPresentationTimeUs - presentationTimeUs; //  + EPS;
    	}
    	previousVideoPresentationTimeUs = presentationTimeUs;
        final boolean b = internalProcessInput(mVideoMediaCodec, mVideoMediaExtractor, mVideoInputBuffers,
        		presentationTimeUs, false);
        if (!b) {
        	if (DEBUG) Log.i(TAG, "video track input reached EOS");
    		while (mIsRunning) {
                final int inputBufIndex = mVideoMediaCodec.dequeueInputBuffer(TIMEOUT_USEC);
                if (inputBufIndex >= 0) {
                	mVideoMediaCodec.queueInputBuffer(inputBufIndex, 0, 0, 0L,
                		MediaCodec.BUFFER_FLAG_END_OF_STREAM);
                	if (DEBUG) Log.v(TAG, "sent input EOS:" + mVideoMediaCodec);
                	break;
                }
        	}
    		synchronized (mSync) {
    			mVideoInputDone = true;
    			mSync.notifyAll();
    		}
        }
	}

	/**
	 * @param frameCallback
	 */
	private final void handleOutputVideo(final IFrameCallback frameCallback) {
//    	if (DEBUG) Log.v(TAG, "handleDrainVideo:");
		while (mIsRunning && !mVideoOutputDone) {
			final int decoderStatus = mVideoMediaCodec.dequeueOutputBuffer(mVideoBufferInfo, TIMEOUT_USEC);
			if (decoderStatus == MediaCodec.INFO_TRY_AGAIN_LATER) {
				return;
			} else if (decoderStatus == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED) {
				mVideoOutputBuffers = mVideoMediaCodec.getOutputBuffers();
				if (DEBUG) Log.d(TAG, "INFO_OUTPUT_BUFFERS_CHANGED:");
			} else if (decoderStatus == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
				final MediaFormat newFormat = mVideoMediaCodec.getOutputFormat();
				if (DEBUG) Log.d(TAG, "video decoder output format changed: " + newFormat);
			} else if (decoderStatus < 0) {
				throw new RuntimeException(
					"unexpected result from video decoder.dequeueOutputBuffer: " + decoderStatus);
			} else { // decoderStatus >= 0
				boolean doRender = false;
				if (mVideoBufferInfo.size > 0) {
					doRender = (mVideoBufferInfo.size != 0)
						&& !internalWriteVideo(mVideoOutputBuffers[decoderStatus],
							0, mVideoBufferInfo.size, mVideoBufferInfo.presentationTimeUs);
					if (doRender) {
						if (!frameCallback.onFrameAvailable(mVideoBufferInfo.presentationTimeUs))
							mVideoStartTime = adjustPresentationTime(mVideoStartTime, mVideoBufferInfo.presentationTimeUs);
					}
				}
				mVideoMediaCodec.releaseOutputBuffer(decoderStatus, doRender);
				if ((mVideoBufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
					if (DEBUG) Log.d(TAG, "video:output EOS");
					synchronized (mSync) {
						mVideoOutputDone = true;
						mSync.notifyAll();
					}
				}
			}
		}
	}

	/**
	 * @param buffer
	 * @param offset
	 * @param size
	 * @param presentationTimeUs
	 * @return if return false, automatically asjust frame rate
	 */
	protected boolean internalWriteVideo(final ByteBuffer buffer,
		final int offset, final int size, final long presentationTimeUs) {

//		if (DEBUG) Log.v(TAG, "internalWriteVideo");
		return false;
	}

	/**
	 * adjusting frame rate
	 * @param startTime
	 * @param presentationTimeUs
	 * @return startTime
	 */
	protected long adjustPresentationTime(
		final long startTime, final long presentationTimeUs) {

		if (startTime > 0) {
			for (long t = presentationTimeUs - (System.nanoTime() / 1000 - startTime);
					t > 0; t = presentationTimeUs - (System.nanoTime() / 1000 - startTime)) {
				synchronized (mVideoSync) {
					try {
						mVideoSync.wait(t / 1000, (int)((t % 1000) * 1000));
					} catch (final InterruptedException e) {
						// ignore
					}
					if ((mState == REQ_STOP) || (mState == REQ_QUIT))
						break;
				}
			}
			return startTime;
		} else {
			return System.nanoTime() / 1000;
		}
	}

	private final void handleStop() {
    	if (DEBUG) Log.v(TAG, "handleStop:");
    	synchronized (mVideoTask) {
    		internalStopVideo();
    		mVideoTrackIndex = -1;
    	}
    	if (mVideoMediaCodec != null) {
    		mVideoMediaCodec.stop();
    		mVideoMediaCodec.release();
    		mVideoMediaCodec = null;
    	}
		if (mVideoMediaExtractor != null) {
			mVideoMediaExtractor.release();
			mVideoMediaExtractor = null;
		}
        mVideoBufferInfo = null;
        mVideoInputBuffers = mVideoOutputBuffers = null;
		if (mMetadata != null) {
			mMetadata.release();
			mMetadata = null;
		}
		synchronized (mSync) {
			mVideoOutputDone = mVideoInputDone = true;
			mState = STATE_STOP;
		}
		mCallback.onFinished();
	}

	protected void internalStopVideo() {
		if (DEBUG) Log.v(TAG, "internalStopVideo:");
	}

	private final void handlePause() {
    	if (DEBUG) Log.v(TAG, "handlePause:");
    	// FIXME unimplemented yet
	}

	private final void handleResume() {
    	if (DEBUG) Log.v(TAG, "handleResume:");
    	// FIXME unimplemented yet
	}

    /**
     * search first track index matched specific MIME
     * @param extractor
     * @param mimeType "video/" or "audio/"
     * @return track index, -1 if not found
     */
    protected static final int selectTrack(final MediaExtractor extractor, final String mimeType) {
        final int numTracks = extractor.getTrackCount();
        MediaFormat format;
        String mime;
        for (int i = 0; i < numTracks; i++) {
            format = extractor.getTrackFormat(i);
            mime = format.getString(MediaFormat.KEY_MIME);
            if (mime.startsWith(mimeType)) {
                if (DEBUG) {
                    Log.d(TAG_STATIC, "Extractor selected track " + i + " (" + mime + "): " + format);
                }
                return i;
            }
        }
        return -1;
    }
}
