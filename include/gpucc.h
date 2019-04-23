/**
 * @summary gpucc.h: Define common constants and macros used throughout the 
 * library along with any public types and entry points exported by the library.
 * The GpuCC library provides a basic front-end for passing GPU program source 
 * code to a compiler back-end that compiles to bytecode. This bytecode can 
 * then be saved to disk for later use or passed to the GPU driver for final 
 * conversion to the GPU ISA and execution.
 *
 * If you define GPUCC_LOADER_IMPLEMENTATION prior to including this header 
 * in _one source file only_, a loader implementation will be synthesized into
 * that translation unit. The loader API loads GpuCC at runtime and populates 
 * a dispatch table. Additionally define GPUCC_LOCAL_RUNTIME_IMPLEMENTATION to 
 * synthesize the GpuCC public API functions that call through a global 
 * dispatch table.
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
    GPUCC_RESULT_CODE_CANNOT_LOAD                 = -10,                       /* */
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

#ifndef GPUCC_NO_PROTOTYPES

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

#endif /* GPUCC_NO_PROTOTYPES */

#ifdef __cplusplus
}; /* extern "C" */
#endif

#endif /* __GPUCC_H__ */

#ifdef GPUCC_LOADER_IMPLEMENTATION

/* @summary Define a general signature for a dynamically-loaded function.
 * Code will have to cast the function pointer to the correct type.
 */
typedef int (*PFN_GpuCC_Unknown)(void);

/* @summary Helper macro for populating a dispatch table with functions loaded at runtime.
 * If the function is not found, the entry point is updated to point to a stub implementation provided by the caller.
 * This macro relies on specific naming conventions:
 * - The signature must be PFN_BlahBlahBlah where BlahBlahBlah corresponds to the _fn argument.
 * - The dispatch table field must be BlahBlahBlah where BlahBlahBlah corresponds to the _fn argument.
 * - The stub function must be named BlahBlahBlah_Stub where BlahBlahBlah corresponds to the _fn argument.
 * @param _disp A pointer to the dispatch table to populate.
 * @param _module The GPUCC_RUNTIME_MODULE representing the module loaded into the process address space.
 * @param _fn The name of the function to dynamically load.
 */
