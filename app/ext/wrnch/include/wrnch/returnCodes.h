/*
 Copyright (c) 2019 wrnch Inc.
 All rights reserved
*/

#ifndef WRNCH_ENGINE_RETURN_CODES
#define WRNCH_ENGINE_RETURN_CODES

#include <wrnch/definitions.h>

typedef enum
{
    wrReturnCode_OK = 0,
    wrReturnCode_BAD_ALLOC,
    wrReturnCode_PATH_ERROR,
    wrReturnCode_LICENSE_ERROR_ACTIVATION,
    wrReturnCode_LICENSE_ERROR_DEACTIVATION,
    wrReturnCode_LICENSE_EXPIRED,
    wrReturnCode_LICENSE_REVOKED,
    wrReturnCode_LICENSE_SUSPENDED,
    wrReturnCode_LICENSE_ERROR_IO,
    wrReturnCode_LICENSE_ERROR_MALFORMED,
    wrReturnCode_LICENSE_ERROR_INVALID_KEY,
    wrReturnCode_LICENSE_ERROR_INVALID_OFFLINE,
    wrReturnCode_LICENSE_ERROR_INVALID_TYPE,
    wrReturnCode_LICENSE_ERROR_VERSION,
    wrReturnCode_LICENSE_ERROR_HOST,
    wrReturnCode_LICENSE_ERROR_NOT_FOUND,
    wrReturnCode_LICENSE_ERROR_INVALID_ACTIVATION,
    wrReturnCode_LICENSE_ERROR_OS,
    wrReturnCode_LICENSE_ERROR_VM,
    wrReturnCode_LICENSE_ERROR_COUNTRY,
    wrReturnCode_LICENSE_ERROR_IP,
    wrReturnCode_LICENSE_ERROR_SYS_TIME,
    wrReturnCode_LICENSE_ERROR_FINGERPRINT_CHANGED,
    wrReturnCode_LICENSE_ERROR_FINGERPRINT_INVALID,
    wrReturnCode_LICENSE_ERROR_ACTIVATION_LIMIT,
    wrReturnCode_LICENSE_ERROR_FLOAT_LIMIT,
    wrReturnCode_LICENSE_ERROR_LEASE,
    wrReturnCode_LICENSE_ERROR_SERVER_ERROR,
    wrReturnCode_LICENSE_ERROR_SERVER_INVALID,
    wrReturnCode_LICENSE_ERROR_SERVER_LICENSE_INVALID,
    wrReturnCode_LICENSE_ERROR_SERVER_LICENSE_SUSPENDED,
    wrReturnCode_LICENSE_ERROR_SERVER_LICENSE_EXPIRED,
    wrReturnCode_LICENSE_ERROR_PERMISSIONS,
    wrReturnCode_LICENSE_GRACE_PERIOD_OVER,
    wrReturnCode_LICENSE_PATH_ERROR,
    wrReturnCode_LICENSE_ERROR_OTHER,
    wrReturnCode_OTHER_ERROR,
    wrReturnCode_NO_MODELS,
    wrReturnCode_MODEL_LOAD_FAILED,
    wrReturnCode_HEAD_NOT_INITIALIZED,
    wrReturnCode_HAND_SEGMENTER_NOT_SET,
    wrReturnCode_HANDS_NOT_INITIALIZED,
    wrReturnCode_3D_NOT_INITIALIZED,
    wrReturnCode_JOINT_DEFINITION_ERROR,
    wrReturnCode_HEAD_NOT_SUPPORTED,
    wrReturnCode_CLASSIFIER_NOT_SUPPORTED,
    wrReturnCode_CLONING_NOT_SUPPORTED,
    wrReturnCode_INCOMPATIBLE_POSE_ESTIMATOR_REQUIREMENTS,
    wrReturnCode_DESERIALIZE_ERROR,
    wrReturnCode_UNSUPPORTED_ON_PLATFORM,
    wrReturnCode_NO_SUCH_GPU,
    wrReturnCode_INCOMPATIBLE_WRNCH_VERSION,
    wrReturnCode_INCOMPATIBLE_CUDA_COMPUTE_CAPABILITY,
    wrReturnCode_CLONE_ERROR,
    wrReturnCode_INCOMPATIBLE_TENSORRT_VERSION,
    wrReturnCode_INVALID_REGEX,
    wrReturnCode_NO_MATCHING_MODEL,
    wrReturnCode_PLUGIN_LOAD_ERROR,
} wrReturnCode;

#ifdef __cplusplus
extern "C"
{
#endif

    WRNCH_DLL_INTERFACE const char* wrReturnCode_Translate(wrReturnCode code);

#ifdef __cplusplus
}
#endif

#endif /* WRNCH_ENGINE_RETURN_CODES */
