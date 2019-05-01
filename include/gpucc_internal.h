/**
 * @summary gpucc_internal.h: Define GpuCC functions available for use in other
 * modules, but not part of the public interface.
 */
#ifndef __GPUCC_INTERNAL_H__
#define __GPUCC_INTERNAL_H__

#pragma once

#ifndef GPUCC_NO_INCLUDES
#   ifndef __GPUCC_H__
#       include "gpucc.h"
#   endif
#endif

/* @summary Define an inlined macro version of gpuccQueryCompilerType.
 * The caller is responsible for ensuring that _c is non-NULL.
 * @param _c A pointer to a GPUCC_PROGRAM_COMPILER object.
 * @return One of the values of the GPUCC_COMPILER_TYPE enumeration.
 */
#ifndef gpuccQueryCompilerType_
#define gpuccQueryCompilerType_(_c)                                            \
    (((GPUCC_PROGRAM_COMPILER_BASE const*)(_c))->CompilerType)
#endif

/* @summary Define an inlined macro version of gpuccQueryBytecodeType.
 * The caller is responsible for ensuring that _c is non-NULL.
 * @param _c A pointer to a GPUCC_PROGRAM_COMPILER object.
 * @return One of the values of the GPUCC_BYTECODE_TYPE enumeration.
 */
#ifndef gpuccQueryBytecodeType_
#define gpuccQueryBytecodeType_(_c)                                            \
    (((GPUCC_PROGRAM_COMPILER_BASE const*)(_c))->BytecodeType)
#endif

/* @summary Define an inlined macro version of gpuccQueryBytecodeCompiler.
 * The caller is responsible for ensuring that _b is non-NULL.
 * @param _b A pointer to a GPUCC_PROGRAM_BYTECODE object.
 * @return A pointer to the GPUCC_PROGRAM_COMPILER managing the bytecode container.
 */
#ifndef gpuccQueryBytecodeCompiler_
#define gpuccQueryBytecodeCompiler_(_b)                                        \
    (((GPUCC_PROGRAM_BYTECODE_BASE const*)(_b))->Compiler)
#endif

/* @summary Define an inlined macro version of gpuccQueryBytecodeEntryPoint.
 * The caller is responsible for ensuring that _b is non-NULL.
 * @param _b A pointer to a GPUCC_PROGRAM_BYTECODE object.
 * @return A pointer to a nul-terminated UTF-8 encoded string specifying the program entry point.
 * The GPUCC_PROGRAM_BYTECODE object owns this buffer and it remains valid until the container is deleted.
 */
#ifndef gpuccQueryBytecodeEntryPoint_
#define gpuccQueryBytecodeEntryPoint_(_b)                                      \
    (((GPUCC_PROGRAM_BYTECODE_BASE const*)(_b))->EntryPoint)
#endif

/* @summary Define an inlined macro version of gpuccQueryBytecodeSourcePath.
 * The caller is responsible for ensuring that _b is non-NULL.
 * @param _b A pointer to a GPUCC_PROGRAM_BYTECODE object.
 * @return A pointer to a nul-terminated UTF-8 encoded string specifying the path of the file containing the program source code.
 * The GPUCC_PROGRAM_BYTECODE object owns this buffer and it remains valid until the container is deleted.
 */
#ifndef gpuccQueryBytecodeSourcePath_
#define gpuccQueryBytecodeSourcePath_(_b)                                      \
    (((GPUCC_PROGRAM_BYTECODE_BASE const*)(_b))->SourcePath)
#endif

/* @summary Define an inlined macro version of gpuccQueryBytecodeCompileResult.
 * This returns the same GPUCC_RESULT that was returned by gpuccCompileProgramBytecode.
 * The caller is responsible for ensuing the _b is non-NULL.
 * @param _b A pointer to a GPUCC_PROGRAM_BYTECODE object.
 * @return A GPUCC_RESULT specifying the results of the program compilation, if any.
 */
#ifndef gpuccQueryBytecodeCompileResult_
#define gpuccQueryBytecodeCompileResult_(_b)                                   \
    (((GPUCC_PROGRAM_BYTECODE_BASE const*)(_b))->CompileResult)
#endif

