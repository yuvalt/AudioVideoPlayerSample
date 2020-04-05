package com.samsungnext.audiovideoplayersample;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.IOException;

import com.samsungnext.media.MediaMoviePlayer;
import com.samsungnext.media.IFrameCallback;
import com.samsungnext.widget.OverlayView;
import com.samsungnext.widget.PlayerTextureView;

import android.app.Activity;
import android.content.Context;
import android.os.Handler;
import android.support.v4.app.Fragment;
import android.os.Bundle;
import android.util.Log;
import android.util.Pair;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.ImageButton;

@SuppressWarnings("unused")
public class PlayerFragment extends Fragment {
	private static final boolean DEBUG = true;	// TODO set false on release
	private static final String TAG = "PlayerFragment";
	
	/**
	 * for camera preview display
	 */
	private PlayerTextureView mPlayerView;	//	private PlayerGLView mPlayerView;
	/**
	 * button for start/stop recording
	 */
	private ImageButton mPlayerButton;

	private MediaMoviePlayer mPlayer;

	public PlayerFragment() {
		// need default constructor
		setRetainInstance(true);
	}

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
		final View rootView = inflater.inflate(R.layout.fragment_main, container, false);

		final OverlayView overlayView = (OverlayView) rootView.findViewById(R.id.overlay_view);

		try {
			final Pair[] bones = Wrnch.init(getContext());
			overlayView.setBones(bones);
		}
		catch (IOException e) {
			Log.v("WRNCH", "WRNCH Init failed: " + e.toString());
			return rootView;
		}

		mPlayerView = (PlayerTextureView)rootView.findViewById(R.id.player_view);
		mPlayerView.setAspectRatio(16 / 9.f);
		mPlayerView.setOverlayView(overlayView);

		mPlayerButton = (ImageButton)rootView.findViewById(R.id.play_button);
		mPlayerButton.setOnClickListener(mOnClickListener);
		mPlayerButton.setVisibility(View.INVISIBLE);
		return rootView;
	}

	@Override
	public void onStart() {
		super.onStart();

		Handler handler = new Handler();
		int delay = 2000; //milliseconds

		handler.postDelayed(new Runnable(){
			public void run() {
				startPlay();
			}
		}, delay);
	}

	@Override
	public void onResume() {
		super.onResume();
		if (DEBUG) Log.v(TAG, "onResume:");
		mPlayerView.onResume();
	}

	@Override
	public void onPause() {
		if (DEBUG) Log.v(TAG, "onPause:");
		stopPlay();
		mPlayerView.onPause();
		super.onPause();
	}

	/**
	 * method when touch record button
	 */
	private final OnClickListener mOnClickListener = new OnClickListener() {
		@Override
		public void onClick(View view) {
			switch (view.getId()) {
			case R.id.play_button:
				if (mPlayer == null)
					startPlay();
				else
					stopPlay();
				break;
			}
		}
	};

	/**
	 * start playing
	 */
	public void startPlay() {
		if (DEBUG) Log.v(TAG, "startPlay:");

		// HINT: CHANGE VIDEO INPUT
//		final Activity activity = getActivity();
//		final File dir = activity.getFilesDir();
//		dir.mkdirs();
//			final File path = new File(dir,"leftlunges_oriana.mp4");
//			final File path = new File(dir,"jumpingjacks_tsella01.mp4");
//			final File path = new File(dir,"yuvalgreenfield.mp4");

		final File path = new File("/data/local/tmp/video.mp4");

		mPlayerButton.setColorFilter(0x7fff0000);	// turn red
		mPlayer = new MediaMoviePlayer(mPlayerView.getSurface(), mIFrameCallback, false);
		mPlayer.prepare(path.toString());
	}

	/**
	 * request stop playing
	 */
	private void stopPlay() {
		if (DEBUG) Log.v(TAG, "stopRecording:mPlayer=" + mPlayer);
		mPlayerButton.setColorFilter(0);	// return to default color
		if (mPlayer != null) {
			mPlayer.release();
			mPlayer = null;
			// you should not wait here
		}
	}

	/**
	 * callback methods from decoder
	 */
	private final IFrameCallback mIFrameCallback = new IFrameCallback() {
		@Override
		public void onPrepared() {
			final float aspect = mPlayer.getWidth() / (float)mPlayer.getHeight();
			final Activity activity = getActivity();
			if ((activity != null) && !activity.isFinishing())
				activity.runOnUiThread(new Runnable() {
					@Override
					public void run() {
						mPlayerView.setAspectRatio(aspect);
					}
				});
			mPlayer.play();
		}

		@Override
		public void onFinished() {
			mPlayer = null;
			final Activity activity = getActivity();
			if ((activity != null) && !activity.isFinishing())
				activity.runOnUiThread(new Runnable() {
					@Override
					public void run() {
						mPlayerButton.setColorFilter(0);	// return to default color
					}
				});
		}

		@Override
		public boolean onFrameAvailable(long presentationTimeUs) {
			return false;
		}
	};

	private final void prepareSampleMovie(File path) throws IOException {
		final Activity activity = getActivity();
		if (!path.exists()) {
			if (DEBUG) Log.i(TAG, "copy sample movie file from res/raw to app private storage");
			final BufferedInputStream in = new BufferedInputStream(activity.getResources().openRawResource(R.raw.easter_egg_nexus9_small));
			final BufferedOutputStream out = new BufferedOutputStream(activity.openFileOutput(path.getName(), Context.MODE_PRIVATE));
			byte[] buf = new byte[8192];
			int size = in.read(buf);
			while (size > 0) {
				out.write(buf, 0, size);
				size = in.read(buf);
			}
			in.close();
			out.flush();
			out.close();
		}
	}
}
