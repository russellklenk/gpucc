/**
 * @summary gpucc.h: Define common constants and macros used throughout the 
 * library along with any public types and entry points exported by the library.
 * The GpuCC library provides a basic front-end for passing GPU program 
 */
#ifndef __GPUCC_H__
#define __GPUCC_H__

#pragma once

#ifndef GPUCC_NO_INCLUDES
#   include <stddef.h>
#   include <stdint.h>
#endif

/* @summary Define the version of the GPUCC API defined by this header.
 */
#ifndef GPUCC_VERSION_CONSTANTS
#   define GPUCC_VERSION_CONSTANTS
#   define GPUCC_VERSION_MAJOR                                                1
#   define GPUCC_VERSION_MINOR                                                0
#   define GPUCC_VERSION_PATCH                                                0
#endif

/* @summary A macro used to specify a "public" API function available for use 
 * within other modules (but not necessarily exported from the library).
 * @param _return_type The return type of the function, such as int or void.
 */
#ifndef GPUCC_API
#   define GPUCC_API(_return_type)                          extern _return_type
#endif

/* Forward-declare opaque types used by the library but not defined publicly. */
struct GPUCC_PROGRAM_BYTECODE;
struct GPUCC_PROGRAM_COMPILER;

typedef enum GPUCC_USAGE_MODE {
    GPUCC_USAGE_MODE_OFFLINE                      =   0,                       /* GpuCC is being used for offline compilation, for example, as part of a build process. */
    GPUCC_USAGE_MODE_RUNTIME                      =   1,                       /* GpuCC is being used for runtime compilation. */
} GPUCC_USAGE_MODE;

/* @summary Define the various types of potentially supported bytecode.
 * A bytecode type is used to select a compiler that produces the given bytecode type.
 */
typedef enum GPUCC_BYTECODE_TYPE {
    GPUCC_BYTECODE_TYPE_UNKNOWN                   =   0,                       /* The bytecode type is not known or not specified. */
    GPUCC_BYTECODE_TYPE_DXIL                      =   1,                       /* The DirectX bytecode type for use with Direct3D on Windows 10. */
    GPUCC_BYTECODE_TYPE_DXBC                      =   2,                       /* The pre-SM6 DirectX bytecode format for use with Direct3D. */
    GPUCC_BYTECODE_TYPE_SPIRV                     =   3,                       /* The SPIR-V bytecode type for use with Vulkan and OpenGL 4.5+. */
    GPUCC_BYTECODE_TYPE_PTX                       =   4,                       /* The PTX bytecode type for use with NVIDIA CUDA. */
} GPUCC_BYTECODE_TYPE;

/* @summary Define the supported target runtimes.
 * Only limited combinations of runtime + bytecode type are valid:
 * TODO
 */
typedef enum GPUCC_TARGET_RUNTIME {
    GPUCC_TARGET_RUNTIME_UNKNOWN                  =   0,                       /* No runtime target is specified. */
    GPUCC_TARGET_RUNTIME_DIRECT3D                 =   1,                       /* The GPU program targets Direct3D 11.x or earlier. */
    GPUCC_TARGET_RUNTIME_DIRECT3D12               =   2,                       /* The GPU program targets Direct3D 12. */
    GPUCC_TARGET_RUNTIME_VULKAN1                  =   3,                       /* The GPU program targets Vulkan 1.x. */
    GPUCC_TARGET_RUNTIME_OPENGL                   =   4,                       /* The GPU program targets OpenGL 4.5 or later. */
    GPUCC_TARGET_RUNTIME_CUDA                     =   5,                       /* The GPU program will be executed as a CUDA kernel. */
} GPUCC_TARGET_RUNTIME;

/* @summary Define the set of result codes that can be produced by the GPUCC library.
 */