/* @summary Define an inlined macro version of gpuccQueryBytecodeSizeBytes.
 * The caller is responsible for ensuring that _b is non-NULL.
 * @param _b A pointer to a GPUCC_PROGRAM_BYTECODE object.
 * @return The number of bytes in the bytecode buffer.
 */
#ifndef gpuccQueryBytecodeSizeBytes_
#define gpuccQueryBytecodeSizeBytes_(_b)                                       \
    (((GPUCC_PROGRAM_BYTECODE_BASE const*)(_b))->BytecodeSize)
#endif

/* @summary Define an inlined macro version of gpuccQueryBytecodeLogSizeBytes.
 * The caller is responsible for ensuring that _b is non-NULL.
 * @param _b A pointer to a GPUCC_PROGRAM_BYTECODE object.
 * @return The number of bytes in the compiler log buffer.
 */
#ifndef gpuccQueryBytecodeLogSizeBytes_
#define gpuccQueryBytecodeLogSizeBytes_(_b)                                    \
    (((GPUCC_PROGRAM_BYTECODE_BASE const*)(_b))->LogBufferSize)
#endif

/* @summary Define an inlined macro version of gpuccQueryBytecodeBuffer.
 * The caller is responsible for ensuring that _b is non-NULL.
 * @param _b A pointer to a GPUCC_PROGRAM_BYTECODE object.
 * @return A pointer to the start of the buffer containing the compiled bytecode.
 */
#ifndef gpuccQueryBytecodeBuffer_
#define gpuccQueryBytecodeBuffer_(_b)                                          \
    (((GPUCC_PROGRAM_BYTECODE_BASE*)(_b))->BytecodeBuffer)
#endif

/* @summary Define an inlined macro version of gpuccQueryBytecodeLogBuffer.
 * The caller is responsible for ensuring that _b is non-NULL.
 * @param _b A pointer to a GPUCC_PROGRAM_BYTECODE object.
 * @return A pointer to the start of the buffer containing the compiler log output.
 */
#ifndef gpuccQueryBytecodeLogBuffer_
#define gpuccQueryBytecodeLogBuffer_(_b)                                       \
    (((GPUCC_PROGRAM_BYTECODE_BASE*)(_b))->LogBuffer)
#endif

/* Forward-declare opaque platform types */
struct GPUCC_THREAD_CONTEXT;
struct GPUCC_PROCESS_CONTEXT;

/* @summary Define the signatures for the functions that must be provided by each compiler type.
 */
typedef struct GPUCC_PROGRAM_BYTECODE* (*PFN_CreateBytecode )(struct GPUCC_PROGRAM_COMPILER *);
typedef struct GPUCC_RESULT            (*PFN_CompileBytecode)(struct GPUCC_PROGRAM_BYTECODE *, char const*, uint64_t, char const*, char const*);
typedef void                           (*PFN_DeleteBytecode )(struct GPUCC_PROGRAM_BYTECODE *);

/* @summary All GPU program compiler implementations must start with an instance 
 * of GPUCC_PROGRAM_COMPILER_BASE, so that the underying type can be retrieved.
 */
typedef struct GPUCC_PROGRAM_COMPILER_BASE {
    PFN_CreateBytecode             CreateBytecode;                             /* Function used to create a new bytecode container object. */
    PFN_DeleteBytecode             DeleteBytecode;                             /* Function used to delete a bytecode container object. */
    PFN_CompileBytecode            CompileBytecode;                            /* Function used to compile GPU program source code into intermediate bytecode. */
    int32_t                        CompilerType;                               /* One of the values of the GPUCC_COMPILER_TYPE enumeration specifying the compiler type. */
    int32_t                        BytecodeType;                               /* One of the values of the GPUCC_BYTECODE_TYPE enumeration specifying the type of bytecode generated by the compiler. */
} GPUCC_PROGRAM_COMPILER_BASE;

/* @summary All GPU program bytecode implementations must start with an instance
 * of GPUCC_PROGRAM_BYTECODE_BASE, so that the underlying compiler and state can
 * be retrieved.
 */
