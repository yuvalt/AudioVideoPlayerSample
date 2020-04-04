/*
 Copyright (c) 2019 wrnch Inc.
 All rights reserved
*/

#ifndef WRNCH_wrPoseIK_CXX_API
#define WRNCH_wrPoseIK_CXX_API

#include <wrnch/poseIK.h>
#include <wrnch/poseTypes.hpp>
#include <wrnch/jointDefinition.hpp>
#include <wrnch/poseParams.hpp>
#include <wrnch/IKProperties.h>


namespace std
{
template<>
struct default_delete<wrPoseIK>
{
    void operator()(wrPoseIK* c_poseIK) const { wrPoseIK_Destroy(c_poseIK); }
};
}  // namespace std

namespace wrnch
{
////////////////// Experimental IK Solver //////////////////
////////////////// For advanced users only //////////////////

enum class OwnershipPolicy : bool
{
    Owned,
    Unowned
};

template<OwnershipPolicy>
class PoseIKPtr;

template<>
class PoseIKPtr<OwnershipPolicy::Owned>
{
  public:
    explicit PoseIKPtr(wrPoseIK* ptr)
    : m{ ptr }
    {
    }
    PoseIKPtr() = default;
    PoseIKPtr(PoseIKPtr&&) = default;
    PoseIKPtr& operator=(PoseIKPtr&&) = default;

    wrPoseIK* Get() { return m.get(); }
    wrPoseIK const* Get() const { return m.get(); }

  private:
    std::unique_ptr<wrPoseIK> m;
};

template<>
class PoseIKPtr<OwnershipPolicy::Unowned>
{
  public:
    explicit PoseIKPtr(wrPoseIK* ptr)
    : m{ ptr }
    {
    }
    PoseIKPtr() = default;
    PoseIKPtr(PoseIKPtr const&) = default;
    PoseIKPtr& operator=(PoseIKPtr const&) = default;
    wrPoseIK* Get() { return m; }
    wrPoseIK const* Get() const { return m; }

  private:
    wrPoseIK* m = nullptr;
};

/// @brief BasicPoseIK allows to control a rig using 3d joints as effectors.
///        The solver internally uses the following referential: Y up, Z forward, X left.
///        All input coordinates needs to be converted to the right referential before calling
///        the Solve function. The output format is always formated with the extended joint
///        definition, and contains position and rotation informations in the same referential
///        as the input data. Positions will be returned scaled to the initial T pose specified
///        at initialization. Rotations are returned as quaternions for each joint, in world
///        space.
template<OwnershipPolicy ownershipPolicy>
class BasicPoseIK
{
    using PtrType = PoseIKPtr<ownershipPolicy>;
    PtrType m_impl;

  public:
    explicit BasicPoseIK(wrPoseIKHandle ikSolverHandle)
    : m_impl(ikSolverHandle)
    {
    }

    /// @brief Creates a BasicPoseIK solver using default internal node positions.
    /// @param inputFormat The input format used when feeding positions to the solver.
    /// @param params Solver parameters
    /// @see IKParams
    BasicPoseIK(JointDefinition inputFormat, IKParams params)
    {
        wrPoseIKHandle handle;
        wrReturnCode code = wrPoseIK_CreateDefault(&handle, inputFormat.Get(), params.Get());
        ExceptionHandler(code);
        m_impl = PtrType(handle);
    }

    /// @brief Creates a BasicPoseIK solver using provided node positions.
    /// @param inputFormat The input format used when feeding positions to the solver
    /// @param initialPose The initial pose used to initiallize the internal IK Solver in a pose3d
    ///        handle. This pose must follow the extended joints definition composed of 30 joints
    ///        and represent a T posed character in the IK solver referential (Y up, Z forward, X
    ///        left).
    /// @param params Solver parameters
    /// @see IKParams
    BasicPoseIK(JointDefinition inputFormat, Pose3dView initialPose, IKParams params)
    {
        wrPoseIKHandle handle;
        wrReturnCode code
            = wrPoseIK_CreateFromPose(&handle, inputFormat.Get(), initialPose.Get(), params.Get());
        ExceptionHandler(code);
        m_impl = PtrType(handle);
    }

