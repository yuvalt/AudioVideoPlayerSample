/*
 Copyright (c) 2019 wrnch Inc.
 All rights reserved
*/

#ifndef WRNCH_DEFINITIONS
#define WRNCH_DEFINITIONS

#if !defined(WRNCH_DLL_INTERFACE)
#if defined(_WIN32) && defined(WRNCH_API_SHARED)
#ifdef WRNCH_DLL_EXPORT
#define WRNCH_DLL_INTERFACE __declspec(dllexport)
#else
#define WRNCH_DLL_INTERFACE __declspec(dllimport)
#endif
#else
#define WRNCH_DLL_INTERFACE
#endif
#endif

#if defined(__cplusplus) && (__cplusplus >= 201402L)
#define WRNCH_DEPRECATED(msg) [[deprecated(msg)]]
#elif defined(__GNUC__) || defined(__clang__)
#define WRNCH_DEPRECATED(msg) __attribute__((deprecated(msg)))
#elif defined(_WIN32)
#define WRNCH_DEPRECATED(msg) __declspec(deprecated(msg))
#else
#define WRNCH_DEPRECATED(msg) 
#endif

#endif /* WRNCH_DEFINITIONS */
