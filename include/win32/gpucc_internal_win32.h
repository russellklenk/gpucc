/**
 * @summary gpucc_internal_win32.h: Define Windows-specific types and helper 
 * functions made available to other internal modules.
 */
#ifndef __GPUCC_INTERNAL_WIN32_H__
#define __GPUCC_INTERNAL_WIN32_H__

#pragma once

#ifndef GPUCC_NO_INCLUDES
#   ifndef __GPUCC_D3DCOMPILERAPI_WIN32_H__
#       include "win32/d3dcompilerapi_win32.h"
#   endif
#endif

#ifndef __GPUCC_INTERNAL_H__
#   error Do not include this file directly - include gpucc_internal.h instead.
#endif

#ifndef gpuccGetProcessContext_
#define gpuccGetProcessContext_()                                              \
    ((GPUCC_PROCESS_CONTEXT_WIN32*) gpuccGetProcessContext())
#endif

#ifndef gpuccGetThreadContext_
#define gpuccGetThreadContext_()                                               \
    ((GPUCC_THREAD_CONTEXT_WIN32 *) gpuccGetThreadContext())
#endif

#ifndef gpuccCompilerFxc_
#define gpuccCompilerFxc_(_c)                                                  \
    ((GPUCC_COMPILER_FXC_WIN32*)(_c))
#endif

// dxcapi.h is part of the Windows SDK
// Need to create a API dispatch table for it, API is pretty simple.
// Unfortunately it expects all strings to be WCHAR.

/* @summary Define the platform-specific GPUCC_PROCESS_CONTEXT structure.
 * There's one process context that's global to the application.
 * The process context is managed in the DllMain function.
 */
typedef struct GPUCC_PROCESS_CONTEXT_WIN32 {
    DWORD                         TlsSlot_ThreadContext;
    uint32_t                      CompilerSupport;
    BOOL                          InitializationFlag;
    BOOL                          StartupFlag;
    D3DCOMPILERAPI_DISPATCH       D3DCompiler_Dispatch;
} GPUCC_PROCESS_CONTEXT_WIN32;

/* @summary Define the platform-specific GPUCC_THREAD_CONTEXT structure.
 * There's one thread context for each thread that attaches to the library.
 * The thread context is managed in the DllMain function.
 */
typedef struct GPUCC_THREAD_CONTEXT_WIN32 {
    GPUCC_RESULT                  LastResult;
} GPUCC_THREAD_CONTEXT_WIN32;

typedef struct GPUCC_COMPILER_FXC_WIN32 {
    GPUCC_PROGRAM_COMPILER_BASE   CommonFields;
    D3DCOMPILERAPI_DISPATCH      *DispatchTable;
    D3D_SHADER_MACRO             *DefineArray;
    uint32_t                      DefineCount;
    int32_t                       TargetRuntime;
    char                         *ShaderModel;
} GPUCC_COMPILER_FXC_WIN32;

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

GPUCC_API(struct GPUCC_PROGRAM_COMPILER*)
gpuccCreateCompilerFxc
(
    struct GPUCC_PROGRAM_COMPILER_INIT *config
);

#ifdef __cplusplus
}; /* extern "C" */
#endif

#endif /* __GPUCC_INTERNAL_WIN32_H__ */
