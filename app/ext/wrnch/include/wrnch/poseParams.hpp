/*
 Copyright (c) 2019 wrnch Inc.
 All rights reserved
*/

#ifndef WRNCH_POSE_PARAMS_CXX_API
#define WRNCH_POSE_PARAMS_CXX_API

#include <wrnch/utils.hpp>
#include <wrnch/poseTypes.hpp>
#include <wrnch/poseParams.h>

#include <new>
#include <type_traits>

namespace wrnch
{
namespace detail
{
template<class PtrType>
class BasicPoseParams
{
  public:
    explicit BasicPoseParams(PtrType ptr)
    : m_impl(ptr)
    {
        utils::CheckBadAlloc(m_impl);
    }

    wrSensitivity GetBoneSensitivity() const { return wrPoseParams_GetBoneSensitivity(m_impl); }
    wrSensitivity GetJointSensitivity() const { return wrPoseParams_GetJointSensitivity(m_impl); }
    wrNetPrecision GetNetPrecision2d() const { return wrPoseParams_GetNetPrecision2d(m_impl); }
    wrNetPrecision GetNetPrecision3d() const { return wrPoseParams_GetNetPrecision3d(m_impl); }
    bool GetEnableTracking() const { return wrPoseParams_GetEnableTracking(m_impl) != 0; }
    int GetPreferredNetWidth2d() const { return wrPoseParams_GetPreferredNetWidth2d(m_impl); }
    int GetPreferredNetHeight2d() const { return wrPoseParams_GetPreferredNetHeight2d(m_impl); }
    int GetPreferredNetWidth3d() const { return wrPoseParams_GetPreferredNetWidth3d(m_impl); }
    int GetPreferredNetHeight3d() const { return wrPoseParams_GetPreferredNetHeight3d(m_impl); }
    float GetSmoothingBetaX() const { return wrPoseParams_GetSmoothingBetaX(m_impl); }
    float GetSmoothingBetaY() const { return wrPoseParams_GetSmoothingBetaY(m_impl); }
    float GetSmoothingBetaZ() const { return wrPoseParams_GetSmoothingBetaZ(m_impl); }

    float GetSmoothingCutoffFreqVelocity() const
    {
        return wrPoseParams_GetSmoothingCutoffFreqVelocity(m_impl);
    }

    float GetSmoothingMinCutoffFreqPosition() const
    {
        return wrPoseParams_GetSmoothingMinCutoffFreqPosition(m_impl);
    }

    int GetMinValidJoints() const { return wrPoseParams_GetMinValidJoints(m_impl); }

    bool operator==(const BasicPoseParams& r) const
    {
        return wrPoseParams_CheckEquivalent(m_impl, r.Get());
    }

    typename std::remove_pointer<PtrType>::type const* Get() const { return m_impl; }

  protected:
    PtrType m_impl;
};

template<class PtrType>
class BasicIKParams
{
  public:
    explicit BasicIKParams(PtrType ptr)
    : m_impl(ptr)
    {
        utils::CheckBadAlloc(m_impl);
    }

    float GetTransReach() const { return wrIKParams_GetTransReach(m_impl); }
    float GetRotReach() const { return wrIKParams_GetRotReach(m_impl); }
    float GetPull() const { return wrIKParams_GetPull(m_impl); }
    float GetResist() const { return wrIKParams_GetResist(m_impl); }
    float GetMaxAngularVelocity() const { return wrIKParams_GetMaxAngularVelocity(m_impl); }
    float GetFPS() const { return wrIKParams_GetFPS(m_impl); }
    bool GetOptimizeForMonocular() const { return wrIKParams_GetOptimizeForMonocular(m_impl); }
    float GetJointVisibilityThresh() const { return wrIKParams_GetJointVisibilityThresh(m_impl); }
    float* GetUpDirection() const { return wrIKParams_GetUpDirection(m_impl); }