    /// @brief Creates a BasicPoseIK solver using provided node positions.
    /// @param inputFormat The input format used when feeding positions to the solver
    /// @param initialPose The initial pose used to initiallize the internal IK Solver.
    ///        This pose must follow the extended joints definition composed of 30 joints
    ///        and represent a T posed character in the IK solver referential (Y up, Z forward, X
    ///        left).
    /// @param numJoints of joints contained in the provided initial pose. Should be the number of
    ///        joints present in the extended joint definition.
    /// @param params Solver parameters
    /// @see IKParams
    BasicPoseIK(JointDefinition inputFormat,
                const float* initialPose,
                unsigned int numJoints,
                IKParams params)
    {
        wrPoseIKHandle handle;
        wrReturnCode code
            = wrPoseIK_Create(&handle, inputFormat.Get(), initialPose, numJoints, params.Get());
        ExceptionHandler(code);
        m_impl = PtrType(handle);
    }

    IKParamsView GetParams() const { return IKParamsView(wrPoseIK_GetParams(m_impl.Get())); }

    void SetParams(IKParams params) { wrPoseIK_SetParams(m_impl.Get(), params.Get()); }

    /// @brief Performs a solving step using the provided 3d positions as effectors.
    ///        The result will be returned in a pose3d containing positions and rotations for each
    ///        joint in the extended joint definition. Rotations are represented here as normalized
    ///        quaternions in world space. Each rotation is independant of its parent node and is
    ///        expressed with respect to the initial TPose (eg. the final rotation of your object
    ///        should be Rotation = Q * Qtpose.conjugate()) Positions will be solved with respect to
    ///        the scale of the initial T posed character, and an average scaling factor can be
    ///        found in the ScaleHint field of the pose3d object. Values are returned in the
    ///        internal IK Solver referential (Y up, Z forward, X left).
    /// @param pose 3d positions of the input effectors. The input positions needs to match the
    ///        input format specified at initialization. The input positions must be expressed in
    ///        the internal solver referential (Y up, Z Forward, X left).
    /// @param visibilities Represents the visibility of each effector in the input pose. Those
    ///        joints will be ignored when performing the solving step. 0 represents a non visible
    ///        joint, 1 represents a visible joint.
    /// @see Pose3d
    Pose3dView Solve(const float* pose, const int* visibilities)
    {
        return Pose3dView(wrPoseIK_Solve(m_impl.Get(), pose, visibilities));
    }
    Pose3dView Solve(Pose3d pose, const int* visibilities)
    {
        return Pose3dView(wrPoseIK_Solve(m_impl.Get(), pose.GetPositions(), visibilities));
    }

    /// @brief Resets the internal state of the IK Solver.
    void Reset() { wrPoseIK_ResetDefault(m_impl.Get()); }

    /// @param initialPose initialPose The initial pose used to initiallize the internal IK Solver.
    ///        This pose must follow the extended joints definition composed of 30 joints
    ///        and represent a T posed character in the IK solver referential (Y up, Z forward, X
    ///        left).
    /// @param numJoints number of joints present in initialPose
    void Reset(const float* initialPose, int numJoints)
    {
        wrPoseIK_Reset(m_impl.Get(), initialPose, numJoints);
    }
    void Reset(wrPose3dHandleConst initialPose)
    {
        wrPoseIK_ResetFromPose(m_impl.Get(), initialPose);
    }

    float GetIKProperty(wrIKProperty property) const
    {
        return wrPoseIK_GetIKProperty(m_impl.Get(), property);
    }
    void SetIKProperty(wrIKProperty property, float value)
    {
        wrPoseIK_SetIKProperty(m_impl.Get(), property, value);
    }

    void SetInputFormat(JointDefinition inputFormat)
    {
        wrPoseIK_SetInputFormat(m_impl.Get(), inputFormat.Get());
    }

