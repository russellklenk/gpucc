/**
 * gpucc_win32.cc: Implement the Windows-specific portions of the internal 
 * GpuCC library interface, along with the startup and shutdown routines.
 */
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <wchar.h>
#include "gpucc.h"
#include "gpucc_internal.h"

/* @summary Helper function to emit printf-style output to OutputDebugString.
 * The output can be viewed in the VS debugger output window or using DbgView.
 * GpuCC pipes debug output from startup and loading functions to this endpoint.
 * @param format A nul-terminated wide character string following printf formatting conventions.
 * @param ... Substitution arguments for the format string.
 */
static void
DebugPrintfW
(
    LPCWSTR format, 
    ...
)
{
    WCHAR buffer[2048];
    va_list       args;
    size_t      mchars = sizeof(buffer) / sizeof(WCHAR);
    int         nchars;

    va_start(args, format);
    nchars = vswprintf_s(buffer, format, args);
    va_end(args);
    if (nchars < 0) {
        assert(0 && "Error formatting debug output");
        OutputDebugStringW(L"GpuCC: Error formatting debug output.\n");
        return;
    }
    UNREFERENCED_PARAMETER(mchars);
    assert(nchars  <  (int)mchars && "Increase DebugPrintfW buffer size");
    OutputDebugStringW(buffer);
}

GPUCC_API(struct GPUCC_RESULT)
gpuccMakeResult
(
    int32_t library_result
)
{
    return GPUCC_RESULT { library_result, ERROR_SUCCESS };
}

GPUCC_API(struct GPUCC_RESULT)
gpuccMakeResult_Win32
(
    int32_t library_result, 
    DWORD  platform_result
)
{
    return GPUCC_RESULT { library_result, (int32_t) platform_result };
}

GPUCC_API(struct GPUCC_RESULT)
gpuccMakeResult_HRESULT
(
    HRESULT platform_result
)
{
    GPUCC_RESULT r;
    if (FAILED(platform_result)) {
        r.LibraryResult  = GPUCC_RESULT_CODE_PLATFORM_ERROR;
        r.PlatformResult =(int32_t) platform_result;
    } else {
        r.LibraryResult  = GPUCC_RESULT_CODE_SUCCESS;
        r.PlatformResult =(int32_t) platform_result;
    } return r;
}

GPUCC_API(struct GPUCC_RESULT)
gpuccGetLastResult
(
    void
)
{
    GPUCC_THREAD_CONTEXT_WIN32 *tctx = gpuccGetThreadContext_();
    return tctx->LastResult;
}

GPUCC_API(struct GPUCC_RESULT)
gpuccSetLastResult
(
    struct GPUCC_RESULT result
)
{
    GPUCC_THREAD_CONTEXT_WIN32 *tctx = gpuccGetThreadContext_();
    GPUCC_RESULT                prev = tctx->LastResult;
    tctx->LastResult = result;
    return prev;
}

GPUCC_API(struct GPUCC_RESULT)
gpuccStartup
(
    uint32_t gpucc_usage_mode
)
{
    GPUCC_PROCESS_CONTEXT_WIN32 *pctx = gpuccGetProcessContext_();
    GPUCC_RESULT               result = gpuccMakeResult(GPUCC_RESULT_CODE_SUCCESS);
    uint32_t        d3dcompiler_flags = D3DCOMPILERAPI_LOADER_FLAGS_NONE;

    if (gpucc_usage_mode != GPUCC_USAGE_MODE_OFFLINE && 
        gpucc_usage_mode != GPUCC_USAGE_MODE_RUNTIME) {
        return gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_USAGE_MODE);
    }
    if (pctx->StartupFlag != FALSE) {
        return gpuccMakeResult(GPUCC_RESULT_CODE_ALREADY_INITIALIZED);
    }

    /* When being used in offline mode, enable development-only features. */
    if (gpucc_usage_mode  == GPUCC_USAGE_MODE_OFFLINE) {
        d3dcompiler_flags |= D3DCOMPILERAPI_LOADER_FLAG_DEVELOPMENT;
    }

    /* Populate dispatch tables for any available compilers. */
    pctx->CompilerSupport = GPUCC_COMPILER_SUPPORT_NONE;
    if (D3DCompilerApiPopulateDispatch(&pctx->D3DCompiler_Dispatch, d3dcompiler_flags) == 0) {
        pctx->CompilerSupport |= GPUCC_COMPILER_SUPPORT_FXC;
    }
    /* ... */
    pctx->StartupFlag = TRUE;
    return result;
}

GPUCC_API(void)
gpuccShutdown
(
    void
)
{
    GPUCC_PROCESS_CONTEXT_WIN32 *pctx = gpuccGetProcessContext_();

    /* Invalidate the dispatch tables for any available compilers. */
    D3DCompilerApiInvalidateDispatch(&pctx->D3DCompiler_Dispatch);
    /* ... */

    pctx->CompilerSupport = GPUCC_COMPILER_SUPPORT_NONE;
    pctx->StartupFlag     = FALSE;
}

