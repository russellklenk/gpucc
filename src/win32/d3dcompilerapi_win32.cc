/**
 * @summary d3dcompilerapi_win32.cc: Implement the D3DCompiler runtime loader 
 * for Windows platforms.
 */
#include <assert.h>
#include "win32/d3dcompilerapi_win32.h"

/* @summary Define a general signature for a dynamically loaded function. 
 * Code will have to case the function pointer to the specific type.
 */
typedef int (*PFN_Unknown)(void);

/* @summary Helper macro for populating a dispatch table with functions loaded at runtime.
 * If the function is not found, the entry point is updated to point to a stub implementation provided by the caller.
 * This macro relies on specific naming conventions:
 * - The signature must be PFN_BlahBlahBlah where BlahBlahBlah corresponds to the _func argument.
 * - The dispatch table field must be BlahBlahBlah where BlahBlahBlah corresponds to the _func argument.
 * - The stub function must be named BlahBlahBlah_Stub where BlahBlahBlah corresponds to the _func argument.
 * @param _disp A pointer to the dispatch table to populate.
 * @param _module The HMODULE representing the module loaded into the process address space.
 * @param _func The name of the function to dynamically load.
 */
#ifndef RuntimeFunctionResolve
#define RuntimeFunctionResolve(_disp, _module, _func)                          \
    for (;;) {                                                                 \
        (_disp)->_func=(PFN_##_func) RuntimeModuleResolve((_module), #_func);  \
        if ((_disp)->_func == NULL) {                                          \
            (_disp)->_func  = _func##_Stub;                                    \
        } break;                                                               \
    }
#endif

static PFN_Unknown
RuntimeModuleResolve
(
    HMODULE     module, 
    char const *symbol
)
{
    if (module != NULL) {
        return (PFN_Unknown) GetProcAddress(module, symbol);
    } else {
        return (PFN_Unknown) NULL;
    }
}

static HRESULT WINAPI
D3DCompile_Stub
(
    LPCVOID                 pSrcData, 
    SIZE_T               srcDataSize, 
    LPCSTR               pSourceName, 
    D3D_SHADER_MACRO const *pDefines, 
    ID3DInclude            *pInclude, 
    LPCSTR               pEntrypoint, 
    LPCSTR                   pTarget, 
    UINT                      flags1, 
    UINT                      flags2, 
    ID3DBlob                **ppCode, 
    ID3DBlob           **ppErrorMsgs
)
{
    if (ppCode     )      *ppCode = NULL;
    if (ppErrorMsgs) *ppErrorMsgs = NULL;
    UNREFERENCED_PARAMETER(pSrcData);
    UNREFERENCED_PARAMETER(srcDataSize);
    UNREFERENCED_PARAMETER(pSourceName);
    UNREFERENCED_PARAMETER(pDefines);
    UNREFERENCED_PARAMETER(pInclude);
    UNREFERENCED_PARAMETER(pEntrypoint);
    UNREFERENCED_PARAMETER(pTarget);
    UNREFERENCED_PARAMETER(flags1);
    UNREFERENCED_PARAMETER(flags2);
    return E_NOTIMPL;
}

static HRESULT WINAPI
D3DCompile2_Stub
(
    LPCVOID                 pSrcData, 
    SIZE_T               srcDataSize, 
    LPCSTR               pSourceName, 
    D3D_SHADER_MACRO const *pDefines, 
    ID3DInclude            *pInclude, 
    LPCSTR               pEntrypoint, 
    LPCSTR                   pTarget, 
    UINT                      flags1, 
    UINT                      flags2, 
    UINT          secondaryDataFlags, 
    LPCVOID           pSecondaryData, 
    SIZE_T         secondaryDataSize, 
    ID3DBlob                **ppCode, 
    ID3DBlob           **ppErrorMsgs
)
{
    if (ppCode     )      *ppCode = NULL;
    if (ppErrorMsgs) *ppErrorMsgs = NULL;
    UNREFERENCED_PARAMETER(pSrcData);
    UNREFERENCED_PARAMETER(srcDataSize);
    UNREFERENCED_PARAMETER(pSourceName);
    UNREFERENCED_PARAMETER(pDefines);
    UNREFERENCED_PARAMETER(pInclude);
    UNREFERENCED_PARAMETER(pEntrypoint);
    UNREFERENCED_PARAMETER(pTarget);
    UNREFERENCED_PARAMETER(flags1);
    UNREFERENCED_PARAMETER(flags2);
    UNREFERENCED_PARAMETER(secondaryDataFlags);
    UNREFERENCED_PARAMETER(pSecondaryData);
    UNREFERENCED_PARAMETER(secondaryDataSize);
    return E_NOTIMPL;
}

static HRESULT WINAPI
D3DCompileFromFile_Stub /* stripped API */
(
    LPCWSTR                pFileName, 
    D3D_SHADER_MACRO const *pDefines, 
    ID3DInclude            *pInclude, 
    LPCSTR               pEntrypoint, 
    LPCSTR                   pTarget, 
    UINT                      flags1, 
    UINT                      flags2, 
    ID3DBlob                **ppCode, 
    ID3DBlob           **ppErrorMsgs
)
{
    if (ppCode     )      *ppCode = NULL;
    if (ppErrorMsgs) *ppErrorMsgs = NULL;
    UNREFERENCED_PARAMETER(pFileName);
    UNREFERENCED_PARAMETER(pDefines);
    UNREFERENCED_PARAMETER(pInclude);
    UNREFERENCED_PARAMETER(pEntrypoint);
    UNREFERENCED_PARAMETER(pTarget);
    UNREFERENCED_PARAMETER(flags1);
    UNREFERENCED_PARAMETER(flags2);
    return E_NOTIMPL;
}

static HRESULT WINAPI
D3DCompressShaders_Stub /* stripped API */
(
    UINT             uNumShaders, 
    D3D_SHADER_DATA *pShaderData, 
    UINT                  uFlags, 
    ID3DBlob  **ppCompressedData
)
{
    if (ppCompressedData) *ppCompressedData = NULL;
    UNREFERENCED_PARAMETER(uNumShaders);
    UNREFERENCED_PARAMETER(pShaderData);
    UNREFERENCED_PARAMETER(uFlags);
    return E_NOTIMPL;
}

static HRESULT WINAPI
D3DCreateBlob_Stub
(
    SIZE_T       size, 
    ID3DBlob **ppBlob
)
{
    if (ppBlob) *ppBlob = NULL;
    UNREFERENCED_PARAMETER(size);
    return E_NOTIMPL;
}

static HRESULT WINAPI
D3DDecompressShaders_Stub /* stripped API */
(
    LPCVOID     pSrcData, 
    SIZE_T   srcDataSize, 
    UINT     uNumShaders, 
    UINT     uStartIndex, 
    UINT       *pIndices, 
    UINT          uFlags, 
    ID3DBlob **ppShaders, 
    UINT  *pTotalShaders
)
{
    if (ppShaders    ) *ppShaders = NULL;
    if (pTotalShaders) *pTotalShaders = 0;
    UNREFERENCED_PARAMETER(pSrcData);
    UNREFERENCED_PARAMETER(srcDataSize);
    UNREFERENCED_PARAMETER(uNumShaders);
    UNREFERENCED_PARAMETER(uStartIndex);
    UNREFERENCED_PARAMETER(pIndices);
    UNREFERENCED_PARAMETER(uFlags);
    return E_NOTIMPL;
}

static HRESULT WINAPI
D3DDisassemble_Stub
(
    LPCVOID         pSrcData, 
    SIZE_T       srcDataSize, 
    UINT               flags, 
    LPCSTR        szComments, 
    ID3DBlob **ppDisassembly
)
{
    if (ppDisassembly) *ppDisassembly = NULL;
    UNREFERENCED_PARAMETER(pSrcData);
    UNREFERENCED_PARAMETER(srcDataSize);
    UNREFERENCED_PARAMETER(flags);
    UNREFERENCED_PARAMETER(szComments);
    return E_NOTIMPL;
}

static HRESULT WINAPI
D3DDisassembleRegion_Stub
(
    LPCVOID          pSrcData, 
    SIZE_T        srcDataSize, 
    UINT                flags, 
    LPCSTR         szComments, 
    SIZE_T    startByteOffset, 
    SIZE_T    numInstructions, 
    SIZE_T *pFinishByteOffset, 
    ID3DBlob  **ppDisassembly
)
{
    if (pFinishByteOffset) *pFinishByteOffset = startByteOffset;
    if (ppDisassembly    ) *ppDisassembly = NULL;
    UNREFERENCED_PARAMETER(pSrcData);
    UNREFERENCED_PARAMETER(srcDataSize);
    UNREFERENCED_PARAMETER(flags);
    UNREFERENCED_PARAMETER(szComments);
    UNREFERENCED_PARAMETER(numInstructions);
    return E_NOTIMPL;
}

static HRESULT WINAPI
D3DGetBlobPart_Stub
(
    LPCVOID   pSrcData, 
    SIZE_T srcDataSize, 
    D3D_BLOB_PART part, 
    UINT         flags, 
    ID3DBlob  **ppBlob
)
{
    if (ppBlob) *ppBlob = NULL;
    UNREFERENCED_PARAMETER(pSrcData);
    UNREFERENCED_PARAMETER(srcDataSize);
    UNREFERENCED_PARAMETER(part);
    UNREFERENCED_PARAMETER(flags);
    return E_NOTIMPL;
}

static HRESULT WINAPI
D3DGetDebugInfo_Stub /* stripped API */
(
    LPCVOID       pSrcData, 
    SIZE_T     srcDataSize, 
    ID3DBlob **ppDebugInfo
)
{
    if (ppDebugInfo) *ppDebugInfo = NULL;
    UNREFERENCED_PARAMETER(pSrcData);
    UNREFERENCED_PARAMETER(srcDataSize);
    return E_NOTIMPL;
}

static HRESULT WINAPI
D3DGetTraceInstructionOffsets_Stub
(
    LPCVOID             pSrcData, 
    SIZE_T           srcDataSize, 
    UINT                   flags, 
    SIZE_T startInstructionIndex, 
    SIZE_T       numInstructions, 
    SIZE_T             *pOffsets, 
    SIZE_T   *pTotalInstructions
)
{
    if (pOffsets          )           *pOffsets = 0;
    if (pTotalInstructions) *pTotalInstructions = 0;
    UNREFERENCED_PARAMETER(pSrcData);
    UNREFERENCED_PARAMETER(srcDataSize);
    UNREFERENCED_PARAMETER(flags);
    UNREFERENCED_PARAMETER(startInstructionIndex);
    UNREFERENCED_PARAMETER(numInstructions);
    return E_NOTIMPL;
}

static HRESULT WINAPI
D3DPreprocess_Stub
(
    LPCVOID                 pSrcData, 
    SIZE_T               srcDataSize, 
    LPCSTR               pSourceName, 
    D3D_SHADER_MACRO const *pDefines, 
    ID3DInclude            *pInclude, 
    ID3DBlob            **ppCodeText, 
    ID3DBlob           **ppErrorMsgs
)
{
    if (ppCodeText )  *ppCodeText = NULL;
    if (ppErrorMsgs) *ppErrorMsgs = NULL;
    UNREFERENCED_PARAMETER(pSrcData);
    UNREFERENCED_PARAMETER(srcDataSize);
    UNREFERENCED_PARAMETER(pSourceName);
    UNREFERENCED_PARAMETER(pDefines);
    UNREFERENCED_PARAMETER(pInclude);
    return E_NOTIMPL;
}

static HRESULT WINAPI
D3DReadFileToBlob_Stub /* stripped API */
(
    LPCWSTR     pFileName, 
    ID3DBlob **ppContents
)
{
    if (ppContents) *ppContents = NULL;
    UNREFERENCED_PARAMETER(pFileName);
    return E_NOTIMPL;
}

static HRESULT WINAPI
D3DReflect_Stub
(
    LPCVOID    pSrcData, 
    SIZE_T  srcDataSize, 
    REFIID         riid, 
    void  **ppReflector
)
{
    if (ppReflector) *ppReflector = NULL;
    UNREFERENCED_PARAMETER(pSrcData);
    UNREFERENCED_PARAMETER(srcDataSize);
    UNREFERENCED_PARAMETER(riid);
    return E_NOTIMPL;
}

static HRESULT WINAPI
D3DReflectLibrary_Stub
(
    LPCVOID   pSrcData, 
    SIZE_T srcDataSize, 
    REFIID        riid, 
    void **ppReflector
)
{
    if (ppReflector) *ppReflector = NULL;
    UNREFERENCED_PARAMETER(pSrcData);
    UNREFERENCED_PARAMETER(srcDataSize);
    UNREFERENCED_PARAMETER(riid);
    return E_NOTIMPL;
}

static HRESULT WINAPI
D3DSetBlobPart_Stub
(
    LPCVOID       pSrcData, 
    SIZE_T     srcDataSize, 
    D3D_BLOB_PART     part, 
    UINT             flags, 
    LPCVOID          pPart, 
    SIZE_T        partSize, 
    ID3DBlob **ppNewShader
)
{
    if (ppNewShader) *ppNewShader = NULL;
    UNREFERENCED_PARAMETER(pSrcData);
    UNREFERENCED_PARAMETER(srcDataSize);
    UNREFERENCED_PARAMETER(part);
    UNREFERENCED_PARAMETER(flags);
    UNREFERENCED_PARAMETER(pPart);
    UNREFERENCED_PARAMETER(partSize);
    return E_NOTIMPL;
}

static HRESULT WINAPI
D3DStripShader_Stub
(
    LPCVOID   pShaderBytecode, 
    SIZE_T     bytecodeLength, 
    UINT          uStripFlags, 
    ID3DBlob **ppStrippedBlob
)
{
    if (ppStrippedBlob) *ppStrippedBlob = NULL;
    UNREFERENCED_PARAMETER(pShaderBytecode);
    UNREFERENCED_PARAMETER(bytecodeLength);
    UNREFERENCED_PARAMETER(uStripFlags);
    return E_NOTIMPL;
}

static HRESULT WINAPI
D3DWriteBlobToFile_Stub /* stripped API */
(
    ID3DBlob   *pBlob, 
    LPCWSTR pFileName, 
    BOOL   bOverwrite
)
{
    UNREFERENCED_PARAMETER(pBlob);
    UNREFERENCED_PARAMETER(pFileName);
    UNREFERENCED_PARAMETER(bOverwrite);
    return E_NOTIMPL;
}

GPUCC_API(int)
D3DCompilerApiPopulateDispatch
(
    struct D3DCOMPILERAPI_DISPATCH *dispatch, 
    uint32_t                    loader_flags
)
{
    HMODULE d3dcompiler_dll = NULL;

    assert(dispatch != NULL);

    d3dcompiler_dll  = LoadLibraryW(L"d3dcompiler_47.dll");
    if (loader_flags & D3DCOMPILERAPI_LOADER_FLAG_DEVELOPMENT) {
        RuntimeFunctionResolve(dispatch, d3dcompiler_dll, D3DCompileFromFile);
        RuntimeFunctionResolve(dispatch, d3dcompiler_dll, D3DCompressShaders);
        RuntimeFunctionResolve(dispatch, d3dcompiler_dll, D3DDecompressShaders);
        RuntimeFunctionResolve(dispatch, d3dcompiler_dll, D3DGetDebugInfo);
        RuntimeFunctionResolve(dispatch, d3dcompiler_dll, D3DReadFileToBlob);
        RuntimeFunctionResolve(dispatch, d3dcompiler_dll, D3DWriteBlobToFile);
    } else { /* development-only APIs not available */
        dispatch->D3DCompileFromFile   = D3DCompileFromFile_Stub;
        dispatch->D3DCompressShaders   = D3DCompressShaders_Stub;
        dispatch->D3DDecompressShaders = D3DDecompressShaders_Stub;
        dispatch->D3DGetDebugInfo      = D3DGetDebugInfo_Stub;
        dispatch->D3DReadFileToBlob    = D3DReadFileToBlob_Stub;
        dispatch->D3DWriteBlobToFile   = D3DWriteBlobToFile_Stub;
    }
    RuntimeFunctionResolve(dispatch    , d3dcompiler_dll, D3DCompile);
    RuntimeFunctionResolve(dispatch    , d3dcompiler_dll, D3DCompile2);
    RuntimeFunctionResolve(dispatch    , d3dcompiler_dll, D3DCreateBlob);
    RuntimeFunctionResolve(dispatch    , d3dcompiler_dll, D3DDisassemble);
    RuntimeFunctionResolve(dispatch    , d3dcompiler_dll, D3DDisassembleRegion);
    RuntimeFunctionResolve(dispatch    , d3dcompiler_dll, D3DGetBlobPart);
    RuntimeFunctionResolve(dispatch    , d3dcompiler_dll, D3DGetTraceInstructionOffsets);
    RuntimeFunctionResolve(dispatch    , d3dcompiler_dll, D3DPreprocess);
    RuntimeFunctionResolve(dispatch    , d3dcompiler_dll, D3DReflect);
    RuntimeFunctionResolve(dispatch    , d3dcompiler_dll, D3DReflectLibrary);
    RuntimeFunctionResolve(dispatch    , d3dcompiler_dll, D3DSetBlobPart);
    RuntimeFunctionResolve(dispatch    , d3dcompiler_dll, D3DStripShader);
    dispatch->ModuleHandle_D3DCompiler = d3dcompiler_dll;
    return d3dcompiler_dll != NULL;
}

GPUCC_API(int)
D3DCompilerApiQuerySupport
(
    struct D3DCOMPILERAPI_DISPATCH *dispatch
)
{
    return dispatch->ModuleHandle_D3DCompiler != NULL;
}

GPUCC_API(void)
D3DCompilerApiInvalidateDispatch
(
    struct D3DCOMPILERAPI_DISPATCH *dispatch
)
{
    dispatch->D3DCompile                    = D3DCompile_Stub;
    dispatch->D3DCompile2                   = D3DCompile2_Stub;
    dispatch->D3DCompileFromFile            = D3DCompileFromFile_Stub;
    dispatch->D3DCompressShaders            = D3DCompressShaders_Stub;
    dispatch->D3DCreateBlob                 = D3DCreateBlob_Stub;
    dispatch->D3DDecompressShaders          = D3DDecompressShaders_Stub;
    dispatch->D3DDisassemble                = D3DDisassemble_Stub;
    dispatch->D3DDisassembleRegion          = D3DDisassembleRegion_Stub;
    dispatch->D3DGetBlobPart                = D3DGetBlobPart_Stub;
    dispatch->D3DGetDebugInfo               = D3DGetDebugInfo_Stub;
    dispatch->D3DGetTraceInstructionOffsets = D3DGetTraceInstructionOffsets_Stub;
    dispatch->D3DPreprocess                 = D3DPreprocess_Stub;
    dispatch->D3DReadFileToBlob             = D3DReadFileToBlob_Stub;
    dispatch->D3DReflect                    = D3DReflect_Stub;
    dispatch->D3DReflectLibrary             = D3DReflectLibrary_Stub;
    dispatch->D3DSetBlobPart                = D3DSetBlobPart_Stub;
    dispatch->D3DStripShader                = D3DStripShader_Stub;
    dispatch->D3DWriteBlobToFile            = D3DWriteBlobToFile_Stub;
    if (dispatch->ModuleHandle_D3DCompiler) {
        FreeLibrary(dispatch->ModuleHandle_D3DCompiler);
        dispatch->ModuleHandle_D3DCompiler  = NULL;
    }
}

