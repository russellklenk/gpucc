#include <assert.h>
#include "gpucc.h"
#include "gpucc_internal.h"
#include "win32/gpucc_compiler_fxc_win32.h"
#include "win32/gpucc_compiler_dxc_win32.h"

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
gpuccStartup
(
    uint32_t gpucc_usage_mode
)
{
    GPUCC_PROCESS_CONTEXT_WIN32 *pctx = gpuccGetProcessContext_();
    GPUCC_RESULT               result = gpuccMakeResult(GPUCC_RESULT_CODE_SUCCESS);
    uint32_t        fxccompiler_flags = FXCCOMPILERAPI_LOADER_FLAGS_NONE;
    uint32_t        dxccompiler_flags = DXCCOMPILERAPI_LOADER_FLAGS_NONE;
    uint32_t        ptxcompiler_flags = PTXCOMPILERAPI_LOADER_FLAGS_NONE;

    if (gpucc_usage_mode != GPUCC_USAGE_MODE_OFFLINE && 
        gpucc_usage_mode != GPUCC_USAGE_MODE_RUNTIME) {
        return gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_USAGE_MODE);
    }
    if (pctx->StartupFlag != FALSE) {
        return gpuccMakeResult(GPUCC_RESULT_CODE_ALREADY_INITIALIZED);
    }

    /* When being used in offline mode, enable development-only features. */
    if (gpucc_usage_mode  == GPUCC_USAGE_MODE_OFFLINE) {
        fxccompiler_flags |= FXCCOMPILERAPI_LOADER_FLAG_DEVELOPMENT;
    }

    /* Populate dispatch tables for any available compilers. */
    pctx->CompilerSupport = GPUCC_COMPILER_SUPPORT_NONE;
    if (FxcCompilerApiPopulateDispatch(&pctx->FxcCompiler_Dispatch, fxccompiler_flags) != 0) {
        pctx->CompilerSupport |= GPUCC_COMPILER_SUPPORT_FXC;
    }
    if (DxcCompilerApiPopulateDispatch(&pctx->DxcCompiler_Dispatch, dxccompiler_flags) != 0) {
        pctx->CompilerSupport |= GPUCC_COMPILER_SUPPORT_DXC;
    }
    if (PtxCompilerApiPopulateDispatch(&pctx->PtxCompiler_Dispatch, ptxcompiler_flags) != 0) {
        pctx->CompilerSupport |= GPUCC_COMPILER_SUPPORT_NVRTC;
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
    PtxCompilerApiInvalidateDispatch(&pctx->PtxCompiler_Dispatch);
    DxcCompilerApiInvalidateDispatch(&pctx->DxcCompiler_Dispatch);
    FxcCompilerApiInvalidateDispatch(&pctx->FxcCompiler_Dispatch);
    /* ... */

    pctx->CompilerSupport = GPUCC_COMPILER_SUPPORT_NONE;
    pctx->StartupFlag     = FALSE;
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
        gpuccDebugPrintf(L"GpuCC: Cannot create compiler. Call gpuccStartup() first.\n");
        gpuccSetLastResult(r);
        return nullptr;
    }
    if (config == nullptr) {
        assert(config != nullptr);
        GPUCC_RESULT r = gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_ARGUMENT);
        gpuccDebugPrintf(L"GpuCC: No compliler configuration was specified.\n");
        gpuccSetLastResult(r);
        return nullptr;
    }
    if (config->DefineCount > 0 && (config->DefineSymbols == nullptr || config->DefineValues == nullptr)) {
        assert(config->DefineSymbols != nullptr);
        assert(config->DefineValues  != nullptr);
        GPUCC_RESULT r = gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_ARGUMENT);
        gpuccDebugPrintf(L"GpuCC: DefineCount is non-zero, but symbols or values array is not specified.\n");
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
            compiler_type = GPUCC_COMPILER_TYPE_DXC;
            need_support  = GPUCC_COMPILER_SUPPORT_DXC;
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
        gpuccDebugPrintf(L"GpuCC: Unable to determine compiler type from bytecode type %s.\n", gpuccBytecodeTypeString(config->BytecodeType));
        gpuccSetLastResult(r);
        return nullptr;
    }
    if ((pctx->CompilerSupport & need_support) == 0) {
        GPUCC_RESULT r = gpuccMakeResult(GPUCC_RESULT_CODE_COMPILER_NOT_SUPPORTED);
        gpuccDebugPrintf(L"GpuCC: The required compiler type %s is not supported on this host platform.\n", gpuccCompilerTypeString(compiler_type));
        gpuccSetLastResult(r);
        return nullptr;
    }

    switch (compiler_type) {
        case GPUCC_COMPILER_TYPE_UNKNOWN:
            c = nullptr;
            break;
        case GPUCC_COMPILER_TYPE_DXC:
            c = gpuccCreateCompilerDxc(config);
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
{   /* TODO: need to invoke compiler cleanup callback first */
    free(compiler);
}

GPUCC_API(struct GPUCC_PROGRAM_BYTECODE*)
gpuccCreateBytecodeContainer
(
    struct GPUCC_PROGRAM_COMPILER *compiler
)
{
    struct GPUCC_PROGRAM_COMPILER_BASE *compiler_ =(GPUCC_PROGRAM_COMPILER_BASE*) compiler;

    if (compiler == nullptr) {
        GPUCC_RESULT r = gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_ARGUMENT);
        gpuccDebugPrintf(L"GpuCC: A valid GPUCC_PROGRAM_COMPILER must be specified to gpuccCreateBytecodeContainer.\n");
        gpuccSetLastResult(r);
        return nullptr;
    }

    return compiler_->CreateBytecode(compiler);
}