static char*
strputa
(
    uint8_t   *&dst, 
    char const *src
)
{
    char *p  =(char*) dst;
    if (src != nullptr) {
        size_t nb = strlen(src);
        memcpy(dst, src, nb); 
        dst += nb; /* string data */
    }
   *dst  = 0;  /* put nul */
    dst += 1;  /* nul */
    return p;
}

GPUCC_API(struct GPUCC_PROGRAM_COMPILER*)
gpuccCreateCompilerFxc
(
    struct GPUCC_PROGRAM_COMPILER_INIT *config
)
{   // Assume that config has been validated by gpuccCreateCompiler.
    GPUCC_PROCESS_CONTEXT_WIN32 *pctx = gpuccGetProcessContext_();
    GPUCC_COMPILER_FXC_WIN32     *fxc = nullptr;
    D3D_SHADER_MACRO          *macros = nullptr;
    uint8_t                     *base = nullptr;
    uint8_t                      *ptr = nullptr;
    size_t                     nbneed = 0;
    unsigned int          level_major = 0;
    unsigned int          level_minor = 0;
    int                       nbmatch = 0;
    char               shader_type[3] ={0, 0, 0};

    /* Validate the target profile. */
    if (config->TargetProfile == nullptr) {
        GPUCC_RESULT r = gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_TARGET_PROFILE);
        DebugPrintfW(L"GpuCC: A target profile, for example, \"cs_5_0\", is required by the FXC compiler.\n");
        gpuccSetLastResult(r);
        return nullptr;
    }
    if ((nbmatch = sscanf_s(config->TargetProfile, "%s_%u_%u", shader_type, 2, &level_major, &level_minor)) != 3) {
        /* The TargetProfile string doesn't match the expected format. */
        GPUCC_RESULT r = gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_TARGET_PROFILE);
        DebugPrintfW(L"GpuCC: Invalid target profile \"%s\" for the FXC compiler.\n", config->TargetProfile);
        gpuccSetLastResult(r);
        return nullptr;
    }
    if (_stricmp(shader_type, "cs") != 0 && 
        _stricmp(shader_type, "vs") != 0 && 
        _stricmp(shader_type, "ps") != 0 && 
        _stricmp(shader_type, "gs") != 0) {
        /* The target profile specifies an unexpected shader type. */
        GPUCC_RESULT r = gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_TARGET_PROFILE);
        DebugPrintfW(L"GpuCC: Invalid target profile \"%s\". Unexpected shader type \'%s\'.\n", config->TargetProfile, shader_type);
        gpuccSetLastResult(r);
        return nullptr;
    }
    if (level_major < 4) {
        /* Legacy D3D9 shader models are not supported. */
        GPUCC_RESULT r = gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_TARGET_PROFILE);
        DebugPrintfW(L"GpuCC: Invalid target profile \"%s\". Legacy shader models are not supported.\n", config->TargetProfile);
        gpuccSetLastResult(r);
        return nullptr;
    }
    if (level_major > 5) {
        /* Shader Model 6 and later requires using the newer dxc compiler. */
        GPUCC_RESULT r = gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_TARGET_PROFILE);
        DebugPrintfW(L"GpuCC: Invalid target profile \"%s\". Shader model 6+ require the newer dxc compiler and DXIL bytecode format.\n", config->TargetProfile);
        gpuccSetLastResult(r);
        return nullptr;
    }

    /* Determine the amount of memory required. */
    nbneed += sizeof(GPUCC_COMPILER_FXC_WIN32);
    nbneed += sizeof(D3D_SHADER_MACRO) * (config->DefineCount + 1);
    nbneed += strlen(config->TargetProfile) + 1;
    for (uint32_t i = 0, n = config->DefineCount; i < n; ++i) {
        nbneed += strlen(config->DefineSymbols[i]) + 1;
        nbneed += strlen(config->DefineValues [i]) + 1;
    }

    /* Allocate as a single block. */
    if ((base = (uint8_t*) malloc(nbneed)) == nullptr) {
        GPUCC_RESULT  _r = gpuccMakeResult(GPUCC_RESULT_CODE_OUT_OF_HOST_MEMORY);
        DebugPrintfW(L"GpuCC: Failed to allocate %Iu bytes to create FXC compiler.\n", nbneed);
        gpuccSetLastResult(_r);
        return nullptr;
    }
    fxc    =(GPUCC_COMPILER_FXC_WIN32*) base;
    ptr    = base + sizeof(GPUCC_COMPILER_FXC_WIN32);

    /* The array of D3D_SHADER_MACRO structures immediately follows the 
     * compiler record. The entries are initialized when interning the 
     * strings below.
     */
    macros =(D3D_SHADER_MACRO *) ptr;
    ptr   += sizeof(D3D_SHADER_MACRO) * (config->DefineCount + 1);

    /* Copy string data into the memory block. */
    fxc->ShaderModel = strputa(ptr, config->TargetProfile);
    for (uint32_t  i = 0, n = config->DefineCount; i < n; ++i) {
        macros[i].Name          = strputa(ptr, config->DefineSymbols[i]);
        macros[i].Definition    = strputa(ptr, config->DefineValues [i]);
    }
    /* The macro array is always terminated with a null entry. */
    macros[config->DefineCount] = D3D_SHADER_MACRO { nullptr, nullptr };

    /* Finish initializing the compiler structure. */
    fxc->CommonFields.CompilerType = GPUCC_COMPILER_TYPE_FXC;
    fxc->CommonFields.BytecodeType = GPUCC_BYTECODE_TYPE_DXBC;
    fxc->DispatchTable             =&pctx->D3DCompiler_Dispatch;
    fxc->DefineArray               = macros;
    fxc->DefineCount               = config->DefineCount;
    fxc->TargetRuntime             = config->TargetRuntime;
    return (struct GPUCC_PROGRAM_COMPILER*) fxc;
}

