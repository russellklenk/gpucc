#ifndef __GPUCC_COMPILER_FXC_WIN32_H__
#define __GPUCC_COMPILER_FXC_WIN32_H__

#pragma once

#ifndef GPUCC_NO_INCLUDES
#   ifndef __GPUCC_FXCCOMPILERAPI_WIN32_H__
#       include "win32/fxccompilerapi_win32.h"
#   endif
#endif

#ifndef gpuccCompilerFxc_
#define gpuccCompilerFxc_(_c)                                                  \
    ((GPUCC_COMPILER_FXC_WIN32*)(_c))
#endif

#ifndef gpuccBytecodeFxc_
#define gpuccBytecodeFxc_(_b)                                                  \
    ((GPUCC_BYTECODE_FXC_WIN32*)(_b))
#endif

/* @summary Define the data maintained by an instance of the fxc (legacy Direct3D) compiler.
 */
typedef struct GPUCC_COMPILER_FXC_WIN32 {
    GPUCC_PROGRAM_COMPILER_BASE   CommonFields;                                /* This must be the first field of any compiler type. */
    FXCCOMPILERAPI_DISPATCH      *DispatchTable;                               /* A pointer to the d3dcompiler dispatch table maintained by the process context. */
    D3D_SHADER_MACRO             *DefineArray;                                 /* An array of D3D_SHADER_MACRO specifying the symbols and values defined for the compiler. */
    uint32_t                      DefineCount;                                 /* The number of valid elements in the D3D_SHADER_MACRO array. */
    int32_t                       TargetRuntime;                               /* One of the values of the GPUCC_TARGET_RUNTIME enumeration specifying the target runtime for shaders built by the compiler. */
    char                         *ShaderModel;                                 /* A nul-terminated string specifying the Direct3D shader model. */
} GPUCC_COMPILER_FXC_WIN32;

/* @summary Define the data maintained by a single DXBC program bytecode container generated by the fxc (legacy Direct3D) compiler.
 */
typedef struct GPUCC_BYTECODE_FXC_WIN32 {
    GPUCC_PROGRAM_BYTECODE_BASE   CommonFields;                                /* This must be the first field of any bytecode container type. */
    ID3DBlob                     *CodeBuffer;                                  /* The buffer for storing the compiled bytecode. This is NULL unless the compilation succeeds. */
    ID3DBlob                     *ErrorLog;                                    /* The buffer for storing the output produced by the compiler. This is NULL if no compilation has been attempted. */
} GPUCC_BYTECODE_FXC_WIN32;

#ifdef __cplusplus
extern "C" {
#endif

/* @summary Allocate and initialize a bytecode container for DXBC bytecode generated by the fxc (legacy Direct3D) compiler.
 * @param compiler A pointer to an instance of GPUCC_COMPILER_FXC_WIN32.
 * @return A pointer to the bytecode container, of type GPUCC_BYTECODE_FXC_WIN32, or NULL if the allocation failed.
 */
GPUCC_API(struct GPUCC_PROGRAM_BYTECODE*)
gpuccCreateProgramBytecodeFxc
(
    struct GPUCC_PROGRAM_COMPILER *compiler
);

/* @summary Release all resources associated with a fxc program bytecode container.
 * @param bytecode A pointer to an instance of GPUCC_BYTECODE_FXC_WIN32.
 */
GPUCC_API(void)
gpuccDeleteProgramBytecodeFxc
(
    struct GPUCC_PROGRAM_BYTECODE *bytecode
);

/* @summary Compile GPU program source code into intermediate bytecode.
 * The caller is responsible for processing any source code includes and supplying the full resulting source code in the source_code buffer.
 * The function blocks the calling thread until compilation has completed.
 * @param container The container that will be used to store the program bytecode, of type GPUCC_BYTECODE_FXC_WIN32.
 * @param source_code Pointer to a buffer containing UTF-8 encoded GPU program source code.
 * @param source_size The number of bytes of program source code in the spurce_code buffer.
 * @oaram source_path A nul-terminated UTF-8 string specifying the path to the source file, for use in log output. This value may be NULL.
 * @param entry_point A nul-terminated string specifying the program entry point.
 * @return The result of the compilation. Use the gpuccSuccess and gpuccFailure macros to determine whether compilation was successful.
 */
GPUCC_API(struct GPUCC_RESULT)
gpuccCompileBytecodeFxc
(
    struct GPUCC_PROGRAM_BYTECODE *container, 
    char const                  *source_code, 
    uint64_t                     source_size, 
    char const                  *source_path, 
    char const                  *entry_point
);

/* @summary Allocate and initialize a new compiler record for accessing the fxc (legacy Direct3D) compiler.
 * @param config Data used to configure the compiler instance.
 * @return A pointer to the compiler, or NULL if an error occurred.
 */
GPUCC_API(struct GPUCC_PROGRAM_COMPILER*)
gpuccCreateCompilerFxc
(
    struct GPUCC_PROGRAM_COMPILER_INIT *config
);

#ifdef __cplusplus
}; /* extern "C" */
#endif

#endif
