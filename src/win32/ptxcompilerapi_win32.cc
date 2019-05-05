/**
 * @summary ptxcompilerapi_win32.cc: Implement the NVRTC compiler runtime loader
 * for Windows platforms.
 */
#include <assert.h>
#include "win32/ptxcompilerapi_win32.h"

/* @summary Define a general signature for a dynamically loaded function. 
 * Code will have to cast the function pointer to the specific type.
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

static const char*
nvrtcGetErrorString_Stub
(
    nvrtcResult result
)
{
    switch (result) {
        case NVRTC_SUCCESS                                    : return "NVRTC_SUCCESS";
        case NVRTC_ERROR_OUT_OF_MEMORY                        : return "NVRTC_ERROR_OUT_OF_MEMORY";
        case NVRTC_ERROR_PROGRAM_CREATION_FAILURE             : return "NVRTC_ERROR_PROGRAM_CREATION_FAILURE";
        case NVRTC_ERROR_INVALID_INPUT                        : return "NVRTC_ERROR_INVALID_INPUT";
        case NVRTC_ERROR_INVALID_PROGRAM                      : return "NVRTC_ERROR_INVALID_PROGRAM";
        case NVRTC_ERROR_INVALID_OPTION                       : return "NVRTC_ERROR_INVALID_OPTION";
        case NVRTC_ERROR_COMPILATION                          : return "NVRTC_ERROR_COMPILATION";
        case NVRTC_ERROR_BUILTIN_OPERATION_FAILURE            : return "NVRTC_ERROR_BUILTIN_OPERATION_FAILURE";
        case NVRTC_ERROR_NO_NAME_EXPRESSIONS_AFTER_COMPILATION: return "NVRTC_ERROR_NO_NAME_EXPRESSIONS_AFTER_COMPILATION";
        case NVRTC_ERROR_NO_LOWERED_NAMES_BEFORE_COMPILATION  : return "NVRTC_ERROR_NO_LOWERED_NAMES_BEFORE_COMPILATION";
        case NVRTC_ERROR_NAME_EXPRESSION_NOT_VALID            : return "NVRTC_ERROR_NAME_EXPRESSION_NOT_VALID";
        case NVRTC_ERROR_INTERNAL_ERROR                       : return "NVRTC_ERROR_INTERNAL_ERROR";
        default                                               : return "nvrtcResult (unknown)";
    }
}

static nvrtcResult
nvrtcVersion_Stub
(
    int *major, 
    int *minor
)
{
    if (major) *major = 0;
    if (minor) *minor = 0;
    return NVRTC_ERROR_INTERNAL_ERROR;
}

static nvrtcResult
nvrtcCreateProgram_Stub
(
    struct _nvrtcProgram      **prog, 
    const char                  *src, 
    const char                 *name, 
    int                   numHeaders, 
    const char * const      *headers, 
    const char * const *includeNames
)
{
    if (prog) *prog = NULL;
    UNREFERENCED_PARAMETER(src);
    UNREFERENCED_PARAMETER(name);
    UNREFERENCED_PARAMETER(numHeaders);
    UNREFERENCED_PARAMETER(headers);
    UNREFERENCED_PARAMETER(includeNames);
    return NVRTC_ERROR_PROGRAM_CREATION_FAILURE;
}

static nvrtcResult
nvrtcDestroyProgram_Stub
(
    struct _nvrtcProgram **prog
)
{
    UNREFERENCED_PARAMETER(prog);
    return NVRTC_ERROR_INVALID_PROGRAM;
}

static nvrtcResult
nvrtcCompileProgram_Stub
(
    struct _nvrtcProgram  *prog, 
    int              numOptions, 
    const char * const *options
)
{
    UNREFERENCED_PARAMETER(prog);
    UNREFERENCED_PARAMETER(numOptions);
    UNREFERENCED_PARAMETER(options);
    return NVRTC_ERROR_INVALID_PROGRAM;
}

static nvrtcResult
nvrtcGetPTXSize_Stub
(
    struct _nvrtcProgram *prog, 
    size_t         *ptxSizeRet
)
{
    if (ptxSizeRet) *ptxSizeRet = 0;
    UNREFERENCED_PARAMETER(prog);
    return NVRTC_ERROR_INVALID_PROGRAM;
}

static nvrtcResult
nvrtcGetPTX_Stub
(
    struct _nvrtcProgram *prog, 
    char                  *ptx
)
{
    UNREFERENCED_PARAMETER(prog);
    UNREFERENCED_PARAMETER(ptx);
    return NVRTC_ERROR_INVALID_PROGRAM;
}

static nvrtcResult
nvrtcGetProgramLogSize_Stub
(
    struct _nvrtcProgram *prog, 
    size_t         *logSizeRet
)
{
    if (logSizeRet) *logSizeRet = 0;
    UNREFERENCED_PARAMETER(prog);
    return NVRTC_ERROR_INVALID_PROGRAM;
}

static nvrtcResult
nvrtcGetProgramLog_Stub
(
    struct _nvrtcProgram *prog, 
    char                  *log
)
{
    UNREFERENCED_PARAMETER(prog);
    UNREFERENCED_PARAMETER(log);
    return NVRTC_ERROR_INVALID_PROGRAM;
}

static nvrtcResult
nvrtcAddNameExpression_Stub
(
    struct _nvrtcProgram         *prog, 
    const char * const name_expression
)
{
    UNREFERENCED_PARAMETER(prog);
    UNREFERENCED_PARAMETER(name_expression);
    return NVRTC_ERROR_INVALID_PROGRAM;
}

static nvrtcResult 
nvrtcGetLoweredName_Stub
(
    struct _nvrtcProgram         *prog, 
    const char * const name_expression, 
    const char          **lowered_name
)
{
    if (lowered_name) *lowered_name = NULL;
    UNREFERENCED_PARAMETER(prog);
    UNREFERENCED_PARAMETER(name_expression);
    return NVRTC_ERROR_INVALID_PROGRAM;
}

GPUCC_API(int)
PtxCompilerApiPopulateDispatch
(
    struct PTXCOMPILERAPI_DISPATCH *dispatch, 
    uint32_t                    loader_flags
)
{
    HMODULE nvrtc_dll = NULL;

    assert(dispatch  != NULL);
    UNREFERENCED_PARAMETER(loader_flags);

    nvrtc_dll = LoadLibraryW(L"nvrtc64_101_0.dll");
    RuntimeFunctionResolve(dispatch, nvrtc_dll, nvrtcGetErrorString);
    RuntimeFunctionResolve(dispatch, nvrtc_dll, nvrtcVersion);
    RuntimeFunctionResolve(dispatch, nvrtc_dll, nvrtcCreateProgram);
    RuntimeFunctionResolve(dispatch, nvrtc_dll, nvrtcDestroyProgram);
    RuntimeFunctionResolve(dispatch, nvrtc_dll, nvrtcCompileProgram);
    RuntimeFunctionResolve(dispatch, nvrtc_dll, nvrtcGetPTXSize);
    RuntimeFunctionResolve(dispatch, nvrtc_dll, nvrtcGetPTX);
    RuntimeFunctionResolve(dispatch, nvrtc_dll, nvrtcGetProgramLogSize);
    RuntimeFunctionResolve(dispatch, nvrtc_dll, nvrtcGetProgramLog);
    RuntimeFunctionResolve(dispatch, nvrtc_dll, nvrtcAddNameExpression);
    RuntimeFunctionResolve(dispatch, nvrtc_dll, nvrtcGetLoweredName);
    dispatch->ModuleHandle_nvrtc64 = nvrtc_dll;
    return nvrtc_dll != NULL;
}

GPUCC_API(int)
PtxCompilerApiQuerySupport
(
    struct PTXCOMPILERAPI_DISPATCH *dispatch
)
{
    return dispatch->ModuleHandle_nvrtc64 != NULL;
}

GPUCC_API(void)
PtxCompilerApiInvalidateDispatch
(
    struct PTXCOMPILERAPI_DISPATCH *dispatch
)
{
    dispatch->nvrtcGetErrorString    = nvrtcGetErrorString_Stub;
    dispatch->nvrtcVersion           = nvrtcVersion_Stub;
    dispatch->nvrtcCreateProgram     = nvrtcCreateProgram_Stub;
    dispatch->nvrtcDestroyProgram    = nvrtcDestroyProgram_Stub;
    dispatch->nvrtcCompileProgram    = nvrtcCompileProgram_Stub;
    dispatch->nvrtcGetPTXSize        = nvrtcGetPTXSize_Stub;
    dispatch->nvrtcGetPTX            = nvrtcGetPTX_Stub;
    dispatch->nvrtcGetProgramLogSize = nvrtcGetProgramLogSize_Stub;
    dispatch->nvrtcGetProgramLog     = nvrtcGetProgramLog_Stub;
    dispatch->nvrtcAddNameExpression = nvrtcAddNameExpression_Stub;
    dispatch->nvrtcGetLoweredName    = nvrtcGetLoweredName_Stub;
    if (dispatch->ModuleHandle_nvrtc64) {
        FreeLibrary(dispatch->ModuleHandle_nvrtc64);
        dispatch->ModuleHandle_nvrtc64 = NULL;
    }
}

