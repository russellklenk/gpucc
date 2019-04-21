/**
 * @summary gpucc_internal_win32.h: Define Windows-specific types and helper 
 * functions made available to other internal modules.
 */
#ifndef __GPUCC_INTERNAL_WIN32_H__
#define __GPUCC_INTERNAL_WIN32_H__

#pragma once

#ifndef GPUCC_NO_INCLUDES
#   include <Windows.h>
#endif

#ifndef __GPUCC_INTERNAL_H__
#   error Do not include this file directly - include gpucc_internal.h instead.
#endif

/* @summary Define the platform-specific GPUCC_PROCESS_CONTEXT structure.
 * There's one process context that's global to the application.
 * The process context is managed in the DllMain function.
 */
typedef struct GPUCC_PROCESS_CONTEXT_WIN32 {
    DWORD                   TlsSlot_ThreadContext;
    BOOL                    InitializationFlag;
    BOOL                    StartupFlag;
} GPUCC_PROCESS_CONTEXT_WIN32;

/* @summary Define the platform-specific GPUCC_THREAD_CONTEXT structure.
 * There's one thread context for each thread that attaches to the library.
 * The thread context is managed in the DllMain function.
 */
typedef struct GPUCC_THREAD_CONTEXT_WIN32 {
    GPUCC_RESULT            LastResult;
} GPUCC_THREAD_CONTEXT_WIN32;

#ifdef __cplusplus
extern "C" {
#endif

GPUCC_API(struct GPUCC_RESULT)
gpuccMakeResult
(
    int32_t library_result
);

GPUCC_API(struct GPUCC_RESULT)
gpuccMakeResult_Win32
(
    int32_t library_result, 
    DWORD  platform_result
);

GPUCC_API(struct GPUCC_RESULT)
gpuccMakeResult_HRESULT
(
    HRESULT platform_result
);

GPUCC_API(struct GPUCC_RESULT)
gpuccSetLastResult
(
    struct GPUCC_RESULT result
);

#ifdef __cplusplus
}; /* extern "C" */
#endif

#endif /* __GPUCC_INTERNAL_WIN32_H__ */
