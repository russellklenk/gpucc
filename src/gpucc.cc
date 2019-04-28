/**
 * @summary Implements the platform-independent portion of the public GpuCC API.
 */
#include "gpucc.h"
#include "gpucc_internal.h"

static char g_EmptyUtf8String[1] = {'\0'};

GPUCC_API(void)
gpuccVersion
(
    int32_t *o_major, 
    int32_t *o_minor, 
    int32_t *o_patch
)
{
    if (o_major) *o_major = GPUCC_VERSION_MAJOR;
    if (o_minor) *o_minor = GPUCC_VERSION_MINOR;
    if (o_patch) *o_patch = GPUCC_VERSION_PATCH;
}

GPUCC_API(int32_t)
gpuccFailure
(
    struct GPUCC_RESULT result
)
{
    return (result.LibraryResult < 0);
}

GPUCC_API(int32_t)
gpuccSuccess
(
    struct GPUCC_RESULT result
)
{
    return (result.LibraryResult >= 0);
}

GPUCC_API(char const*)
gpuccErrorString
(
    int32_t gpucc_result_code
)
{
    switch (gpucc_result_code) {
        case GPUCC_RESULT_CODE_SUCCESS                   : return "GPUCC_RESULT_CODE_SUCCESS";
        case GPUCC_RESULT_CODE_ALREADY_INITIALIZED       : return "GPUCC_RESULT_CODE_ALREADY_INITIALIZED";
        case GPUCC_RESULT_CODE_EMPTY_BYTECODE_CONTAINER  : return "GPUCC_RESULT_CODE_EMPTY_BYTECODE_CONTAINER";
        case GPUCC_RESULT_CODE_NOT_INITIALIZED           : return "GPUCC_RESULT_CODE_NOT_INITIALIZED";
        case GPUCC_RESULT_CODE_PLATFORM_ERROR            : return "GPUCC_RESULT_CODE_PLATFORM_ERROR";
        case GPUCC_RESULT_CODE_INVALID_USAGE_MODE        : return "GPUCC_RESULT_CODE_INVALID_USAGE_MODE";
        case GPUCC_RESULT_CODE_COMPILER_NOT_SUPPORTED    : return "GPUCC_RESULT_CODE_COMPILER_NOT_SUPPORTED";
        case GPUCC_RESULT_CODE_OUT_OF_HOST_MEMORY        : return "GPUCC_RESULT_CODE_OUT_OF_HOST_MEMORY";
        case GPUCC_RESULT_CODE_INVALID_TARGET_PROFILE    : return "GPUCC_RESULT_CODE_INVALID_TARGET_PROFILE";
        case GPUCC_RESULT_CODE_INVALID_TARGET_RUNTIME    : return "GPUCC_RESULT_CODE_INVALID_TARGET_RUNTIME";
        case GPUCC_RESULT_CODE_INVALID_BYTECODE_TYPE     : return "GPUCC_RESULT_CODE_INVALID_BYTECODE_TYPE";
        case GPUCC_RESULT_CODE_INVALID_ARGUMENT          : return "GPUCC_RESULT_CODE_INVALID_ARGUMENT";
        case GPUCC_RESULT_CODE_CANNOT_LOAD               : return "GPUCC_RESULT_CODE_CANNOT_LOAD";
        case GPUCC_RESULT_CODE_COMPILE_FAILED            : return "GPUCC_RESULT_CODE_COMPILE_FAILED";
        case GPUCC_RESULT_CODE_INVALID_BYTECODE_CONTAINER: return "GPUCC_RESULT_CODE_INVALID_BYTECODE_CONTAINER";
        default                                          : return "GPUCC_RESULT_CODE (unknown)";
    }
}

GPUCC_API(char const*)
gpuccBytecodeTypeString
(
    int32_t gpucc_bytecode_type
)
{
    switch (gpucc_bytecode_type) {
        case GPUCC_BYTECODE_TYPE_UNKNOWN: return "GPUCC_BYTECODE_TYPE_UNKNOWN";
        case GPUCC_BYTECODE_TYPE_DXIL   : return "GPUCC_BYTECODE_TYPE_DXIL";
        case GPUCC_BYTECODE_TYPE_DXBC   : return "GPUCC_BYTECODE_TYPE_DXBC";
        case GPUCC_BYTECODE_TYPE_SPIRV  : return "GPUCC_BYTECODE_TYPE_SPIRV";
        case GPUCC_BYTECODE_TYPE_PTX    : return "GPUCC_BYTECODE_TYPE_PTX";
        default                         : return "GPUCC_BYTECODE_TYPE (unknown)";
    }
}

GPUCC_API(char const*)
gpuccCompilerTypeString
(
    int32_t gpucc_compiler_type
)
{
    switch (gpucc_compiler_type) {
        case GPUCC_COMPILER_TYPE_UNKNOWN: return "GPUCC_COMPILER_TYPE_UNKNOWN";
        case GPUCC_COMPILER_TYPE_DXC    : return "GPUCC_COMPILER_TYPE_DXC";
        case GPUCC_COMPILER_TYPE_FXC    : return "GPUCC_COMPILER_TYPE_FXC";
        case GPUCC_COMPILER_TYPE_SHADERC: return "GPUCC_COMPILER_TYPE_SHADERC";
        case GPUCC_COMPILER_TYPE_NVRTC  : return "GPUCC_COMPILER_TYPE_NVRTC";
        default                         : return "GPUCC_COMPILER_TYPE (unknown)";
    }
}

