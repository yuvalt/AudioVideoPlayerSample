/*
 Copyright (c) 2019 wrnch Inc.
 All rights reserved
*/
#ifndef WRNCH_ACTIVITY_MODEL_CXX_API
#define WRNCH_ACTIVITY_MODEL_CXX_API

#include <wrnch/activityModel.h>
#include <wrnch/activityPoseEstimatorRequirements.hpp>
#include <wrnch/individualActivityModel.hpp>

/// @file activityModel.hpp

namespace std
{
template<>
struct default_delete<wrActivityModel>
{
    void operator()(wrActivityModel* c_activityModel) const
    {
        wrActivityModel_Destroy(c_activityModel);
    }
};

template<>
struct default_delete<wrActivityModelBuilder>
{
    void operator()(wrActivityModelBuilder* c_activityModelBuilder) const
    {
        wrActivityModelBuilder_Destroy(c_activityModelBuilder);
    }
};
}  // namespace std

namespace wrnch
{
/// @brief `ActivityModel` RAII C++ wrapper around the (incomplete) C type `wrActivityModel`.
///         Estimates pose-based activities (or "gestures")
///         for people ("individuals") over time. The underlying model is usually temporal, and
///         requires processing of poses through time for best behavior. Holds a collection of
///         per-person temporal models (instances of `IndividualActivityModel`), accessed through
///         the `IndividualModel(int personId)` member function.
/// @see `IndividualActivityModel`
/// @see individualActivityModel.h
/// @see activityModel.h
class ActivityModel
{
  public:
    ActivityModel(ActivityModel&&) = default;
    ActivityModel& operator=(ActivityModel&&) = default;
    ~ActivityModel() = default;

    /// @brief Return the number of classes estimated by the activity model
    int NumClasses() const;

    /// @brief Return a vector of strings containing the class names estimated by the activity
    ///         model.
    std::vector<std::string> ClassNames() const;

    /// @brief Attempt to access the individual activity model of a given person. If no such person
    ///         is estimated by `activityModel`, return an empty view (which is compares equal to
    ///         `nullptr` under `operator==`). Generally speaking, this should only be
    ///         called on `personId` found by the last call to ProcessFrame the relevant
    ///         poseEstimator.
    IndividualActivityModelView IndividualModel(int personId) const;

    /// @brief Return the number of individual models held by the activity model
    int NumIndividualModels() const;

    /// @brief semantically equivalent to:
    ///         `for (personId const : activityModel.PersonIds()) { visitFun(personId, userData); }`
    void VisitPersonIds(void (*visitFun)(int personId, void* userData), void* userData) const;

    /// @brief Return a list of personIds that have been estimated by this activity model. Note that
    ///         some of these personIds may not have been seen for many frames.
    std::vector<int> PersonIds() const;

    /// @brief Process the poses stored in `poseEstimator`, rolling the state of the per-person
    ///         (IndividualActivityModel) submodels forward.
    /// @param poseEstimator the pose estimator storing poses to be processed by `this`.
    /// @param imageWidth the width of the image last processed by `poseEstimator`
    /// @param imageHeight the height of the image last processed by `poseEstimator`
    void ProcessPoses(PoseEstimator const& poseEstimator, int imageWidth, int imageHeight);

    wrActivityModel const* Get() const;
    wrActivityModel const* GetPtr() const;
    wrActivityModel* Get();
    wrActivityModel* GetPtr();

    friend class Builder;

    class Builder
    {
      public:
        explicit Builder(std::string const& modelPath);
        Builder(Builder&&) = default;
        Builder& operator=(Builder&&) = default;
        ~Builder() = default;

        Builder& SetModelPath(std::string const& modelPath);
        Builder& SetDeviceId(int deviceId);
        Builder& SetNumModels(int numModels);

        char const* ModelPath() const;
        int DeviceId() const;
        int NumModels() const;

        ActivityModel Build() const;
        ActivityModel BuildReflected() const;

        ActivityPoseEstimatorRequirements PoseEstimatorRequirements() const;

        wrActivityModelBuilder* Get();
        wrActivityModelBuilder const* Get() const;

      private:
        std::unique_ptr<wrActivityModelBuilder> m_impl;
    };

  private:
    explicit ActivityModel(wrActivityModel* impl)
    : m_impl{ impl }
    {
    }

