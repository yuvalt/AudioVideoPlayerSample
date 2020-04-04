/*
 Copyright (c) 2019 wrnch Inc.
 All rights reserved
*/

#ifndef WRNCH_POSE_ESTIMATOR_CXX_API
#define WRNCH_POSE_ESTIMATOR_CXX_API

#include <wrnch/jointDefinition.hpp>
#include <wrnch/poseParams.hpp>
#include <wrnch/poseEstimatorOptions.hpp>
#include <wrnch/poseTypes.hpp>
#include <wrnch/poseTypeTraits.hpp>
#include <wrnch/utils.hpp>
#include <wrnch/poseIK.hpp>

#include <wrnch/engine.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <memory>
#include <new>


/// @file   poseEstimator.hpp
/// @brief  wrPoseEstimator
///	@note
///         -# Where required, the modelName2d must be the full 2D pose model file name, e.g.,
///         wrnch_pose2d_seg_v4.2.enc
///         -# Where required, the licensePath must either be '.', to search current directory,
///         or the path to the license. If it points directly to a file, the file is
///         -# On iOS, all paths must be relative to the root of the app bundle
///         -# Licensing
///             + License File (deprecated):
///                 * License can be read from the license file placed in the licensePath or set
///                    as licenseString in the PoseEstimatorConfigParms. If not provided, the
///                    default license search path is the user home directory (except iOS, where it
///                    will search the root of the app bundle). You can pass '.' to search current
///                    executable directory. If licensePath points directly to a file, the file is
///                    probed for a valid license. If it points to a directory, then all files
///                    ending with '.lic' are probed for a valid license.
///                 * To verify everything is being done correctly, you can call one of
///                    LicenseCheck, LicenseCheckString, or LicenseCheckPath, which will
///                    return wrReturnCode_OK or an error code.
///             + License Key:
///                 * Either call LicenseCheckString before any other operation or use
///                   PoseEstimatorConfigParams to initialize your pose estimator, in both cases
///                   set licenseString to the value of the key.
///                 * When first used, the license will be activated for the current device.
///                   If your license supports deactivations, or for floating licenses, you can
///                   deactivate the license for the current device by calling LicenseDeactivate
///                   with the same key as parameter.
///                 * If you are using a locally hosted license server, you must also set the server
///                  URL.  This must include the protocol and port,  e.g. https://example.com:8043.
///                  This requires using PoseEstimatorConfigParams to initialize your pose
///                  estimator.  Local license servers are only supported for desktop Linux and
///					 Windows builds.
///                 * On iOS, you must also set the device fingerprint to a unique ID. We recommend
///                  using the identifierForVendor or a privately stored GUID. This requires using
///                  PoseEstimatorConfigParams to initialize your pose estimator.
namespace std
{
template<>
struct default_delete<wrPoseEstimator>
{
    void operator()(wrPoseEstimator* poseEstimator) const
    {
        wrPoseEstimator_Destroy(poseEstimator);
    }
};

template<>
struct default_delete<wrPoseEstimatorConfigParams_>
{
    void operator()(wrPoseEstimatorConfigParams_* cfg) const
    {
        wrPoseEstimatorConfigParams_Destroy(cfg);
    }
};
}  // namespace std

namespace wrnch
{
inline unsigned int GetNumberOfAvailableGPUs() { return wrGetNumberOfAvailableGPUs(); }

/// LicenseCheck and LicenseCheckPath are only to be used with license files. LicenseCheckString
///     can be used with either license files or keys.
inline wrReturnCode LicenseCheck() { return wrLicense_Check(); }
inline wrReturnCode LicenseCheckPath(const std::string& licensePath)
{
    return wrLicense_CheckPath(licensePath.c_str());
}
inline wrReturnCode LicenseCheckString(const std::string& licenseString)
{
    return wrLicense_CheckString(licenseString.c_str());
}

/// @brief Returns the license activation id or an empty string
/// @see   LicenseDeactivate
/// @note  This value is needed on mobile platforms to deactivate a license
///         and is only available after a pose estimator has been initialized.
///         It is recommended that this data be persisted in your app so it
///         can be used to deactivate the license on the device when the app
///         is uninstalled.
inline std::string LicenseGetActivationID(const std::string& licenseString,
                                          const std::string& deviceFingerprint)
{
    char* activationIDPtr = nullptr;

    wrReturnCode code = wrLicense_GetActivationID(
        licenseString.c_str(), deviceFingerprint.c_str(), &activationIDPtr);

    std::string activationID
        = (code == wrReturnCode_OK) ? std::string(activationIDPtr) : std::string();

    wrUtil_FreeManagedString(activationIDPtr);
    return activationID;
}

/// @brief Deletes license activations which were generated by a license key
/// @see   LicenseGetActivationID
/// @param activationIdOrKey on mobile platforms, the activation id obtained from
///         by calling LicenseGetActivationID or the license key on other
///         platforms
/// @param licenseServerURL for locally hosted license servers only, normally this
///         should be set to an empty string. If needed, the server URL must include the
///         protocol and port, e.g. https://example.com:8043
/// @note  This function deletes an activation on our license server and should only
///         be used when uninstalling an application or moving a license activation
///         to a new computer (if your license permits it)
inline wrReturnCode LicenseDeactivate(const std::string& activationIdOrKey,
                                      const std::string& licenseServerURL = std::string())
{
    return wrLicense_Deactivate(activationIdOrKey.c_str(),
                                licenseServerURL.empty() ? nullptr : licenseServerURL.c_str());
}

inline wrReturnCode LicenseGenerateOfflineActivation(const std::string& licenseKey,
                                                     const std::string& offlineActivationFilePath)
{
    return wrLicense_GenerateOfflineActivation(licenseKey.c_str(),
                                               offlineActivationFilePath.c_str());
}

inline wrReturnCode LicenseGenerateOfflineDeactivation(
    const std::string& licenseKey, const std::string& offlineDeactivationFilePath)
{
    return wrLicense_GenerateOfflineDeactivation(licenseKey.c_str(),
                                                 offlineDeactivationFilePath.c_str());
}

inline wrReturnCode LicenseOfflineActivation(const std::string& offlineActivationFilePath)
{
    return wrLicense_OfflineActivation(offlineActivationFilePath.c_str());
}

inline const char* ReturnCodeToString(const wrReturnCode& code)
{
    return wrReturnCode_Translate(code);
}

class PoseEstimatorConfigParams
{
  public:
    explicit PoseEstimatorConfigParams(const char* modelsDirectory)
    : m_impl(wrPoseEstimatorConfigParams_Create(modelsDirectory))
    {
        utils::CheckBadAlloc(m_impl.get());
    }
    PoseEstimatorConfigParams(PoseEstimatorConfigParams const& other)
    : m_impl(wrPoseEstimatorConfigParams_Clone(other.Get()))
    {
    }
    PoseEstimatorConfigParams(PoseEstimatorConfigParams&&) = default;
    PoseEstimatorConfigParams& operator=(PoseEstimatorConfigParams&&) = default;
    PoseEstimatorConfigParams& WithDeviceId(int deviceId)
    {
        wrPoseEstimatorConfigParams_SetDeviceId(m_impl.get(), deviceId);
        return *this;
    }
    PoseEstimatorConfigParams& WithModelName2d(const std::string& modelName2d)
    {
        wrPoseEstimatorConfigParams_SetModelName2D(m_impl.get(), modelName2d.c_str());
        return *this;
    }
    PoseEstimatorConfigParams& WithLicenseString(const std::string& licenseString)
    {
        wrPoseEstimatorConfigParams_SetLicenseString(m_impl.get(), licenseString.c_str());
        return *this;
    }
    PoseEstimatorConfigParams& WithLicensePath(const std::string& licensePath)
    {
        wrPoseEstimatorConfigParams_SetLicensePath(m_impl.get(), licensePath.c_str());
        return *this;
    }
    PoseEstimatorConfigParams& WithDeviceFingerprint(const std::string& deviceFingerprint)
    {
        wrPoseEstimatorConfigParams_SetDeviceFingerprint(m_impl.get(), deviceFingerprint.c_str());
        return *this;
    }
    PoseEstimatorConfigParams& WithLicenseServerURL(const std::string& licenseServerURL)
    {
        wrPoseEstimatorConfigParams_SetLicenseServerURL(m_impl.get(), licenseServerURL.c_str());
        return *this;
    }
    PoseEstimatorConfigParams& WithPoseParams(const PoseParams& params)
    {
        wrPoseEstimatorConfigParams_SetPoseParams(m_impl.get(), params.Get());
        return *this;
    }
    PoseEstimatorConfigParams& WithOutputFormat(const JointDefinition& outputFormat)
    {
        wrPoseEstimatorConfigParams_SetOutputFormat(m_impl.get(), outputFormat.Get());
        return *this;
    }
    PoseEstimatorConfigParams& With2dModelRegex(std::string const& modelRegexStr)
    {
        wrReturnCode const code
            = wrPoseEstimatorConfigParams_Set2dModelRegex(m_impl.get(), modelRegexStr.c_str());
        if (code != wrReturnCode_OK)
        {
            throw utils::Exception(code);
        }
        return *this;
    }