typedef struct GPUCC_PROGRAM_BYTECODE_BASE {
    struct GPUCC_PROGRAM_COMPILER *Compiler;                                   /* The compiler used to create the bytecode container. */
    struct GPUCC_RESULT            CompileResult;                              /* A GPUCC_RESULT specifying the results of the compilation. */
    char                          *EntryPoint;                                 /* A nul-terminated UTF-8 string specifying the program entry point, or NULL if no compilation has been attempted yet. */
    char                          *SourcePath;                                 /* A nul-terminated UTF-8 string specifying the source file path, if any, or NULL if no compilation has been attempted yet. */
    char                          *LogBuffer;                                  /* A nul-terminated UTF-8 string containing any output from the compilation process. */
    uint64_t                       LogBufferSize;                              /* The number of bytes of data in the log buffer, including the nul. */
    uint64_t                       BytecodeSize;                               /* The buffer containing the compiled bytecode. */
    uint8_t                       *BytecodeBuffer;                             /* The number of bytes of compiled bytecode. */
} GPUCC_PROGRAM_BYTECODE_BASE;

/* @summary Define a simple structure for returning information about a string 
 * in some character encoding. The trailing nul is included in all counts.
 */
typedef struct GPUCC_STRING_INFO {
    size_t                         ByteCount;                                  /* The number of bytes, including the trailing nul. */
    size_t                         CharCount;                                  /* The number of characters, including the trailing nul. */
} GPUCC_STRING_INFO;

#ifdef __cplusplus
extern "C" {
#endif

/* @summary Retrieve the global per-process data for the platform.
 * @return A pointer to the platform-specific GPUCC_PROCESS_CONTEXT type.
 */
GPUCC_API(struct GPUCC_PROCESS_CONTEXT*)
gpuccGetProcessContext
(
    void
);

/* @summary Retrieve the global per-thread data for the calling thread.
 * @return A pointer to the platform-specific thread-local data.
 */
GPUCC_API(struct GPUCC_THREAD_CONTEXT *)
gpuccGetThreadContext
(
    void
);

/* @summary Construct a GPUCC_RESULT value specifying only the GpuCC result code.
 * The platform result code is set to zero, indicating that no platform error was observed.
 * @param library_result One of the values of the GPUCC_RESULT_CODE enumeration.
 * @return The GPUCC_RESULT structure.
 */
GPUCC_API(struct GPUCC_RESULT)
gpuccMakeResult
(
    int32_t library_result
);

/* @summary Set the GPUCC_RESULT value for the calling thread.
 * @param result The result of the most recent operation on the calling thread.
 * @return The prior result value from the calling thread.
 */
GPUCC_API(struct GPUCC_RESULT)
gpuccSetLastResult
(
    struct GPUCC_RESULT result
);

/* @summary Copy the program entry point and source path strings into a program bytecode container.
 * This function is called when a GPU program is being compiled.
 * @param bytecode The destination bytecode container.
 * @param entry_point A nul-terminated UTF-8 string specifying the program entry point function.
 * @param source_path A nul-terminated UTF-8 string specifying the path of the source file containing the program entry point.
 * @return A GPUCC_RESULT value.
 */
GPUCC_API(struct GPUCC_RESULT)
gpuccSetProgramEntryPoint
(
    struct GPUCC_PROGRAM_BYTECODE *bytecode, 
    char const                 *entry_point, 
    char const                 *source_path
);

/* @summary Determine whether or not a bytecode container has previously been used as the target of a GPU program compilation.
 * When a bytecode container has been used to store compilation results, it is considered to be "not empty" and cannot be reused.
 * @param bytecode The bytecode container to check.
 * @return Non-zero if the bytecode container has not been used, or zero if the bytecode container is already storing compilation results.
 */
GPUCC_API(int32_t)
gpuccBytecodeContainerIsEmpty
(
    struct GPUCC_PROGRAM_BYTECODE *bytecode
);

#ifdef __cplusplus
}; /* extern "C" */
#endif

#endif /* __GPUCC_INTERNAL_H__ */

#if   defined(__APPLE__)
#   if defined(TARGET_OS_IPHONE) || defined(TARGET_OS_IPHONE_SIMULATOR)
#       error No GpuCC implementation for iOS (yet).
#   else
#       error No GpuCC implementation for macOS (yet).
#   endif
#elif defined(_WIN32) || defined(_WIN64)
#   include "win32/gpucc_internal_win32.h"
#elif defined(__linux__) || defined(__gnu_linux__)
#   error No GpuCC implementation for Linux (yet).
#else
#   error No GpuCC implementation for your platform (yet).
#endif

