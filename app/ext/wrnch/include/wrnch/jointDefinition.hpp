/*
 Copyright (c) 2019 wrnch Inc.
 All rights reserved
*/

#ifndef WRNCH_JOINT_DEFINITION_CXX_API
#define WRNCH_JOINT_DEFINITION_CXX_API

#include <wrnch/utils.hpp>
#include <wrnch/jointDefinition.h>

#include <new>
#include <stdexcept>
#include <vector>


namespace wrnch
{
class JointDefinition
{
  public:
    JointDefinition()
    : m_impl(nullptr)
    {
    }
    explicit JointDefinition(const wrJointDefinition* other)
    : m_impl(other)
    {
    }

    unsigned int GetNumJoints() const { return wrJointDefinition_GetNumJoints(CheckedGet()); }

    std::vector<std::string> GetJointNames() const
    {
        std::vector<char const*> namePtrs(GetNumJoints(), nullptr);
        GetJointNames(namePtrs.data());
        return { namePtrs.begin(), namePtrs.end() };
    }

    void GetJointNames(const char** outNames) const
    {
        wrJointDefinition_GetJointNames(CheckedGet(), outNames);
    }
    void GetJointNames(const char** outNames, wrReturnCode& code) const
    {
        CheckInitialized(code);
        if (code == wrReturnCode_OK)
        {
            wrJointDefinition_GetJointNames(m_impl, outNames);
        }
    }

    unsigned int GetNumBones() const { return wrJointDefinition_GetNumBones(CheckedGet()); }

    std::vector<unsigned int> GetBonePairs() const
    {
        std::vector<unsigned int> res(GetNumBones() * 2);
        GetBonePairs(res.data());
        return res;
    }

    void GetBonePairs(unsigned int* pairs) const
    {
        wrJointDefinition_GetBonePairs(CheckedGet(), pairs);
    }
    void GetBonePairs(unsigned int* pairs, wrReturnCode& code) const
    {
        CheckInitialized(code);
        if (code == wrReturnCode_OK)
        {
            wrJointDefinition_GetBonePairs(Get(), pairs);
        }
    }

    int GetJointIndex(const char* jointName) const
    {
        int index = wrJointDefinition_GetJointIndex(CheckedGet(), jointName);
        utils::Check(index >= 0, "No known joint with name " + std::string(jointName));
        return index;
    }
    int GetJointIndex(const char* jointName, wrReturnCode& code) const
    {
        CheckInitialized(code);
        int index = -1;
        if (code == wrReturnCode_OK)
        {
            index = wrJointDefinition_GetJointIndex(CheckedGet(), jointName);
            if (index < 0)
            {
                code = wrReturnCode_JOINT_DEFINITION_ERROR;
            }
        }
        return index;
    }

    const char* GetName() const { return wrJointDefinition_GetName(CheckedGet()); }
    const char* GetName(wrReturnCode& code) const
    {
        CheckInitialized(code);
        if (code == wrReturnCode_OK)
        {
            return wrJointDefinition_GetName(Get());
        }
        return nullptr;
    }

    void PrintJointDefinition() const { wrJointDefinition_PrintJointDefinition(CheckedGet()); }
    void PrintJointDefinition(wrReturnCode& code) const
    {
        CheckInitialized(code);
        if (code == wrReturnCode_OK)
        {
            wrJointDefinition_PrintJointDefinition(CheckedGet());
        }
    }

    bool operator==(const JointDefinition& other) const
    {
        return wrJointDefinition_Equals(m_impl, other.m_impl) == 1;
    }
    bool operator!=(const JointDefinition& other) const { return !(*this == other); }

    /// @brief Query if a jointDefinition is symmetric along the Y-axis (ie, is horizontally
    ///         symmetric).
    bool IsSymmetricAlongY() const { return wrJointDefinition_IsSymmetricAlongY(m_impl) != 0; }

    /// @brief Given a symmetric-along-y jointDefinition and a joint index, get the corresponding
    ///         reflected index. The behavior is undefined if `*this` is not horizontally
    ///         symmetric (as returned by `bool IsSymmetricAlongY()`), or if jointIndex
    ///         is greather than `GetNumJoints()`
    /// @param jointIndex the joint index to reflect
    int ReflectedIndexOverY(int index) const
    {
        return wrJointDefinition_ReflectedIndexOverY(m_impl, index);
    }
    const wrJointDefinition* Get() const { return m_impl; }

    const wrJointDefinition* CheckedGet() const
    {
        const wrJointDefinition* ptr = Get();
        utils::Check(ptr != nullptr, "Attempting to access uninitialized JointDefinition");
        return ptr;
    }
    const wrJointDefinition* CheckedGet(wrReturnCode& code) const
    {
        const wrJointDefinition* ptr = Get();
        code = (ptr == nullptr) ? wrReturnCode_JOINT_DEFINITION_ERROR : wrReturnCode_OK;
        return ptr;
    }

  private:
    const wrJointDefinition* m_impl;
    void CheckInitialized(wrReturnCode& code) const
    {
        code = (m_impl == nullptr) ? wrReturnCode_JOINT_DEFINITION_ERROR : wrReturnCode_OK;
    }
};

struct JointDefinitionRegistry
{
    static JointDefinition Get(const char* name)
    {
        const wrJointDefinition* ptr = wrJointDefinition_Get(name);
        utils::Check(ptr != nullptr, "No known joint definition with name " + std::string(name));
        return JointDefinition(ptr);
    }
    static JointDefinition Get(const char* name, wrReturnCode& code)
    {
        const wrJointDefinition* ptr = wrJointDefinition_Get(name);
        code = (ptr == nullptr) ? wrReturnCode_JOINT_DEFINITION_ERROR : wrReturnCode_OK;
        return JointDefinition(ptr);
    }
    static unsigned int GetNumDefinitions() { return wrJointDefinition_GetNumDefinitions(); }
    static void GetAvailableDefinitions(const char** definitionsList)
    {
        wrJointDefinition_GetAvailableDefinitions(definitionsList);
    }
};
}

#endif /* WRNCH_JOINT_DEFINITION_CXX_API */
