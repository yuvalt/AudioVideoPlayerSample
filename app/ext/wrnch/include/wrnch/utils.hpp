/*
 Copyright (c) 2019 wrnch Inc.
 All rights reserved
*/

#ifndef WRNCH_UTILS_CXX_API
#define WRNCH_UTILS_CXX_API

#include <wrnch/returnCodes.h>

#include <cstring>    // NULL macro
#include <string>     // for std::string
#include <stdexcept>  // for std::runtime_error


namespace wrnch
{
namespace utils
{
class Exception : public std::runtime_error
{
  public:
    explicit Exception(wrReturnCode code)
    : std::runtime_error(wrReturnCode_Translate(code))
    , m_code(code)
    {
    }

    Exception(wrReturnCode code, const std::string& msg)
    : std::runtime_error(std::string(wrReturnCode_Translate(code)) + " " + msg)
    , m_code(code)
    {
    }

    wrReturnCode Code() const { return m_code; }

  private:
    wrReturnCode m_code;
};

class LicenseError : public Exception
{
  public:
    explicit LicenseError(wrReturnCode code)
    : Exception(code)
    {
    }

    LicenseError()
    : Exception(wrReturnCode_LICENSE_ERROR_OTHER, "License error: Unknown.")
    {
    }
};

class InitializationError : public Exception
{
  public:
    explicit InitializationError(wrReturnCode code)
    : Exception(code)
    {
    }

    InitializationError()
    : Exception(wrReturnCode_OTHER_ERROR)
    {
    }
};

inline void CheckReturnCodeOK(wrReturnCode const code)
{
    if (code != wrReturnCode_OK)
    {
        throw Exception(code);
    }
}

inline void CheckBadAlloc(const void* ptr)
{
    if (ptr == nullptr)
    {
        throw std::bad_alloc();
    }
}

inline void Check(bool condition, const char* msg)
{
    if (!condition)
    {
        throw std::runtime_error(msg);
    }
}

inline void Check(bool condition, const std::string& msg)
{
    void (*otherCheck)(bool, const char*);
    otherCheck = &Check;

    otherCheck(condition, msg.c_str());
}
}  // namespace utils
}  // namespace wrnch

#endif /* WRNCH_UTILS_CXX_API */
