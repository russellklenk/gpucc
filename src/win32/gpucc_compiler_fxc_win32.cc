#include "gpucc.h"
#include "gpucc_internal.h"
#include "win32/gpucc_compiler_fxc_win32.h"

GPUCC_API(struct GPUCC_PROGRAM_BYTECODE*)
gpuccCreateProgramBytecodeFxc
(
    struct GPUCC_PROGRAM_COMPILER *compiler
)
{
    GPUCC_BYTECODE_FXC_WIN32 *code = nullptr;

    if ((code = (GPUCC_BYTECODE_FXC_WIN32*) malloc(sizeof(GPUCC_BYTECODE_FXC_WIN32))) == nullptr) {
        GPUCC_RESULT r = gpuccMakeResult(GPUCC_RESULT_CODE_OUT_OF_HOST_MEMORY);
        gpuccDebugPrintf(L"GpuCC: Failed to allocate %Iu bytes to create FXC bytecode.\n", sizeof(GPUCC_BYTECODE_FXC_WIN32));
        gpuccSetLastResult(r);
        return nullptr;
    } memset(code, 0, sizeof(GPUCC_BYTECODE_FXC_WIN32));

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
    code->ErrorLog                     = nullptr; /* Set on compile */
    return (struct GPUCC_PROGRAM_BYTECODE*) code;
}

GPUCC_API(void)
gpuccDeleteProgramBytecodeFxc
(
    struct GPUCC_PROGRAM_BYTECODE *bytecode
)
{
    GPUCC_BYTECODE_FXC_WIN32 *code = gpuccBytecodeFxc_(bytecode);

    if (code->ErrorLog != nullptr) {
        ID3DBlob  *log  = code->ErrorLog;
        code->CommonFields.LogBufferSize  = 0;
        code->CommonFields.LogBuffer      = nullptr;
        code->ErrorLog                    = nullptr;
        log->Release();
    }
    if (code->CodeBuffer != nullptr) {
        ID3DBlob    *buf  = code->CodeBuffer;
        code->CommonFields.BytecodeBuffer = nullptr;
        code->CommonFields.BytecodeSize   = 0;
        code->CodeBuffer                  = nullptr;
        buf->Release();
    }
    if (code->CommonFields.EntryPoint != nullptr) {
        free(code->CommonFields.EntryPoint);
        code->CommonFields.EntryPoint  = nullptr;
        code->CommonFields.SourcePath  = nullptr;
    }
    /* TODO: Decrement ref count on compiler object? */
}

