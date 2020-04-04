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
    static {
        System.loadLibrary("native-lib");
    }

    static native int[] initWrnchJNI(String dir);
    static native float[] processWrnchJNI(byte[] pic, int cols, int rows);

    static public Pair<Integer,Integer>[] init(Context context) throws IOException {
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

        int[] bones = initWrnchJNI(files.getAbsolutePath());
        Pair<Integer,Integer>[] result = new Pair[bones.length / 2];

        for (int i = 0; i < result.length; ++i) {
            result[i] = new Pair<>(bones[i*2], bones[i*2+1]);
        }

        return result;
    }

    static public Point[] process(byte[] img, int cols, int rows, int origWidth, int origHeight) {
        final float[] joints = processWrnchJNI(img, cols, rows);

        Log.v("WRNCH", "GOT JOINTS: " + Integer.toString(joints.length / 2));

        Point[] result = new Point[joints.length / 2];
        if (result.length == 0) {
            final int x = 1;
        }
        for (int i = 0; i < result.length; ++i) {
            float x = joints[i * 2];
            float y = joints[i * 2 + 1];
//            Log.v("WRNCH", "Joint1: " + Float.toString(x) + "," + Float.toString(y));
            int xx = (int) (x * (float) origWidth);
            int yy = (int) (y * (float) origHeight);
//            Log.v("WRNCH", "Joint2: " + Integer.toString(xx) + "," + Integer.toString(yy));
            result[i] = new Point(xx, yy);
//            Log.v("WRNCH", "Joint3: " + Integer.toString(result[i].x) + "," + Integer.toString(result[i].y));
        }

        return result;
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