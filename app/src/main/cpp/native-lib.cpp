#include <jni.h>
#include <android/log.h>
#include <wrnch/engine.hpp>
#include <string>
#include <vector>

static wrPoseEstimatorHandle pose_estimator;
static wrPoseEstimatorOptionsHandle pose_options;
static constexpr const char* licenseKey = "3A83A2-46FB01-48CB9F-EE06BF-3698DE-E05B71";
std::vector< std::string > joint_names_{};
std::vector< std::pair< int, int > > bone_pairs_{};

extern "C" JNIEXPORT void JNICALL
Java_com_serenegiant_audiovideoplayersample_MainActivity_initWrnchJNI(
        JNIEnv* env,
        jobject /* this */) {

    __android_log_print(ANDROID_LOG_INFO, "WRNCH", "WRNCH version: %s", wrnch_version());

    auto pose_params = wrPoseParams_Create();
    wrPoseParams_SetBoneSensitivity(pose_params, wrSensitivity::wrSensitivity_HIGH);
    wrPoseParams_SetJointSensitivity(pose_params, wrSensitivity::wrSensitivity_HIGH);
    wrPoseParams_SetEnableTracking(pose_params, 1);
    wrPoseParams_SetPreferredNetWidth2d(pose_params, 324);
    wrPoseParams_SetPreferredNetHeight2d(pose_params, 184);

    const char* modelsDirectory = "";
    auto params = wrPoseEstimatorConfigParams_Create(modelsDirectory);
    wrPoseEstimatorConfigParams_SetLicenseString(params, licenseKey);
    wrPoseEstimatorConfigParams_SetDeviceFingerprint(params, "smartfitness603DK");
    wrPoseEstimatorConfigParams_SetOutputFormat(params, wrJointDefinition_Get("j23"));
    wrPoseEstimatorConfigParams_SetPoseParams(params, pose_params);

    __android_log_print(ANDROID_LOG_INFO, "WRNCH", "Checking license");
    wrReturnCode rc = wrPoseEstimator_CreateFromConfig(&pose_estimator, params);
    __android_log_print(ANDROID_LOG_INFO, "WRNCH", "Create result: [%s]\n", wrReturnCode_Translate(rc));

    rc = wrPoseEstimator_ReinitializeFromConfig(&pose_estimator, params);
    __android_log_print(ANDROID_LOG_INFO, "WRNCH", "Reinitialization result: [%s]\n", wrReturnCode_Translate(rc));

    int isMaskSupported = wrPoseEstimator_SupportsMaskEstimation(pose_estimator);
    __android_log_print(ANDROID_LOG_INFO, "WRNCH", "Is mask supported: %d", isMaskSupported);

    pose_options = wrPoseEstimatorOptions_Create();
    wrPoseEstimatorOptions_SetEnableJointSmoothing(pose_options, 1);
    wrPoseEstimatorOptions_SetEstimatePoseFace(pose_options, 1);
#if ENABLE_MASK
    wrPoseEstimatorOptions_SetEstimateMask(pose_options, 1);
#endif

    wrJointDefinitionHandleConst format = wrPoseEstimator_GetHuman2DOutputFormat(pose_estimator);

    auto num_joints = wrJointDefinition_GetNumJoints(format);
    auto num_bones = wrJointDefinition_GetNumBones(format);
    //SF_ASSERT(num_joints == 23, "Num joints expected 23. Recieved: " + std::to_string(num_joints));

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
}
