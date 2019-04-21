/**
 * @summary gpucc_internal.h: Define GpuCC functions available for use in other
 * modules, but not part of the public interface.
 */
#ifndef __GPUCC_INTERNAL_H__
#define __GPUCC_INTERNAL_H__

#pragma once

#ifndef GPUCC_NO_INCLUDES
#   ifndef __GPUCC_H__
#       include "gpucc.h"
#   endif
#endif

/* Forward-declare opaque platform types */
struct GPUCC_THREAD_CONTEXT;
struct GPUCC_PROCESS_CONTEXT;

#ifdef __cplusplus
extern "C" {
#endif

GPUCC_API(struct GPUCC_PROCESS_CONTEXT*)
gpuccGetProcessContext
(
    void
);

GPUCC_API(struct GPUCC_THREAD_CONTEXT *)
gpuccGetThreadContext
(
    void
);

GPUCC_API(struct GPUCC_RESULT)
gpuccSetLastResult
(
    struct GPUCC_RESULT result
);

#ifdef __cplusplus
}; /* extern "C" */
#endif

#endif /* __GPUCC_INTERNAL_H__ */

#if   defined(__APPLE__)
#   if defined(TARGET_OS_IPHONE) || defined(TARGET_OS_IPHONE_SIMULATOR)
#       error No GpuCC implementation for iOS (yet).
#   else
#       error No GpuCC implementation for macOS (yet).
#   endif
#elif defined(_WIN32) || defined(_WIN64)
#   include "win32/gpucc_internal_win32.h"
#elif defined(__linux__) || defined(__gnu_linux__)
#   error No GpuCC implementation for Linux (yet).
#else
#   error No GpuCC implementation for your platform (yet).
#endif
