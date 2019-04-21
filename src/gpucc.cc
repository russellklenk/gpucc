/**
 * @summary Implements the platform-independent portion of the public GpuCC API.
 */
#include "gpucc.h"

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
        case GPUCC_RESULT_CODE_SUCCESS            : return "GPUCC_RESULT_CODE_SUCCESS";
        case GPUCC_RESULT_CODE_NOT_INITIALIZED    : return "GPUCC_RESULT_CODE_NOT_INITIALIZED";
        case GPUCC_RESULT_CODE_ALREADY_INITIALIZED: return "GPUCC_RESULT_CODE_ALREADY_INITIALIZED";
        case GPUCC_RESULT_CODE_PLATFORM_ERROR     : return "GPUCC_RESULT_CODE_PLATFORM_ERROR";
        default                                   : return "GPUCC_RESULT_CODE (unknown)";
    }
}