    JointDefinition GetOutputFormat() const
    {
        return JointDefinition(wrPoseIK_GetOutputFormat(m_impl.Get()));
    }

    JointDefinition GetInputFormat() const
    {
        return JointDefinition(wrPoseIK_GetInputFormat(m_impl.Get()));
    }

    /// @brief Get the initial TPose specified in the solver. The T pose will be returned as a
    ///        Pose3dView, containing positions only. We assume unit quaternions when in TPose.
    Pose3dView GetTPose() const { return Pose3dView(wrPoseIK_GetTPose(m_impl.Get())); }

    /// @brief Toggles the solving of foot contact in IK
    /// @param contact
    void SetFloorContact(bool contact) { wrPoseIK_SetFloorContact(m_impl.Get(), contact); }

    /// @brief Set the transform of the floor plane used when solving foot contact in IK.
    ///        This transform is applied to a plane with a normal pointing up in the Y direction
    ///        and containing the world origin.
    /// @param transform The translation of the floor plane (x, y, z, w)
    /// @param rotation A quaternion expressing the rotation of the floor plane.
    void SetFloorTransform(const float* const transform, const float* const rotation)
    {
        wrPoseIK_SetFloorTransform(m_impl.Get(), transform, rotation);
    }

    wrPoseIKHandle Get() { return m_impl.Get(); }
    wrPoseIKHandleConst Get() const { return m_impl.Get(); }

  private:
    void ExceptionHandler(wrReturnCode code) const { ExceptionHandler(code, ""); }
    void ExceptionHandler(wrReturnCode code, const char* msg) const
    {
        ExceptionHandlerStatic(code, msg, Get());
    }
    static void ExceptionHandlerStatic(wrReturnCode code, wrPoseIK const* c_poseIK)
    {
        ExceptionHandlerStatic(code, "", c_poseIK);
    }
    static void ExceptionHandlerStatic(wrReturnCode code, const char* msg, wrPoseIK const* c_poseIK)
    {
        switch (code)
        {
            case wrReturnCode_OK:
                break;
            case wrReturnCode_BAD_ALLOC:
                utils::CheckBadAlloc(c_poseIK);
                break;
            default:
                throw utils::Exception(code, msg);
        }
    }
};

/// @brief PoseIK an owned PoseIK solver
///        When a PoseIKView goes out of scope, the underlying wrPoseIK is destroyed.
class PoseIK : public BasicPoseIK<OwnershipPolicy::Owned>
{
  public:
    using base_class = BasicPoseIK<OwnershipPolicy::Owned>;

    explicit PoseIK(wrPoseIKHandle ikSolverHandle)
    : base_class(ikSolverHandle)
    {
    }

    PoseIK(PoseIK&&) = default;
    PoseIK& operator=(PoseIK&&) = default;

    PoseIK(JointDefinition inputFormat, IKParams params)
    : base_class(inputFormat, params)
    {
    }

    PoseIK(JointDefinition inputFormat, Pose3dView initialPose, IKParams params)
    : base_class(inputFormat, initialPose, params)
    {
    }

    PoseIK(JointDefinition inputFormat,
           const float* initialPose,
           unsigned int numJoints,
           IKParams params)
    : base_class(inputFormat, initialPose, numJoints, params)
    {
    }
};

/// @brief PoseIKView an unowned PoseIK solver
///        Returned by PoseEstimator::GetIKSolver, this is a "view" type, meaning the ownership of
///        the underlying wrPoseIK object is someone else's. When a PoseIKView goes out of scope, it
///        does not destroy the underlying wrPoseIK
class PoseIKView : public BasicPoseIK<OwnershipPolicy::Unowned>
{
  public:
    using base_class = BasicPoseIK<OwnershipPolicy::Unowned>;

    explicit PoseIKView(wrPoseIKHandle ikSolverHandle)
    : base_class(ikSolverHandle)
    {
    }
};


} /*namespace wrnch*/

#endif /* WRNCH_wrPoseIK_CXX_API */