typedef enum GPUCC_ERROR_CODE {
    GPUCC_RESULT_CODE_SUCCESS                     =   0,                       /* No error was encountered by the GPUCC library. */
    GPUCC_RESULT_CODE_ALREADY_INITIALIZED         =   1,                       /* The gpuccStartup function has already been called. */
    GPUCC_RESULT_CODE_NOT_INITIALIZED             =  -1,                       /* The gpuccStartup function has not been called. */
    GPUCC_RESULT_CODE_PLATFORM_ERROR              =  -2,                       /* The underlying platform returned an error code. */
    GPUCC_RESULT_CODE_INVALID_USAGE_MODE          =  -3,                       /* The supplied GPUCC_USAGE_MODE is invalid. */
    GPUCC_RESULT_CODE_COMPILER_NOT_SUPPORTED      =  -4,                       /* The required compiler is not supported on the current platform. */
    GPUCC_RESULT_CODE_OUT_OF_HOST_MEMORY          =  -5,                       /* An attempt to allocate host memory failed. */
    GPUCC_RESULT_CODE_INVALID_TARGET_PROFILE      =  -6,                       /* The specified target profile is invalid. */
    GPUCC_RESULT_CODE_INVALID_TARGET_RUNTIME      =  -7,                       /* */
    GPUCC_RESULT_CODE_INVALID_BYTECODE_TYPE       =  -8,                       /* */
    GPUCC_RESULT_CODE_INVALID_ARGUMENT            =  -9,                       /* */
} GPUCC_RESULT_CODE;

/* @summary Define the set of supported GPU program compilers. Not all compilers are supported on all platforms.
 */
typedef enum GPUCC_COMPILER_TYPE {
    GPUCC_COMPILER_TYPE_UNKNOWN                   =   0,                       /* */
    GPUCC_COMPILER_TYPE_DXC                       =   1,                       /* */
    GPUCC_COMPILER_TYPE_FXC                       =   2,                       /* */
    GPUCC_COMPILER_TYPE_SHADERC                   =   3,                       /* */
    GPUCC_COMPILER_TYPE_NVRTC                     =   4,                       /* */
    GPUCC_COMPILER_TYPE_COUNT
} GPUCC_COMPILER_TYPE;

/* @summary Define a set of flags, one for each value of the GPUCC_COMPILER_TYPE enumeration, that can be bitwise OR'd together to indicate whether a particular compiler type is supported by the host.
 */
typedef enum GPUCC_COMPILER_SUPPORT {
    GPUCC_COMPILER_SUPPORT_NONE                   = (0UL <<  0),               /* */
    GPUCC_COMPILER_SUPPORT_DXC                    = (1UL <<  0),               /* */
    GPUCC_COMPILER_SUPPORT_FXC                    = (1UL <<  1),               /* */
    GPUCC_COMPILER_SUPPORT_SHADERC                = (1UL <<  2),               /* */
    GPUCC_COMPILER_SUPPORT_NVRTC                  = (1UL <<  3),               /* */
} GPUCC_COMPILER_SUPPORT;

/* @summary A structure for returning an error result from a GPUCC API call.
 * Use the gpuccFailure and gpuccSuccess functions to determine whether the result represents a failed call.
 */
typedef struct GPUCC_RESULT {
    int32_t      LibraryResult;                                                /* One of the values of the GPUCC_ERROR_CODE enumeration. */
    int32_t      PlatformResult;                                               /* The error code returned by the underlying platform (errno, GetLastError, etc.) cast to a 32-bit signed integer. */
} GPUCC_RESULT;

/* @summary Define the data used to initialize a GpuCC program compiler.
 * Data is copied from this structure into the compiler data at the time of the gpuccCreateCompiler call.
 */
typedef struct GPUCC_PROGRAM_COMPILER_INIT {
    char const **DefineSymbols;                                                /* An array of DefineCount nul-terminated strings specifying defined preprocessor symbols. */
    char const **DefineValues;                                                 /* An array of DefineCount nul-terminated strings specifying the value (if any) associated with the corresponding preprocessor symbol. */
    char const  *TargetProfile;                                                /* A nul-terminated string specifying the target shader profile. */
    int32_t      TargetRuntime;                                                /* One of the values of the GPUCC_TARGET_RUNTIME enumeration specifying the target runtime environment. */
    int32_t      BytecodeType;                                                 /* One of the values of the GPUCC_BYTECODE_TYPE enumeration specifying the type of bytecode to be generated by the compiler. */
    uint32_t     DefineCount;                                                  /* The number of items in the DefineSymbols and DefineValues arrays. */
} GPUCC_PROGRAM_COMPILER_INIT;