    std::string GetModelsDirectory() const
    {
        return std::string(wrPoseEstimatorConfigParams_GetModelsDirectory(m_impl.get()));
    }

    wrPoseEstimatorConfigParamsConst Get() const { return m_impl.get(); }

  private:
    std::unique_ptr<wrPoseEstimatorConfigParams_> m_impl;
};

class PoseEstimator
{
  public:
    template<class CPPType, class CType>
    class PoseIter
    {
      public:
        typedef ptrdiff_t difference_type;
        typedef CPPType value_type;

      public:
        PoseIter()
        : m_impl(nullptr)
        {
        }
        explicit PoseIter(CType const* pose)
        : m_impl(pose)
        {
        }
        PoseIter(const PoseIter& other)
        : m_impl(other.m_impl)
        {
        }
        PoseIter& operator=(PoseIter const& other)
        {
            m_impl = other.m_impl;
            return *this;
        }

        CPPType operator*() const { return CPPType(m_impl); }

        PoseIter& operator++()
        {
            m_impl = Next(m_impl);
            return *this;
        }
        PoseIter operator++(int)
        {
            PoseIter retval(*this);
            operator++();
            return retval;
        }
        PoseIter operator+(difference_type n) const { return PoseIter(Advance(m_impl, n)); }
        PoseIter& operator+=(difference_type n)
        {
            m_impl = Advance(m_impl, n);
            return *this;
        }
        PoseIter operator-(difference_type n) const { return operator+(-n); }
        PoseIter& operator-=(difference_type n) { return operator+=(-n); }

        difference_type operator-(PoseIter other) const { return Difference(m_impl, other.m_impl); }

        CPPType operator[](difference_type n) const { return *operator+(n); }

        bool operator==(PoseIter rhs) const { return m_impl == rhs.m_impl; }
        bool operator!=(PoseIter rhs) const { return m_impl != rhs.m_impl; }
        bool operator<(PoseIter rhs) const { return m_impl < rhs.m_impl; }
        bool operator>(PoseIter rhs) const { return m_impl > rhs.m_impl; }
        bool operator<=(PoseIter rhs) const { return m_impl <= rhs.m_impl; }
        bool operator>=(PoseIter rhs) const { return m_impl >= rhs.m_impl; }

      private:
        static wrPose2dHandleConst Next(wrPose2dHandleConst pose)
        {
            return wrPoseEstimator_GetPose2DNext(pose);
        }
        static wrPoseFaceHandleConst Next(wrPoseFaceHandleConst pose)
        {
            return wrPoseEstimator_GetFacePosesNext(pose);
        }
        static wrPoseHeadHandleConst Next(wrPoseHeadHandleConst pose)
        {
            return wrPoseEstimator_GetHeadPosesNext(pose);
        }
        static wrPose3dHandleConst Next(wrPose3dHandleConst pose)
        {
            return wrPoseEstimator_GetPose3DNext(pose);
        }
        static wrHandBoxPairHandleConst Next(wrHandBoxPairHandleConst pose)
        {
            return wrPoseEstimator_GetHandBoxesNext(pose);
        }

        static wrPose2dHandleConst Advance(wrPose2dHandleConst pose, difference_type n)
        {
            return wrPose2d_Advance(pose, n);
        }
        static wrPoseFaceHandleConst Advance(wrPoseFaceHandleConst pose, difference_type n)
        {
            return wrPoseFace_Advance(pose, n);
        }
        static wrPoseHeadHandleConst Advance(wrPoseHeadHandleConst pose, difference_type n)
        {
            return wrPoseHead_Advance(pose, n);
        }
        static wrPose3dHandleConst Advance(wrPose3dHandleConst pose, difference_type n)
        {
            return wrPose3d_Advance(pose, n);
        }
        static wrHandBoxPairHandleConst Advance(wrHandBoxPairHandleConst pose, difference_type n)
        {
            return wrHandBoxPair_Advance(pose, n);
        }

        static difference_type Difference(wrPose2dHandleConst minuend,
                                          wrPose2dHandleConst subtrahend)
        {
            return wrPose2d_Difference(minuend, subtrahend);
        }
        static difference_type Difference(wrPoseFaceHandleConst minuend,
                                          wrPoseFaceHandleConst subtrahend)
        {
            return wrPoseFace_Difference(minuend, subtrahend);
        }
        static difference_type Difference(wrPoseHeadHandleConst minuend,
                                          wrPoseHeadHandleConst subtrahend)
        {
            return wrPoseHead_Difference(minuend, subtrahend);
        }
        static difference_type Difference(wrPose3dHandleConst minuend,
                                          wrPose3dHandleConst subtrahend)
        {
            return wrPose3d_Difference(minuend, subtrahend);
        }
        static difference_type Difference(wrHandBoxPairHandleConst minuend,
                                          wrHandBoxPairHandleConst subtrahend)
        {
            return wrHandBoxPair_Difference(minuend, subtrahend);
        }

      private:
        CType const* m_impl;
    };

    PoseEstimator(PoseEstimator&&) = default;
    PoseEstimator& operator=(PoseEstimator&&) = default;

    PoseEstimator() = default;

    explicit PoseEstimator(wrPoseEstimatorHandle estimator)
    : m_impl(estimator)
    {
    }