GPUCC_API(struct GPUCC_PROGRAM_COMPILER*)
gpuccCreateCompiler
(
    struct GPUCC_PROGRAM_COMPILER_INIT *config
)
{
    GPUCC_PROCESS_CONTEXT_WIN32   *pctx = gpuccGetProcessContext_();
    GPUCC_COMPILER_TYPE   compiler_type = GPUCC_COMPILER_TYPE_UNKNOWN;
    GPUCC_COMPILER_SUPPORT need_support = GPUCC_COMPILER_SUPPORT_NONE;
    struct GPUCC_PROGRAM_COMPILER    *c = nullptr;

    if (pctx->StartupFlag == FALSE) {
        GPUCC_RESULT r = gpuccMakeResult(GPUCC_RESULT_CODE_NOT_INITIALIZED);
        DebugPrintfW(L"GpuCC: Cannot create compiler. Call gpuccStartup() first.\n");
        gpuccSetLastResult(r);
        return nullptr;
    }
    if (config == nullptr) {
        assert(config != nullptr);
        GPUCC_RESULT r = gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_ARGUMENT);
        DebugPrintfW(L"GpuCC: No compliler configuration was specified.\n");
        gpuccSetLastResult(r);
        return nullptr;
    }
    if (config->DefineCount > 0 && (config->DefineSymbols == nullptr || config->DefineValues == nullptr)) {
        assert(config->DefineSymbols != nullptr);
        assert(config->DefineValues  != nullptr);
        GPUCC_RESULT r = gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_ARGUMENT);
        DebugPrintfW(L"GpuCC: DefineCount is non-zero, but symbols or values array is not specified.\n");
        gpuccSetLastResult(r);
        return nullptr;
    }
    switch (config->BytecodeType) {
        case GPUCC_BYTECODE_TYPE_UNKNOWN:
            break;
        case GPUCC_BYTECODE_TYPE_DXIL:
            compiler_type = GPUCC_COMPILER_TYPE_DXC;
            need_support  = GPUCC_COMPILER_SUPPORT_DXC;
            break;
        case GPUCC_BYTECODE_TYPE_DXBC:
            compiler_type = GPUCC_COMPILER_TYPE_FXC;
            need_support  = GPUCC_COMPILER_SUPPORT_FXC;
            break;
        case GPUCC_BYTECODE_TYPE_SPIRV:
            compiler_type = GPUCC_COMPILER_TYPE_SHADERC;
            need_support  = GPUCC_COMPILER_SUPPORT_SHADERC;
            break;
        case GPUCC_BYTECODE_TYPE_PTX:
            compiler_type = GPUCC_COMPILER_TYPE_NVRTC;
            need_support  = GPUCC_COMPILER_SUPPORT_NVRTC;
            break;
        default:
            break;
    }
    if (compiler_type == GPUCC_COMPILER_TYPE_UNKNOWN) {
        GPUCC_RESULT r = gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_BYTECODE_TYPE);
        DebugPrintfW(L"GpuCC: Unable to determine compiler type from bytecode type %s.\n", gpuccBytecodeTypeString(config->BytecodeType));
        gpuccSetLastResult(r);
        return nullptr;
    }
    if ((pctx->CompilerSupport & need_support) == 0) {
        GPUCC_RESULT r = gpuccMakeResult(GPUCC_RESULT_CODE_COMPILER_NOT_SUPPORTED);
        DebugPrintfW(L"GpuCC: The required compiler type %s is not supported on this host platform.\n", gpuccCompilerTypeString(compiler_type));
        gpuccSetLastResult(r);
        return nullptr;
    }

    switch (compiler_type) {
        case GPUCC_COMPILER_TYPE_UNKNOWN:
            c = nullptr;
            break;
        case GPUCC_COMPILER_TYPE_DXC:
            c = nullptr;
            break;
        case GPUCC_COMPILER_TYPE_FXC:
            c = gpuccCreateCompilerFxc(config);
            break;
        case GPUCC_COMPILER_TYPE_SHADERC:
            c = nullptr;
            break;
        case GPUCC_COMPILER_TYPE_NVRTC:
            c = nullptr;
            break;
        default:
            c = nullptr;
            break;
    }
    return c;
}

GPUCC_API(void)
gpuccDeleteCompiler
(
    struct GPUCC_PROGRAM_COMPILER *compiler
)
{
    free(compiler);
}

