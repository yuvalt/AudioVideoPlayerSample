package com.serenegiant.audiovideoplayersample;
/*
 * AudioVideoPlayerSample
 * Sample project to play audio and video from MPEG4 file using MediaCodec.
 *
 * Copyright (c) 2014 saki t_saki@serenegiant.com
 *
 * File name: MainActivity.java
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 * All files in the folder are under this Apache License, Version 2.0.
*/


import android.content.Context;
import android.content.res.AssetManager;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

class Wrnch {
	static {
		System.loadLibrary("native-lib");
	}

	static native void initWrnchJNI(String dir);

	static public void init(Context context) throws IOException {
		final File files = context.getFilesDir();
		files.mkdir();

		final AssetManager am = context.getAssets();
		copyFile(am.open("wrsnpe_android_pose2d.enc"), files, "wrsnpe_android_pose2d.enc");
		copyFile(am.open("libSNPE.so"), files, "libSNPE.so");
		copyFile(am.open("libsnpe_adsp.so"), files, "libsnpe_adsp.so");
		copyFile(am.open("libsnpe_dsp_domains_system.so"), files, "libsnpe_dsp_domains_system.so");
		copyFile(am.open("libsnpe_dsp_domains_v2.so"), files, "libsnpe_dsp_domains_v2.so");
		copyFile(am.open("libsnpe_dsp_domains_v2_system.so"), files, "libsnpe_dsp_domains_v2_system.so");
		copyFile(am.open("libsnpe_dsp_v65_domains_v2_skel.so"), files, "libsnpe_dsp_v65_domains_v2_skel.so");
		copyFile(am.open("libsnpe_dsp_v66_domains_v2_skel.so"), files, "libsnpe_dsp_v66_domains_v2_skel.so");

		initWrnchJNI(files.getAbsolutePath());
	}

	private static void copyFile(InputStream in, File dir, String outputFile) throws IOException {
		OutputStream out;

		out = new FileOutputStream(new File(dir, outputFile));

		byte[] buffer = new byte[8192];
		int read;

		while ((read = in.read(buffer)) != -1) {
			out.write(buffer, 0, read);
		}

		in.close();
		out.flush();
		out.close();
	}
}

public class MainActivity extends AppCompatActivity {
	public PlayerFragment frag;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		if (savedInstanceState == null) {
			Log.v("XX", "onCreate1");
			try {
				Wrnch.init(getApplicationContext());
			}
			catch (IOException e) {
				Log.v("WRNCH", e.getMessage());
			}
			Log.v("XX", "onCreate1");

			frag = new PlayerFragment();

			getSupportFragmentManager().beginTransaction()
					.add(R.id.container, frag).commit();
		}
	}

	@Override
	public void onWindowFocusChanged(boolean hasFocus) {
		super.onWindowFocusChanged(hasFocus);
		if (hasFocus) {
			hideSystemUI();
		}
	}

	private void hideSystemUI() {
		// Enables regular immersive mode.
		// For "lean back" mode, remove SYSTEM_UI_FLAG_IMMERSIVE.
		// Or for "sticky immersive," replace it with SYSTEM_UI_FLAG_IMMERSIVE_STICKY
		View decorView = getWindow().getDecorView();
		decorView.setSystemUiVisibility(
				View.SYSTEM_UI_FLAG_IMMERSIVE
						// Set the content to appear under the system bars so that the
						// content doesn't resize when the system bars hide and show.
						| View.SYSTEM_UI_FLAG_LAYOUT_STABLE
						| View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
						| View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
						// Hide the nav bar and status bar
						| View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
						| View.SYSTEM_UI_FLAG_FULLSCREEN);
	}

//	public native void processWrnchJni(byte[] bgr, int width, int height);
}
