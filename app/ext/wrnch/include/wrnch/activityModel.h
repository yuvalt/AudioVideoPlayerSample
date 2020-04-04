/*
 Copyright (c) 2019 wrnch Inc.
 All rights reserved
*/
#ifndef WRNCH_ACTIVITY_MODEL_C_API
#define WRNCH_ACTIVITY_MODEL_C_API

#include <wrnch/activityPoseEstimatorRequirements.h>
#include <wrnch/definitions.h>
#include <wrnch/individualActivityModel.h>
#include <wrnch/poseEstimator.h>
#include <wrnch/returnCodes.h>


#ifdef __cplusplus
extern "C"
{
#endif

    /// @file activityModel.h
    /// @brief Estimates pose-based activities (or "gestures") for people ("individuals") over time.
    ///         The underlying model is usually temporal, and requires processing of poses through
    ///         time for best behavior. Holds a collection of per-person temporal models (instances
    ///         of wrIndividualActivityModel), accessed through `wrActivityModel_IndividualModel`.
    /// @see `wrIndividualActivityModel`
    struct wrActivityModel;
    typedef struct wrActivityModel* wrActivityModelHandle;
    typedef struct wrActivityModel const* wrActivityModelHandleConst;

    WRNCH_DLL_INTERFACE void wrActivityModel_Destroy(wrActivityModelHandle model);

    /// @brief Process the poses stored in `poseEstimator`, rolling the state of the per-person,
    ///         ie wrIndividualActivityModel, models forward.
    /// @param model the activity model to modify
    /// @param poseEstimator the pose estimator storing poses to be processed by `model`.
    /// @param imageWidth the width of the image last processed by `poseEstimator`
    /// @param imageHeight the height of the image last processed by `poseEstimator`
    WRNCH_DLL_INTERFACE void wrActivityModel_ProcessPoses(wrActivityModelHandle model,
                                                          wrPoseEstimatorHandleConst poseEstimator,
                                                          int imageWidth,
                                                          int imageHeight);

    /// @brief Return the number of classes estimated by `activityModel`
    WRNCH_DLL_INTERFACE int wrActivityModel_NumClasses(wrActivityModelHandleConst activityModel);

    /// @brief Fill out class names estimated by `activityModel` into `outNames`.
    /// @note The behavior is undefined unless `outNames` is valid for at least
    ///         `wrActivityModel_NumClasses(activityModel)` `char const*'s`
    WRNCH_DLL_INTERFACE void wrActivityModel_ClassNames(wrActivityModelHandleConst activityModel,
                                                        char const** outNames);

    /// @brief Attempt to access the individual activity model of a given person. If no such person
    ///         has been estimated by activityModel, return null. Generally speaking, this should
    ///         only be called on `personId` found by the last call to ProcessFrame on the relevant
    ///         poseEstimator.
    WRNCH_DLL_INTERFACE wrIndividualActivityModelHandleConst
    wrActivityModel_IndividualModel(wrActivityModelHandleConst activityModel, int personId);

    /// @brief Return the number of individual models held by `activityModel`
    WRNCH_DLL_INTERFACE int wrActivityModel_NumIndividualModels(
        wrActivityModelHandleConst activityModel);

    /// @brief internally, for each `personId` that would be written in `wrActivityModel_PersonIds`,
    ///         calls visitFun(personId, userData)`
    WRNCH_DLL_INTERFACE void wrActivityModel_VisitPersonIds(
        wrActivityModelHandleConst, void (*visitFun)(int personId, void* userData), void* userData);

    /// @brief Fill out the estimated personIds held in `activityModel`.
    /// @note The behavior is undefined unless personIds is valid for at least
    ///         `wrActivityModel_NumIndividualModels(activityModel)` ints.
    WRNCH_DLL_INTERFACE void wrActivityModel_PersonIds(wrActivityModelHandleConst activityModel,
                                                       int* personIds);

    struct wrActivityModelBuilder;
    typedef struct wrActivityModelBuilder* wrActivityModelBuilderHandle;
    typedef struct wrActivityModelBuilder const* wrActivityModelBuilderHandleConst;

    WRNCH_DLL_INTERFACE wrReturnCode wrActivityModelBuilder_Create(char const* modelPath,
                                                                   wrActivityModelBuilderHandle*);
    WRNCH_DLL_INTERFACE void wrActivityModelBuilder_Destroy(wrActivityModelBuilderHandle);
    WRNCH_DLL_INTERFACE void wrActivityModelBuilder_SetModelPath(wrActivityModelBuilderHandle,
                                                                 char const* modelPath);
    WRNCH_DLL_INTERFACE char const* wrActivityModelBuilder_GetModelPath(
        wrActivityModelBuilderHandleConst);
    WRNCH_DLL_INTERFACE wrReturnCode
    wrActivityModelBuilder_SetDeviceId(wrActivityModelBuilderHandle, int deviceId);
    WRNCH_DLL_INTERFACE int wrActivityModelBuilder_GetDeviceId(wrActivityModelBuilderHandleConst);
    WRNCH_DLL_INTERFACE void wrActivityModelBuilder_SetNumModels(wrActivityModelBuilderHandle,
                                                                 int numModels);
    WRNCH_DLL_INTERFACE int wrActivityModelBuilder_GetNumModels(wrActivityModelBuilderHandleConst);
    WRNCH_DLL_INTERFACE wrReturnCode wrActivityModelBuilder_Build(wrActivityModelBuilderHandleConst,
                                                                  wrActivityModelHandle*);
    WRNCH_DLL_INTERFACE wrReturnCode wrActivityModelBuilder_BuildReflected(
        wrActivityModelBuilderHandleConst, wrActivityModelHandle*);
    WRNCH_DLL_INTERFACE wrReturnCode wrActivityModelBuilder_PoseEstimatorRequirements(
        wrActivityModelBuilderHandleConst builder,
        wrPoseEstimatorRequirementsHandle* outRequirements);

#ifdef __cplusplus
}
#endif

#endif /* WRNCH_ACTIVITY_MODEL_C_API*/
