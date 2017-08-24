/*
For license details see ../../LICENSE
*/

/**
 * \file errno.h
 * \date Aug 23, 2017
 */

#ifndef _AX_ERRNO_H_
#define _AX_ERRNO_H_

#include "ax/exports.h"

#include <errno.h>

#define AX_EOF     (-4095)
#define AX_UNKNOWN (-4094)

#define AX_EAI_ADDRFAMILY  (-3000)
#define AX_EAI_AGAIN       (-3001)
#define AX_EAI_BADFLAGS    (-3002)
#define AX_EAI_CANCELED    (-3003)
#define AX_EAI_FAIL        (-3004)
#define AX_EAI_FAMILY      (-3005)
#define AX_EAI_MEMORY      (-3006)
#define AX_EAI_NODATA      (-3007)
#define AX_EAI_NONAME      (-3008)
#define AX_EAI_OVERFLOW    (-3009)
#define AX_EAI_SERVICE     (-3010)
#define AX_EAI_SOCKTYPE    (-3011)
#define AX_EAI_BADHINTS    (-3013)
#define AX_EAI_PROTOCOL    (-3014)

#if defined(E2BIG) && !defined(_WIN32)
#  define AX_E2BIG (-E2BIG)
#else
#  define AX_E2BIG (-4093)
#endif

#if defined(EACCES) && !defined(_WIN32)
#  define AX_EACCES (-EACCES)
#else
#  define AX_EACCES (-4092)
#endif

#if defined(EADDRINUSE) && !defined(_WIN32)
#  define AX_EADDRINUSE (-EADDRINUSE)
#else
#  define AX_EADDRINUSE (-4091)
#endif

#if defined(EADDRNOTAVAIL) && !defined(_WIN32)
#  define AX_EADDRNOTAVAIL (-EADDRNOTAVAIL)
#else
#  define AX_EADDRNOTAVAIL (-4090)
#endif

#if defined(EAFNOSUPPORT) && !defined(_WIN32)
#  define AX_EAFNOSUPPORT (-EAFNOSUPPORT)
#else
#  define AX_EAFNOSUPPORT (-4089)
#endif

#if defined(EAGAIN) && !defined(_WIN32)
#  define AX_EAGAIN (-EAGAIN)
#else
#  define AX_EAGAIN (-4088)
#endif

#if defined(EALREADY) && !defined(_WIN32)
#  define AX_EALREADY (-EALREADY)
#else
#  define AX_EALREADY (-4084)
#endif

#if defined(EBADF) && !defined(_WIN32)
#  define AX_EBADF (-EBADF)
#else
#  define AX_EBADF (-4083)
#endif

#if defined(EBUSY) && !defined(_WIN32)
#  define AX_EBUSY (-EBUSY)
#else
#  define AX_EBUSY (-4082)
#endif

#if defined(ECANCELED) && !defined(_WIN32)
#  define AX_ECANCELED (-ECANCELED)
#else
#  define AX_ECANCELED (-4081)
#endif

#if defined(ECHARSET) && !defined(_WIN32)
#  define AX_ECHARSET (-ECHARSET)
#else
#  define AX_ECHARSET (-4080)
#endif

#if defined(ECONNABORTED) && !defined(_WIN32)
#  define AX_ECONNABORTED (-ECONNABORTED)
#else
#  define AX_ECONNABORTED (-4079)
#endif

#if defined(ECONNREFUSED) && !defined(_WIN32)
#  define AX_ECONNREFUSED (-ECONNREFUSED)
#else
#  define AX_ECONNREFUSED (-4078)
#endif

#if defined(ECONNRESET) && !defined(_WIN32)
#  define AX_ECONNRESET (-ECONNRESET)
#else
#  define AX_ECONNRESET (-4077)
#endif

#if defined(EDESTADDRREQ) && !defined(_WIN32)
#  define AX_EDESTADDRREQ (-EDESTADDRREQ)
#else
#  define AX_EDESTADDRREQ (-4076)
#endif

#if defined(EEXIST) && !defined(_WIN32)
#  define AX_EEXIST (-EEXIST)
#else
#  define AX_EEXIST (-4075)
#endif

