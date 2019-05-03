#include <assert.h>
#include "gpucc.h"
#include "gpucc_internal.h"
#include "win32/gpucc_compiler_dxc_win32.h"

static WCHAR const *DxcArg_NoUnusedArguments    = L"-Qunused-arguments";              /* Don't emit warning for unused driver arguments */
static WCHAR const *DxcArg_AllResourcesBound    = L"-all_resources_bound";            /* Enables aggressive flattening */
static WCHAR const *DxcArg_Enable16BitTypes     = L"-enable-16bit-types";             /* Enable 16 bit types and disable min prevision types. Available in HLSL 2018 and SM 6.2.*/
static WCHAR const *DxcArg_LegacyMacroExpansion = L"-flegacy-macro-expansion";        /* Expand the operands before performing token-pasting operation (fxc behavior) */
static WCHAR const *DxcArg_ForceRootsigVer1_0   = L"-force_rootsig_ver rootsig_1_0";  /* Force root signature version 1.0 */
static WCHAR const *DxcArg_ForceRootsigVer1_1   = L"-force_rootsig_ver rootsig_1_1";  /* Force root signature version 1.1 */
static WCHAR const *DxcArg_EnableStrictMode     = L"-Ges";                            /* Enable strict mode */
static WCHAR const *DxcArg_AvoidFlowControl     = L"-Gfa";                            /* Avoid flow control constructs */
static WCHAR const *DxcArg_PreferFlowControl    = L"-Gfp";                            /* Prefer flow control constructs */
static WCHAR const *DxcArg_ForceIEEEStrictness  = L"-Gis";                            /* Force IEEE compliance */
static WCHAR const *DxcArg_NoWarnings           = L"-no-warnings";                    /* Suppress warning messages */
static WCHAR const *DxcArg_NoLegacyCBufLoad     = L"-not_use_legacy_cbuf_load";       /* Do not use legacy cbuffer load */
static WCHAR const *DxcArg_DisableOptimizations = L"-Od";                             /* Disable optimizations */
static WCHAR const *DxcArg_PackOptimized        = L"-pack_optimized";                 /* Optimize signature packing assuming identical signature provided for each connecting stage */
static WCHAR const *DxcArg_PackPrefixStable     = L"-pack_prefix_stable";             /* Pack signatures preserving prefix-stable property - appended elements will not disturb placement of prior elements */
static WCHAR const *DxcArg_WarningsAsErrors     = L"-WX";                             /* Treat warnings as errors */
static WCHAR const *DxcArg_EnableDebugInfo      = L"-Zi";                             /* Generate debug information */
static WCHAR const *DxcArg_PackColumnMajor      = L"-Zpc";                            /* Pack matrices in column-major order */
static WCHAR const *DxcArg_PackRowMajor         = L"-Zpr";                            /* Pack matrices in row-major order */
static WCHAR const *DxcArg_OptimizerLevel0      = L"-O0";                             /* Optimization level 0 */
static WCHAR const *DxcArg_OptimizerLevel1      = L"-O1";                             /* Optimization level 1 */
static WCHAR const *DxcArg_OptimizerLevel2      = L"-O2";                             /* Optimization level 2 */
static WCHAR const *DxcArg_OptimizerLevel3      = L"-O3";                             /* Optimization level 3 (default) */
static WCHAR const *DxcArg_OptimizerLevel4      = L"-O4";                             /* Optimization level 4 */
static WCHAR const *DxcArg_SPIRVReflect         = L"-fspv-reflect";                   /* Emit additional SPIR-V instructions to aid reflection */
static WCHAR const *DxcArg_TargetVulkan1_0      = L"-fspv-target-env=vulkan1.0";      /* Target Vulkan 1.0 */
static WCHAR const *DxcArg_TargetVulkan1_1      = L"-fspv-target-env=vulkan1.1";      /* Target Vulkan 1.1 */
static WCHAR const *DxcArg_VulkanInvertY        = L"-fvk-invert-y";                   /* Invert SV_Position.y before writing to stage output in VS/DS/GS to match Vulkan coordinate system */
static WCHAR const *DxcArg_VulkanUseDxLayout    = L"-fvk-use-dx-layout";              /* Use DirectX memory layout for Vulkan resources */
static WCHAR const *DxcArg_VulkanUseDxPositionW = L"-fvk-use-dx-position-w";          /* Use 1 / SV_Position.w after reading from stage input in PS to match Vulkan semantics */
static WCHAR const *DxcArg_VulkanUseGlLayout    = L"-fvk-use-gl-layout";              /* Use OpenGL std140/std430 memory layout for Vulkan resources */
static WCHAR const *DxcArg_GenerateSPIRV        = L"-spirv";                          /* Generate SPIR-V bytecode instead of DXIL */

