#include <jni.h>
#include <iostream>
#include <android/log.h>

extern "C" JNIEXPORT void JNICALL
Java_com_serenegiant_audiovideoplayersample_MainActivity_initWrnchJNI(
        JNIEnv* env,
        jobject /* this */) {

    __android_log_write(ANDROID_LOG_INFO, "WRNCH", "Initialize");
    std::cout << "Hello World!" << std::endl;
}