GPUCC_API(void)
gpuccDeleteBytecodeContainer
(
    struct GPUCC_PROGRAM_BYTECODE *bytecode
)
{
    if (bytecode) {
        struct GPUCC_PROGRAM_COMPILER_BASE *compiler_ =(GPUCC_PROGRAM_COMPILER_BASE*) gpuccQueryBytecodeCompiler_(bytecode);
        assert(compiler_ != nullptr);
        compiler_->DeleteBytecode(bytecode);
    }
}

GPUCC_API(struct GPUCC_RESULT)
gpuccCompileProgramBytecode
(
    struct GPUCC_PROGRAM_BYTECODE *container, 
    char const                  *source_code, 
    uint64_t                     source_size, 
    char const                  *source_path, 
    char const                  *entry_point
)
{
    GPUCC_PROGRAM_BYTECODE_BASE *container_ = nullptr;
    GPUCC_PROGRAM_COMPILER_BASE  *compiler_ = nullptr;
    GPUCC_RESULT                     result = gpuccMakeResult(GPUCC_RESULT_CODE_SUCCESS);

    if (container == nullptr) {
        result = gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_ARGUMENT);
        gpuccDebugPrintf(L"GpuCC: No bytecode container was supplied.\n");
        gpuccSetLastResult(result);
        return result;
    }
    if (source_code == nullptr || source_size == 0) {
        result = gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_ARGUMENT);
        gpuccDebugPrintf(L"GpuCC: No program source code was supplied.\n");
        gpuccSetLastResult(result);
        return result;
    }
    if (gpuccBytecodeContainerIsEmpty(container) == 0) {
        result = gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_BYTECODE_CONTAINER);
        gpuccDebugPrintf(L"GpuCC: The supplied bytecode container has already been used to compile a program and cannot be reused.\n");
        gpuccSetLastResult(result);
        return result;
    }

    /* Intern strings used for debug output. */
    container_ =(GPUCC_PROGRAM_BYTECODE_BASE*) container;
    compiler_  =(GPUCC_PROGRAM_COMPILER_BASE*) gpuccQueryBytecodeCompiler(container);
    if (gpuccFailure((result = gpuccSetProgramEntryPoint(container, entry_point, source_path)))) {
        /* gpuccSetProgramEntryPoint called gpuccSetLastResult  */
        gpuccDebugPrintf(L"GpuCC: Cannot copy program entry point and source path. Compilation cannot proceed.\n");
        return result;
    }

    /* Finally, perform the actual compilation. */
    result = compiler_->CompileBytecode(container, source_code, source_size, container_->SourcePath, container_->EntryPoint);
    gpuccSetLastResult(gpuccMakeResult(GPUCC_RESULT_CODE_SUCCESS));
    return result;
}

