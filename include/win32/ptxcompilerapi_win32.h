/**
 * @summary ptxcompilerapi_win32.h: Define an interface used for dynamically 
 * loading nvrtc64_###_#.dll into the process address space and resolving the 
 * available entry points. This is needed so that CUDA C source code can be 
 * compiled into PTX bytecode.
 */
#ifndef __GPUCC_PTXCOMPILERAPI_WIN32_H__
#define __GPUCC_PTXCOMPILERAPI_WIN32_H__

#pragma once

#ifndef GPUCC_NO_INCLUDES
#   ifndef __GPUCC_H__
#       include "gpucc.h"
#   endif
#   include <Windows.h>
#   include "nvrtc.h"
#endif

/**
 * Function pointer types.
 */
typedef const char* (*PFN_nvrtcGetErrorString   )(nvrtcResult);
typedef nvrtcResult (*PFN_nvrtcVersion          )(int*, int*);
typedef nvrtcResult (*PFN_nvrtcCreateProgram    )(struct _nvrtcProgram**, const char*, const char*, int, const char * const *, const char * const *);
typedef nvrtcResult (*PFN_nvrtcDestroyProgram   )(struct _nvrtcProgram**);
typedef nvrtcResult (*PFN_nvrtcCompileProgram   )(struct _nvrtcProgram*, int, const char * const *);
typedef nvrtcResult (*PFN_nvrtcGetPTXSize       )(struct _nvrtcProgram*, size_t*);
typedef nvrtcResult (*PFN_nvrtcGetPTX           )(struct _nvrtcProgram*, char*);
typedef nvrtcResult (*PFN_nvrtcGetProgramLogSize)(struct _nvrtcProgram*, size_t*);
typedef nvrtcResult (*PFN_nvrtcGetProgramLog    )(struct _nvrtcProgram*, char*);
typedef nvrtcResult (*PFN_nvrtcAddNameExpression)(struct _nvrtcProgram*, const char * const);
typedef nvrtcResult (*PFN_nvrtcGetLoweredName   )(struct _nvrtcProgram*, const char * const, const char**);

/* @summary Define the data associated with the dispatch table used to call functions from nvrtc64_###_#.dll.
 */
typedef struct PTXCOMPILERAPI_DISPATCH {
    PFN_nvrtcGetErrorString    nvrtcGetErrorString;
    PFN_nvrtcVersion           nvrtcVersion;
    PFN_nvrtcCreateProgram     nvrtcCreateProgram;
    PFN_nvrtcDestroyProgram    nvrtcDestroyProgram;
    PFN_nvrtcCompileProgram    nvrtcCompileProgram;
    PFN_nvrtcGetPTXSize        nvrtcGetPTXSize;
    PFN_nvrtcGetPTX            nvrtcGetPTX;
    PFN_nvrtcGetProgramLogSize nvrtcGetProgramLogSize;
    PFN_nvrtcGetProgramLog     nvrtcGetProgramLog;
    PFN_nvrtcAddNameExpression nvrtcAddNameExpression;
    PFN_nvrtcGetLoweredName    nvrtcGetLoweredName;
    HMODULE                    ModuleHandle_nvrtc64;
} PTXCOMPILERAPI_DISPATCH;

/* @summary Define a series of flags that can be bitwise OR'd together to control loader behavior.
 */
typedef enum PTXCOMPILERAPI_LOADER_FLAGS {
    PTXCOMPILERAPI_LOADER_FLAGS_NONE     = (0UL << 0),                         /* No special behavior is requested. */
} PTXCOMPILERAPI_LOADER_FLAGS;

#ifdef __cplusplus
extern "C" {
#endif

/* @summary Load nvrtc64.dll into the process address space and resolve entry points.
 * Any missing entry points are set to stub functions, so none of the function pointers will be NULL.
 * @param dispatch The dispatch table to populate.
 * @param loader_flags One or more values of the PTXCOMPILERAPI_LOADER_FLAGS enumeration.
 * @return Zero if the dispatch table is successfully populated.
 */
GPUCC_API(int)
PtxCompilerApiPopulateDispatch
(
    struct PTXCOMPILERAPI_DISPATCH *dispatch, 
    uint32_t                    loader_flags
);

/* @summary Determine whether the NVRTC API is supported on the host.
 * @param dispatch The dispatch table to query.
 * @return Non-zero if the NVRTC API is supported on the host, or zero otherwise.
 */
GPUCC_API(int)
PtxCompilerApiQuerySupport
(
    struct PTXCOMPILERAPI_DISPATCH *dispatch
);

/* @summary Free resources associated with a dispatch table.
 * This function invalidates the entry points associated with the dispatch table.
 * @param dispatch The dispatch table to invalidate.
 */
GPUCC_API(void)
PtxCompilerApiInvalidateDispatch
(
    struct PTXCOMPILERAPI_DISPATCH *dispatch
);

#ifdef __cplusplus
}; /* extern "C" */
#endif

#endif /* __CPUCC_PTXCOMPILERAPI_WIN32_H__ */

