/**
 * @summary dxccompilerapi_win32.h: Define an interface used for dynamically 
 * loading the newer Clang/LLVM-based dxcompiler.dll into the process address 
 * space and resolving available entry points. This is needed so that signed 
 * HLSL source code targeting SM 6.0 and later can be compiled into DXIL 
 * bytecode. Utilizing this compiler and the resulting DXIL bytecode requires 
 * Windows 10 Creators Update or later and a recent GPU driver.
 *
 * In addition to generating DXIL bytecode, the dxcompiler can also produce 
 * SPIR-V bytecode for use by Vulkan and OpenGL 4.5 and later from HLSL source.
 */
#ifndef __GPUCC_DXCCOMPILERAPI_WIN32_H__
#define __GPUCC_DXCCOMPILERAPI_WIN32_H__

#pragma once

#ifndef GPUCC_NO_INCLUDES
#   ifndef __GPUCC_H__
#       include "gpucc.h"
#   endif
#   include <Windows.h>
#   include <dxcapi.h> /* This requires a Windows 10 SDK */
#endif

/**
 * Function pointer types.
 */
typedef HRESULT (WINAPI *PFN_DxcCreateInstance )(REFCLSID, REFIID, LPVOID*);
typedef HRESULT (WINAPI *PFN_DxcCreateInstance2)(IMalloc*, REFCLSID, REFIID, LPVOID*);

/* @summary Define the data associated with the dispatch table used to call functions from dxcompiler.dll and dxil.dll.
 */
typedef struct DXCCOMPILERAPI_DISPATCH {
    PFN_DxcCreateInstance    DxcCreateInstance;
    PFN_DxcCreateInstance2   DxcCreateInstance2;
    HMODULE                  ModuleHandle_Dxil;
    HMODULE                  ModuleHandle_DxCompiler;
} DXCCOMPILERAPI_DISPATCH;

/* @summary Define a series of flags that can be bitwise OR'd together to control loader behavior.
 */
typedef enum DXCCOMPILERAPI_LOADER_FLAGS {
    DXCCOMPILERAPI_LOADER_FLAGS_NONE     = (0UL << 0),                         /* No special behavior is requested. */
} DXCCOMPILERAPI_LOADER_FLAGS;

#ifdef __cplusplus
extern "C" {
#endif

/* @summary Load dxcompiler.dll into the process address space and resolve entry points.
 * Any missing entry points are set to stub functions, so none of the function pointers will be NULL.
 * @param dispatch The dispatch table to populate.
 * @param loader_flags One or more values of the DXCCOMPILERAPI_LOADER_FLAGS enumeration.
 * @return Zero if the dispatch table is successfully populated.
 */
GPUCC_API(int)
DxcCompilerApiPopulateDispatch
(
    struct DXCCOMPILERAPI_DISPATCH *dispatch, 
    uint32_t                    loader_flags
);

/* @summary Determine whether the DxcCompiler API is supported on the host.
 * @param dispatch The dispatch table to query.
 * @return Non-zero if the DxcCompiler API is supported on the host, or zero otherwise.
 */
GPUCC_API(int)
DxcCompilerApiQuerySupport
(
    struct DXCCOMPILERAPI_DISPATCH *dispatch
);

/* @summary Determine whether the DxcCompiler API can produce signed DXIL on the host.
 * @param dispatch The dispatch table to query.
 * @return Non-zero if the dxil.dll is present on the host, or zero otherwise.
 */
GPUCC_API(int)
DxcCompilerApiQuerySigningSupport
(
    struct DXCCOMPILERAPI_DISPATCH *dispatch
);

/* @summary Free resources associated with a dispatch table.
 * This function invalidates the entry points associated with the dispatch table.
 * @param dispatch The dispatch table to invalidate.
 */
GPUCC_API(void)
DxcCompilerApiInvalidateDispatch
(
    struct DXCCOMPILERAPI_DISPATCH *dispatch
);

#ifdef __cplusplus
}; /* extern "C" */
#endif

#endif /* __GPUCC_DXCCOMPILERAPI_WIN32_H__ */