    bool operator==(const BasicIKParams& r) const
    {
        return wrIKParams_CheckEquivalent(m_impl, r.Get());
    }

    typename std::remove_pointer<PtrType>::type const* Get() const { return m_impl; }

  protected:
    PtrType m_impl;
};
}

class MutablePoseParams : public detail::BasicPoseParams<wrPoseParams*>
{
  public:
    typedef detail::BasicPoseParams<wrPoseParams*> Base;

    MutablePoseParams()
    : Base(wrPoseParams_Create())
    {
        utils::CheckBadAlloc(m_impl);
    }
    MutablePoseParams(const MutablePoseParams& other)
    : Base(wrPoseParams_Clone(other.m_impl))
    {
        utils::CheckBadAlloc(m_impl);
    }

    MutablePoseParams& operator=(const MutablePoseParams& rhs)
    {
        wrPoseParams_Copy(rhs.m_impl, m_impl);
        return *this;
    }

    static MutablePoseParams TakeOwnershipOf(wrPoseParams* params)
    {
        MutablePoseParams result(params);
        return result;
    }

    ~MutablePoseParams() { wrPoseParams_Destroy(m_impl); }

    /// @brief set the tracker kind to be used by a subsequent pose estimator.
    ///        Trackers attempt to set consistent ids on persons in frame through time.
    ///        ids are accessed `GetId` family of functions on pose types, eg, `Pose2d::GetId`.
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
    /// @param trackerKind the kind to set. `wrTrackerKind_V1` selects the V1 tracker,
    ///        `wrTrackerKind_V2` selects the V2 tracker. Calling
    ///        `params.SetTrackerKind(wrTrackerKind_None)` is equivalent to calling
    ///        `params.SetEnableTracking(false)`. Any other value has no effect.
    void SetTrackerKind(wrTrackerKind trackerKind)
    {
        wrPoseParams_SetTrackerKind(m_impl, trackerKind);
    }

    wrTrackerKind GetTrackerKind() const { return wrPoseParams_GetTrackerKind(m_impl); }

    void SetBoneSensitivity(wrSensitivity sensitivity)
    {
        wrPoseParams_SetBoneSensitivity(m_impl, sensitivity);
    }
    void SetJointSensitivity(wrSensitivity sensitivity)
    {
        wrPoseParams_SetJointSensitivity(m_impl, sensitivity);
    }
    void SetNetPrecision2d(wrNetPrecision precision)
    {
        wrPoseParams_SetNetPrecision2d(m_impl, precision);
    }
    void SetNetPrecision3d(wrNetPrecision precision)
    {
        wrPoseParams_SetNetPrecision3d(m_impl, precision);
    }

    /// @brief enable or disable the (stateful) tracking module on a pose estimator (which is on by
    ///        default).
    /// @param yesNo `false` means no (ie disable tracking), `true` means yes (ie enable tracking).
    void SetEnableTracking(bool yesNo) { wrPoseParams_SetEnableTracking(m_impl, yesNo); }

    /// @brief set the 2d net width.
    /// @param width preferred width.
    /// @note  On iOS and CPU-Only builds, setting this value has no effect on a resulting estimator
    ///        -- the net dimensions are fixed in the on-disk models.
    void SetPreferredNetWidth2d(int width) { wrPoseParams_SetPreferredNetWidth2d(m_impl, width); }

    /// @brief set the 2d net height.
    /// @param height preferred height.
    /// @note  On iOS and CPU-Only builds, setting this value has no effect on a resulting estimator
    ///        -- the net dimensions are fixed in the on-disk models.
    void SetPreferredNetHeight2d(int height)
    {
        wrPoseParams_SetPreferredNetHeight2d(m_impl, height);
    }

    /// @brief set the 3d net width.
    /// @param width preferred width.
    /// @note  On iOS and CPU-Only builds, setting this value has no effect on a resulting estimator
    ///        -- the net dimensions are fixed in the on-disk models.
    void SetPreferredNetWidth3d(int width) { wrPoseParams_SetPreferredNetWidth3d(m_impl, width); }

