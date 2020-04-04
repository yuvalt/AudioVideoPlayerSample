/*
 Copyright (c) 2019 wrnch Inc.
 All rights reserved
*/

#ifndef WRNCH_POSE_PARAMS_C_API
#define WRNCH_POSE_PARAMS_C_API

#include <wrnch/definitions.h>
#include <wrnch/poseTypes.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        wrHandSegmenterType_NAIVE,
        wrHandSegmenterType_FOUR_CHANNEL,
        wrHandSegmenterType_UNKNOWN
    } wrHandSegmenterType;

    struct wrPoseParams;
    typedef struct wrPoseParams* wrPoseParamsHandle;
    typedef const struct wrPoseParams* wrPoseParamsHandleConst;

    WRNCH_DLL_INTERFACE wrPoseParamsHandle wrPoseParams_Create(void);
    WRNCH_DLL_INTERFACE wrPoseParamsHandle wrPoseParams_Clone(wrPoseParamsHandleConst);
    WRNCH_DLL_INTERFACE void wrPoseParams_Copy(wrPoseParamsHandleConst src,
                                               wrPoseParamsHandle dest);
    WRNCH_DLL_INTERFACE void wrPoseParams_Destroy(wrPoseParamsHandle);

    typedef enum
    {
        wrSensitivity_LOW,
        wrSensitivity_MEDIUM,
        wrSensitivity_HIGH
    } wrSensitivity;

    typedef enum
    {
        wrNetPrecision_INT8,
        wrNetPrecision_FP16,
        wrNetPrecision_FP32,
        wrNetPrecision_AUTO
    } wrNetPrecision;

    typedef enum
    {
        wrTrackerKind_V1 = 0,
        wrTrackerKind_V2,
        wrTrackerKind_None
    } wrTrackerKind;


    /// @brief set the tracker kind to be used by a subsequent pose estimator.
    ///        Trackers attempt to set consistent ids on persons in frame through time.
    ///        ids are accessed `XXX_GetId` family of functions on pose types, eg, `wrPose2d_GetId`.
    ///        wrnchAI implements two trackers, V1 and V2, described below.
    ///
    ///        V1: The legacy (and default) tracker which is based on Kalman filtering of joints.
    ///            This tracker is faster than V2 but suffers from more id switches between people.
    ///
    ///        V2: The new (and recommended) tracker which uses both position and "appearance"
    ///            information of persons. This tracker works best with
    ///            stable lighting conditions, BGR (color) frames, and accurate, complete
    ///            detections. In order to improve detections, a higher 2d net resolution may be
    ///            needed. In order to have complete detections, setting a high min valid joints on
    ///            PoseParams (for example, 12 or more) which configure a PoseEstimator may help.
    ///            Note that the default min valid joints used by PoseEstimators is currently 3.
    ///            Also note that this tracker comes with a nontrivial runtime cost -- users not
    ///            interested in tracking should disable tracking (by using
    ///            `wrPoseParams_SetEnableTracking(poseParams, 0)`).
    /// @param handle the poseParams object to modify.
    /// @param trackerKind the kind to set. `wrTrackerKind_V1` selects the V1 tracker,
    ///        `wrTrackerKind_V2` selects the V2 tracker. Calling
    ///        `wrPoseParams_SetTrackerKind(handle, wrTrackerKind_None)` is equivalent to calling
    ///        `wrPoseParams_SetEnableTracking(handle, 0)`. Any other value has no effect.
    WRNCH_DLL_INTERFACE void wrPoseParams_SetTrackerKind(wrPoseParamsHandle handle,
                                                         wrTrackerKind trackerKind);
    WRNCH_DLL_INTERFACE wrTrackerKind wrPoseParams_GetTrackerKind(wrPoseParamsHandleConst handle);

    WRNCH_DLL_INTERFACE void wrPoseParams_SetBoneSensitivity(wrPoseParamsHandle, wrSensitivity);
    WRNCH_DLL_INTERFACE void wrPoseParams_SetJointSensitivity(wrPoseParamsHandle, wrSensitivity);

    /// @brief enable or disable the (stateful) tracking module on a pose estimator (which is on by
    ///        default).
    /// @param handle the poseParams object to modify
    /// @param yesNo 0 means no (ie disable tracking), anything else means yes (ie enable tracking)
    WRNCH_DLL_INTERFACE void wrPoseParams_SetEnableTracking(wrPoseParamsHandle handle, int yesNo);
    /// @brief set the preferred net width for a 2d estimator.
    /// @param params the pose params to modify.
    /// @param width the preferred 2d net width.
    /// @note  On iOS and CPU-Only builds, setting this value has no effect on a resulting estimator -- the net
    ///        dimensions are fixed in the on-disk models.
    WRNCH_DLL_INTERFACE void wrPoseParams_SetPreferredNetWidth2d(wrPoseParamsHandle params,
                                                                 int width);
    /// @brief set the preferred net height for a 2d estimator.
    /// @param params the pose params to modify.
    /// @param height the preferred 2d net height.
    /// @note  On iOS and CPU-Only builds, setting this value has no effect on a resulting estimator -- the net
    ///        dimensions are fixed in the on-disk models.
    WRNCH_DLL_INTERFACE void wrPoseParams_SetPreferredNetHeight2d(wrPoseParamsHandle params,
                                                                  int height);
    /// @brief set the preferred net width for a 3d estimator.
    /// @param params the pose params to modify.
    /// @param width the preferred 3d net width.
    /// @note  On iOS and CPU-Only builds, setting this value has no effect on a resulting estimator -- the net
    ///        dimensions are fixed in the on-disk models.
    WRNCH_DLL_INTERFACE void wrPoseParams_SetPreferredNetWidth3d(wrPoseParamsHandle params,
                                                                 int width);
    /// @brief set the preferred net height for a 3d estimator.
    /// @param params the pose params to modify.
    /// @param height the preferred 3d net height.
    /// @note  On iOS and CPU-Only builds, setting this value has no effect on a resulting estimator -- the net
    ///        dimensions are fixed in the on-disk models.
    WRNCH_DLL_INTERFACE void wrPoseParams_SetPreferredNetHeight3d(wrPoseParamsHandle params,
                                                                  int height);
    WRNCH_DLL_INTERFACE void wrPoseParams_SetSmoothingBetaX(wrPoseParamsHandle, float betaX);
    WRNCH_DLL_INTERFACE void wrPoseParams_SetSmoothingBetaY(wrPoseParamsHandle, float betaY);
    WRNCH_DLL_INTERFACE void wrPoseParams_SetSmoothingBetaZ(wrPoseParamsHandle, float betaZ);
    WRNCH_DLL_INTERFACE void wrPoseParams_SetSmoothingCutoffFreqVelocity(wrPoseParamsHandle handle,
                                                                         float cutoffFreq);
    WRNCH_DLL_INTERFACE void wrPoseParams_SetSmoothingMinCutoffFreqPosition(
        wrPoseParamsHandle handle, float cutoffFreq);
    WRNCH_DLL_INTERFACE void wrPoseParams_SetMinValidJoints(wrPoseParamsHandle, int minJoints);
    WRNCH_DLL_INTERFACE void wrPoseParams_SetNetPrecision2d(wrPoseParamsHandle, wrNetPrecision);
    WRNCH_DLL_INTERFACE void wrPoseParams_SetNetPrecision3d(wrPoseParamsHandle, wrNetPrecision);
    WRNCH_DLL_INTERFACE wrSensitivity wrPoseParams_GetBoneSensitivity(wrPoseParamsHandleConst);
    WRNCH_DLL_INTERFACE wrSensitivity wrPoseParams_GetJointSensitivity(wrPoseParamsHandleConst);
    WRNCH_DLL_INTERFACE int wrPoseParams_GetEnableTracking(wrPoseParamsHandleConst);
    WRNCH_DLL_INTERFACE int wrPoseParams_GetPreferredNetWidth2d(wrPoseParamsHandleConst);
    WRNCH_DLL_INTERFACE int wrPoseParams_GetPreferredNetHeight2d(wrPoseParamsHandleConst);
    WRNCH_DLL_INTERFACE float wrPoseParams_GetSmoothingBetaX(wrPoseParamsHandleConst);
    WRNCH_DLL_INTERFACE float wrPoseParams_GetSmoothingBetaY(wrPoseParamsHandleConst);
    WRNCH_DLL_INTERFACE float wrPoseParams_GetSmoothingBetaZ(wrPoseParamsHandleConst);
    WRNCH_DLL_INTERFACE float wrPoseParams_GetSmoothingCutoffFreqVelocity(
        wrPoseParamsHandleConst handle);
    WRNCH_DLL_INTERFACE float wrPoseParams_GetSmoothingMinCutoffFreqPosition(
        wrPoseParamsHandleConst handle);
    WRNCH_DLL_INTERFACE int wrPoseParams_GetMinValidJoints(wrPoseParamsHandleConst);
    WRNCH_DLL_INTERFACE wrNetPrecision wrPoseParams_GetNetPrecision2d(wrPoseParamsHandleConst);
    WRNCH_DLL_INTERFACE wrNetPrecision wrPoseParams_GetNetPrecision3d(wrPoseParamsHandleConst);
    WRNCH_DLL_INTERFACE int wrPoseParams_CheckEquivalent(wrPoseParamsHandleConst,
                                                         wrPoseParamsHandleConst);
    WRNCH_DLL_INTERFACE int wrPoseParams_GetPreferredNetWidth3d(wrPoseParamsHandleConst);
    WRNCH_DLL_INTERFACE int wrPoseParams_GetPreferredNetHeight3d(wrPoseParamsHandleConst handle);

    struct wrIKParams;
    typedef struct wrIKParams* wrIKParamsHandle;
    typedef const struct wrIKParams* wrIKParamsHandleConst;

    WRNCH_DLL_INTERFACE wrIKParamsHandle wrIKParams_Create(void);
    WRNCH_DLL_INTERFACE wrIKParamsHandle wrIKParams_Clone(wrIKParamsHandleConst);
    WRNCH_DLL_INTERFACE void wrIKParams_Copy(wrIKParamsHandleConst src, wrIKParamsHandle dest);
    WRNCH_DLL_INTERFACE void wrIKParams_Destroy(wrIKParamsHandle);
    WRNCH_DLL_INTERFACE void wrIKParams_SetTransReach(wrIKParamsHandle, float transReach);
    WRNCH_DLL_INTERFACE void wrIKParams_SetRotReach(wrIKParamsHandle, float rotReach);
    WRNCH_DLL_INTERFACE void wrIKParams_SetPull(wrIKParamsHandle, float pull);
    WRNCH_DLL_INTERFACE void wrIKParams_SetResist(wrIKParamsHandle, float resist);
    WRNCH_DLL_INTERFACE void wrIKParams_SetMaxAngularVelocity(wrIKParamsHandle,
                                                              float maxAngularVelocity);
    WRNCH_DLL_INTERFACE void wrIKParams_SetFPS(wrIKParamsHandle, float fps);
    WRNCH_DLL_INTERFACE void wrIKParams_SetJointVisibilityThresh(wrIKParamsHandle, float thresh);
    WRNCH_DLL_INTERFACE void wrIKParams_SetOptimizeForMonocular(wrIKParamsHandle, int isMono);
    WRNCH_DLL_INTERFACE void wrIKParams_SetUpDirection(wrIKParamsHandle, float x, float y, float z);
    WRNCH_DLL_INTERFACE float wrIKParams_GetTransReach(wrIKParamsHandleConst);
    WRNCH_DLL_INTERFACE float wrIKParams_GetRotReach(wrIKParamsHandleConst);
    WRNCH_DLL_INTERFACE float wrIKParams_GetPull(wrIKParamsHandleConst);
    WRNCH_DLL_INTERFACE float wrIKParams_GetResist(wrIKParamsHandleConst);
    WRNCH_DLL_INTERFACE float wrIKParams_GetMaxAngularVelocity(wrIKParamsHandleConst);
    WRNCH_DLL_INTERFACE float wrIKParams_GetFPS(wrIKParamsHandleConst);
    WRNCH_DLL_INTERFACE float wrIKParams_GetJointVisibilityThresh(wrIKParamsHandleConst);
    WRNCH_DLL_INTERFACE int wrIKParams_GetOptimizeForMonocular(wrIKParamsHandleConst);
    WRNCH_DLL_INTERFACE int wrIKParams_CheckEquivalent(wrIKParamsHandleConst,
                                                       wrIKParamsHandleConst);

#ifdef __cplusplus
}
#endif

#endif /* WRNCH_POSE_PARAMS_C_API */
