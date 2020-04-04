/*
 Copyright (c) 2019 wrnch Inc.
 All rights reserved
*/

#ifndef WRNCH_ACTIVITY_POSE_ESTIMATOR_REQUIREMENTS_CXX_API
#define WRNCH_ACTIVITY_POSE_ESTIMATOR_REQUIREMENTS_CXX_API

#include <wrnch/activityPoseEstimatorRequirements.h>

/// @file activityPoseEstimatorRequirements.hpp

namespace std
{
template<>
struct default_delete<wrPoseEstimatorRequirements>
{
    void operator()(wrPoseEstimatorRequirements* requirements) const
    {
        wrPoseEstimatorRequirements_Destroy(requirements);
    }
};
}  // namespace std

namespace wrnch
{
/// @brief ActivityPoseEstimatorRequirements
///         Provides a C++ wrapper to the C-struct `wrPoseEstimatorRequirements` (of incomplete
///         type) and the free functions acting on it.
/// @see activityPoseEstimatorRequirements.h
class ActivityPoseEstimatorRequirements
{
  public:
    explicit ActivityPoseEstimatorRequirements(wrPoseEstimatorRequirements* ptr)
    : m_ptr{ ptr }
    {
    }

    ActivityPoseEstimatorRequirements(ActivityPoseEstimatorRequirements&&) = default;
    ActivityPoseEstimatorRequirements& operator=(ActivityPoseEstimatorRequirements&&) = default;
    ~ActivityPoseEstimatorRequirements() = default;

    /// @brief Return true if hands are required, false otherwise.
    bool RequiresHands() const;

    /// @brief Return the "preferred" joint format to be used in a pose estimator.
    JointDefinition PreferredOutputFormat() const;

    /// @brief Attempt to create a minimal PoseEstimator compatible which is compatible with
    ///         `*this`. Throws an exception if a poseEstimator cannot be created.
    /// @param poseModelDir the name of the directory holding the wrnch pose models.
    /// @param licenseString if non-empty, the license string
    /// @param deviceId the GPU device id on which the pose estimator should be created.
    /// @param desiredNetWidth the desired "network width" of the pose estimator
    /// @param desiredNetHeight the desired "network height" of the pose estimator
    PoseEstimator CreateCompatibleEstimator(std::string const& poseModelDir,
                                            int deviceId,
                                            int desiredNetWidth,
                                            int desiredNetHeight,
                                            std::string const& licenseString = std::string()) const;

    PoseEstimator CreateCompatibleEstimator(std::string const& poseModelDir,
                                            int const deviceId,
                                            int const desiredNetWidth,
                                            int const desiredNetHeight,
                                            std::string const& licenseString,
                                            wrReturnCode& outcode) const;

    /// @brief Create minimal PoseEstimatorOptions which are compatible with requirements
    PoseEstimatorOptions CreateCompatibleOptions() const;

    /// @brief Return `true` if poseEstimator is compatible with `*this`, else return `false`.
    /// @param poseEstimator an instance of `PoseEstimator`, `PoseEstimatorConst`, or
    /// `PoseEstimator`
    template<class PoseEstimator_t>
    bool IsCompatible(PoseEstimator_t const& poseEstimator) const;

    /// @brief Return `true` if `options` is compatible with `*this`, else return `false`.
    /// @param options an instance of `PoseEstimator`, `PoseEstimatorConst`, or
    /// `PoseEstimator`
    bool IsCompatible(PoseEstimatorOptions const& options) const;

    wrPoseEstimatorRequirements const* GetPtr() const;
    wrPoseEstimatorRequirements const* Get() const;

  private:
    std::unique_ptr<wrPoseEstimatorRequirements> m_ptr;
};

inline bool ActivityPoseEstimatorRequirements::RequiresHands() const
{
    return wrPoseEstimatorRequirements_RequiresHands(GetPtr()) != 0;
}

inline wrPoseEstimatorRequirements const* ActivityPoseEstimatorRequirements::GetPtr() const
{
    return Get();
}

inline wrPoseEstimatorRequirements const* ActivityPoseEstimatorRequirements::Get() const
{
    return m_ptr.get();
}

inline JointDefinition ActivityPoseEstimatorRequirements::PreferredOutputFormat() const
{
    return JointDefinition(wrPoseEstimatorRequirements_PreferredOutputFormat(GetPtr()));
}

inline PoseEstimator ActivityPoseEstimatorRequirements::CreateCompatibleEstimator(
    std::string const& poseModelDir,
    int const deviceId,
    int const desiredNetWidth,
    int const desiredNetHeight,
    std::string const& licenseString) const
{
    wrReturnCode retcode;

    PoseEstimator res = CreateCompatibleEstimator(
        poseModelDir, deviceId, desiredNetWidth, desiredNetHeight, licenseString, retcode);

    if (retcode != wrReturnCode_OK)
    {
        throw utils::Exception(retcode);
    }

    return res;
}

inline PoseEstimator ActivityPoseEstimatorRequirements::CreateCompatibleEstimator(
    std::string const& poseModelDir,
    int const deviceId,
    int const desiredNetWidth,
    int const desiredNetHeight,
    std::string const& licenseString,
    wrReturnCode& outcode) const
{
    wrPoseEstimator* cPoseEstimator{ nullptr };

    char const* const licenseStringPtr = licenseString.empty() ? nullptr : licenseString.c_str();

    outcode = wrPoseEstimatorRequirements_CreateCompatibleEstimator(GetPtr(),
                                                                    &cPoseEstimator,
                                                                    poseModelDir.c_str(),
                                                                    licenseStringPtr,
                                                                    deviceId,
                                                                    desiredNetWidth,
                                                                    desiredNetHeight);
    return PoseEstimator{ cPoseEstimator };
}

inline PoseEstimatorOptions ActivityPoseEstimatorRequirements::CreateCompatibleOptions() const
{
    return PoseEstimatorOptions{ wrPoseEstimatorRequirements_CreateCompatibleOptions(GetPtr()) };
}

template<class PoseEstimator_t>
bool ActivityPoseEstimatorRequirements::IsCompatible(PoseEstimator_t const& poseEstimator) const
{
    return wrPoseEstimatorRequirements_IsEstimatorCompatible(GetPtr(), poseEstimator.GetPtr()) != 0;
}

inline bool ActivityPoseEstimatorRequirements::IsCompatible(
    PoseEstimatorOptions const& options) const
{
    return wrPoseEstimatorRequirements_AreOptionsCompatible(GetPtr(), options.GetPtr()) != 0;
}

}  // namespace wrnch

#endif /*WRNCH_ACTIVITY_POSE_ESTIMATOR_REQUIREMENTS_CXX_API*/