GPUCC_API(struct GPUCC_RESULT)
gpuccCompileBytecodeFxc
(
    struct GPUCC_PROGRAM_BYTECODE *container, 
    char const                  *source_code, 
    uint64_t                     source_size, 
    char const                  *source_path, 
    char const                  *entry_point
)
{
    GPUCC_COMPILER_FXC_WIN32  *compiler_ = gpuccCompilerFxc_(gpuccQueryBytecodeCompiler_(container));
    GPUCC_BYTECODE_FXC_WIN32 *container_ = gpuccBytecodeFxc_(container);
    FXCCOMPILERAPI_DISPATCH    *dispatch = compiler_->DispatchTable;
    GPUCC_RESULT                  result = gpuccMakeResult(GPUCC_RESULT_CODE_SUCCESS);
    ID3DBlob                       *code = nullptr;
    ID3DBlob                        *log = nullptr;
    HRESULT                          res = S_OK;
    DWORD                         flags1 = D3DCOMPILE_DEBUG | D3DCOMPILE_WARNINGS_ARE_ERRORS;
    DWORD                         flags2 = 0;

    /* TODO: Need some way to specify flags1 */
    res = dispatch->D3DCompile
    (
        source_code, 
        source_size, 
        source_path, 
        compiler_->DefineArray, 
        nullptr, /* ID3DInclude* pInclude */
        entry_point, 
        compiler_->ShaderModel, 
        flags1, 
        flags2, 
        &code, 
        &log
    );
    if (FAILED(res)) {
        result = gpuccMakeResult(GPUCC_RESULT_CODE_COMPILE_FAILED);
    }

    if (code != nullptr) {
        container_->CommonFields.BytecodeSize   =(uint64_t) code->GetBufferSize();
        container_->CommonFields.BytecodeBuffer =(uint8_t*) code->GetBufferPointer();
    } else {
        container_->CommonFields.BytecodeSize   = 0;
        container_->CommonFields.BytecodeBuffer = nullptr;
    } container_->CodeBuffer = code;

    if (log != nullptr) {
        container_->CommonFields.LogBufferSize  =(uint64_t) log->GetBufferSize();
        container_->CommonFields.LogBuffer      =(char   *) log->GetBufferPointer();
    } else {
        container_->CommonFields.LogBufferSize  = 0;
        container_->CommonFields.LogBuffer      = nullptr;
    } container_->ErrorLog = log;

    return result;
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
    int                    version_mj = 0;
    int                    version_mi = 0;
    char               shader_type[3] ={0, 0, 0};

    /* Validate the target profile. */
    if (config->TargetProfile == nullptr) {
        GPUCC_RESULT r = gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_TARGET_PROFILE);
        gpuccDebugPrintf(L"GpuCC: A target profile, for example, \"cs_5_0\", is required by the FXC compiler.\n");
        gpuccSetLastResult(r);
        return nullptr;
    }
    if (gpuccExtractDirect3DShaderModel(config->TargetProfile, shader_type, &version_mj, &version_mi) != 0) {
        /* The TargetProfile string doesn't match the expected format. */
        GPUCC_RESULT r = gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_TARGET_PROFILE);
        gpuccDebugPrintf(L"GpuCC: Invalid target profile \"%S\" for the FXC compiler.\n", config->TargetProfile);
        gpuccSetLastResult(r);
        return nullptr;
    }
    if (_stricmp(shader_type, "cs") != 0 && 
        _stricmp(shader_type, "vs") != 0 && 
        _stricmp(shader_type, "ps") != 0 && 
        _stricmp(shader_type, "gs") != 0) {
        /* The target profile specifies an unexpected shader type. */
        GPUCC_RESULT r = gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_TARGET_PROFILE);
        gpuccDebugPrintf(L"GpuCC: Invalid target profile \"%S\". Unexpected shader type \'%s\'.\n", config->TargetProfile, shader_type);
        gpuccSetLastResult(r);
        return nullptr;
    }
    if (version_mj < 4) {
        /* Legacy D3D9 shader models are not supported. */
        GPUCC_RESULT r = gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_TARGET_PROFILE);
        gpuccDebugPrintf(L"GpuCC: Invalid target profile \"%S\". Legacy shader models are not supported.\n", config->TargetProfile);
        gpuccSetLastResult(r);
        return nullptr;
    }
    if (version_mj > 5) {
        /* Shader Model 6 and later requires using the newer dxc compiler. */
        GPUCC_RESULT r = gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_TARGET_PROFILE);
        gpuccDebugPrintf(L"GpuCC: Invalid target profile \"%S\". Shader model 6+ require the newer dxc compiler and DXIL bytecode format.\n", config->TargetProfile);
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
        GPUCC_RESULT r = gpuccMakeResult(GPUCC_RESULT_CODE_OUT_OF_HOST_MEMORY);
        gpuccDebugPrintf(L"GpuCC: Failed to allocate %Iu bytes to create FXC compiler.\n", nbneed);
        gpuccSetLastResult(r);
        return nullptr;
    }
    fxc =(GPUCC_COMPILER_FXC_WIN32*) base;
    ptr = base + sizeof(GPUCC_COMPILER_FXC_WIN32);

    /* The array of D3D_SHADER_MACRO structures immediately follows the 
     * compiler record. The entries are initialized when interning the 
     * strings below.
     */
    macros =(D3D_SHADER_MACRO *) ptr;
    ptr   += sizeof(D3D_SHADER_MACRO) * (config->DefineCount + 1);

    /* Copy string data into the memory block. */
    fxc->ShaderModel = gpuccPutStringUtf8(ptr, config->TargetProfile);
    for (uint32_t  i = 0 , n = config->DefineCount; i < n; ++i) {
        macros[i].Name       = gpuccPutStringUtf8(ptr, config->DefineSymbols[i]);
        macros[i].Definition = gpuccPutStringUtf8(ptr, config->DefineValues [i]);
    }
    /* The macro array is always terminated with a null entry. */
    macros[config->DefineCount] = D3D_SHADER_MACRO { nullptr, nullptr };

    /* Finish initializing the compiler structure. */
    fxc->CommonFields.CompilerType        = GPUCC_COMPILER_TYPE_FXC;
    fxc->CommonFields.BytecodeType        = GPUCC_BYTECODE_TYPE_DXBC;
    fxc->CommonFields.CreateBytecode      = gpuccCreateProgramBytecodeFxc;
    fxc->CommonFields.DeleteBytecode      = gpuccDeleteProgramBytecodeFxc;
    fxc->CommonFields.CompileBytecode     = gpuccCompileBytecodeFxc;
    fxc->DispatchTable                    =&pctx->FxcCompiler_Dispatch;
    fxc->DefineArray                      = macros;
    fxc->DefineCount                      = config->DefineCount;
    fxc->TargetRuntime                    = config->TargetRuntime;
    return (struct GPUCC_PROGRAM_COMPILER*) fxc;
}