#ifndef gpuccResolveRuntimeFunction
#define gpuccResolveRuntimeFunction(_disp, _module, _fn)                       \
    for (;;) {                                                                 \
        (_disp)->_fn=(PFN_##_fn) gpuccRuntimeModuleResolve((_module), #_fn);   \
        if ((_disp)->_fn == NULL) {                                            \
            (_disp)->_fn  = _fn##_Stub;                                        \
        } break;                                                               \
    }
#endif

/**
 * Implement a small shim layer for each platform that the loader can rely on 
 * for dynamically loading a DLL or shared object library into the process 
 * address space and resolving entry points.
 */
#if   defined(__APPLE__)
#   if defined(TARGET_OS_IPHONE) || defined(TARGET_OS_IPHONE_SIMULATOR)
#       error No GpuCC loader implementation for iOS (yet).
#   else
#       error No GpuCC loader implementation for macOS (yet).
#   endif
#elif defined(_WIN32) || defined(_WIN64)
/** BEGIN WINDOWS **/
#   ifndef GPUCC_LOADER_NO_INCLUDES
#       include <Windows.h>
#   endif

#   ifndef GPUCC_LOADER_UNUSED
#       define GPUCC_LOADER_UNUSED(_x) (void)(_x)
#   endif

    /* @summary Declare the runtime module type. On Windows, this is the type returned by LoadLibraryW.
     */
    typedef HMODULE GPUCC_RUNTIME_MODULE;

    /* @summary Resolve a function entry point declared with C linkage.
     * This function is used by the gpuccResolveRuntimeFunction macro.
     * @param module The module handle of the DLL.
     * @param symbol The unmangled symbol name.
     * @return A pointer to the function loaded into the process address space, or NULL if the entry point is not found.
     */
    static PFN_GpuCC_Unknown
    gpuccRuntimeModuleResolve
    (
        GPUCC_RUNTIME_MODULE module, 
        char const          *symbol
    )
    {
        if (module != NULL) {
            return (PFN_GpuCC_Unknown) GetProcAddress(module, symbol);
        } else {
            return (PFN_GpuCC_Unknown) NULL;
        }
    }

    /* @summary Load the GpuCC DLL into the address space of the calling process.
     * @return The module handle, or NULL if GpuCC.dll could not be loaded.
     */
    static GPUCC_RUNTIME_MODULE
    gpuccRuntimeModuleLoad
    (
        void
    )
    {
        return (GPUCC_RUNTIME_MODULE) LoadLibraryW(L"gpucc.dll");
    }

    /* @summary Unload the GpuCC DLL from the address space of the calling process.
     */
    static void
    gpuccRuntimeModuleUnload
    (
        GPUCC_RUNTIME_MODULE module
    )
    {
        if (module != NULL) {
            FreeLibrary(module);
        }
    }

/*** END WINDOWS ***/
#elif defined(__linux__) || defined(__gnu_linux__)
#   error No GpuCC loader implementation for Linux (yet).
#else
#   error No GpuCC loader implementation for your platform (yet).
#endif

/*** FUNCTION POINTER TYPES ***/
typedef void                           (*PFN_gpuccVersion           )(int32_t*, int32_t*, int32_t*);
typedef int32_t                        (*PFN_gpuccFailure           )(struct GPUCC_RESULT);
typedef int32_t                        (*PFN_gpuccSuccess           )(struct GPUCC_RESULT);
typedef char const*                    (*PFN_gpuccErrorString       )(int32_t);
typedef char const*                    (*PFN_gpuccBytecodeTypeString)(int32_t);
typedef char const*                    (*PFN_gpuccCompilerTypeString)(int32_t);
typedef struct GPUCC_RESULT            (*PFN_gpuccStartup           )(uint32_t gpucc_usage_mode);
typedef void                           (*PFN_gpuccShutdown          )(void);
typedef struct GPUCC_RESULT            (*PFN_gpuccGetLastResult     )(void);
typedef struct GPUCC_PROGRAM_COMPILER* (*PFN_gpuccCreateCompiler    )(struct GPUCC_PROGRAM_COMPILER_INIT*);
typedef void                           (*PFN_gpuccDeleteCompiler    )(struct GPUCC_PROGRAM_COMPILER*);
typedef int32_t                        (*PFN_gpuccQueryCompilerType )(struct GPUCC_PROGRAM_COMPILER*);
typedef int32_t                        (*PFN_gpuccQueryBytecodeType )(struct GPUCC_PROGRAM_COMPILER*);

typedef struct GPUCC_LOADER_DISPATCH {
    PFN_gpuccVersion            gpuccVersion;
    PFN_gpuccFailure            gpuccFailure;
    PFN_gpuccSuccess            gpuccSuccess;
    PFN_gpuccErrorString        gpuccErrorString;
    PFN_gpuccBytecodeTypeString gpuccBytecodeTypeString;
    PFN_gpuccCompilerTypeString gpuccCompilerTypeString;
    PFN_gpuccStartup            gpuccStartup;
    PFN_gpuccShutdown           gpuccShutdown;
    PFN_gpuccGetLastResult      gpuccGetLastResult;
    PFN_gpuccCreateCompiler     gpuccCreateCompiler;
    PFN_gpuccDeleteCompiler     gpuccDeleteCompiler;
    PFN_gpuccQueryCompilerType  gpuccQueryCompilerType;
    PFN_gpuccQueryBytecodeType  gpuccQueryBytecodeType;
    GPUCC_RUNTIME_MODULE        ModuleHandle_GpuCC;
} GPUCC_LOADER_DISPATCH;

/*** STUB IMPLEMENTATIONS ***/
static void
gpuccVersion_Stub
(
    int32_t *o_major, 
    int32_t *o_minor, 
    int32_t *o_patch
)
{
    if (o_major) *o_major = 0;
    if (o_minor) *o_minor = 0;
    if (o_patch) *o_patch = 0;
}

static int32_t
gpuccFailure_Stub
(
    struct GPUCC_RESULT r
)
{
    return (r.LibraryResult < 0);
}

static int32_t
gpuccSuccess_Stub
(
    struct GPUCC_RESULT r
)
{
    return (r.LibraryResult >= 0);
}

static char const*
gpuccErrorString_Stub
(
    int32_t gpucc_result_code
)
{
    switch (gpucc_result_code) {
        case GPUCC_RESULT_CODE_SUCCESS               : return "GPUCC_RESULT_CODE_SUCCESS";
        case GPUCC_RESULT_CODE_ALREADY_INITIALIZED   : return "GPUCC_RESULT_CODE_ALREADY_INITIALIZED";
        case GPUCC_RESULT_CODE_NOT_INITIALIZED       : return "GPUCC_RESULT_CODE_NOT_INITIALIZED";
        case GPUCC_RESULT_CODE_PLATFORM_ERROR        : return "GPUCC_RESULT_CODE_PLATFORM_ERROR";
        case GPUCC_RESULT_CODE_INVALID_USAGE_MODE    : return "GPUCC_RESULT_CODE_INVALID_USAGE_MODE";
        case GPUCC_RESULT_CODE_COMPILER_NOT_SUPPORTED: return "GPUCC_RESULT_CODE_COMPILER_NOT_SUPPORTED";
        case GPUCC_RESULT_CODE_OUT_OF_HOST_MEMORY    : return "GPUCC_RESULT_CODE_OUT_OF_HOST_MEMORY";
        case GPUCC_RESULT_CODE_INVALID_TARGET_PROFILE: return "GPUCC_RESULT_CODE_INVALID_TARGET_PROFILE";
        case GPUCC_RESULT_CODE_INVALID_TARGET_RUNTIME: return "GPUCC_RESULT_CODE_INVALID_TARGET_RUNTIME";
        case GPUCC_RESULT_CODE_INVALID_BYTECODE_TYPE : return "GPUCC_RESULT_CODE_INVALID_BYTECODE_TYPE";
        case GPUCC_RESULT_CODE_INVALID_ARGUMENT      : return "GPUCC_RESULT_CODE_INVALID_ARGUMENT";
        case GPUCC_RESULT_CODE_CANNOT_LOAD           : return "GPUCC_RESULT_CODE_CANNOT_LOAD";
        default                                      : return "GPUCC_RESULT_CODE (unknown)";
    }
}

static char const*
gpuccBytecodeTypeString_Stub
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

static char const*
gpuccCompilerTypeString_Stub
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

static struct GPUCC_RESULT
gpuccStartup_Stub
(
    uint32_t gpucc_usage_mode
)
{
    GPUCC_LOADER_UNUSED(gpucc_usage_mode);
    return GPUCC_RESULT{ GPUCC_RESULT_CODE_CANNOT_LOAD, 0 };
}

static void
gpuccShutdown_Stub
(
    void
)
{
    /* empty */
}

static struct GPUCC_RESULT
gpuccGetLastResult_Stub
(
    void
)
{
    return GPUCC_RESULT{ GPUCC_RESULT_CODE_CANNOT_LOAD, 0 };
}

static struct GPUCC_PROGRAM_COMPILER*
gpuccCreateCompiler_Stub
(
    struct GPUCC_PROGRAM_COMPILER_INIT *config
)
{
    GPUCC_LOADER_UNUSED(config);
    return NULL;
}

static void
gpuccDeleteCompiler_Stub
(
    struct GPUCC_PROGRAM_COMPILER *compiler
)
{
    GPUCC_LOADER_UNUSED(compiler);
}

static int32_t
gpuccQueryCompilerType_Stub
(
    struct GPUCC_PROGRAM_COMPILER *compiler
)
{
    GPUCC_LOADER_UNUSED(compiler);
    return GPUCC_COMPILER_TYPE_UNKNOWN;
}

static int32_t
gpuccQueryBytecodeType_Stub
(
    struct GPUCC_PROGRAM_COMPILER *compiler
)
{
    GPUCC_LOADER_UNUSED(compiler);
    return GPUCC_BYTECODE_TYPE_UNKNOWN;
}

/*** LOADER IMPLEMENTATION ***/
static void
gpuccLoaderStubDispatch
(
    struct GPUCC_LOADER_DISPATCH *dispatch
)
{
    dispatch->gpuccVersion              = gpuccVersion_Stub;
    dispatch->gpuccFailure              = gpuccFailure_Stub;
    dispatch->gpuccSuccess              = gpuccSuccess_Stub;
    dispatch->gpuccErrorString          = gpuccErrorString_Stub;
    dispatch->gpuccBytecodeTypeString   = gpuccBytecodeTypeString_Stub;
    dispatch->gpuccCompilerTypeString   = gpuccCompilerTypeString_Stub;
    dispatch->gpuccStartup              = gpuccStartup_Stub;
    dispatch->gpuccShutdown             = gpuccShutdown_Stub;
    dispatch->gpuccGetLastResult        = gpuccGetLastResult_Stub;
    dispatch->gpuccCreateCompiler       = gpuccCreateCompiler_Stub;
    dispatch->gpuccDeleteCompiler       = gpuccDeleteCompiler_Stub;
    dispatch->gpuccQueryCompilerType    = gpuccQueryCompilerType_Stub;
    dispatch->gpuccQueryBytecodeType    = gpuccQueryBytecodeType_Stub;
    dispatch->ModuleHandle_GpuCC        = NULL;
}
static int32_t
gpuccLoaderPopulateDispatchFrom
(
    struct GPUCC_LOADER_DISPATCH *dispatch, 
    GPUCC_RUNTIME_MODULE            module
)
{
    gpuccResolveRuntimeFunction(dispatch, module, gpuccVersion);
    gpuccResolveRuntimeFunction(dispatch, module, gpuccFailure);
    gpuccResolveRuntimeFunction(dispatch, module, gpuccSuccess);
    gpuccResolveRuntimeFunction(dispatch, module, gpuccErrorString);
    gpuccResolveRuntimeFunction(dispatch, module, gpuccBytecodeTypeString);
    gpuccResolveRuntimeFunction(dispatch, module, gpuccCompilerTypeString);
    gpuccResolveRuntimeFunction(dispatch, module, gpuccStartup);
    gpuccResolveRuntimeFunction(dispatch, module, gpuccShutdown);
    gpuccResolveRuntimeFunction(dispatch, module, gpuccGetLastResult);
    gpuccResolveRuntimeFunction(dispatch, module, gpuccCreateCompiler);
    gpuccResolveRuntimeFunction(dispatch, module, gpuccDeleteCompiler);
    gpuccResolveRuntimeFunction(dispatch, module, gpuccQueryCompilerType);
    gpuccResolveRuntimeFunction(dispatch, module, gpuccQueryBytecodeType);
    dispatch->ModuleHandle_GpuCC        = module;
    return module != NULL;
}

static int32_t
gpuccLoaderPopulateDispatch
(
    struct GPUCC_LOADER_DISPATCH *dispatch
)
{
    GPUCC_RUNTIME_MODULE module = gpuccRuntimeModuleLoad();
    return gpuccLoaderPopulateDispatchFrom(dispatch, module);
}

static void
gpuccLoaderInvalidateDispatch
(
    struct GPUCC_LOADER_DISPATCH *dispatch
)
{
    GPUCC_RUNTIME_MODULE module = dispatch->ModuleHandle_GpuCC;
    gpuccLoaderStubDispatch(dispatch);
    gpuccRuntimeModuleUnload(module);
}

#ifdef GPUCC_LOCAL_RUNTIME_IMPLEMENTATION

    GPUCC_LOADER_DISPATCH g_gpuccDispatch = {};

    GPUCC_API(struct GPUCC_RESULT)
    gpuccLocalRuntimeStartup
    (
        uint32_t gpucc_usage_mode
    )
    {
        gpuccLoaderPopulateDispatch(&g_gpuccDispatch);
        return g_gpuccDispatch.gpuccStartup(gpucc_usage_mode);
    }

    GPUCC_API(void)
    gpuccLocalRuntimeShutdown
    (
        void
    )
    {
        g_gpuccDispatch.gpuccShutdown();
        gpuccLoaderInvalidateDispatch(&g_gpuccDispatch);
    }

    GPUCC_API(void)
    gpuccVersion
    (
        int32_t *o_major, 
        int32_t *o_minor, 
        int32_t *o_patch
    )
    {
        g_gpuccDispatch.gpuccVersion(o_major, o_minor, o_patch);
    }

    GPUCC_API(int32_t)
    gpuccFailure
    (
        struct GPUCC_RESULT r
    )
    {
        return g_gpuccDispatch.gpuccFailure(r);
    }

    GPUCC_API(int32_t)
    gpuccSuccess
    (
        struct GPUCC_RESULT r
    )
    {
        return g_gpuccDispatch.gpuccSuccess(r);
    }

    GPUCC_API(char const*)
    gpuccErrorString
    (
        int32_t gpucc_result_code
    )
    {
        return g_gpuccDispatch.gpuccErrorString(gpucc_result_code);
    }

    GPUCC_API(char const*)
    gpuccBytecodeTypeString
    (
        int32_t gpucc_bytecode_type
    )
    {
        return g_gpuccDispatch.gpuccBytecodeTypeString(gpucc_bytecode_type);
    }

    GPUCC_API(char const*)
    gpuccCompilerTypeString
    (
        int32_t gpucc_compiler_type
    )
    {
        return g_gpuccDispatch.gpuccCompilerTypeString(gpucc_compiler_type);
    }

    GPUCC_API(struct GPUCC_RESULT)
    gpuccStartup
    (
        uint32_t gpucc_usage_mode
    )
    {
        return g_gpuccDispatch.gpuccStartup(gpucc_usage_mode);
    }

    GPUCC_API(void)
    gpuccShutdown
    (
        void
    )
    {
        g_gpuccDispatch.gpuccShutdown();
    }

    GPUCC_API(struct GPUCC_RESULT)
    gpuccGetLastResult
    (
        void
    )
    {
        return g_gpuccDispatch.gpuccGetLastResult();
    }

    GPUCC_API(struct GPUCC_PROGRAM_COMPILER*)
    gpuccCreateCompiler
    (
        struct GPUCC_PROGRAM_COMPILER_INIT *config
    )
    {
        return g_gpuccDispatch.gpuccCreateCompiler(config);
    }

    GPUCC_API(void)
    gpuccDeleteCompiler 
    (
        struct GPUCC_PROGRAM_COMPILER *compiler
    )
    {
        g_gpuccDispatch.gpuccDeleteCompiler(compiler);
    }

    GPUCC_API(int32_t)
    gpuccQueryCompilerType
    (
        struct GPUCC_PROGRAM_COMPILER *compiler
    )
    {
        return g_gpuccDispatch.gpuccQueryCompilerType(compiler);
    }

    GPUCC_API(int32_t)
    gpuccQueryBytecodeType
    (
        struct GPUCC_PROGRAM_COMPILER *compiler
    )
    {
        return g_gpuccDispatch.gpuccQueryBytecodeType(compiler);
    }

#endif /* GPUCC_LOCAL_RUNTIME_IMPLEMENTATION */

#endif /* GPUCC_LOADER_IMPLEMENTATION */

