#include <assert.h>
#include <stdio.h>
#include "gpucc.h"
#include "gpucc_internal.h"
#include "win32/gpucc_compiler_ptx_win32.h"

static char const *PtxArg_GpuArchitecture  = "-arch";
static char const *PtxArg_EnableDebugInfo  = "-G";
static char const *PtxArg_GenerateLineInfo = "-lineinfo";
static char const *PtxArg_SuppressWarnings = "-w";

static struct GPUCC_RESULT
gpuccMakeResult_nvrtc
(
    nvrtcResult nvrtc_result
)
{
    GPUCC_RESULT r;
    if (nvrtc_result != NVRTC_SUCCESS) {
        r.LibraryResult  = GPUCC_RESULT_CODE_PLATFORM_ERROR;
        r.PlatformResult =(int32_t) nvrtc_result;
    } else {
        r.LibraryResult  = GPUCC_RESULT_CODE_SUCCESS;
        r.PlatformResult =(int32_t) NVRTC_SUCCESS;
    } return r;
}

/* @summary Write a NVRTC compiler argument into the argument array.
 * The argument array is fixed-length, and the argument strings are string literals.
 * @param ptx The destination compiler instance.
 * @param arg Pointer to a string literal specifying the argument.
 */
static void
gpuccPtxStoreArg
(
    struct GPUCC_COMPILER_PTX_WIN32 *ptx, 
    char const                      *arg
)
{
    if (ptx->ArgumentCount < GPUCC_COMPILER_PTX_WIN32_MAX_ARGS) {
        ptx->ClArguments[ptx->ArgumentCount] = arg;
        ptx->ArgumentCount++;
    } else { /* Need to increase GPUCC_COMPILER_PTX_WIN32_MAX_ARGS */
        assert(ptx->ArgumentCount < GPUCC_COMPILER_PTX_WIN32_MAX_ARGS);
        return;
    }
}

GPUCC_API(struct GPUCC_PROGRAM_BYTECODE*)
gpuccCreateProgramBytecodePtx
(
    struct GPUCC_PROGRAM_COMPILER *compiler
)
{
    GPUCC_BYTECODE_PTX_WIN32 *code = nullptr;

    if ((code = (GPUCC_BYTECODE_PTX_WIN32*) malloc(sizeof(GPUCC_BYTECODE_PTX_WIN32))) == nullptr) {
        GPUCC_RESULT r = gpuccMakeResult(GPUCC_RESULT_CODE_OUT_OF_HOST_MEMORY);
        gpuccDebugPrintf(L"GpuCC: Failed to allocate %Iu bytes to create PTX bytecode.\n", sizeof(GPUCC_BYTECODE_PTX_WIN32));
        gpuccSetLastResult(r);
        return nullptr;
    } memset(code, 0, sizeof(GPUCC_BYTECODE_PTX_WIN32));

    /* TODO: Increment ref count on compiler object? */

    code->CommonFields.Compiler        = compiler;
    code->CommonFields.CompileResult   = gpuccMakeResult(GPUCC_RESULT_CODE_EMPTY_BYTECODE_CONTAINER);
    code->CommonFields.EntryPoint      = nullptr; /* Set on compile */
    code->CommonFields.SourcePath      = nullptr; /* Set on compile */
    code->CommonFields.LogBuffer       = nullptr; /* Set on compile */
    code->CommonFields.LogBufferSize   = 0;       /* Set on compile */
    code->CommonFields.BytecodeSize    = 0;       /* Set on compile */
    code->CommonFields.BytecodeBuffer  = nullptr; /* Set on compile */
    code->CodeBuffer                   = nullptr; /* Set on compile */
    code->LogBuffer                    = nullptr; /* Set on compile */
    return (struct GPUCC_PROGRAM_BYTECODE*) code;
}

GPUCC_API(void)
gpuccDeleteProgramBytecodePtx
(
    struct GPUCC_PROGRAM_BYTECODE *bytecode
)
{
    GPUCC_BYTECODE_PTX_WIN32 *code = gpuccBytecodePtx_(bytecode);

    if (code->LogBuffer != nullptr) {
        char       *buf  = code->LogBuffer;
        code->CommonFields.LogBufferSize = 0;
        code->CommonFields.LogBuffer     = nullptr;
        code->LogBuffer                  = nullptr;
        free(buf);
    }
    if (code->CodeBuffer != nullptr) {
        uint8_t     *buf  = code->CodeBuffer;
        code->CommonFields.BytecodeBuffer = nullptr;
        code->CommonFields.BytecodeSize   = 0;
        code->CodeBuffer                  = nullptr;
        free(buf);
    }
    if (code->CommonFields.EntryPoint != nullptr) {
        free(code->CommonFields.EntryPoint);
        code->CommonFields.EntryPoint  = nullptr;
        code->CommonFields.SourcePath  = nullptr;
    }
    /* TODO: Decrement ref count on compiler object? */
}

