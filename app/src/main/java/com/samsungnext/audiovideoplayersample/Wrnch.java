package com.samsungnext.audiovideoplayersample;

import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Point;
import android.util.Log;
import android.util.Pair;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class Wrnch {
    private static final boolean DEBUG = false;

    static {
        System.loadLibrary("native-lib");
    }

    static native int[] initWrnchJNI(String dir);
    static native float[] processWrnchJNI(byte[] pic, int cols, int rows);

    static public Pair<Integer,Integer>[] init(Context context) throws IOException {
        final File files = context.getFilesDir();
        files.mkdir();

        final AssetManager am = context.getAssets();
        copyFile(context, am.open("wrsnpe_android_pose2d.enc"), new File(files, "wrsnpe_android_pose2d.enc"));
        copyFile(context, am.open("libSNPE.so"),  new File(files, "libSNPE.so"));
        copyFile(context, am.open("libsnpe_adsp.so"),  new File(files, "libsnpe_adsp.so"));
        copyFile(context, am.open("libsnpe_dsp_domains_system.so"),  new File(files, "libsnpe_dsp_domains_system.so"));
        copyFile(context, am.open("libsnpe_dsp_domains_v2.so"),  new File(files, "libsnpe_dsp_domains_v2.so"));
        copyFile(context, am.open("libsnpe_dsp_domains_v2_system.so"),  new File(files, "libsnpe_dsp_domains_v2_system.so"));
        copyFile(context, am.open("libsnpe_dsp_v65_domains_v2_skel.so"),  new File(files, "libsnpe_dsp_v65_domains_v2_skel.so"));
        copyFile(context, am.open("libsnpe_dsp_v66_domains_v2_skel.so"),  new File(files, "libsnpe_dsp_v66_domains_v2_skel.so"));

        int[] bones = initWrnchJNI(files.getAbsolutePath());
        Pair<Integer,Integer>[] result = new Pair[bones.length / 2];

        for (int i = 0; i < result.length; ++i) {
            result[i] = new Pair<>(bones[i*2], bones[i*2+1]);
        }

        return result;
    }

    static public Point[] process(byte[] img, int cols, int rows, int origWidth, int origHeight) {
        final float[] joints = processWrnchJNI(img, cols, rows);

        if (DEBUG) Log.v("WRNCH", "GOT JOINTS: " + Integer.toString(joints.length / 2));

        Point[] result = new Point[joints.length / 2];
        for (int i = 0; i < result.length; ++i) {
            float x = joints[i * 2];
            float y = joints[i * 2 + 1];

            int xx = (int) (x * (float) origWidth);
            int yy = (int) (y * (float) origHeight);

            result[i] = new Point(xx, yy);

            if (DEBUG) Log.v("WRNCH", "Joint: " + Integer.toString(result[i].x) + "," + Integer.toString(result[i].y));
        }

        return result;
    }


    private static void copyFile(Context context, InputStream in, File path) throws IOException {
        OutputStream out = context.openFileOutput(path.getName(), Context.MODE_PRIVATE);

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