/* @summary Write a DXC compiler argument into the argument array.
 * The argument array is fixed-length, and the argument strings are string literals.
 * @param dxc The destination compiler instance.
 * @param arg Pointer to a string literal specifying the argument.
 */
static void
gpuccDxcStoreArg
(
    struct GPUCC_COMPILER_DXC_WIN32 *dxc, 
    WCHAR const                     *arg
)
{
    if (dxc->ArgumentCount < GPUCC_COMPILER_DXC_WIN32_MAX_ARGS) {
        dxc->ClArguments[dxc->ArgumentCount] = arg;
        dxc->ArgumentCount++;
    } else { /* Need to increase GPUCC_COMPILER_DXC_WIN32_MAX_ARGS */
        assert(dxc->ArgumentCount < GPUCC_COMPILER_DXC_WIN32_MAX_ARGS);
        return;
    }
}

GPUCC_API(struct GPUCC_PROGRAM_BYTECODE*)
gpuccCreateProgramBytecodeDxc
(
    struct GPUCC_PROGRAM_COMPILER *compiler
)
{
    GPUCC_BYTECODE_DXC_WIN32 *code = nullptr;

    if ((code = (GPUCC_BYTECODE_DXC_WIN32*) malloc(sizeof(GPUCC_BYTECODE_DXC_WIN32))) == nullptr) {
        GPUCC_RESULT r = gpuccMakeResult(GPUCC_RESULT_CODE_OUT_OF_HOST_MEMORY);
        gpuccDebugPrintf(L"GpuCC: Failed to allocate %Iu bytes to create DXC bytecode.\n", sizeof(GPUCC_BYTECODE_DXC_WIN32));
        gpuccSetLastResult(r);
        return nullptr;
    } memset(code, 0, sizeof(GPUCC_BYTECODE_DXC_WIN32));

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
gpuccDeleteProgramBytecodeDxc
(
    struct GPUCC_PROGRAM_BYTECODE *bytecode
)
{
    GPUCC_BYTECODE_DXC_WIN32 *container_ = gpuccBytecodeDxc_(bytecode);

    if (container_->CodeBuffer != nullptr) {
        IDxcBlob *buf = container_->CodeBuffer;
        container_->CommonFields.BytecodeSize   = 0;
        container_->CommonFields.BytecodeBuffer = nullptr;
        container_->CodeBuffer                  = nullptr;
        buf->Release();
    }
    if (container_->ErrorLog != nullptr) {
        IDxcBlobEncoding *buf = container_->ErrorLog;
        container_->CommonFields.LogBufferSize  = 0;
        container_->CommonFields.LogBuffer      = nullptr;
        container_->ErrorLog                    = nullptr;
        buf->Release();
    }
    if (container_->CommonFields.EntryPoint != nullptr) {
        free(container_->CommonFields.EntryPoint);
        container_->CommonFields.EntryPoint  = nullptr;
        container_->CommonFields.SourcePath  = nullptr;
    }
    /* TODO: Decrement ref count on compiler object? */
}

GPUCC_API(struct GPUCC_RESULT)
gpuccCompileBytecodeDxc
(
    struct GPUCC_PROGRAM_BYTECODE *container, 
    char const                  *source_code, 
    uint64_t                     source_size, 
    char const                  *source_path, 
    char const                  *entry_point
)
{
    GPUCC_COMPILER_DXC_WIN32  *compiler_ = gpuccCompilerDxc_(gpuccQueryBytecodeCompiler_(container));
    GPUCC_BYTECODE_DXC_WIN32 *container_ = gpuccBytecodeDxc_(container);
    IDxcOperationResult       *op_result = nullptr;
    IDxcBlobEncoding           *src_blob = nullptr;
    IDxcBlobEncoding           *log_blob = nullptr;
    IDxcBlob                  *code_blob = nullptr;
    IDxcLibrary                     *lib = compiler_->DxcLibrary;
    IDxcCompiler                    *dxc = compiler_->DxcCompiler;
    WCHAR                  *wsource_path = nullptr;
    WCHAR                  *wentry_point = nullptr;
    GPUCC_RESULT                  result = gpuccMakeResult(GPUCC_RESULT_CODE_SUCCESS);
    HRESULT                          res = S_OK;
    HRESULT                  compile_res = S_OK;

    UNREFERENCED_PARAMETER(source_path);
    UNREFERENCED_PARAMETER(entry_point);

    /* dxc expects UTF-16 strings; perform conversions. */
    if ((wsource_path = gpuccConvertUtf8ToUtf16(container_->CommonFields.SourcePath)) == nullptr) {
        /* gpuccUtf8ToUtf16 called gpuccSetLastResult for us. */
        goto cleanup_and_fail;
    }
    if ((wentry_point = gpuccConvertUtf8ToUtf16(container_->CommonFields.EntryPoint)) == nullptr) {
        /* gpuccUtf8ToUtf16 called gpuccSetLastResult for us. */
        goto cleanup_and_fail;
    }

    /* Create a blob around the caller-supplied code buffer. */
    res = lib->CreateBlobWithEncodingFromPinned(source_code, (UINT32) source_size, CP_UTF8, &src_blob);
    if (FAILED(res)) {
        GPUCC_RESULT r = gpuccMakeResult_HRESULT(res);
        gpuccDebugPrintf(L"GpuCC: Failed to create blob wrapper for source code with HRESULT %08X.\n", res);
        gpuccSetLastResult(r);
        goto cleanup_and_fail;
    }

    /* Pass the code buffer to the compiler. */
    res = dxc->Compile
    (
        src_blob, 
        wsource_path, 
        wentry_point, 
        compiler_->ShaderModel, 
        compiler_->ClArguments, 
        compiler_->ArgumentCount, 
        compiler_->DefineArray, 
        compiler_->DefineCount, 
        nullptr, /* include handler */
        &op_result
    );

    if (op_result != nullptr) {
        /* Retrieve the compilation log. */
        IDxcBlobEncoding *log_blob_base = nullptr;
        if (SUCCEEDED((res = op_result->GetErrorBuffer(&log_blob_base)))) {
            /* Convert the log output to UTF-8 from whatever source encoding it's in. */
            if (FAILED((res = lib->GetBlobAsUtf8(log_blob_base, &log_blob)))) {
                GPUCC_RESULT r = gpuccMakeResult_HRESULT(res);
                gpuccDebugPrintf(L"GpuCC: Failed to get UTF-8 compilation log with HRESULT %08X.\n", res);
                gpuccSetLastResult(r);
            } log_blob_base->Release();
        } else {
            GPUCC_RESULT r = gpuccMakeResult_HRESULT(res);
            gpuccDebugPrintf(L"GpuCC: Failed to get native compilation log with HRESULT %08X.\n", res);
            gpuccSetLastResult(r);
        }

        /* Check to see whether compilation was successful. */
        if (SUCCEEDED((res = op_result->GetStatus(&compile_res)))) {
            if (SUCCEEDED(compile_res)) {
                result = gpuccMakeResult(GPUCC_RESULT_CODE_SUCCESS);
                /* Retrieve the blob containing the bytecode. */
                if (FAILED((res = op_result->GetResult(&code_blob)))) {
                    GPUCC_RESULT r = gpuccMakeResult_HRESULT(res);
                    gpuccDebugPrintf(L"GpuCC: Failed to retrieve code buffer with HRESULT %08X.\n", res);
                    gpuccSetLastResult(r);
                }
            } else { /* Compilation failed. */
                result = gpuccMakeResult(GPUCC_RESULT_CODE_COMPILE_FAILED);
            }
        } else {
            GPUCC_RESULT r = gpuccMakeResult_HRESULT(res);
            gpuccDebugPrintf(L"GpuCC: Failed to retrieve compilation status with HRESULT %08X.\n", res);
            gpuccSetLastResult(r);
        }

        if (code_blob != nullptr) {
            container_->CommonFields.BytecodeSize   =(uint64_t) code_blob->GetBufferSize();
            container_->CommonFields.BytecodeBuffer =(uint8_t*) code_blob->GetBufferPointer();
        } else { /* Failed to get the code blob. */
            container_->CommonFields.BytecodeSize   = 0;
            container_->CommonFields.BytecodeBuffer = nullptr;
        } container_->CodeBuffer = code_blob;

        if (log_blob != nullptr) {
            container_->CommonFields.LogBufferSize =(uint64_t) log_blob->GetBufferSize();
            container_->CommonFields.LogBuffer     =(char   *) log_blob->GetBufferPointer();
        } else { /* Failed to get the log blob. */
            container_->CommonFields.LogBufferSize = 0;
            container_->CommonFields.LogBuffer     = nullptr;
        } container_->ErrorLog = log_blob;
    } else { /* The attempt to compile failed (ie. compilation was not performed) */
        GPUCC_RESULT r = gpuccMakeResult_HRESULT(res);
        gpuccDebugPrintf(L"GpuCC: A compilation attempt aborted with HRESULT %08X.\n", res);
        gpuccSetLastResult(r);
    }

    /* Clean up temporary memory. */
    gpuccFreeStringBuffer(wentry_point);
    gpuccFreeStringBuffer(wsource_path);
    src_blob->Release();
    return result;

cleanup_and_fail:
    gpuccFreeStringBuffer(wentry_point);
    gpuccFreeStringBuffer(wsource_path);
    if (src_blob) {
        src_blob->Release();
    } return gpuccMakeResult(GPUCC_RESULT_CODE_COMPILE_FAILED);
}

static void
gpuccCleanupCompilerDxc
(
    struct GPUCC_PROGRAM_COMPILER *compiler
)
{
    GPUCC_COMPILER_DXC_WIN32 *compiler_ = gpuccCompilerDxc_(compiler);

    if (compiler_->DxcCompiler) {
        IDxcCompiler *cc = compiler_->DxcCompiler;
        compiler_->DxcCompiler = nullptr;
        cc->Release();
    }
    if (compiler_->DxcLibrary) {
        IDxcLibrary *lib = compiler_->DxcLibrary;
        compiler_->DxcLibrary = nullptr;
        lib->Release();
    }
}

GPUCC_API(struct GPUCC_PROGRAM_COMPILER*)
gpuccCreateCompilerDxc
(
    struct GPUCC_PROGRAM_COMPILER_INIT *config
)
{   // Assume that config has been validated by gpuccCreateCompiler.
    GPUCC_PROCESS_CONTEXT_WIN32 *pctx = gpuccGetProcessContext_();
    GPUCC_COMPILER_DXC_WIN32     *dxc = nullptr;
    DXCCOMPILERAPI_DISPATCH *dispatch =&pctx->DxcCompiler_Dispatch;
    DxcDefine                 *macros = nullptr;
    IDxcLibrary                  *lib = nullptr;
    IDxcCompiler                *dxcc = nullptr;
    uint8_t                     *base = nullptr;
    uint8_t                      *ptr = nullptr;
    uint8_t                      *end = nullptr;
    HRESULT                       res = S_OK;
    size_t                     nbneed = 0;
    int                    version_mj = 0;
    int                    version_mi = 0;
    char               shader_type[3] ={0, 0, 0};
    GPUCC_STRING_INFO              si;

    /* Validate the target profile. */
    if (config->TargetProfile == nullptr) {
        GPUCC_RESULT r = gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_TARGET_PROFILE);
        gpuccDebugPrintf(L"GpuCC: A target profile, for example, \"cs_5_0\", is required by the DXC compiler.\n");
        gpuccSetLastResult(r);
        return nullptr;
    }
    if (gpuccExtractDirect3DShaderModel(config->TargetProfile, shader_type, &version_mj, &version_mi) != 0) {
        /* The TargetProfile string doesn't match the expected format. */
        GPUCC_RESULT r = gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_TARGET_PROFILE);
        gpuccDebugPrintf(L"GpuCC: Invalid target profile \"%S\" for the DXC compiler.\n", config->TargetProfile);
        gpuccSetLastResult(r);
        return nullptr;
    }
    if (_stricmp(shader_type, "cs") != 0 && /* Compute shader  */
        _stricmp(shader_type, "vs") != 0 && /* Vertex shader   */
        _stricmp(shader_type, "ps") != 0 && /* Pixel shader    */
        _stricmp(shader_type, "gs") != 0 && /* Geometry shader */
        _stricmp(shader_type, "ds") != 0 && /* Domain shader   */
        _stricmp(shader_type, "hs") != 0) { /* Hull shader     */
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

    /* Determine the amount of memory required. */
    nbneed += sizeof(GPUCC_COMPILER_DXC_WIN32);
    nbneed += sizeof(DxcDefine) * config->DefineCount;
    nbneed += sizeof(WCHAR   *) * GPUCC_COMPILER_DXC_WIN32_MAX_ARGS;

    if (gpuccStringInfoUtf8ToUtf16(&si, config->TargetProfile) != 0) {
        DWORD        p = GetLastError();
        GPUCC_RESULT r = gpuccMakeResult_Win32(GPUCC_RESULT_CODE_INVALID_TARGET_PROFILE, p);
        gpuccDebugPrintf(L"GpuCC: Could not convert target profile string to UTF-16 %08X.\n", p);
        gpuccSetLastResult(r);
        return nullptr;
    } nbneed += si.ByteCount;

    for (uint32_t i = 0, n = config->DefineCount; i < n; ++i) {
        if (gpuccStringInfoUtf8ToUtf16(&si, config->DefineSymbols[i]) != 0) {
            DWORD        p = GetLastError();
            GPUCC_RESULT r = gpuccMakeResult_Win32(GPUCC_RESULT_CODE_INVALID_ARGUMENT, p);
            gpuccDebugPrintf(L"GpuCC: Could not convert preprocessor symbol string %u \"%S\" to UTF-16 %08X.\n", i, config->DefineSymbols[i], p);
            gpuccSetLastResult(r);
            return nullptr;
        } nbneed += si.ByteCount;

        if (gpuccStringInfoUtf8ToUtf16(&si, config->DefineValues[i]) != 0) {
            DWORD        p = GetLastError();
            GPUCC_RESULT r = gpuccMakeResult_Win32(GPUCC_RESULT_CODE_INVALID_ARGUMENT, p);
            gpuccDebugPrintf(L"GpuCC: Could not convert preprocessor value string %u \"%S\" to UTF-16 %08X.\n", i, config->DefineValues[i], p);
            gpuccSetLastResult(r);
            return nullptr;
        } nbneed += si.ByteCount;
    }

    /* Allocate as a single block. */
    if ((base = (uint8_t*) malloc(nbneed)) == nullptr) {
        GPUCC_RESULT r = gpuccMakeResult(GPUCC_RESULT_CODE_OUT_OF_HOST_MEMORY);
        gpuccDebugPrintf(L"GpuCC: Failed to allocate %Iu bytes to create DXC compiler.\n", nbneed);
        gpuccSetLastResult(r);
        return nullptr;
    }
    dxc =(GPUCC_COMPILER_DXC_WIN32*) base;
    ptr = base + sizeof(GPUCC_COMPILER_DXC_WIN32);
    end = base + nbneed;

    /* The array of DxcDefine structures immediately follows the compiler 
     * record. The entries are initialized when interning the strings below.
     */
    macros =(DxcDefine *) ptr;
    ptr   += sizeof(DxcDefine) * config->DefineCount;

    /* Copy string data into the memory block. */
    dxc->ShaderModel = gpuccInternUtf8ToUtf16(ptr, end, config->TargetProfile);
    for (uint32_t  i = 0 , n = config->DefineCount; i < n; ++i) {
        macros[i].Name   = gpuccInternUtf8ToUtf16(ptr, end, config->DefineSymbols[i]);
        macros[i].Value  = gpuccInternUtf8ToUtf16(ptr, end, config->DefineValues [i]);
    }

    /* ptr now points to the start of the fixed-length arguments list. 
     * Build the arguments list (an array of pointers to string literals.
     */
    dxc->ClArguments   =(WCHAR const**) ptr;
    dxc->ArgumentCount = 0;
    /* Common arguments */
    if (config->CompilerFlags & GPUCC_COMPILER_FLAG_DEBUG) {
        gpuccDxcStoreArg(dxc, DxcArg_EnableDebugInfo);
    }
    if (config->CompilerFlags & GPUCC_COMPILER_FLAG_DISABLE_OPTIMIZATIONS) {
        gpuccDxcStoreArg(dxc, DxcArg_DisableOptimizations);
    } else {
        gpuccDxcStoreArg(dxc, DxcArg_OptimizerLevel4);
    }
    if (config->CompilerFlags & GPUCC_COMPILER_FLAG_WARNINGS_AS_ERRORS) {
        gpuccDxcStoreArg(dxc, DxcArg_WarningsAsErrors);
    }
    if (config->CompilerFlags & GPUCC_COMPILER_FLAG_ROW_MAJOR_MATRICES) {
        gpuccDxcStoreArg(dxc, DxcArg_PackRowMajor);
    } else {
        gpuccDxcStoreArg(dxc, DxcArg_PackColumnMajor);
    }
    if (config->CompilerFlags & GPUCC_COMPILER_FLAG_ENABLE_16BIT_TYPES) {
        if (version_mj <= 6 && version_mi < 2) {
            gpuccDebugPrintf(L"GpuCC: Native 16-bit types require shader model 6.2 or later. Support will not be enabled.\n");
        } else {
            gpuccDxcStoreArg(dxc, DxcArg_Enable16BitTypes);
        }
    }
    if (config->CompilerFlags & GPUCC_COMPILER_FLAG_AVOID_FLOW_CONTROL) {
        gpuccDxcStoreArg(dxc, DxcArg_AvoidFlowControl);
    }
    if (config->CompilerFlags & GPUCC_COMPILER_FLAG_ENABLE_IEEE_STRICT) {
        gpuccDxcStoreArg(dxc, DxcArg_ForceIEEEStrictness);
    }
    if (config->BytecodeType == GPUCC_BYTECODE_TYPE_DXIL) { /* DXIL-specific arguments */
        /* ... */
    }
    if (config->BytecodeType == GPUCC_BYTECODE_TYPE_SPIRV) { /* Vulkan/OpenGL SPIRV-specific arguments */
        gpuccDxcStoreArg(dxc, DxcArg_GenerateSPIRV);
        gpuccDxcStoreArg(dxc, DxcArg_VulkanUseGlLayout);
    }
    if (config->TargetRuntime == GPUCC_TARGET_RUNTIME_VULKAN_1_0) {
        gpuccDxcStoreArg(dxc, DxcArg_TargetVulkan1_0);
    }
    if (config->TargetRuntime == GPUCC_TARGET_RUNTIME_VULKAN_1_1) {
        gpuccDxcStoreArg(dxc, DxcArg_TargetVulkan1_1);
    }
    if (config->TargetRuntime == GPUCC_TARGET_RUNTIME_VULKAN_1_0 || 
        config->TargetRuntime == GPUCC_TARGET_RUNTIME_VULKAN_1_1) {
        if (_stricmp(shader_type, "vs") == 0 || 
            _stricmp(shader_type, "gs") == 0 || 
            _stricmp(shader_type, "ds") == 0) {
            gpuccDxcStoreArg(dxc, DxcArg_VulkanInvertY);
        }
    }

    /* Finally, initialize an IDxcLibrary instance for creating blobs, etc. 
     * as well as the IDxcCompiler instance for actually compiling the code.
     * TODO: Need to figure out how to clean these up.
     */
    if (FAILED((res = dispatch->DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&lib))))) {
        GPUCC_RESULT r = gpuccMakeResult_HRESULT(res);
        gpuccDebugPrintf(L"");
        gpuccSetLastResult(r);
        free(base);
        return nullptr;
    }
    if (FAILED((res = dispatch->DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcc))))) {
        GPUCC_RESULT r = gpuccMakeResult_HRESULT(res);
        gpuccDebugPrintf(L"");
        gpuccSetLastResult(r);
        lib->Release();
        free(base);
        return nullptr;
    }

    /* Finish initializing the compiler structure. */
    dxc->CommonFields.CompilerType        = GPUCC_COMPILER_TYPE_DXC;
    dxc->CommonFields.BytecodeType        = config->BytecodeType;
    dxc->CommonFields.CreateBytecode      = gpuccCreateProgramBytecodeDxc;
    dxc->CommonFields.DeleteBytecode      = gpuccDeleteProgramBytecodeDxc;
    dxc->CommonFields.CompileBytecode     = gpuccCompileBytecodeDxc;
    dxc->CommonFields.CleanupCompiler     = gpuccCleanupCompilerDxc;
    dxc->DispatchTable                    =&pctx->DxcCompiler_Dispatch;
    dxc->DxcLibrary                       = lib;
    dxc->DxcCompiler                      = dxcc;
    dxc->DefineArray                      = macros;
    dxc->DefineCount                      = config->DefineCount;
    dxc->TargetRuntime                    = config->TargetRuntime;
    return (struct GPUCC_PROGRAM_COMPILER*) dxc;
}