#ifdef __cplusplus
extern "C" {
#endif

/* @summary Retrieve the version of the GpuCC library.
 * @param o_major On return, this location is updated with the major version component.
 * @param o_minor On return, this location is updated with the minor version component.
 * @param o_patch On return, this location is updated with the patch version component.
 */
GPUCC_API(void)
gpuccVersion
(
    int32_t *o_major, 
    int32_t *o_minor, 
    int32_t *o_patch
);

/* @summary Inspect a GPUCC_RESULT to determine whether it represets a failure.
 * @param result The result structure to inspect.
 * @return Non-zero if the result represents a failure.
 */
GPUCC_API(int32_t)
gpuccFailure
(
    struct GPUCC_RESULT result
);

/* @summary Inspect a GPUCC_RESULT to determine whether it represents a success.
 * @param result The result structure to inspect.
 * @return Non-zero if the result represents a success.
 */
GPUCC_API(int32_t)
gpuccSuccess
(
    struct GPUCC_RESULT result
);

/* @summary Convert a GPUCC_RESULT_CODE into a string representation.
 * The resulting string should not be freed by the application, and remains valid until GpuCC is unloaded from the process.
 * @param gpucc_result_code One of the values of the GPUCC_RESULT_CODE enumeration.
 * @return A pointer to a nul-terminated UTF-8 encoded string.
 */
GPUCC_API(char const*)
gpuccErrorString
(
    int32_t gpucc_result_code
);

GPUCC_API(char const*)
gpuccBytecodeTypeString
(
    int32_t gpucc_bytecode_type
);

GPUCC_API(char const*)
gpuccCompilerTypeString
(
    int32_t gpucc_compiler_type
);

/* @summary Perform any initialization required when GpuCC is loaded into the process.
 * This function cannot be safely called by multiple threads concurrently.
 * @param gpucc_usage_mode One of the values of the GPUCC_USAGE_MODE enumeration indicating how the process will use the library.
 * @return A result code indicating whether the library is ready to use.
 */
GPUCC_API(struct GPUCC_RESULT)
gpuccStartup
(
    uint32_t gpucc_usage_mode
);

/**
 * Functions defined below this point require gpuccStartup to have been called.
 */

/* @summary Perform any final cleanup immediately prior to unloading GpuCC from the process.
 * This function cannot be safely called by multiple threads concurrently.
 */
GPUCC_API(void)
gpuccShutdown
(
    void
);

/* @summary Retrieve the result code of the most recent GpuCC operation executed on the calling thread.
 * @return The most recent GPUCC_RESULT status for the calling thread.
 */
GPUCC_API(struct GPUCC_RESULT)
gpuccGetLastResult
(
    void
);

/* @summary Create a new GPU program compiler with the given configuration.
 * @param config Data used to configure the compiler instance. Data is copied into the compiler storage before the function returns.
 * @return A pointer to the new compiler structure, or NULL if an error occurred.
 */
GPUCC_API(struct GPUCC_PROGRAM_COMPILER*)
gpuccCreateCompiler
(
    struct GPUCC_PROGRAM_COMPILER_INIT *config
);

/* @summary Free resources associated with a compiler instance.
 * @param compiler The compiler object to delete.
 */
GPUCC_API(void)
gpuccDeleteCompiler
(
    struct GPUCC_PROGRAM_COMPILER *compiler
);

/* @summary Retrieve type information for a particular compiler instance.
 * @param compiler A GPUCC_PROGRAM_COMPILER returned by a prior call to gpuccCreateCompiler.
 * @return One of the values of the GPUCC_COMPILER_TYPE enumeration.
 */
GPUCC_API(int32_t)
gpuccQueryCompilerType
(
    struct GPUCC_PROGRAM_COMPILER *compiler
);

/* @summary Retrieve the type of bytecode generated by a particular compiler instance.
 * @param compiler A GPUCC_PROGRAM_COMPILER returned by a prior call to gpuccCreateCompiler.
 * @return One of the values of the GPUCC_BYTECODE_TYPE enumeration.
 */
GPUCC_API(int32_t)
gpuccQueryBytecodeType
(
    struct GPUCC_PROGRAM_COMPILER *compiler
);

#ifdef __cplusplus
}; /* extern "C" */
#endif

#endif /* __GPUCC_H__ */

#ifdef GPUCC_LOADER_IMPLEMENTATION
#endif /* GPUCC_LOADER_IMPLEMENTATION */

