package com.serenegiant.audiovideoplayersample;

import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Point;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class Wrnch {
    static {
        System.loadLibrary("native-lib");
    }

    static native void initWrnchJNI(String dir);
    static native float[] processWrnchJNI(byte[] pic, int cols, int rows);

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

    static public Point[] process(byte[] img, int cols, int rows, int origWidth, int origHeight) {
        final float[] joints = processWrnchJNI(img, cols, rows);
        Point[] result = new Point[joints.length / 2];
        for (int i = 0; i < joints.length / 2; i++) {
            result[i] = new Point((int) (joints[i] * origWidth), (int) (joints[i + 1] * origHeight));
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