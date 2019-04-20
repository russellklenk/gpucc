/**
 * @summary gpucc.h: Define common constants and macros used throughout the 
 * library.
 */
#ifndef __GPUCC_H__
#define __GPUCC_H__

#pragma once

#ifndef GPUCC_NO_INCLUDES
#   include <stddef.h>
#   include <stdint.h>
#endif

/* @summary A macro used to specify a "public" API function available for use 
 * within other modules (but not necessarily exported from the library).
 * @param _return_type The return type of the function, such as int or void.
 */
#ifndef GPUCC_API
#   define GPUCC_API(_return_type)                          extern _return_type
#endif

/* @summary Define the various types of potentially supported bytecode.
 * A bytecode type is used to select a compiler that produces the given bytecode type.
 */
typedef enum GPUCC_BYTECODE_TYPE {
    GPUCC_BYTECODE_TYPE_UNKNOWN  =   0,                                        /* The bytecode type is not known or not specified. */
    GPUCC_BYTECODE_TYPE_DXIL     =   1,                                        /* The DirectX bytecode type for use with Direct3D. */
    GPUCC_BYTECODE_TYPE_SPIRV    =   2,                                        /* The SPIR-V bytecode type for use with Vulkan and OpenGL 4.6+. */
    GPUCC_BYTECODE_TYPE_PTX      =   3,                                        /* The PTX bytecode type for use with NVIDIA CUDA. */
} GPUCC_BYTECODE_TYPE;

/* @summary Define the set of error codes that can be produced by the GPUCC library.
 */
typedef enum GPUCC_ERROR_CODE {
    GPUCC_ERROR_CODE_NO_ERROR    =   0,                                        /* No error was encountered by the GPUCC library. */
} GPUCC_ERROR_CODE;

/* @summary A structure for returning an error result from a GPUCC API call.
 * Use the gpuccFailed and gpuccSucceeded functions to determine whether the structure represents a failed call.
 */
typedef struct GPUCC_ERROR {
    int32_t LibraryError;                                                      /* One of the values of the GPUCC_ERROR_CODE enumeration. */
    int32_t PlatformError;                                                     /* The error code returned by the underlying platform (errno, GetLastError, etc.) cast to a 32-bit signed integer. */
} GPUCC_ERROR;

#endif /* __GPUCC_H__ */

