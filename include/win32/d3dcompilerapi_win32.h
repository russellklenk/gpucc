/**
 * @summary d3dcompilerapi_win32.h: Define an interface used for dynamically 
 * loading d3dcompiler_##.dll into the process address space and resolving 
 * available entry points. This is needed so that HLSL source code can be 
 * compiled into byte code at runtime, which is useful during development.
 * Any entry points specific to Direct3D versions prior to 12 are not present.
 */
#ifndef __GPUCC_D3DCOMPILERAPI_WIN32_H__
#define __GPUCC_D3DCOMPILERAPI_WIN32_H__

#pragma once

#ifndef GPUCC_NO_INCLUDES
#   ifndef __GPUCC_H__
#       include "gpucc.h"
#   endif
#   include <Windows.h>
#   include <d3dcompiler.h>
#endif

/**
 * Function pointer types.
 */
typedef HRESULT (WINAPI *PFN_D3DCompile                   )(LPCVOID, SIZE_T, LPCSTR, D3D_SHADER_MACRO const*, ID3DInclude*, LPCSTR, LPCSTR, UINT, UINT, ID3DBlob**, ID3DBlob**);
typedef HRESULT (WINAPI *PFN_D3DCompile2                  )(LPCVOID, SIZE_T, LPCSTR, D3D_SHADER_MACRO const*, ID3DInclude*, LPCSTR, LPCSTR, UINT, UINT, UINT, LPCVOID, SIZE_T, ID3DBlob**, ID3DBlob**);
typedef HRESULT (WINAPI *PFN_D3DCompileFromFile           )(LPCWSTR, D3D_SHADER_MACRO const*, ID3DInclude*, LPCSTR, LPCSTR, UINT, UINT, ID3DBlob**, ID3DBlob**);
typedef HRESULT (WINAPI *PFN_D3DCompressShaders           )(UINT, D3D_SHADER_DATA*, UINT, ID3DBlob**);
typedef HRESULT (WINAPI *PFN_D3DCreateBlob                )(SIZE_T, ID3DBlob**);
typedef HRESULT (WINAPI *PFN_D3DDecompressShaders         )(LPCVOID, SIZE_T, UINT, UINT, UINT*, UINT, ID3DBlob**, UINT*);
typedef HRESULT (WINAPI *PFN_D3DDisassemble               )(LPCVOID, SIZE_T, UINT, LPCSTR, ID3DBlob**);
typedef HRESULT (WINAPI *PFN_D3DDisassembleRegion         )(LPCVOID, SIZE_T, UINT, LPCSTR, SIZE_T, SIZE_T, SIZE_T*, ID3DBlob**);
typedef HRESULT (WINAPI *PFN_D3DGetBlobPart               )(LPCVOID, SIZE_T, D3D_BLOB_PART, UINT, ID3DBlob**);
typedef HRESULT (WINAPI *PFN_D3DGetDebugInfo              )(LPCVOID, SIZE_T, ID3DBlob**);
typedef HRESULT (WINAPI *PFN_D3DGetTraceInstructionOffsets)(LPCVOID, SIZE_T, UINT, SIZE_T, SIZE_T, SIZE_T*, SIZE_T*);
typedef HRESULT (WINAPI *PFN_D3DPreprocess                )(LPCVOID, SIZE_T, LPCSTR, D3D_SHADER_MACRO const*, ID3DInclude*, ID3DBlob**, ID3DBlob**);
typedef HRESULT (WINAPI *PFN_D3DReadFileToBlob            )(LPCWSTR, ID3DBlob**);
typedef HRESULT (WINAPI *PFN_D3DReflect                   )(LPCVOID, SIZE_T, REFIID, void**);
typedef HRESULT (WINAPI *PFN_D3DReflectLibrary            )(LPCVOID, SIZE_T, REFIID, void**);
typedef HRESULT (WINAPI *PFN_D3DSetBlobPart               )(LPCVOID, SIZE_T, D3D_BLOB_PART, UINT, LPCVOID, SIZE_T, ID3DBlob**);
typedef HRESULT (WINAPI *PFN_D3DStripShader               )(LPCVOID, SIZE_T, UINT, ID3DBlob**);
typedef HRESULT (WINAPI *PFN_D3DWriteBlobToFile           )(ID3DBlob*, LPCWSTR, BOOL);

