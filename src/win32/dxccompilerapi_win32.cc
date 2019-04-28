/**
 * @summary dxccompilerapi_win32.cc: Implement the Dxc compiler runtime loader 
 * for Windows platforms.
 */
#include <assert.h>
#include "win32/dxccompilerapi_win32.h"

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
DxcCreateInstance_Stub
(
    REFCLSID rclsid, 
    REFIID     riid, 
    LPVOID     *ppv
)
{
    if (ppv) *ppv = NULL;
    UNREFERENCED_PARAMETER(rclsid);
    UNREFERENCED_PARAMETER(riid);
    return E_NOTIMPL;
}

static HRESULT WINAPI
DxcCreateInstance2_Stub
(
    IMalloc *pMalloc, 
    REFCLSID  rclsid, 
    REFIID      riid, 
    LPVOID      *ppv
)
{
    if (ppv) *ppv = NULL;
    UNREFERENCED_PARAMETER(pMalloc);
    UNREFERENCED_PARAMETER(rclsid);
    UNREFERENCED_PARAMETER(riid);
    return E_NOTIMPL;
}

GPUCC_API(int)
DxcCompilerApiPopulateDispatch
(
    struct DXCCOMPILERAPI_DISPATCH *dispatch, 
    uint32_t                    loader_flags
)
{
    HMODULE dxcompiler_dll = NULL;
    HMODULE       dxil_dll = NULL;

    assert(dispatch != NULL);
    UNREFERENCED_PARAMETER(loader_flags);

    dxcompiler_dll   = LoadLibraryW(L"dxcompiler.dll");
    dxil_dll         = LoadLibraryW(L"dxil.dll");
    RuntimeFunctionResolve(dispatch   , dxcompiler_dll, DxcCreateInstance);
    RuntimeFunctionResolve(dispatch   , dxcompiler_dll, DxcCreateInstance2);
    dispatch->ModuleHandle_DxCompiler = dxcompiler_dll;
    dispatch->ModuleHandle_Dxil       = dxil_dll;
    return dxcompiler_dll != NULL;
}

GPUCC_API(int)
DxcCompilerApiQuerySupport
(
    struct DXCCOMPILERAPI_DISPATCH *dispatch
)
{
    return dispatch->ModuleHandle_DxCompiler != NULL;
}

GPUCC_API(int)
DxcCompilerApiQuerySigningSupport
(
    struct DXCCOMPILERAPI_DISPATCH *dispatch
)
{
    return dispatch->ModuleHandle_Dxil != NULL;
}

GPUCC_API(void)
DxcCompilerApiInvalidateDispatch
(
    struct DXCCOMPILERAPI_DISPATCH *dispatch
)
{
    dispatch->DxcCreateInstance  = DxcCreateInstance_Stub;
    dispatch->DxcCreateInstance2 = DxcCreateInstance2_Stub;
    if (dispatch->ModuleHandle_DxCompiler) {
        FreeLibrary(dispatch->ModuleHandle_DxCompiler);
        dispatch->ModuleHandle_DxCompiler = NULL;
    }
    if (dispatch->ModuleHandle_Dxil) {
        FreeLibrary(dispatch->ModuleHandle_Dxil);
        dispatch->ModuleHandle_Dxil = NULL;
    }
}

