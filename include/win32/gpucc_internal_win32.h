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

#ifndef gpuccBytecodeFxc_
#define gpuccBytecodeFxc_(_b)                                                  \
    ((GPUCC_BYTECODE_FXC_WIN32*)(_b))
#endif

// dxcapi.h is part of the Windows SDK
// Need to create a API dispatch table for it, API is pretty simple.
// Unfortunately it expects all strings to be WCHAR.

/* @summary Define the platform-specific GPUCC_PROCESS_CONTEXT structure.
 * There's one process context that's global to the application.
 * The process context is managed in the DllMain function.
 */
typedef struct GPUCC_PROCESS_CONTEXT_WIN32 {
    DWORD                         TlsSlot_ThreadContext;                       /* The index of the thread-local storage slot for storing each thread's GPUCC_THREAD_CONTEXT_WIN32 pointer. */
    uint32_t                      CompilerSupport;                             /* One or more bitwise-OR'd GPUCC_COMPILER_SUPPORT flags indicating which compilers are supported. */
    BOOL                          InitializationFlag;                          /* Set to TRUE when the process context has been initialized, or FALSE otherwise. */
    BOOL                          StartupFlag;                                 /* Set to TRUE when gpuccStartup completes successfully, or FALSE otherwise. */
    D3DCOMPILERAPI_DISPATCH       D3DCompiler_Dispatch;                        /* The dispatch table for the legacy Direct3D compiler, loaded from d3dcompiler_47.dll. */
} GPUCC_PROCESS_CONTEXT_WIN32;

/* @summary Define the platform-specific GPUCC_THREAD_CONTEXT structure.
 * There's one thread context for each thread that attaches to the library.
 * The thread context is managed in the DllMain function.
 */
typedef struct GPUCC_THREAD_CONTEXT_WIN32 {
    GPUCC_RESULT                  LastResult;                                  /* The result code returned by the most recent GpuCC operation on the thread. */
} GPUCC_THREAD_CONTEXT_WIN32;

/* @summary Define the data maintained by an instance of the fxc (legacy Direct3D) compiler.
 */
typedef struct GPUCC_COMPILER_FXC_WIN32 {
    GPUCC_PROGRAM_COMPILER_BASE   CommonFields;                                /* This must be the first field of any compiler type. */
    D3DCOMPILERAPI_DISPATCH      *DispatchTable;                               /* A pointer to the d3dcompiler dispatch table maintained by the process context. */
    D3D_SHADER_MACRO             *DefineArray;                                 /* An array of D3D_SHADER_MACRO specifying the symbols and values defined for the compiler. */
    uint32_t                      DefineCount;                                 /* The number of valid elements in the D3D_SHADER_MACRO array. */
    int32_t                       TargetRuntime;                               /* One of the values of the GPUCC_TARGET_RUNTIME enumeration specifying the target runtime for shaders built by the compiler. */
    char                         *ShaderModel;                                 /* A nul-terminated string specifying the Direct3D shader model. */
} GPUCC_COMPILER_FXC_WIN32;

/* @summary Define the data maintained by a single DXBC program bytecode container generated by the fxc (legacy Direct3D) compiler.
 */
typedef struct GPUCC_BYTECODE_FXC_WIN32 {
    GPUCC_PROGRAM_BYTECODE_BASE   CommonFields;                                /* This must be the first field of any bytecode container type. */
    ID3DBlob                     *CodeBuffer;                                  /* The buffer for storing the compiled bytecode. This is NULL unless the compilation succeeds. */
    ID3DBlob                     *ErrorLog;                                    /* The buffer for storing the output produced by the compiler. This is NULL if no compilation has been attempted. */
} GPUCC_BYTECODE_FXC_WIN32;

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