    explicit PoseEstimator(const char* modelsDirectory)
    {
        wrPoseEstimator* ptr{ nullptr };
        wrReturnCode code = wrPoseEstimator_CreateDefault(&ptr, modelsDirectory);
        m_impl.reset(ptr);
        ExceptionHandler(code, modelsDirectory);
    }
    explicit PoseEstimator(const PoseEstimatorConfigParams& configData)
    {
        wrPoseEstimator* ptr{ nullptr };
        wrReturnCode code = wrPoseEstimator_CreateFromConfig(&ptr, configData.Get());
        m_impl.reset(ptr);
        ExceptionHandler(code, configData.GetModelsDirectory().c_str());
    }
    PoseEstimator(const char* modelsDirectory, const char* modelName2d)
    {
        wrPoseEstimator* ptr{ nullptr };
        wrReturnCode code
            = wrPoseEstimator_CreateDefaultFromModel(&ptr, modelsDirectory, modelName2d);
        m_impl.reset(ptr);
        ExceptionHandler(code, modelName2d);
    }
    PoseEstimator(const char* modelsDirectory,
                  int deviceId,
                  const PoseParams& params,
                  const JointDefinition& outputFormat)
    {
        wrPoseEstimator* ptr{ nullptr };
        wrReturnCode code = wrPoseEstimator_Create(
            &ptr, modelsDirectory, deviceId, params.Get(), outputFormat.Get());
        m_impl.reset(ptr);
        ExceptionHandler(code, modelsDirectory);
    }
    PoseEstimator(const char* modelsDirectory,
                  const char* modelName2d,
                  int deviceId,
                  const PoseParams& params,
                  const JointDefinition& outputFormat)
    {
        wrPoseEstimator* ptr{ nullptr };
        wrReturnCode code = wrPoseEstimator_CreateFromModel(
            &ptr, modelsDirectory, modelName2d, deviceId, params.Get(), outputFormat.Get());
        m_impl.reset(ptr);
        ExceptionHandler(code, modelName2d);
    }

    void Release() { m_impl.release(); }

    void Reset() { wrPoseEstimator_Reset(GetPtr()); }

    PoseEstimator Clone() const
    {
        wrReturnCode code;
        auto clone = Clone(code);
        ExceptionHandler(code);
        return clone;
    }

    PoseEstimator Clone(wrReturnCode& code) const
    {
        wrPoseEstimatorHandle cloneHandle;
        code = wrPoseEstimator_Clone(GetPtr(), &cloneHandle);
        ExceptionHandler(code);
        return PoseEstimator(cloneHandle);
    }

    PoseEstimator Clone(int deviceId) const
    {
        wrReturnCode code;
        auto clone = Clone(deviceId, code);
        ExceptionHandler(code);
        return clone;
    }

    PoseEstimator Clone(int deviceId, wrReturnCode& code) const
    {
        wrPoseEstimatorHandle cloneHandle;
        code = wrPoseEstimator_CloneOnDevice(GetPtr(), &cloneHandle, deviceId);
        return PoseEstimator(cloneHandle);
    }

    /// @brief Serialize a pose estimator to memory. Such serialized data may be persisted to
    ///        disk, and (under appropriate circumstances) deserialized using
    ///        `PoseEstimator::Deserialize`.
    /// @note Not supported on Apple platforms or with CPU-Only build.
    SerializedData Serialize() const
    {
        auto serializedEstimator = wrPoseEstimator_Serialize(GetPtr());
        if (serializedEstimator == nullptr)
        {
            throw utils::Exception(wrReturnCode_UNSUPPORTED_ON_PLATFORM);
        }
        return SerializedData{ serializedEstimator };
    }

    /// @brief Serialize the current results held in a pose estimator to JSON. Thre resulting
    ///         bytes are JSON parsable.
    SerializedData ResultsToJson() const
    {
        return SerializedData{ wrPoseEstimator_ResultsToJson(GetPtr()) };
    }

    /// @brief Attempt to deserialize a serialized pose estimator. Failures to deserialize
    ///         result in an exception being thrown. Note that the serializedData should be from a
    ///         pose estimator serialized on an identical device (including driver versions) using
    ///         `PoseEstimator::Serialize`. If this
    ///         fails to be the case, an exception is raised. In the successful case, any
    ///         properties that were set on the serialized estimator will hold to be true on a
    ///         deserialized estimator (for example, if hand models were initialized on the
    ///         serialized estimator, a successful call to `Deserialize` results in an estimator
    ///         with hand models initialized.
    /// @note Not supported on Apple platforms or with CPU-Only build.
    static PoseEstimator Deserialize(char const* data,
                                     int numBytes,
                                     int deviceId = 0,
                                     const std::string& licenseString = std::string(),
                                     const std::string& licensePath = std::string())
    {
        wrPoseEstimatorHandle c_poseEstimator = nullptr;
        wrReturnCode const code = wrPoseEstimator_DeserializeWithLicenseData(
            data,
            numBytes,
            deviceId,
            (licenseString.empty() ? nullptr : licenseString.c_str()),
            (licensePath.empty() ? nullptr : licensePath.c_str()),
            &c_poseEstimator);
        ExceptionHandlerStatic(code, c_poseEstimator);
        assert(c_poseEstimator != nullptr);
        return PoseEstimator(c_poseEstimator);
    }

    void Initialize(const char* modelsDirectory)
    {
        wrReturnCode code;
        Initialize(modelsDirectory, code);
        this->ExceptionHandler(code, modelsDirectory);
    }
    void Initialize(const char* modelsDirectory, wrReturnCode& code)
    {
        this->m_impl.reset();

        wrPoseEstimator* ptr{ nullptr };

        code = wrPoseEstimator_CreateDefault(&ptr, modelsDirectory);

        this->m_impl.reset(ptr);
    }
    void Initialize(const PoseEstimatorConfigParams& configData)
    {
        wrReturnCode code;
        Initialize(configData, code);
        this->ExceptionHandler(code, configData.GetModelsDirectory().c_str());
    }
    void Initialize(const PoseEstimatorConfigParams& configData, wrReturnCode& code)
    {
        this->m_impl.reset();

        wrPoseEstimator* ptr{ nullptr };

        code = wrPoseEstimator_CreateFromConfig(&ptr, configData.Get());

        this->m_impl.reset(ptr);
    }
    void Initialize(const char* modelsDirectory, const char* modelName2d)
    {
        wrReturnCode code;
        Initialize(modelsDirectory, modelName2d, code);
        this->ExceptionHandler(code, modelName2d);
    }
    void Initialize(const char* modelsDirectory, const char* modelName2d, wrReturnCode& code)
    {
        this->m_impl.reset();

        wrPoseEstimator* ptr{ nullptr };

        code = wrPoseEstimator_CreateDefaultFromModel(&ptr, modelsDirectory, modelName2d);

        this->m_impl.reset(ptr);
    }
    void Initialize(const char* modelsDirectory,
                    int deviceId,
                    const PoseParams& params,
                    const JointDefinition& outputFormat)
    {
        wrReturnCode code;
        Initialize(modelsDirectory, deviceId, params, outputFormat, code);
        this->ExceptionHandler(code,
                               std::string("Requested models directory ") + modelsDirectory
                                   + ", requested deviceId " + std::to_string(deviceId) + ".");
    }
    void Initialize(const char* modelsDirectory,
                    int deviceId,
                    const PoseParams& params,
                    const JointDefinition& outputFormat,
                    wrReturnCode& code)
    {
        wrPoseEstimator* ptr{ nullptr };
        code = wrPoseEstimator_Create(
            &ptr, modelsDirectory, deviceId, params.Get(), outputFormat.Get());
        this->m_impl.reset(ptr);
    }
    void Initialize(const char* modelsDirectory,
                    const char* modelName2d,
                    int deviceId,
                    const PoseParams& params,
                    const JointDefinition& outputFormat)
    {
        wrReturnCode code;
        Initialize(modelsDirectory, modelName2d, deviceId, params, outputFormat, code);
        this->ExceptionHandler(code, modelName2d);
    }
    void Initialize(const char* modelsDirectory,
                    const char* modelName2d,
                    int deviceId,
                    const PoseParams& params,
                    const JointDefinition& outputFormat,
                    wrReturnCode& code)
    {
        wrPoseEstimator* ptr = this->m_impl.release();
        code = wrPoseEstimator_CreateFromModel(
            &ptr, modelsDirectory, modelName2d, deviceId, params.Get(), outputFormat.Get());
        this->m_impl.reset(ptr);
    }

