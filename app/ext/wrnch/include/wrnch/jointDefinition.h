/*
 Copyright (c) 2019 wrnch Inc.
 All rights reserved
*/

#ifndef WRNCH_JOINT_DEFINITON_C_API
#define WRNCH_JOINT_DEFINITON_C_API

#include <wrnch/definitions.h>

#ifdef __cplusplus
extern "C"
{
#endif

    struct wrJointDefinition;
    typedef const struct wrJointDefinition* wrJointDefinitionHandleConst;

    WRNCH_DLL_INTERFACE wrJointDefinitionHandleConst wrJointDefinition_Get(const char* name);
    WRNCH_DLL_INTERFACE unsigned int wrJointDefinition_GetNumDefinitions(void);
    WRNCH_DLL_INTERFACE void wrJointDefinition_GetAvailableDefinitions(const char** namesList);

    WRNCH_DLL_INTERFACE unsigned int wrJointDefinition_GetNumJoints(wrJointDefinitionHandleConst);
    WRNCH_DLL_INTERFACE void wrJointDefinition_GetJointNames(wrJointDefinitionHandleConst,
                                                             char const** names);
    WRNCH_DLL_INTERFACE unsigned int wrJointDefinition_GetNumBones(wrJointDefinitionHandleConst);
    WRNCH_DLL_INTERFACE void wrJointDefinition_GetBonePairs(wrJointDefinitionHandleConst,
                                                            unsigned int* pairs);
    WRNCH_DLL_INTERFACE int wrJointDefinition_GetJointIndex(wrJointDefinitionHandleConst,
                                                            const char* jointName);
    WRNCH_DLL_INTERFACE const char* wrJointDefinition_GetName(wrJointDefinitionHandleConst);
    WRNCH_DLL_INTERFACE void wrJointDefinition_PrintJointDefinition(wrJointDefinitionHandleConst);
    WRNCH_DLL_INTERFACE int wrJointDefinition_Equals(wrJointDefinitionHandleConst,
                                                     wrJointDefinitionHandleConst);

    /// @brief Query if a jointDefinition is symmetric along the Y-axis (ie, is horizontally
    ///         symmetric). Return 1 if true, 0 otherwise.
    /// @param jointDef the wrJointDefinition to query.
    WRNCH_DLL_INTERFACE int wrJointDefinition_IsSymmetricAlongY(
        wrJointDefinitionHandleConst jointDef);

    /// @brief Given a symmetric-along-y jointDefinition and a joint index, get the corresponding
    ///         reflected index. The behavior is undefined if `jointDef` is not horizontally
    ///         symmetric (as returned by `wrJointDefinition_IsSymmetricAlongY`), or if jointIndex
    ///         is greather than `wrJointDefinition_GetNumJoints(jointDef)`
    /// @param jointDef the joint definition to query.
    /// @param jointIndex the joint index to reflect
    WRNCH_DLL_INTERFACE int wrJointDefinition_ReflectedIndexOverY(
        wrJointDefinitionHandleConst jointDef, int jointIndex);

#ifdef __cplusplus
}
#endif

#endif /* WRNCH_JOINT_DEFINITON_C_API */
