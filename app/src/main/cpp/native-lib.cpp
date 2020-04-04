#include <jni.h>
#include <android/log.h>
#include <wrnch/engine.hpp>
#include <string>
#include <vector>

static wrPoseEstimatorHandle pose_estimator;
static wrPoseEstimatorOptionsHandle pose_options;
std::vector< std::string > joint_names_{};
std::vector< std::pair< int, int > > bone_pairs_{};

extern "C" JNIEXPORT void JNICALL
Java_com_serenegiant_audiovideoplayersample_Wrnch_initWrnchJNI(
        JNIEnv* env,
        jobject /* this */,
        jstring dirStr) {

    const char* dir = env->GetStringUTFChars(dirStr, 0);

    __android_log_print(ANDROID_LOG_INFO, "WRNCH", "WRNCH version: %s", wrnch_version());

    char path[2048];
    snprintf(path, sizeof(path), "%s;/system/lib/rfsa/adsp;/system/vendor/lib/rfsa/adsp;/dsp", dir);

    auto rc = setenv("ADSP_LIBRARY_PATH", path, 1);
    if (rc < 0) {
        __android_log_print(ANDROID_LOG_ERROR, "WRNCH", "Failed to set ADSP_LIBRARY_PATH");
        return;
    }

    auto pose_params = wrPoseParams_Create();
    wrPoseParams_SetBoneSensitivity(pose_params, wrSensitivity::wrSensitivity_HIGH);
    wrPoseParams_SetJointSensitivity(pose_params, wrSensitivity::wrSensitivity_HIGH);
    wrPoseParams_SetEnableTracking(pose_params, 1);
    wrPoseParams_SetPreferredNetWidth2d(pose_params, 324);
    wrPoseParams_SetPreferredNetHeight2d(pose_params, 184);

    auto params = wrPoseEstimatorConfigParams_Create(dir);
    wrPoseEstimatorConfigParams_SetLicenseString(params, "3A83A2-46FB01-48CB9F-EE06BF-3698DE-E05B71");
    wrPoseEstimatorConfigParams_SetDeviceFingerprint(params, "smartfitness603DK");
    wrPoseEstimatorConfigParams_SetOutputFormat(params, wrJointDefinition_Get("j23"));
    wrPoseEstimatorConfigParams_SetPoseParams(params, pose_params);

    auto wrc = wrPoseEstimator_CreateFromConfig(&pose_estimator, params);
    if (wrc != wrReturnCode_OK) {
        __android_log_print(ANDROID_LOG_ERROR, "WRNCH", "wrPoseEstimator_CreateFromConfig: %s", wrReturnCode_Translate(wrc));
        return;
    }

    wrc = wrPoseEstimator_ReinitializeFromConfig(&pose_estimator, params);
    if (wrc != wrReturnCode_OK) {
        __android_log_print(ANDROID_LOG_ERROR, "WRNCH", "wrPoseEstimator_ReinitializeFromConfig: %s", wrReturnCode_Translate(wrc));
        return;
    }

    pose_options = wrPoseEstimatorOptions_Create();
    wrPoseEstimatorOptions_SetEnableJointSmoothing(pose_options, 1);
    wrPoseEstimatorOptions_SetEstimatePoseFace(pose_options, 1);

    wrJointDefinitionHandleConst format = wrPoseEstimator_GetHuman2DOutputFormat(pose_estimator);

    auto num_joints = wrJointDefinition_GetNumJoints(format);
    auto num_bones = wrJointDefinition_GetNumBones(format);
    if (num_joints != 23) {
        __android_log_print(ANDROID_LOG_ERROR, "WRNCH", "Num joints expected 23. Recieved: %d", num_joints);
        return;
    }

    char const **c_joint_names = new char const *[23];
    wrJointDefinition_GetJointNames(format, c_joint_names);
    for(int i = 0; i < num_joints; i++) {
        joint_names_.push_back(std::string(c_joint_names[i]));
    }

    uint *c_bone_pairs = new uint[num_bones * 2];
    wrJointDefinition_GetBonePairs(format, c_bone_pairs);
    for(int i = 0; i < num_bones; i++) {
        bone_pairs_.emplace_back(c_bone_pairs[i*2+0], c_bone_pairs[i*2+1]);
    }

    __android_log_print(ANDROID_LOG_INFO, "WRNCH", "WRNCH Init Done");
}