    void Reinitialize(const char* modelsDirectory)
    {
        wrReturnCode code;
        Reinitialize(modelsDirectory, code);
        this->ExceptionHandler(code, modelsDirectory);
    }
    void Reinitialize(const char* modelsDirectory, wrReturnCode& code)
    {
        wrPoseEstimator* ptr = this->m_impl.release();
        code = wrPoseEstimator_ReinitializeDefault(&ptr, modelsDirectory);
        this->m_impl.reset(ptr);
    }
    void Reinitialize(const PoseEstimatorConfigParams& configData)
    {
        wrReturnCode code;
        Reinitialize(configData, code);
        this->ExceptionHandler(code, configData.GetModelsDirectory().c_str());
    }
    void Reinitialize(const PoseEstimatorConfigParams& configData, wrReturnCode& code)
    {
        wrPoseEstimator* ptr = this->m_impl.release();
        code = wrPoseEstimator_ReinitializeFromConfig(&ptr, configData.Get());
        this->m_impl.reset(ptr);
    }
    void Reinitialize(const char* modelsDirectory, const char* modelName2d)
    {
        wrReturnCode code;
        Reinitialize(modelsDirectory, modelName2d, code);
        this->ExceptionHandler(code, modelName2d);
    }
    void Reinitialize(const char* modelsDirectory, const char* modelName2d, wrReturnCode& code)
    {
        wrPoseEstimator* ptr = this->m_impl.release();
        code = wrPoseEstimator_ReinitializeDefaultFromModel(&ptr, modelName2d, modelsDirectory);
        this->m_impl.reset(ptr);
    }

    void Reinitialize(const char* modelsDirectory,
                      int deviceId,
                      const PoseParams& params,
                      const JointDefinition& outputFormat)
    {
        wrReturnCode code;
        Reinitialize(modelsDirectory, deviceId, params, outputFormat, code);
        this->ExceptionHandler(code, modelsDirectory);
    }
    void Reinitialize(const char* modelsDirectory,
                      int deviceId,
                      const PoseParams& params,
                      const JointDefinition& outputFormat,
                      wrReturnCode& code)
    {
        wrPoseEstimator* ptr = this->m_impl.release();
        code = wrPoseEstimator_Reinitialize(
            &ptr, modelsDirectory, deviceId, params.Get(), outputFormat.Get());
        this->m_impl.reset(ptr);
    }
    void Reinitialize(const char* modelsDirectory,
                      const char* modelName2d,
                      int deviceId,
                      const PoseParams& params,
                      const JointDefinition& outputFormat)
    {
        wrReturnCode code;
        Reinitialize(modelsDirectory, modelName2d, deviceId, params, outputFormat, code);
        this->ExceptionHandler(code, modelsDirectory);
    }
    void Reinitialize(const char* modelsDirectory,
                      const char* modelName2d,
                      int deviceId,
                      const PoseParams& params,
                      const JointDefinition& outputFormat,
                      wrReturnCode& code)
    {
        wrPoseEstimator* ptr = this->m_impl.release();
        code = wrPoseEstimator_ReinitializeFromModel(
            &ptr, modelsDirectory, modelName2d, deviceId, params.Get(), outputFormat.Get());
        this->m_impl.reset(ptr);
    }

    /// @brief Query is pose estimator supports inverse kinematics
    /// @deprecated Inverse kinematics will always be supported. Function always returns true.
    static bool HasIK() { return wrPoseEstimator_HasIK() != 0; }

    wrHandSegmenterType GetHandSegmenterType() const
    {
        return wrPoseEstimator_GetHandSegmenterType(GetPtr());
    }

    wrTrackerKind GetTrackerKind() const { return wrPoseEstimator_GetTrackerKind(GetPtr()); }

    JointDefinition GetHuman2DOutputFormat() const
    {
        return JointDefinition(wrPoseEstimator_GetHuman2DOutputFormat(GetPtr()));
    }

    JointDefinition GetHuman3DOutputFormat() const
    {
        return JointDefinition(wrPoseEstimator_GetHuman3DOutputFormat(GetPtr()));
    }

    JointDefinition GetHuman3DRawOutputFormat() const
    {
        return JointDefinition(wrPoseEstimator_GetHuman3DRawOutputFormat(GetPtr()));
    }

    JointDefinition GetHandOutputFormat() const
    {
        return JointDefinition(wrPoseEstimator_GetHandOutputFormat(GetPtr()));
    }

    JointDefinition GetFaceOutputFormat() const
    {
        return JointDefinition(wrPoseEstimator_GetFaceOutputFormat(GetPtr()));
    }

    PoseParamsView GetParams() const { return PoseParamsView(wrPoseEstimator_GetParams(GetPtr())); }

    IKParamsView GetIKParams() const { return IKParamsView(wrPoseEstimator_GetIKParams(GetPtr())); }

    PoseIKView GetIKSolver(int ikSolverId)
    {
        return PoseIKView(wrPoseEstimator_GetIKSolver(GetPtr(), ikSolverId));
    }

    bool Is3dInitialized() const { return wrPoseEstimator_Is3dInitialized(GetPtr()) != 0; }

    bool IsHeadInitialized() const { return wrPoseEstimator_IsHeadInitialized(GetPtr()) != 0; }

    bool AreHandsInitialized() const { return wrPoseEstimator_AreHandsInitialized(GetPtr()) != 0; }

    bool SupportsMaskEstimation() const
    {
        return wrPoseEstimator_SupportsMaskEstimation(GetPtr()) != 0;
    }

    unsigned int GetMaxPersons() const { return wrPoseEstimator_GetMaxPersons(GetPtr()); }

    unsigned int GetNumHumans2D() const { return wrPoseEstimator_GetNumHumans2D(GetPtr()); }

    /// @brief	Obtain estimated 2D poses
    /// @see	Pose2d()
    /// @see	GetNumHumans2D()
    /// @param	outPoses a pointer to sufficient memory to store GetNumHumans2D()
    ///			Pose2D objects.
    void GetAllHumans2D(Pose2d* outPoses) const { GetHumans2DLimited(outPoses, GetNumHumans2D()); }

    // @brief	Obtain limited number of estimated 2D poses
    /// @see	Pose2d()
    /// @see	GetNumHumans2D()
    /// @param	outPoses a pointer to sufficient memory to store maxPersons
    ///			Pose2D objects.
    /// @param  maxPersons the maximum number of persons to be written in poses. If more than
    ///			this many persons have been detected in handle, the specific persons
    ///			returned is unspecified.
    /// @return	The number of poses stored in outPoses after return
    unsigned int GetHumans2DLimited(Pose2d* outPoses, unsigned int maxPersons) const
    {
        wrPose2dHandleConst begin = wrPoseEstimator_GetHumans2DBegin(GetPtr());
        wrPose2dHandleConst end = wrPoseEstimator_GetHumans2DEnd(GetPtr());

        unsigned int maxToWrite = std::min(maxPersons, GetNumHumans2D());
        unsigned int numWritten = 0;

        for (wrPose2dHandleConst pose = begin; numWritten < maxToWrite && pose < end;
             pose = wrPoseEstimator_GetPose2DNext(pose), numWritten++)
        {
            outPoses[numWritten].Copy(pose);
        }

        return numWritten;
    }

