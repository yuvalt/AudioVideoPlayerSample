#include <jni.h>
#include <android/log.h>
#include <wrnch/engine.hpp>
#include <string>
#include <vector>

static wrPoseEstimatorHandle pose_estimator;
static wrPoseEstimatorOptionsHandle pose_options;
//std::vector< std::string > joint_names_{};
//std::vector< std::pair< int, int > > bone_pairs_{};

extern "C" JNIEXPORT jintArray JNICALL
Java_com_samsungnext_audiovideoplayersample_Wrnch_initWrnchJNI(
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
        return env->NewIntArray(0);
    }

    auto pose_params = wrPoseParams_Create();
    wrPoseParams_SetBoneSensitivity(pose_params, wrSensitivity::wrSensitivity_HIGH);
    wrPoseParams_SetJointSensitivity(pose_params, wrSensitivity::wrSensitivity_HIGH);
    wrPoseParams_SetEnableTracking(pose_params, 1);
//    wrPoseParams_SetPreferredNetWidth2d(pose_params, 324);
//    wrPoseParams_SetPreferredNetHeight2d(pose_params, 184);

    auto params = wrPoseEstimatorConfigParams_Create(dir);
    wrPoseEstimatorConfigParams_SetLicenseString(params, "3A83A2-46FB01-48CB9F-EE06BF-3698DE-E05B71");
    wrPoseEstimatorConfigParams_SetDeviceFingerprint(params, "smartfitness603DK");
    wrPoseEstimatorConfigParams_SetOutputFormat(params, wrJointDefinition_Get("j23"));
    wrPoseEstimatorConfigParams_SetPoseParams(params, pose_params);

    auto wrc = wrPoseEstimator_CreateFromConfig(&pose_estimator, params);
    if (wrc != wrReturnCode_OK) {
        __android_log_print(ANDROID_LOG_ERROR, "WRNCH", "wrPoseEstimator_CreateFromConfig: %s", wrReturnCode_Translate(wrc));
        return env->NewIntArray(0);
    }

    wrc = wrPoseEstimator_ReinitializeFromConfig(&pose_estimator, params);
    if (wrc != wrReturnCode_OK) {
        __android_log_print(ANDROID_LOG_ERROR, "WRNCH", "wrPoseEstimator_ReinitializeFromConfig: %s", wrReturnCode_Translate(wrc));
        return env->NewIntArray(0);
    }

    pose_options = wrPoseEstimatorOptions_Create();
    wrPoseEstimatorOptions_SetEnableJointSmoothing(pose_options, 1);
    wrPoseEstimatorOptions_SetEstimatePoseFace(pose_options, 1);
    wrPoseEstimatorOptions_SetRotationMultipleOf90(pose_options, 0);

    wrJointDefinitionHandleConst format = wrPoseEstimator_GetHuman2DOutputFormat(pose_estimator);

    auto num_joints = wrJointDefinition_GetNumJoints(format);
    auto num_bones = wrJointDefinition_GetNumBones(format);
    if (num_joints != 23) {
        __android_log_print(ANDROID_LOG_ERROR, "WRNCH", "Num joints expected 23. Recieved: %d", num_joints);
        return env->NewIntArray(0);
    }

//    char const **c_joint_names = new char const *[23];
//    wrJointDefinition_GetJointNames(format, c_joint_names);
//    for(int i = 0; i < num_joints; i++) {
//        joint_names_.push_back(std::string(c_joint_names[i]));
//    }

    uint *c_bone_pairs = new uint[num_bones * 2];
    wrJointDefinition_GetBonePairs(format, c_bone_pairs);
//    for(int i = 0; i < num_bones; i++) {
//        bone_pairs_.emplace_back(c_bone_pairs[i*2+0], c_bone_pairs[i*2+1]);
//    }

    auto result = env->NewIntArray(num_bones * 2);
    env->SetIntArrayRegion(result, 0, num_bones * 2, (jint*) c_bone_pairs);

    __android_log_print(ANDROID_LOG_INFO, "WRNCH", "WRNCH Init Done");

    return result;
}

extern "C" JNIEXPORT jfloatArray JNICALL
Java_com_samsungnext_audiovideoplayersample_Wrnch_processWrnchJNI(
        JNIEnv* env,
        jobject /* this */,
        jbyteArray img,
        jint cols,
        jint rows) {

    jboolean isCopy;
    jbyte* b = env->GetByteArrayElements(img, &isCopy);

    auto rc = wrPoseEstimator_ProcessFrame(pose_estimator, (unsigned char*) b, cols, rows, pose_options);
    if (rc != wrReturnCode_OK) {
        __android_log_print(ANDROID_LOG_ERROR, "WRNCH", "wrPoseEstimator_ProcessFrame: %s", wrReturnCode_Translate(rc));
        return env->NewFloatArray(0);
    }

    auto it = wrPoseEstimator_GetHumans2DBegin(pose_estimator);

    for (int i = 0; i < wrPoseEstimator_GetNumHumans2D(pose_estimator); i++)
    {
        auto pose_score = wrPose2d_GetScore(it);
        auto num_joints = wrPose2d_GetNumJoints(it);
        auto joints = wrPose2d_GetJoints(it);
//        auto scores = wrPose2d_GetScores(it);

        auto is_main = wrPose2d_GetIsMain(it);
        __android_log_print(ANDROID_LOG_INFO, "WRNCH", "POSE SCORE: %.2f %d %d", pose_score, is_main, num_joints);

        if (is_main == 1) {
            auto result = env->NewFloatArray(num_joints * 2);
            env->SetFloatArrayRegion(result, 0, num_joints * 2, joints);
            env->ReleaseByteArrayElements(img, b, 0);
            return result;
        }

//        printf("Pose [%i / %i] : is main: %i.\n", i, wrPoseEstimator_GetNumHumans2D(pose_estimator), is_main);
//        types::WrenchPose pose(sensor_data->point_cloud, pose_score, num_joints, joints, scores, joint_names);
//        printf("Pose [%i / %i]: [%f | %s]\n", i, wrPoseEstimator_GetNumHumans2D(pose_estimator), pose_score, (pose.is_tracked() ? "Tracked!" : "Not tracked.."));
//        poses.push_back(std::make_shared< types::PoseBase >(pose));

        it = wrPoseEstimator_GetPose2DNext(it);
    }

    env->ReleaseByteArrayElements(img, b, 0);

    auto result = env->NewFloatArray(0);
    env->ReleaseByteArrayElements(img, b, 0);

    return result;
}