  private:
    std::unique_ptr<wrActivityModel> m_impl;
};

inline bool operator==(ActivityModel model, std::nullptr_t) noexcept
{
    return model.GetPtr() == nullptr;
}
inline bool operator==(std::nullptr_t, ActivityModel model) noexcept
{
    return model.GetPtr() == nullptr;
}
inline bool operator!=(ActivityModel model, std::nullptr_t) noexcept
{
    return model.GetPtr() != nullptr;
}
inline bool operator!=(std::nullptr_t, ActivityModel model) noexcept
{
    return model.GetPtr() != nullptr;
}

inline int ActivityModel::NumClasses() const { return wrActivityModel_NumClasses(Get()); }

inline std::vector<std::string> ActivityModel::ClassNames() const
{
    int const numClasses = NumClasses();

    std::vector<std::string> result(numClasses);
    std::vector<char const*> intermediateResult(numClasses);

    wrActivityModel_ClassNames(Get(), intermediateResult.data());

    for (int ix = 0; ix < numClasses; ++ix)
    {
        result[ix] = intermediateResult[ix];
    }

    return result;
}

inline IndividualActivityModelView ActivityModel::IndividualModel(int personId) const
{
    return IndividualActivityModelView{ wrActivityModel_IndividualModel(Get(), personId) };
}

inline int ActivityModel::NumIndividualModels() const
{
    return wrActivityModel_NumIndividualModels(Get());
}

inline void ActivityModel::VisitPersonIds(void (*visitFun)(int personId, void* userData),
                                          void* userData) const
{
    wrActivityModel_VisitPersonIds(Get(), visitFun, userData);
}

inline std::vector<int> ActivityModel::PersonIds() const
{
    std::vector<int> result(NumIndividualModels());
    wrActivityModel_PersonIds(GetPtr(), result.data());
    return result;
}

inline void ActivityModel::ProcessPoses(PoseEstimator const& poseEstimator,
                                        int const imageWidth,
                                        int const imageHeight)
{
    wrActivityModel_ProcessPoses(Get(), poseEstimator.Get(), imageWidth, imageHeight);
}

inline wrActivityModel* ActivityModel::Get() { return GetPtr(); }
inline wrActivityModel* ActivityModel::GetPtr() { return m_impl.get(); }
inline wrActivityModel const* ActivityModel::Get() const { return GetPtr(); }
inline wrActivityModel const* ActivityModel::GetPtr() const { return m_impl.get(); }

inline ActivityModel::Builder::Builder(std::string const& modelPath)
{
    wrActivityModelBuilderHandle c_builder;
    wrReturnCode const retcode = wrActivityModelBuilder_Create(modelPath.c_str(), &c_builder);
    utils::CheckReturnCodeOK(retcode);
    m_impl.reset(c_builder);
}

inline ActivityModel::Builder& ActivityModel::Builder::SetModelPath(std::string const& modelDir)
{
    wrActivityModelBuilder_SetModelPath(Get(), modelDir.c_str());
    return *this;
}

inline ActivityModel::Builder& ActivityModel::Builder::SetDeviceId(int deviceId)
{
    wrReturnCode const code = wrActivityModelBuilder_SetDeviceId(Get(), deviceId);
    utils::CheckReturnCodeOK(code);
    return *this;
}

inline ActivityModel::Builder& ActivityModel::Builder::SetNumModels(int numModels)
{
    wrActivityModelBuilder_SetNumModels(Get(), numModels);
    return *this;
}

inline char const* ActivityModel::Builder::ModelPath() const
{
    return wrActivityModelBuilder_GetModelPath(Get());
}

inline int ActivityModel::Builder::DeviceId() const
{
    return wrActivityModelBuilder_GetDeviceId(Get());
}

inline int ActivityModel::Builder::NumModels() const
{
    return wrActivityModelBuilder_GetNumModels(Get());
}

inline ActivityModel ActivityModel::Builder::Build() const
{
    wrActivityModel* c_activityModel{ nullptr };
    wrReturnCode const code = wrActivityModelBuilder_Build(Get(), &c_activityModel);
    utils::CheckReturnCodeOK(code);
    return ActivityModel{ c_activityModel };
}

inline ActivityModel ActivityModel::Builder::BuildReflected() const
{
    wrActivityModel* c_activityModel{ nullptr };
    wrReturnCode const code = wrActivityModelBuilder_BuildReflected(Get(), &c_activityModel);
    utils::CheckReturnCodeOK(code);
    return ActivityModel{ c_activityModel };
}

inline ActivityPoseEstimatorRequirements ActivityModel::Builder::PoseEstimatorRequirements() const
{
    wrPoseEstimatorRequirements* requirementsPtr{ nullptr };
    wrReturnCode const code
        = wrActivityModelBuilder_PoseEstimatorRequirements(Get(), &requirementsPtr);
    utils::CheckReturnCodeOK(code);
    return ActivityPoseEstimatorRequirements{ requirementsPtr };
}

inline wrActivityModelBuilder* ActivityModel::Builder::Get() { return m_impl.get(); }

inline wrActivityModelBuilder const* ActivityModel::Builder::Get() const { return m_impl.get(); }
}  // namespace wrnch

#endif /* WRNCH_ACTIVITY_MODEL_CXX_API*/