    unsigned int GetNumHands() const { return wrPoseEstimator_GetNumHands(GetPtr()); }
    unsigned int GetNumLeftHands() const { return wrPoseEstimator_GetNumLeftHands(GetPtr()); }
    unsigned int GetNumRightHands() const { return wrPoseEstimator_GetNumRightHands(GetPtr()); }

    unsigned int GetNumHeads() const { return wrPoseEstimator_GetNumHeads(GetPtr()); }

    /// @brief	Obtain estimated head poses
    /// @see	PoseHead()
    /// @see	GetNumHeads()
    /// @param	outPoses a pointer to sufficient memory to store GetNumHeads()
    ///			PoseHead objects.
    void GetAllHeadPoses(PoseHead* outPoses) const { GetHeadPosesLimited(outPoses, GetNumHeads()); }

    /// @brief	Obtain estimated head poses
    /// @see	PoseHead()
    /// @see	GetNumHeads()
    /// @param	outPoses a pointer to sufficient memory to store maxHeads
    ///			PoseHead objects.
    /// @param  maxHeads the maximum number of head poses to be stored in poses. If this number
    ///         is greater than the number of poses actually stored in handle, the specific
    ///         poses written to poses is unspecified
    /// @return	The number of head poses stored in outPoses after return
    unsigned int GetHeadPosesLimited(PoseHead* outPoses, unsigned int maxHeads) const
    {
        wrPoseHeadHandleConst begin = wrPoseEstimator_GetHeadPosesBegin(GetPtr());
        wrPoseHeadHandleConst end = wrPoseEstimator_GetHeadPosesEnd(GetPtr());

        unsigned int maxToWrite = std::min(maxHeads, GetNumHeads());
        unsigned int numWritten = 0;

        for (wrPoseHeadHandleConst pose = begin; numWritten < maxToWrite && pose < end;
             pose = wrPoseEstimator_GetHeadPosesNext(pose), numWritten++)
        {
            outPoses[numWritten].Copy(pose);
        }

        return numWritten;
    }

    unsigned int GetNumPoseFace() const { return wrPoseEstimator_GetNumPoseFace(GetPtr()); }
    /// @brief	Obtain estimated facial features
    /// @see	PoseFace()
    /// @see	GetNumPoseFace()
    /// @param	outPoseFace a pointer to sufficient memory to store GetNumPoseFace()
    ///			PoseFace objects.
    void GetAllPoseFace(PoseFace* outPoseFace) const
    {
        GetPoseFaceLimited(outPoseFace, GetNumPoseFace());
    }

    /// @brief	Obtain estimated facial features
    /// @see	PoseFace()
    /// @see	GetNumPoseFace()
    /// @param	outFacePoses pointer to sufficient memory to store maxFacePoses
    ///			PoseFace objects.
    /// @param  maxFacePoses the maximum number of facial features to be stored in outFacePoses.
    ///         If this number is greater than the number of poses actually stored in handle, the
    ///         specific poses written to poses is unspecified
    /// @return	The number of facial features stored in outFacePoses after return
    unsigned int GetPoseFaceLimited(PoseFace* outFacePoses, unsigned int maxFacePoses) const
    {
        wrPoseFaceHandleConst begin = wrPoseEstimator_GetFacePosesBegin(GetPtr());
        wrPoseFaceHandleConst end = wrPoseEstimator_GetFacePosesEnd(GetPtr());

        unsigned int maxToWrite = std::min(maxFacePoses, GetNumPoseFace());
        unsigned int numWritten = 0;

        for (wrPoseFaceHandleConst pose = begin; numWritten < maxToWrite && pose < end;
             pose = wrPoseEstimator_GetFacePosesNext(pose), numWritten++)
        {
            outFacePoses[numWritten].Copy(pose);
        }

        return numWritten;
    }

    void GetMaskDims(int& outMaskWidth, int& outMaskHeight, int& outMaskDepth) const
    {
        wrPoseEstimator_GetMaskDims(GetPtr(), &outMaskWidth, &outMaskHeight, &outMaskDepth);
    }

    /// @deprecated This function is deprecated. Instead, use PoseEstimator::GetMaskView
    /// @see        PoseEstimator::GetMaskView
    /// @brief	Obtain estimated masks
    /// @param  outMask a pointer to at least maskWidth * maskHeight * maskDepth bytes
    ///         to be filled out with segmentation mask data
    /// @see	GetMaskDims
    ///	@note
    ///         -# The dimensions of the mask may not be the same as the input image.
    ///         -# Mask data is stored as four 8-bit single channel images in row-major
    ///            oder whose values represent the confidence that a human is present in
    ///            that pixel.  The four generated masks are stored contiguously, as follows:
    ///            (0) body; (1) right hand; (2) left hand; and (3) both hands.
    void GetMask(unsigned char* outMask) const { wrPoseEstimator_GetMask(GetPtr(), outMask); }

    /// @brief	Obtain estimated masks
    /// @see	GetMaskDims
    /// @return	pointer valid for  width*height*depth bytes storing the mask data, valid for
    ///         at least maskWidth * maskHeight * maskDepth bytes
    ///	@note
    ///         -# The dimensions of the mask may not be the same as the input image.
    ///         -# Mask data is stored as four 8-bit single channel images in row-major
    ///            oder whose values represent the confidence that a human is present in
    ///            that pixel.  The four generated masks are stored contiguously, as follows:
    ///            (0) body; (1) right hand; (2) left hand; and (3) both hands.
    const unsigned char* GetMaskView() const { return wrPoseEstimator_GetMaskView(GetPtr()); }


    unsigned int GetNumHumans3D() const { return wrPoseEstimator_GetNumHumans3D(GetPtr()); }

    /// @brief	Obtain estimated 3D poses
    /// @see	Pose3d()
    /// @see	GetNumHumans3D()
    /// @param	outPoses a pointer to sufficient memory to store GetNumHumans3D()
    ///			Pose3d objects.
    void GetAllHumans3D(Pose3d* outPoses) const { GetHumans3DLimited(outPoses, GetNumHumans3D()); }

    /// @brief	Obtain estimated 3D poses
    /// @see	Pose3d()
    /// @see	GetNumHumans3D()
    /// @param	outPoses a pointer to sufficient memory to store maxPersons
    ///			Pose3d objects.
    /// @param  maxPersons the maximum number of persons to be stored in poses. If this number
    ///         is greater than the number of poses actually stored in handle, the specific
    ///         poses written to poses is unspecified
    /// return  the number of poses actually written
    unsigned int GetHumans3DLimited(Pose3d* outPoses, unsigned int maxPersons) const
    {
        wrPose3dHandleConst begin = wrPoseEstimator_GetHumans3DBegin(GetPtr());
        wrPose3dHandleConst end = wrPoseEstimator_GetHumans3DEnd(GetPtr());

        unsigned int maxToWrite = std::min(maxPersons, GetNumHumans3D());
        unsigned int numWritten = 0;

        for (wrPose3dHandleConst pose = begin; numWritten < maxToWrite && pose < end;
             pose = wrPoseEstimator_GetPose3DNext(pose), numWritten++)
        {
            outPoses[numWritten].Copy(pose);
        }

        return numWritten;
    }