GPUCC_API(struct GPUCC_RESULT)
gpuccCompileBytecodePtx
(
    struct GPUCC_PROGRAM_BYTECODE *container, 
    char const                  *source_code, 
    uint64_t                     source_size, 
    char const                  *source_path, 
    char const                  *entry_point
)
{
    GPUCC_COMPILER_PTX_WIN32  *compiler_ = gpuccCompilerPtx_(gpuccQueryBytecodeCompiler_(container));
    GPUCC_BYTECODE_PTX_WIN32 *container_ = gpuccBytecodePtx_(container);
    PTXCOMPILERAPI_DISPATCH    *dispatch = compiler_->DispatchTable;
    GPUCC_RESULT                  result = gpuccMakeResult(GPUCC_RESULT_CODE_SUCCESS);
    uint8_t                        *code = nullptr;
    char                            *log = nullptr;
    size_t                     code_size = 0;
    size_t                      log_size = 0;
    nvrtcProgram                 program = nullptr;
    nvrtcResult                      res = NVRTC_SUCCESS;

    UNREFERENCED_PARAMETER(entry_point);
    UNREFERENCED_PARAMETER(source_size);

    if ((res = dispatch->nvrtcCreateProgram(&program, source_code, source_path, 0, NULL, NULL)) != NVRTC_SUCCESS) {
        GPUCC_RESULT r = gpuccMakeResult_nvrtc(res);
        gpuccDebugPrintf(L"GpuCC: nvrtcCreateProgram failed with %s.\n", dispatch->nvrtcGetErrorString(res));
        gpuccSetLastResult(r);
        goto cleanup_and_fail;
    }
    if ((res = dispatch->nvrtcCompileProgram(program, compiler_->ArgumentCount, compiler_->ClArguments)) != NVRTC_SUCCESS) {
        GPUCC_RESULT r = gpuccMakeResult_nvrtc(res);
        gpuccDebugPrintf(L"GpuCC: nvrtcCompileProgram failed with %s.\n", dispatch->nvrtcGetErrorString(res));
        gpuccSetLastResult(r);
        goto cleanup_and_fail;
    }
    if ((res = dispatch->nvrtcGetPTXSize(program, &code_size)) != NVRTC_SUCCESS) {
        GPUCC_RESULT r = gpuccMakeResult_nvrtc(res);
        gpuccDebugPrintf(L"GpuCC: nvrtcGetPTXSize failed with %s.\n", dispatch->nvrtcGetErrorString(res));
        gpuccSetLastResult(r);
        goto cleanup_and_fail;
    }
    if ((res = dispatch->nvrtcGetProgramLogSize(program, &log_size)) != NVRTC_SUCCESS) {
        GPUCC_RESULT r = gpuccMakeResult_nvrtc(res);
        gpuccDebugPrintf(L"GpuCC: nvrtcGetProgramLogSize failed with %s.\n", dispatch->nvrtcGetErrorString(res));
        gpuccSetLastResult(r);
        goto cleanup_and_fail;
    }

    /* Allocate buffers to hold the code and/or program log.
     * Once the PTX code and program log are retrieved, the nvrtcProgram 
     * could theoretically be re-used to re-compile the source code with 
     * different options, but there's no need for that in this case.
     */
    if (log_size  != 0 && ((log  = (char   *) malloc( log_size))) == nullptr) {
        GPUCC_RESULT r = gpuccMakeResult_errno(GPUCC_RESULT_CODE_OUT_OF_HOST_MEMORY);
        gpuccDebugPrintf(L"GpuCC: Failed to allocate %Iu bytes for program log buffer.\n", log_size);
        gpuccSetLastResult(r);
        goto cleanup_and_fail;
    }
    if (code_size != 0 && ((code = (uint8_t*) malloc(code_size))) == nullptr) {
        GPUCC_RESULT r = gpuccMakeResult_errno(GPUCC_RESULT_CODE_OUT_OF_HOST_MEMORY);
        gpuccDebugPrintf(L"GpuCC: Failed to allocate %Iu bytes for PTX bytecode buffer.\n", code_size);
        gpuccSetLastResult(r);
        goto cleanup_and_fail;
    }
    if (code_size != 0 && ((res  = dispatch->nvrtcGetPTX(program, (char*)code))) != NVRTC_SUCCESS) {
        GPUCC_RESULT r = gpuccMakeResult_nvrtc(res);
        gpuccDebugPrintf(L"GpuCC: nvrtcGetPTX failed with %s.\n", dispatch->nvrtcGetErrorString(res));
        gpuccSetLastResult(r);
        goto cleanup_and_fail;
    }
    if (log_size  != 0 && ((res  = dispatch->nvrtcGetProgramLog(program,  log))) != NVRTC_SUCCESS) {
        GPUCC_RESULT r = gpuccMakeResult_nvrtc(res);
        gpuccDebugPrintf(L"GpuCC: nvrtcGetProgramLog failed with %s.\n", dispatch->nvrtcGetErrorString(res));
        gpuccSetLastResult(r);
        goto cleanup_and_fail;
    }

    /* The NVRTC program object can be destroyed since it won't be used again. */
    dispatch->nvrtcDestroyProgram(&program);

    if (code_size != 0 && code != nullptr) {
        container_->CommonFields.BytecodeSize   =(uint64_t) code_size;
        container_->CommonFields.BytecodeBuffer =(uint8_t*) code;
    } else {
        container_->CommonFields.BytecodeSize   = 0;
        container_->CommonFields.BytecodeBuffer = nullptr;
    } container_->CodeBuffer = code;

    if (log_size != 0 && log != nullptr) {
        container_->CommonFields.LogBufferSize  =(uint64_t) log_size;
        container_->CommonFields.LogBuffer      =(char   *) log;
    } else {
        container_->CommonFields.LogBufferSize  = 0;
        container_->CommonFields.LogBuffer      = nullptr;
    } container_->LogBuffer = log;

    return result;

cleanup_and_fail:
    free(log);
    free(code);
    if (program && dispatch) {
        dispatch->nvrtcDestroyProgram(&program);
    }
    return gpuccMakeResult(GPUCC_RESULT_CODE_COMPILE_FAILED);
}