/* @summary Define the data associated with the dispatch table used to call functions from d3dcompiler.dll.
 */
typedef struct D3DCOMPILERAPI_DISPATCH {
    PFN_D3DCompile                    D3DCompile;
    PFN_D3DCompile2                   D3DCompile2;
    PFN_D3DCompileFromFile            D3DCompileFromFile;
    PFN_D3DCompressShaders            D3DCompressShaders;
    PFN_D3DCreateBlob                 D3DCreateBlob;
    PFN_D3DDecompressShaders          D3DDecompressShaders;
    PFN_D3DDisassemble                D3DDisassemble;
    PFN_D3DDisassembleRegion          D3DDisassembleRegion;
    PFN_D3DGetBlobPart                D3DGetBlobPart;
    PFN_D3DGetDebugInfo               D3DGetDebugInfo;
    PFN_D3DGetTraceInstructionOffsets D3DGetTraceInstructionOffsets;
    PFN_D3DPreprocess                 D3DPreprocess;
    PFN_D3DReadFileToBlob             D3DReadFileToBlob;
    PFN_D3DReflect                    D3DReflect;
    PFN_D3DReflectLibrary             D3DReflectLibrary;
    PFN_D3DSetBlobPart                D3DSetBlobPart;
    PFN_D3DStripShader                D3DStripShader;
    PFN_D3DWriteBlobToFile            D3DWriteBlobToFile;
    HMODULE                           ModuleHandle_D3DCompiler;
} D3DCOMPILERAPI_DISPATCH;

/* @summary Define a series of flags that can be bitwise OR'd together to control loader behavior.
 */
typedef enum D3DCOMPILERAPI_LOADER_FLAGS {
    D3DCOMPILERAPI_LOADER_FLAGS_NONE       = (0UL << 0),                       /* No special behavior is requested. */
    D3DCOMPILERAPI_LOADER_FLAG_DEVELOPMENT = (1UL << 0),                       /* Attempt to resolve development-only entry points that are not available in submitted Windows Store applications. */
} D3DCOMPILERAPI_LOADER_FLAGS;

#ifdef __cplusplus
extern "C" {
#endif

/* @summary Load d3dcompiler.dll into the process address space and resolve entry points.
 * Any missing entry points are set to stub functions, so none of the function pointers will be NULL.
 * @param dispatch The dispatch table to populate.
 * @param loader_flags One or more values of the D3DCOMPILERAPI_LOADER_FLAGS enumeration.
 * @return Zero if the dispatch table is successfully populated.
 */
GPUCC_API(int)
D3DCompilerApiPopulateDispatch
(
    struct D3DCOMPILERAPI_DISPATCH *dispatch, 
    uint32_t                    loader_flags
);

/* @summary Determine whether the D3DCompiler API is supported on the host.
 * @param dispatch The dispatch table to query.
 * @return Non-zero if the D3DCompiler API is supported on the host, or zero otherwise.
 */
GPUCC_API(int)
D3DCompilerApiQuerySupport
(
    struct D3DCOMPILERAPI_DISPATCH *dispatch
);

/* @summary Free resources associated with a dispatch table.
 * This function invalidates the entry points associated with the dispatch table.
 * @param dispatch The dispatch table to invalidate.
 */
GPUCC_API(void)
D3DCompilerApiInvalidateDispatch
(
    struct D3DCOMPILERAPI_DISPATCH *dispatch
);

#ifdef __cplusplus
}; /* extern "C" */
#endif

#endif /* __GPUCC_D3DCOMPILERAPI_WIN32_H__ */