    unsigned int GetNumRawHumans3D() const { return wrPoseEstimator_GetNumRawHumans3D(GetPtr()); }

    /// @brief	Obtain estimated 3D poses in xyz coordinates
    /// @see	Pose3d()
    /// @see	GetNumRawHumans3D()
    /// @param	outPoses a pointer to sufficient memory to store GetNumRawHumans3D()
    ///			Pose3d objects.
    void GetAllRawHumans3D(Pose3d* outPoses) const
    {
        GetRawHumans3DLimited(outPoses, GetNumRawHumans3D());
    }

    /// @brief	Obtain estimated 3D poses in xyz coordinates
    /// @see	Pose3d()
    /// @see	GetNumRawHumans3D()
    /// @param	outPoses a pointer to sufficient memory to store maxPersons
    ///			Pose3d objects.
    /// @param  maxPersons the maximum number of persons to be written in poses. If more than
    ///			this many persons have been detected in handle, the specific persons
    ///			returned is unspecified.
    /// @return The number of poses stored in outPoses after return
    unsigned int GetRawHumans3DLimited(Pose3d* outPoses, unsigned int maxPersons) const
    {
        wrPose3dHandleConst begin = wrPoseEstimator_GetRawHumans3DBegin(GetPtr());
        wrPose3dHandleConst end = wrPoseEstimator_GetRawHumans3DEnd(GetPtr());

        unsigned int maxToWrite = std::min(maxPersons, GetNumRawHumans3D());
        unsigned int numWritten = 0;

        for (wrPose3dHandleConst pose = begin; numWritten < maxToWrite && pose < end;
             pose = wrPoseEstimator_GetPose3DNext(pose), numWritten++)
        {
            outPoses[numWritten].Copy(pose);
        }

        return numWritten;
    }

    unsigned int GetNumHandBoxes() const { return wrPoseEstimator_GetNumHandBoxes(GetPtr()); }
    /// @brief	Obtain estimated handboxes
    /// @see	HandBoxPair()
    /// @see	GetNumHumans2D()
    /// @param	outBoxes a pointer to sufficient memory to store GetNumHumans2D() HandBoxPair
    ///			bounding boxes.
    /// @return The number of hand boxes stored in outBoxes after return
    void GetAllHandBoxes(HandBoxPair* outBoxes) const
    {
        GetHandBoxesLimited(outBoxes, GetNumHandBoxes());
    }

    /// @brief	Obtain estimated handboxes
    /// @see	HandBoxPair()
    /// @see	GetNumHumans2D()
    /// @param	outBoxes a pointer to sufficient memory to store maxHandBoxPairs HandBoxPair
    ///			bounding boxes.
    /// @param  maxHandBoxPairs the maximum number of hand box pairs to be stored in outBoxes.
    ///         If this number is greater than the number of hand box pairs actually stored in
    ///         handle, the specific boxes written is unspecified
    /// @return The number of hand boxes stored in outBoxes after return
    unsigned int GetHandBoxesLimited(HandBoxPair* outBoxes, unsigned int maxHandBoxPairs) const
    {
        wrHandBoxPairHandleConst begin = wrPoseEstimator_GetHandBoxesBegin(GetPtr());
        wrHandBoxPairHandleConst end = wrPoseEstimator_GetHandBoxesEnd(GetPtr());

        unsigned int maxToWrite = std::min(maxHandBoxPairs, GetNumHandBoxes());
        unsigned int numWritten = 0;

        for (wrHandBoxPairHandleConst pose = begin; numWritten < maxToWrite && pose < end;
             pose = wrPoseEstimator_GetHandBoxesNext(pose), numWritten++)
        {
            outBoxes[numWritten].Copy(pose);
        }

        return numWritten;
    }

    Pose3dView GetTPose3D(int id) const
    {
        return Pose3dView(wrPoseEstimator_GetTPose3D(GetPtr(), id));
    }
    void GetDefaultTPose3D(Pose3d& poseOut) const
    {
        wrPoseEstimator_GetDefaultTPose3D(GetPtr(), poseOut.Get());
    }
    unsigned int GetInputWidth() const { return wrPoseEstimator_GetInputWidth(GetPtr()); }
    unsigned int GetInputHeight() const { return wrPoseEstimator_GetInputHeight(GetPtr()); }
    float GetIKProperty(int prop, int id) const
    {
        return wrPoseEstimator_GetIKProperty(GetPtr(), prop, id);
    }
    unsigned int GetNumIKSolvers() const { return wrPoseEstimator_GetNumIKSolvers(GetPtr()); }

    wrPoseEstimator const* GetPtr() const { return m_impl.get(); }
    wrPoseEstimator const* Get() const { return GetPtr(); }

    /* Available PoseEstimator Iterators */
    typedef PoseIter<Pose2dView, wrPose2d> Humans2dIter;
    typedef PoseIter<Pose2dView, wrPose2d> PoseHandsIter;
    typedef PoseIter<PoseFaceView, wrPoseFace> PoseFaceIter;
    typedef PoseIter<PoseHeadView, wrPoseHead> PoseHeadIter;
    typedef PoseIter<Pose3dView, wrPose3d> Humans3dIter;
    typedef PoseIter<Pose3dView, wrPose3d> Humans3dRawIter;
    typedef PoseIter<HandBoxPairView, wrHandBoxPair> HandBoxIter;

    template<class PoseIterType>
    class PoseTypeRange
    {
      public:
        typedef std::size_t size_type;
        typedef typename PoseIterType::value_type value_type;

      public:
        explicit PoseTypeRange(PoseIterType rangeBegin, PoseIterType rangeEnd)
        : m_begin(rangeBegin)
        , m_end(rangeEnd)
        {
        }

        PoseTypeRange(const PoseTypeRange& other)
        : m_begin(other.m_begin)
        , m_end(other.m_end)
        {
        }

        PoseIterType begin() const { return m_begin; }

        PoseIterType end() const { return m_end; }

        value_type operator[](size_type n) const { return m_begin[n]; }

        size_type size() const { return m_end - m_begin; }

      private:
        PoseIterType m_begin;
        PoseIterType m_end;
    };

    typedef PoseTypeRange<Humans2dIter> Humans2dRange;
    typedef PoseTypeRange<PoseHandsIter> HandsRange;
    typedef PoseTypeRange<PoseFaceIter> FaceRange;
    typedef PoseTypeRange<PoseHeadIter> HeadRange;
    typedef PoseTypeRange<Humans3dIter> Humans3dRange;
    typedef PoseTypeRange<Humans3dRawIter> Humans3dRawRange;
    typedef PoseTypeRange<HandBoxIter> HandBoxRange;

    Humans2dIter Humans2dBegin() const
    {
        return Humans2dIter(wrPoseEstimator_GetHumans2DBegin(GetPtr()));
    }
    Humans2dIter Humans2dEnd() const
    {
        return Humans2dIter(wrPoseEstimator_GetHumans2DEnd(GetPtr()));
    }
    Humans2dRange Humans2d() const { return Humans2dRange(Humans2dBegin(), Humans2dEnd()); }