#if defined(EFAULT) && !defined(_WIN32)
#  define AX_EFAULT (-EFAULT)
#else
#  define AX_EFAULT (-4074)
#endif

#if defined(EHOSTUNREACH) && !defined(_WIN32)
#  define AX_EHOSTUNREACH (-EHOSTUNREACH)
#else
#  define AX_EHOSTUNREACH (-4073)
#endif

#if defined(EINTR) && !defined(_WIN32)
#  define AX_EINTR (-EINTR)
#else
#  define AX_EINTR (-4072)
#endif

#if defined(EINVAL) && !defined(_WIN32)
#  define AX_EINVAL (-EINVAL)
#else
#  define AX_EINVAL (-4071)
#endif

#if defined(EIO) && !defined(_WIN32)
#  define AX_EIO (-EIO)
#else
#  define AX_EIO (-4070)
#endif

#if defined(EISCONN) && !defined(_WIN32)
#  define AX_EISCONN (-EISCONN)
#else
#  define AX_EISCONN (-4069)
#endif

#if defined(EISDIR) && !defined(_WIN32)
#  define AX_EISDIR (-EISDIR)
#else
#  define AX_EISDIR (-4068)
#endif

#if defined(ELOOP) && !defined(_WIN32)
#  define AX_ELOOP (-ELOOP)
#else
#  define AX_ELOOP (-4067)
#endif

#if defined(EMFILE) && !defined(_WIN32)
#  define AX_EMFILE (-EMFILE)
#else
#  define AX_EMFILE (-4066)
#endif

#if defined(EMSGSIZE) && !defined(_WIN32)
#  define AX_EMSGSIZE (-EMSGSIZE)
#else
#  define AX_EMSGSIZE (-4065)
#endif

#if defined(ENAMETOOLONG) && !defined(_WIN32)
#  define AX_ENAMETOOLONG (-ENAMETOOLONG)
#else
#  define AX_ENAMETOOLONG (-4064)
#endif

#if defined(ENETDOWN) && !defined(_WIN32)
#  define AX_ENETDOWN (-ENETDOWN)
#else
#  define AX_ENETDOWN (-4063)
#endif

#if defined(ENETUNREACH) && !defined(_WIN32)
#  define AX_ENETUNREACH (-ENETUNREACH)
#else
#  define AX_ENETUNREACH (-4062)
#endif

#if defined(ENFILE) && !defined(_WIN32)
#  define AX_ENFILE (-ENFILE)
#else
#  define AX_ENFILE (-4061)
#endif

#if defined(ENOBUFS) && !defined(_WIN32)
#  define AX_ENOBUFS (-ENOBUFS)
#else
#  define AX_ENOBUFS (-4060)
#endif

#if defined(ENODEV) && !defined(_WIN32)
#  define AX_ENODEV (-ENODEV)
#else
#  define AX_ENODEV (-4059)
#endif

#if defined(ENOENT) && !defined(_WIN32)
#  define AX_ENOENT (-ENOENT)
#else
#  define AX_ENOENT (-4058)
#endif

#if defined(ENOMEM) && !defined(_WIN32)
#  define AX_ENOMEM (-ENOMEM)
#else
#  define AX_ENOMEM (-4057)
#endif

#if defined(ENONET) && !defined(_WIN32)
#  define AX_ENONET (-ENONET)
#else
#  define AX_ENONET (-4056)
#endif

#if defined(ENOSPC) && !defined(_WIN32)
#  define AX_ENOSPC (-ENOSPC)
#else
#  define AX_ENOSPC (-4055)
#endif

#if defined(ENOSYS) && !defined(_WIN32)
#  define AX_ENOSYS (-ENOSYS)
#else
#  define AX_ENOSYS (-4054)
#endif

#if defined(ENOTCONN) && !defined(_WIN32)
#  define AX_ENOTCONN (-ENOTCONN)
#else
#  define AX_ENOTCONN (-4053)
#endif

#if defined(ENOTDIR) && !defined(_WIN32)
#  define AX_ENOTDIR (-ENOTDIR)
#else
#  define AX_ENOTDIR (-4052)
#endif

#if defined(ENOTEMPTY) && !defined(_WIN32)
#  define AX_ENOTEMPTY (-ENOTEMPTY)
#else
#  define AX_ENOTEMPTY (-4051)
#endif

