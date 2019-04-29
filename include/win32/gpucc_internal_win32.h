/**
 * @summary gpucc_internal_win32.h: Define Windows-specific types and helper 
 * functions made available to other internal modules.
 */
#ifndef __GPUCC_INTERNAL_WIN32_H__
#define __GPUCC_INTERNAL_WIN32_H__

#pragma once

#ifndef GPUCC_NO_INCLUDES
#   ifndef __GPUCC_FXCCOMPILERAPI_WIN32_H__
#       include "win32/fxccompilerapi_win32.h"
#   endif
#   ifndef __GPUCC_DXCCOMPILERAPI_WIN32_H__
#       include "win32/dxccompilerapi_win32.h"
#   endif
#   ifndef __GPUCC_PTXCOMPILERAPI_WIN32_H__
#       include "win32/ptxcompilerapi_win32.h"
#   endif
#endif

#ifndef __GPUCC_INTERNAL_H__
#   error Do not include this file directly - include gpucc_internal.h instead.
#endif

/* @summary Retrieve the process-global data.
 * @return A pointer to GPUCC_PROCESS_CONTEXT_WIN32.
 */
#ifndef gpuccGetProcessContext_
#define gpuccGetProcessContext_()                                              \
    ((GPUCC_PROCESS_CONTEXT_WIN32*) gpuccGetProcessContext())
#endif

/* @summary Retrieve the per-thread global data.
 * @return A pointer to GPUCC_THREAD_CONTEXT_WIN32.
 */
#ifndef gpuccGetThreadContext_
#define gpuccGetThreadContext_()                                               \
    ((GPUCC_THREAD_CONTEXT_WIN32 *) gpuccGetThreadContext())
#endif

/* @summary Define the platform-specific GPUCC_PROCESS_CONTEXT structure.
 * There's one process context that's global to the application.
 * The process context is managed in the DllMain function.
 */
typedef struct GPUCC_PROCESS_CONTEXT_WIN32 {
    DWORD                         TlsSlot_ThreadContext;                       /* The index of the thread-local storage slot for storing each thread's GPUCC_THREAD_CONTEXT_WIN32 pointer. */
    uint32_t                      CompilerSupport;                             /* One or more bitwise-OR'd GPUCC_COMPILER_SUPPORT flags indicating which compilers are supported. */
    BOOL                          InitializationFlag;                          /* Set to TRUE when the process context has been initialized, or FALSE otherwise. */
    BOOL                          StartupFlag;                                 /* Set to TRUE when gpuccStartup completes successfully, or FALSE otherwise. */
    FXCCOMPILERAPI_DISPATCH       FxcCompiler_Dispatch;                        /* The dispatch table for the legacy Direct3D compiler, loaded from d3dcompiler_47.dll. */
    DXCCOMPILERAPI_DISPATCH       DxcCompiler_Dispatch;                        /* The dispatch table for the newer Clang/LLVM-based Direct3D compiler, loaded from dxcompiler.dll. */
    PTXCOMPILERAPI_DISPATCH       PtxCompiler_Dispatch;                        /* The dispatch table for the nVidia RTC (runtime CUDA) compiler, loaded from nvrtc64_###_#.dll. */
} GPUCC_PROCESS_CONTEXT_WIN32;

/* @summary Define the platform-specific GPUCC_THREAD_CONTEXT structure.
 * There's one thread context for each thread that attaches to the library.
 * The thread context is managed in the DllMain function.
 */
typedef struct GPUCC_THREAD_CONTEXT_WIN32 {
    GPUCC_RESULT                  LastResult;                                  /* The result code returned by the most recent GpuCC operation on the thread. */
} GPUCC_THREAD_CONTEXT_WIN32;

#ifdef __cplusplus
extern "C" {
#endif

/* @summary Helper function to emit printf-style output to OutputDebugString.
 * The output can be viewed in the VS debugger output window or using DbgView.
 * GpuCC pipes debug output from startup and loading functions to this endpoint.
 * @param format A nul-terminated wide character string following printf formatting conventions.
 * @param ... Substitution arguments for the format string.
 */
GPUCC_API(void)
gpuccDebugPrintf
(
    WCHAR const *format, 
    ...
);

/* @summary Write a nul-terminated UTF-8 encoded string to a buffer.
 * @param dst A pointer to the address where the string should be written. On return, points to one-past the nul byte in the buffer.
 * @param src A pointer to the start of the nul-terminated source string to copy.
 * @return A pointer to the start of the string in the destination buffer.
 */
GPUCC_API(char*)
gpuccPutStringUtf8
(
    uint8_t   *&dst, 
    char const *src
);

/* @summary Write a nul-terminated UTF-16 encoded string to a buffer.
 * @param dst A pointer to the address where the string should be written. On return, points to one-past the nul byte in the buffer.
 * @param src A pointer to the start of the nul-terminated source string to copy.
 * @return A pointer to the start of the string in the destination buffer.
 */
GPUCC_API(WCHAR*)
gpuccPutStringUtf16
(
    uint8_t    *&dst,
    WCHAR const *src
);

/* @summary Parse a Direct3D shader model target profile of the format "ss_j_i", where ss indicates the shader stage, j indicates the shader model major version, and i indicates the shader model minor version.
 * @param target A nul-terminated string specifying the Direct3D shader model.
 * @param o_stage Pointer to a three-byte buffer that on return stores the nul-terminated shader stage string, for example "cs", "vs", "gs" or "fs".
 * @param o_sm_major On return, stores the major version component of the shader model target profile.
 * @param o_sm_minor On return, stores the minor version component of the shader model target profile.
 * @return Zero if the shader model was successfully parsed, or non-zero if the target string could not be parsed.
 */
GPUCC_API(int32_t)
gpuccExtractDirect3DShaderModel
(
    char const *target, 
    char    o_stage[3], 
    int    *o_sm_major, 
    int    *o_sm_minor
);

/* @summary Construct a GPUCC_RESULT value specifying the GpuCC result code and taking the platform result code from errno.
 * This function is used when an error occurs after calling a standard C library function.
 * @param library_result One of the values of the GPUCC_RESULT_CODE enumeration.
 * @return The GPUCC_RESULT structure.
 */
GPUCC_API(struct GPUCC_RESULT)
gpuccMakeResult_errno
(
    int32_t library_result
);

/* @summary Construct a GPUCC_RESULT value specifying both a GpuCC result code and a Win32 API result.
 * This function is used when an error occurs after calling a Win32 API function.
 * @param library_result One of the values of the GPUCC_RESULT_CODE enumeration.
 * @param platform_result The value returned by the Win32 GetLastError function.
 * @return The GPUCC_RESULT structure.
 */
GPUCC_API(struct GPUCC_RESULT)
gpuccMakeResult_Win32
(
    int32_t library_result, 
    DWORD  platform_result
);

/* @summary Construct a GPUCC_RESULT value specifying both a GpuCC result code and a COM HRESULT.
 * This function is used when an error occurs after calling a COM API function.
 * @param library_result One of the values of the GPUCC_RESULT_CODE enumeration.
 * @param platform_result The value returned by the COM API call.
 * @return The GPUCC_RESULT structure.
 */
GPUCC_API(struct GPUCC_RESULT)
gpuccMakeResult_HRESULT
(
    HRESULT platform_result
);

#ifdef __cplusplus
}; /* extern "C" */
#endif

#endif /* __GPUCC_INTERNAL_WIN32_H__ */