    PoseHandsIter LeftHandsBegin() const
    {
        return PoseHandsIter(wrPoseEstimator_LeftHandsBegin(GetPtr()));
    }
    PoseHandsIter LeftHandsEnd() const
    {
        return PoseHandsIter(wrPoseEstimator_LeftHandsEnd(GetPtr()));
    }
    PoseHandsIter RightHandsBegin() const
    {
        return PoseHandsIter(wrPoseEstimator_RightHandsBegin(GetPtr()));
    }
    PoseHandsIter RightHandsEnd() const
    {
        return PoseHandsIter(wrPoseEstimator_RightHandsEnd(GetPtr()));
    }
    HandsRange LeftHands() const { return HandsRange(LeftHandsBegin(), LeftHandsEnd()); }
    HandsRange RightHands() const { return HandsRange(RightHandsBegin(), RightHandsEnd()); }

    PoseFaceIter PoseFacesBegin() const
    {
        return PoseFaceIter(wrPoseEstimator_GetFacePosesBegin(GetPtr()));
    }
    PoseFaceIter PoseFacesEnd() const
    {
        return PoseFaceIter(wrPoseEstimator_GetFacePosesEnd(GetPtr()));
    }
    FaceRange Faces() const { return FaceRange(PoseFacesBegin(), PoseFacesEnd()); }

    PoseHeadIter PoseHeadsBegin() const
    {
        return PoseHeadIter(wrPoseEstimator_GetHeadPosesBegin(GetPtr()));
    }
    PoseHeadIter PoseHeadsEnd() const
    {
        return PoseHeadIter(wrPoseEstimator_GetHeadPosesEnd(GetPtr()));
    }
    HeadRange Heads() const { return HeadRange(PoseHeadsBegin(), PoseHeadsEnd()); }

    Humans3dIter Humans3dBegin() const
    {
        return Humans3dIter(wrPoseEstimator_GetHumans3DBegin(GetPtr()));
    }
    Humans3dIter Humans3dEnd() const
    {
        return Humans3dIter(wrPoseEstimator_GetHumans3DEnd(GetPtr()));
    }
    Humans3dRange Humans3d() const { return Humans3dRange(Humans3dBegin(), Humans3dEnd()); }

    Humans3dRawIter RawHumans3dBegin() const
    {
        return Humans3dRawIter(wrPoseEstimator_GetRawHumans3DBegin(GetPtr()));
    }
    Humans3dRawIter RawHumans3dEnd() const
    {
        return Humans3dRawIter(wrPoseEstimator_GetRawHumans3DEnd(GetPtr()));
    }
    Humans3dRawRange Humans3dRaw() const
    {
        return Humans3dRawRange(RawHumans3dBegin(), RawHumans3dEnd());
    }

    HandBoxIter HandBoxesBegin() const
    {
        return HandBoxIter(wrPoseEstimator_GetHandBoxesBegin(GetPtr()));
    }
    HandBoxIter HandBoxesEnd() const
    {
        return HandBoxIter(wrPoseEstimator_GetHandBoxesEnd(GetPtr()));
    }
    HandBoxRange HandBoxes() const { return HandBoxRange(HandBoxesBegin(), HandBoxesEnd()); }

    bool HasTracker() const { return wrPoseEstimator_HasTracker(GetPtr()) != 0; }
    wrIdState GetIdState(int id) const { return wrPoseEstimator_GetIdState(GetPtr(), id); }
    void ClearTrackerMemory() { wrPoseEstimator_ClearTrackerMemory(GetPtr()); }

    wrPoseEstimator* Get() { return GetPtr(); }
    wrPoseEstimator* GetPtr() { return this->m_impl.get(); }

    /// @brief	Process a frame to generate pose estimation data which is stored internally
    ///			in the wrPoseEstimator.  The data can be accessed via the GetAll*
    ///			and GetNum* functions defined in this class
    /// @see	PoseEstimatorOptions
    /// @see	wrReturnCode
    /// @see	ProcessFrame(const unsigned char* bgrData,int width,int height,const
    /// PoseEstimatorOptions& options,wrReturnCode& code)
    /// @param	bgrData a pointer to the frame colour data which must be in 3-channel
    ///			openCV format, i.e., row-major sequence of [b1,g1,r1,b2,g2,r2, ... ]
    ///			intensity values in the range [0,255]
    /// @param	width frame width in pixels. The behavior is undefined unless width is
    ///         (strictly) greater than zero.
    /// @param	height frame height in pixels. The behavior is undefined unless height is
    ///         (strictly) greater than zero.
    /// @param	options the selected PoseEstimatorOptions(), e.g. whether to estimate hands,
    ///			heads, 3D, etc., which will be used by the PoseEstimator in processing the frame
    void ProcessFrame(const unsigned char* bgrData,
                      int width,
                      int height,
                      const PoseEstimatorOptions& options)
    {
        wrReturnCode code;
        ProcessFrame(bgrData, width, height, options, code);
        this->ExceptionHandler(code);
    }

    void ProcessFrame(const unsigned char* bgrData,
                      int width,
                      int height,
                      const PoseEstimatorOptions& options,
                      wrReturnCode& code)
    {
        code = wrPoseEstimator_ProcessFrame(GetPtr(), bgrData, width, height, options.Get());
    }

    /// @brief	Process a grayscale frame to generate pose estimation data which is stored
    ///         internally in the wrPoseEstimator.  The data can be accessed via the GetAll*
    ///			and GetNum* functions defined in this class
    /// @see	PoseEstimatorOptions
    /// @see	wrReturnCode
    /// @see	ProcessFrameGrayScale(const unsigned char* grayData,int width,int height,const
    /// PoseEstimatorOptions& options,wrReturnCode& code)
    /// @param	grayData a pointer to the frame grayscale data which must be in 1-channel
    ///			openCV format, i.e., row-major sequence  with intensity values in the range
    ///         [0,255]
    /// @param	width frame width in pixels. The behavior is undefined unless width is
    ///         (strictly) greater than zero.
    /// @param	height frame height in pixels. The behavior is undefined unless height is
    ///         (strictly) greater than zero.
    /// @param	options the selected PoseEstimatorOptions(), e.g. whether to estimate hands,
    ///			heads, 3D, etc., which will be used by the PoseEstimator in processing the frame
    void ProcessFrameGrayScale(const unsigned char* grayData,
                               int width,
                               int height,
                               const PoseEstimatorOptions& options)
    {
        wrReturnCode code;
        ProcessFrameGrayScale(grayData, width, height, options, code);
        this->ExceptionHandler(code);
    }

    void ProcessFrameGrayScale(const unsigned char* grayData,
                               int width,
                               int height,
                               const PoseEstimatorOptions& options,
                               wrReturnCode& code)
    {
        code = wrPoseEstimator_ProcessFrameGrayScale(
            GetPtr(), grayData, width, height, options.Get());
    }

    void Initialize3D(const IKParams& params, std::string const& modelsDirectory)
    {
        wrReturnCode code;
        Initialize3D(params, code, modelsDirectory.c_str());
        this->ExceptionHandler(code);
    }

    void Initialize3D(const IKParams& params,
                      wrReturnCode& code,
                      std::string const& modelsDirectory)
    {
        code = wrPoseEstimator_Initialize3D(GetPtr(), params.Get(), modelsDirectory.c_str());
    }

    void InitializeHands2D(std::string const& modelsDirectory)
    {
        wrReturnCode code;
        InitializeHands2D(code, modelsDirectory.c_str());
        this->ExceptionHandler(code);
    }