    /// @brief set the 3d net height.
    /// @param height preferred height.
    /// @note  On iOS and CPU-Only builds, setting this value has no effect on a resulting estimator
    ///        -- the net dimensions are fixed in the on-disk models.
    void SetPreferredNetHeight3d(int height)
    {
        wrPoseParams_SetPreferredNetHeight3d(m_impl, height);
    }
    void SetSmoothingBetaX(float beta) { wrPoseParams_SetSmoothingBetaX(m_impl, beta); }
    void SetSmoothingBetaY(float beta) { wrPoseParams_SetSmoothingBetaY(m_impl, beta); }
    void SetSmoothingBetaZ(float beta) { wrPoseParams_SetSmoothingBetaZ(m_impl, beta); }

    void SetSmoothingCutoffFreqVelocity(float cutoffFreq)
    {
        wrPoseParams_SetSmoothingCutoffFreqVelocity(m_impl, cutoffFreq);
    }
    void SetSmoothingMinCutoffFreqPosition(float cutoffFreq)
    {
        wrPoseParams_SetSmoothingMinCutoffFreqPosition(m_impl, cutoffFreq);
    }

    void SetMinValidJoints(int minJoints) { wrPoseParams_SetMinValidJoints(m_impl, minJoints); }

    using Base::Get;
    wrPoseParams* Get() { return this->m_impl; }

  private:
    explicit MutablePoseParams(wrPoseParams* params)
    : Base(params)
    {
    }
};

typedef MutablePoseParams PoseParams;
typedef detail::BasicPoseParams<const wrPoseParams*> PoseParamsView;

class MutableIKParams : public detail::BasicIKParams<wrIKParams*>
{
  public:
    typedef detail::BasicIKParams<wrIKParams*> Base;

    MutableIKParams()
    : Base(wrIKParams_Create())
    {
        utils::CheckBadAlloc(m_impl);
    }
    MutableIKParams(const MutableIKParams& other)
    : Base(wrIKParams_Clone(other.m_impl))
    {
        utils::CheckBadAlloc(m_impl);
    }

    MutableIKParams& operator=(const MutableIKParams& rhs)
    {
        wrIKParams_Copy(rhs.m_impl, m_impl);
        return *this;
    }

    static MutableIKParams TakeOwnershipOf(wrIKParams* params)
    {
        MutableIKParams result(params);
        return result;
    }

    ~MutableIKParams() { wrIKParams_Destroy(m_impl); }

    void SetTransReach(float reach) { wrIKParams_SetTransReach(m_impl, reach); }
    void SetRotReach(float reach) { wrIKParams_SetRotReach(m_impl, reach); }
    void SetPull(float pull) { wrIKParams_SetPull(m_impl, pull); }
    void SetResist(float resist) { wrIKParams_SetResist(m_impl, resist); }
    void SetMaxAngularVelocity(float velocity)
    {
        wrIKParams_SetMaxAngularVelocity(m_impl, velocity);
    }
    void SetFPS(float fps) { wrIKParams_SetFPS(m_impl, fps); }
    void SetOptimizeForMonocular(bool isMono)
    {
        wrIKParams_SetOptimizeForMonocular(m_impl, isMono ? 1 : 0);
    }
    void SetJointVisibilityThresh(float thresh)
    {
        wrIKParams_SetJointVisibilityThresh(m_impl, thresh);
    }

    void SetUpDirection(float x, float y, float z) { wrIKParams_SetUpDirection(m_impl, x, y, z); }

    using Base::Get;
    wrIKParams* Get() { return this->m_impl; }

  private:
    explicit MutableIKParams(wrIKParams* params)
    : Base(params)
    {
    }
};

typedef MutableIKParams IKParams;
typedef detail::BasicIKParams<const wrIKParams*> IKParamsView;
}  // namespace wrnch

#endif /* WRNCH_POSE_PARAMS_CXX_API */
