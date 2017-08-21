/*! exports.h */

#ifndef _AX_VERSION_H_
#define _AX_VERSION_H_

#if defined(_WIN32) && !defined(__GCC__)
#  if defined(AX_STATIC)
#    define AX_API
#  else
#    ifdef BUILDING_AX
#      define AX_API __declspec(dllexport)
#    else
#      define AX_API __declspec(dllimport)
#    endif
#  endif
#  ifndef _CRT_SECURE_NO_WARNINGS
#    define _CRT_SECURE_NO_WARNINGS
#  endif
#else
#  ifdef BUILDING_AX
#    define AX_API __attribute__ ((visibility ("default")))
#  else
#    define AX_API
#  endif
#endif

#if defined(__cplusplus)
#  define AX_EXTERN_C extern "C"
#  define AX_C_API AX_EXTERN_C AX_API
#else
#  define AX_EXTERN_C
#  define AX_C_API AX_API
#endif

#endif/*_AX_VERSION_H_*/