    void InitializeHands2D(wrReturnCode& code, std::string const& modelsDirectory)
    {
        code = wrPoseEstimator_InitializeHands2D(GetPtr(), modelsDirectory.c_str());
    }

    void SetHandSegmenter(wrHandSegmenterType type)
    {
        wrPoseEstimator_SetHandSegmenter(GetPtr(), type);
    }

    void Initialize3D(std::string const& modelsDirectory)
    {
        wrReturnCode code;
        Initialize3D(code, modelsDirectory.c_str());
        this->ExceptionHandler(code);
    }
    void Initialize3D(wrReturnCode& code, std::string const& modelsDirectory)
    {
        code = wrPoseEstimator_Initialize3D(GetPtr(), nullptr, modelsDirectory.c_str());
    }

    void InitializeHead(std::string const& modelsDirectory)
    {
        wrReturnCode code;
        InitializeHead(modelsDirectory, code);
        this->ExceptionHandler(code);
    }
    void InitializeHead(std::string const& modelsDirectory, wrReturnCode& code)
    {
        code = wrPoseEstimator_InitializeHeadDefault(GetPtr(), modelsDirectory.c_str());
    }

    void InitializeHead(float betaX,
                        float betaY,
                        float minFreqCutoffPosition,
                        float freqCutoffVelocity,
                        std::string const& modelsDirectory)
    {
        wrReturnCode code;
        InitializeHead(
            betaX, betaY, minFreqCutoffPosition, freqCutoffVelocity, modelsDirectory, code);
        this->ExceptionHandler(code);
    }
    void InitializeHead(float betaX,
                        float betaY,
                        float minFreqCutoffPosition,
                        float freqCutoffVelocity,
                        std::string const& modelsDirectory,
                        wrReturnCode& code)
    {
        code = wrPoseEstimator_InitializeHead(GetPtr(),
                                              betaX,
                                              betaY,
                                              minFreqCutoffPosition,
                                              freqCutoffVelocity,
                                              modelsDirectory.c_str());
    }

    void SetParams(const PoseParams& params) { wrPoseEstimator_SetParams(GetPtr(), params.Get()); }
    void SetIKProperty(int prop, float value, int id)
    {
        wrPoseEstimator_SetIKProperty(GetPtr(), prop, value, id);
    }

  private:
    void ExceptionHandler(wrReturnCode code) const { ExceptionHandler(code, ""); }
    void ExceptionHandler(wrReturnCode code, std::string const& msg) const
    {
        ExceptionHandler(code, msg.c_str());
    }
    void ExceptionHandler(wrReturnCode code, const char* msg) const
    {
        ExceptionHandlerStatic(code, msg, GetPtr());
    }
    static void ExceptionHandlerStatic(wrReturnCode code, wrPoseEstimator const* c_poseEstimator)
    {
        ExceptionHandlerStatic(code, "", c_poseEstimator);
    }
    static void ExceptionHandlerStatic(wrReturnCode code,
                                       const char* msg,
                                       wrPoseEstimator const* c_poseEstimator)
    {
        switch (code)
        {
            case wrReturnCode_OK:
                break;
            case wrReturnCode_BAD_ALLOC:
                utils::CheckBadAlloc(c_poseEstimator);
                break;
            case wrReturnCode_LICENSE_ERROR_ACTIVATION:
            case wrReturnCode_LICENSE_EXPIRED:
            case wrReturnCode_LICENSE_REVOKED:
            case wrReturnCode_LICENSE_ERROR_IO:
            case wrReturnCode_LICENSE_ERROR_MALFORMED:
            case wrReturnCode_LICENSE_ERROR_INVALID_OFFLINE:
            case wrReturnCode_LICENSE_ERROR_INVALID_TYPE:
            case wrReturnCode_LICENSE_ERROR_VERSION:
            case wrReturnCode_LICENSE_ERROR_HOST:
            case wrReturnCode_LICENSE_ERROR_NOT_FOUND:
            case wrReturnCode_LICENSE_ERROR_OS:
            case wrReturnCode_LICENSE_SUSPENDED:
            case wrReturnCode_LICENSE_ERROR_VM:
            case wrReturnCode_LICENSE_ERROR_COUNTRY:
            case wrReturnCode_LICENSE_ERROR_IP:
            case wrReturnCode_LICENSE_ERROR_SYS_TIME:
            case wrReturnCode_LICENSE_ERROR_FINGERPRINT_CHANGED:
            case wrReturnCode_LICENSE_ERROR_FINGERPRINT_INVALID:
            case wrReturnCode_LICENSE_ERROR_ACTIVATION_LIMIT:
            case wrReturnCode_LICENSE_ERROR_FLOAT_LIMIT:
            case wrReturnCode_LICENSE_ERROR_LEASE:
            case wrReturnCode_LICENSE_ERROR_SERVER_ERROR:
            case wrReturnCode_LICENSE_ERROR_SERVER_INVALID:
            case wrReturnCode_LICENSE_ERROR_SERVER_LICENSE_INVALID:
            case wrReturnCode_LICENSE_ERROR_SERVER_LICENSE_SUSPENDED:
            case wrReturnCode_LICENSE_ERROR_SERVER_LICENSE_EXPIRED:
            case wrReturnCode_LICENSE_ERROR_PERMISSIONS:
            case wrReturnCode_LICENSE_ERROR_OTHER:
                throw utils::LicenseError(code);
            case wrReturnCode_PATH_ERROR:
            case wrReturnCode_HEAD_NOT_INITIALIZED:
            case wrReturnCode_HANDS_NOT_INITIALIZED:
            case wrReturnCode_HAND_SEGMENTER_NOT_SET:
            case wrReturnCode_3D_NOT_INITIALIZED:
            case wrReturnCode_INCOMPATIBLE_POSE_ESTIMATOR_REQUIREMENTS:
            case wrReturnCode_DESERIALIZE_ERROR:
            case wrReturnCode_NO_SUCH_GPU:
            case wrReturnCode_INCOMPATIBLE_WRNCH_VERSION:
            case wrReturnCode_INCOMPATIBLE_CUDA_COMPUTE_CAPABILITY:
            case wrReturnCode_INCOMPATIBLE_TENSORRT_VERSION:
            case wrReturnCode_INVALID_REGEX:
            case wrReturnCode_NO_MATCHING_MODEL:
            case wrReturnCode_PLUGIN_LOAD_ERROR:
                throw utils::InitializationError(code);
            default:  // wrReturnCode_NO_MODELS, wrReturnCode_MODEL_LOAD_FAILED,
                      // wrReturnCode_OTHER_ERROR
                throw utils::Exception(code, std::string(msg));
        }
    }


  private:
    std::unique_ptr<wrPoseEstimator> m_impl;
};

inline bool operator==(PoseEstimator const& poseEstimator, std::nullptr_t) noexcept
{
    return poseEstimator.GetPtr() == nullptr;
}
inline bool operator==(std::nullptr_t, PoseEstimator const& poseEstimator) noexcept
{
    return poseEstimator.GetPtr() == nullptr;
}
inline bool operator!=(PoseEstimator const& poseEstimator, std::nullptr_t) noexcept
{
    return poseEstimator.GetPtr() != nullptr;
}
inline bool operator!=(std::nullptr_t, PoseEstimator const& poseEstimator) noexcept
{
    return poseEstimator.GetPtr() != nullptr;
}
}  // namespace wrnch
#endif /* WRNCH_POSE_ESTIMATOR_CXX_API */