GPUCC_API(int32_t)
gpuccQueryCompilerType
(
    struct GPUCC_PROGRAM_COMPILER *compiler
)
{
    if (compiler) {
        gpuccSetLastResult(gpuccMakeResult(GPUCC_RESULT_CODE_SUCCESS));
        return gpuccQueryCompilerType_(compiler);
    } else {
        gpuccSetLastResult(gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_ARGUMENT));
        return GPUCC_COMPILER_TYPE_UNKNOWN;
    }
}

GPUCC_API(int32_t)
gpuccQueryBytecodeType
(
    struct GPUCC_PROGRAM_COMPILER *compiler
)
{
    if (compiler) {
        gpuccSetLastResult(gpuccMakeResult(GPUCC_RESULT_CODE_SUCCESS));
        return gpuccQueryBytecodeType_(compiler);
    } else {
        gpuccSetLastResult(gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_ARGUMENT));
        return GPUCC_COMPILER_TYPE_UNKNOWN;
    }
}

GPUCC_API(struct GPUCC_PROGRAM_COMPILER*)
gpuccQueryBytecodeCompiler
(
    struct GPUCC_PROGRAM_BYTECODE *bytecode
)
{
    if (bytecode) {
        gpuccSetLastResult(gpuccMakeResult(GPUCC_RESULT_CODE_SUCCESS));
        return gpuccQueryBytecodeCompiler_(bytecode);
    } else {
        gpuccSetLastResult(gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_ARGUMENT));
        return nullptr;
    }
}

GPUCC_API(char const*)
gpuccQueryBytecodeEntryPoint
(
    struct GPUCC_PROGRAM_BYTECODE *bytecode
)
{
    char const *entry = nullptr;
    if (bytecode) {
        gpuccSetLastResult(gpuccMakeResult(GPUCC_RESULT_CODE_SUCCESS));
        entry = gpuccQueryBytecodeEntryPoint_(bytecode);
    } else {
        gpuccSetLastResult(gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_ARGUMENT));
    }
    if (entry == nullptr) {
        entry = g_EmptyUtf8String;
    } return entry;
}

GPUCC_API(char const*)
gpuccQueryBytecodeSourcePath
(
    struct GPUCC_PROGRAM_BYTECODE *bytecode
)
{
    char const *path = nullptr;
    if (bytecode) {
        gpuccSetLastResult(gpuccMakeResult(GPUCC_RESULT_CODE_SUCCESS));
        path = gpuccQueryBytecodeSourcePath_(bytecode);
    } else {
        gpuccSetLastResult(gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_ARGUMENT));
    }
    if (path == nullptr) {
        path = g_EmptyUtf8String;
    } return path;
}

GPUCC_API(struct GPUCC_RESULT)
gpuccQueryBytecodeCompileResult
(
    struct GPUCC_PROGRAM_BYTECODE *bytecode
)
{
    if (bytecode) {
        gpuccSetLastResult(gpuccMakeResult(GPUCC_RESULT_CODE_SUCCESS));
        return gpuccQueryBytecodeCompileResult_(bytecode);
    } else {
        gpuccSetLastResult(gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_ARGUMENT));
        return GPUCC_RESULT { GPUCC_RESULT_CODE_EMPTY_BYTECODE_CONTAINER, 0 };
    }
}

GPUCC_API(uint64_t)
gpuccQueryBytecodeSizeBytes
(
    struct GPUCC_PROGRAM_BYTECODE *bytecode
)
{
    if (bytecode != nullptr) {
        gpuccSetLastResult(gpuccMakeResult(GPUCC_RESULT_CODE_SUCCESS));
        return gpuccQueryBytecodeSizeBytes_(bytecode);
    } else {
        gpuccSetLastResult(gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_ARGUMENT));
        return 0;
    }
}

GPUCC_API(uint64_t)
gpuccQueryBytecodeLogSizeBytes
(
    struct GPUCC_PROGRAM_BYTECODE *bytecode
)
{
    if (bytecode != nullptr) {
        gpuccSetLastResult(gpuccMakeResult(GPUCC_RESULT_CODE_SUCCESS));
        return gpuccQueryBytecodeLogSizeBytes_(bytecode);
    } else {
        gpuccSetLastResult(gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_ARGUMENT));
        return 0;
    }
}

GPUCC_API(uint8_t*)
gpuccQueryBytecodeBuffer
(
    struct GPUCC_PROGRAM_BYTECODE *bytecode
)
{
    if (bytecode != nullptr) {
        gpuccSetLastResult(gpuccMakeResult(GPUCC_RESULT_CODE_SUCCESS));
        return gpuccQueryBytecodeBuffer_(bytecode);
    } else {
        gpuccSetLastResult(gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_ARGUMENT));
        return 0;
    }
}

GPUCC_API(char*)
gpuccQueryBytecodeLogBuffer
(
    struct GPUCC_PROGRAM_BYTECODE *bytecode
)
{
    if (bytecode != nullptr) {
        gpuccSetLastResult(gpuccMakeResult(GPUCC_RESULT_CODE_SUCCESS));
        return gpuccQueryBytecodeLogBuffer_(bytecode);
    } else {
        gpuccSetLastResult(gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_ARGUMENT));
        return 0;
    }
}