static void
gpuccCleanupCompilerPtx
(
    struct GPUCC_PROGRAM_COMPILER *compiler
)
{
    UNREFERENCED_PARAMETER(compiler);
}

GPUCC_API(struct GPUCC_PROGRAM_COMPILER*)
gpuccCreateCompilerPtx
(
    struct GPUCC_PROGRAM_COMPILER_INIT *config
)
{   // Assume that config has been validated by gpuccCreateCompiler.
    GPUCC_PROCESS_CONTEXT_WIN32 *pctx = gpuccGetProcessContext_();
    GPUCC_COMPILER_PTX_WIN32     *ptx = nullptr;
    uint8_t                     *base = nullptr;
    uint8_t                      *ptr = nullptr;
    char                    **defines = nullptr;
    char                     **clargs = nullptr;
    size_t                     nbneed = 0;

    /* Validate the target profile. */
    if (config->TargetProfile == nullptr) {
        GPUCC_RESULT r = gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_TARGET_PROFILE);
        gpuccDebugPrintf(L"GpuCC: A target profile, for example, \"compute_70\", is required by the PTX compiler.\n");
        gpuccSetLastResult(r);
        return nullptr;
    }

    /* Determine the amount of memory required. */
    nbneed += sizeof(GPUCC_COMPILER_PTX_WIN32);
    nbneed += sizeof(char*) * config->DefineCount;
    nbneed += sizeof(char*) * GPUCC_COMPILER_PTX_WIN32_MAX_ARGS;
    nbneed += strlen(config->TargetProfile) + 1;
    for (uint32_t i = 0, n = config->DefineCount; i < n; ++i) {
        nbneed += strlen("-D ");
        nbneed += strlen(config->DefineSymbols[i]);
        if (config->DefineValues[i] != nullptr) {
            size_t vlen = strlen(config->DefineValues[i]);
            if (vlen != 0) {
                nbneed += 1; /* '=' */
                nbneed += vlen;
            }
        } nbneed += 1; /* nul */
    }

    /* Allocate as a single block. */
    if ((base = (uint8_t*) malloc(nbneed)) == nullptr) {
        GPUCC_RESULT r = gpuccMakeResult(GPUCC_RESULT_CODE_OUT_OF_HOST_MEMORY);
        gpuccDebugPrintf(L"GpuCC: Failed to allocate %Iu bytes to create PTX compiler.\n", nbneed);
        gpuccSetLastResult(r);
        return nullptr;
    }
    ptx =(GPUCC_COMPILER_PTX_WIN32*) base;
    ptr = base + sizeof(GPUCC_COMPILER_PTX_WIN32);

    /* The array of preprocessor define arguments immediately follows the 
     * compiler record. The entries are initialized when interning the 
     * strings below.
     */
    defines =(char**) ptr;
    ptr    += sizeof(char*) * config->DefineCount;

    /* The array of compilation arguments immediately follows the define array.
     * The entries are initialized below.
     */
    clargs  =(char**) ptr;
    ptr    += sizeof(char*) * GPUCC_COMPILER_PTX_WIN32_MAX_ARGS;

    /* Copy string data into the memory block. */
    ptx->GpuArchitecture   = gpuccPutStringUtf8(ptr, config->TargetProfile);
    for (uint32_t i = 0, n = config->DefineCount; i < n; ++i) {
        size_t vlen = 0;

        defines[i]  =(char*) ptr;
        if (config->DefineValues[i] != nullptr) {
            vlen = strlen(config->DefineValues[i]);
        }
        if (vlen > 0) {
            ptr += _snprintf_s((char*) ptr, nbneed, _TRUNCATE, "-D %s=%s", config->DefineSymbols[i], config->DefineValues[i]) + 1;
        } else {
            ptr += _snprintf_s((char*) ptr, nbneed, _TRUNCATE, "-D %s"   , config->DefineSymbols[i]) + 1;
        }
    }
    
    /* Initialize the compiler arguments array. */
    ptx->ClArguments   =(char const**) clargs;
    ptx->ArgumentCount = 0;

    /* Specify compilation arguments. */
    gpuccPtxStoreArg(ptx, PtxArg_GpuArchitecture);
    gpuccPtxStoreArg(ptx, ptx->GpuArchitecture);
    if (config->CompilerFlags & GPUCC_COMPILER_FLAG_DEBUG) {
        gpuccPtxStoreArg(ptx, PtxArg_EnableDebugInfo);
        gpuccPtxStoreArg(ptx, PtxArg_GenerateLineInfo);
    }
    if (config->CompilerFlags & GPUCC_COMPILER_FLAG_DISABLE_OPTIMIZATIONS) {
    }
    if (config->CompilerFlags & GPUCC_COMPILER_FLAG_WARNINGS_AS_ERRORS) {
    }
    if (config->CompilerFlags & GPUCC_COMPILER_FLAG_ROW_MAJOR_MATRICES) {
    }
    if (config->CompilerFlags & GPUCC_COMPILER_FLAG_ENABLE_16BIT_TYPES) {
        gpuccDebugPrintf(L"GpuCC: Shader model targets pre-6.2 do not support native 16-bit types. Native support will be disabled.\n");
    }
    if (config->CompilerFlags & GPUCC_COMPILER_FLAG_AVOID_FLOW_CONTROL) {
    }
    if (config->CompilerFlags & GPUCC_COMPILER_FLAG_ENABLE_IEEE_STRICT) {
    }
    for (uint32_t i = 0, n = config->DefineCount; i < n; ++i) {
        gpuccPtxStoreArg(ptx, defines[i]);
    }

    int mj, mi;
    pctx->PtxCompiler_Dispatch.nvrtcVersion(&mj, &mi);
    gpuccDebugPrintf(L"GpuCC: NVRTC is version %d.%d.\n", mj, mi);

    /* Finish initializing the compiler structure. */
    ptx->CommonFields.CompilerType        = GPUCC_COMPILER_TYPE_NVRTC;
    ptx->CommonFields.BytecodeType        = GPUCC_BYTECODE_TYPE_PTX;
    ptx->CommonFields.CreateBytecode      = gpuccCreateProgramBytecodePtx;
    ptx->CommonFields.DeleteBytecode      = gpuccDeleteProgramBytecodePtx;
    ptx->CommonFields.CompileBytecode     = gpuccCompileBytecodePtx;
    ptx->CommonFields.CleanupCompiler     = gpuccCleanupCompilerPtx;
    ptx->TargetRuntime                    = config->TargetRuntime;
    ptx->DispatchTable                    =&pctx->PtxCompiler_Dispatch;
    ptx->DefineCount                      = config->DefineCount;
    ptx->DefineArray                      = defines;
    return (struct GPUCC_PROGRAM_COMPILER*) ptx;
}