#if defined(ENOTSOCK) && !defined(_WIN32)
#  define AX_ENOTSOCK (-ENOTSOCK)
#else
#  define AX_ENOTSOCK (-4050)
#endif

#if defined(ENOTSUP) && !defined(_WIN32)
#  define AX_ENOTSUP (-ENOTSUP)
#else
#  define AX_ENOTSUP (-4049)
#endif

#if defined(EPERM) && !defined(_WIN32)
#  define AX_EPERM (-EPERM)
#else
#  define AX_EPERM (-4048)
#endif

#if defined(EPIPE) && !defined(_WIN32)
#  define AX_EPIPE (-EPIPE)
#else
#  define AX_EPIPE (-4047)
#endif

#if defined(EPROTO) && !defined(_WIN32)
#  define AX_EPROTO (-EPROTO)
#else
#  define AX_EPROTO (-4046)
#endif

#if defined(EPROTONOSUPPORT) && !defined(_WIN32)
#  define AX_EPROTONOSUPPORT (-EPROTONOSUPPORT)
#else
#  define AX_EPROTONOSUPPORT (-4045)
#endif

#if defined(EPROTOTYPE) && !defined(_WIN32)
#  define AX_EPROTOTYPE (-EPROTOTYPE)
#else
#  define AX_EPROTOTYPE (-4044)
#endif

#if defined(EROFS) && !defined(_WIN32)
#  define AX_EROFS (-EROFS)
#else
#  define AX_EROFS (-4043)
#endif

#if defined(ESHUTDOWN) && !defined(_WIN32)
#  define AX_ESHUTDOWN (-ESHUTDOWN)
#else
#  define AX_ESHUTDOWN (-4042)
#endif

#if defined(ESPIPE) && !defined(_WIN32)
#  define AX_ESPIPE (-ESPIPE)
#else
#  define AX_ESPIPE (-4041)
#endif

#if defined(ESRCH) && !defined(_WIN32)
#  define AX_ESRCH (-ESRCH)
#else
#  define AX_ESRCH (-4040)
#endif

#if defined(ETIMEDOUT) && !defined(_WIN32)
#  define AX_ETIMEDOUT (-ETIMEDOUT)
#else
#  define AX_ETIMEDOUT (-4039)
#endif

#if defined(ETXTBSY) && !defined(_WIN32)
#  define AX_ETXTBSY (-ETXTBSY)
#else
#  define AX_ETXTBSY (-4038)
#endif

#if defined(EXDEV) && !defined(_WIN32)
#  define AX_EXDEV (-EXDEV)
#else
#  define AX_EXDEV (-4037)
#endif

#if defined(EFBIG) && !defined(_WIN32)
#  define AX_EFBIG (-EFBIG)
#else
#  define AX_EFBIG (-4036)
#endif

#if defined(ENOPROTOOPT) && !defined(_WIN32)
#  define AX_ENOPROTOOPT (-ENOPROTOOPT)
#else
#  define AX_ENOPROTOOPT (-4035)
#endif

#if defined(ERANGE) && !defined(_WIN32)
#  define AX_ERANGE (-ERANGE)
#else
#  define AX_ERANGE (-4034)
#endif

#if defined(ENXIO) && !defined(_WIN32)
#  define AX_ENXIO (-ENXIO)
#else
#  define AX_ENXIO (-4033)
#endif

#if defined(EMLINK) && !defined(_WIN32)
#  define AX_EMLINK (-EMLINK)
#else
#  define AX_EMLINK (-4032)
#endif

#if defined(EHOSTDOWN) && !defined(_WIN32)
#  define AX_EHOSTDOWN (-EHOSTDOWN)
#elif defined(__APPLE__) || \
      defined(__DragonFly__) || \
      defined(__FreeBSD__) || \
      defined(__FreeBSD_kernel__) || \
      defined(__NetBSD__) || \
      defined(__OpenBSD__)
#  define AX_EHOSTDOWN (-64)
#else
#  define AX_EHOSTDOWN (-4031)
#endif

AX_API
ax_const_str ax_error_str(int err);

AX_API
ax_const_str ax_error_name(int err);

#endif/*_AX_ERRNO_H_*/
