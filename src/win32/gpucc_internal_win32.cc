/**
 * gpucc_win32.cc: Implement the Windows-specific portions of the internal 
 * GpuCC library interface, along with the startup and shutdown routines.
 */
#include "gpucc.h"
#include "gpucc_internal.h"

GPUCC_API(struct GPUCC_RESULT)
gpuccMakeResult
(
    int32_t library_result
)
{
    return GPUCC_RESULT { library_result, ERROR_SUCCESS };
}

GPUCC_API(struct GPUCC_RESULT)
gpuccMakeResult_Win32
(
    int32_t library_result, 
    DWORD  platform_result
)
{
    return GPUCC_RESULT { library_result, (int32_t) platform_result };
}

GPUCC_API(struct GPUCC_RESULT)
gpuccMakeResult_HRESULT
(
    HRESULT platform_result
)
{
    GPUCC_RESULT r;
    if (FAILED(platform_result)) {
        r.LibraryResult  = GPUCC_RESULT_CODE_PLATFORM_ERROR;
        r.PlatformResult =(int32_t) platform_result;
    } else {
        r.LibraryResult  = GPUCC_RESULT_CODE_SUCCESS;
        r.PlatformResult =(int32_t) platform_result;
    } return r;
}

GPUCC_API(struct GPUCC_RESULT)
gpuccGetLastResult
(
    void
)
{
    GPUCC_THREAD_CONTEXT_WIN32 *tctx =(GPUCC_THREAD_CONTEXT_WIN32*) gpuccGetThreadContext();
    return tctx->LastResult;
}

GPUCC_API(struct GPUCC_RESULT)
gpuccSetLastResult
(
    struct GPUCC_RESULT result
)
{
    GPUCC_THREAD_CONTEXT_WIN32 *tctx =(GPUCC_THREAD_CONTEXT_WIN32*) gpuccGetThreadContext();
    GPUCC_RESULT                prev = tctx->LastResult;
    tctx->LastResult = result;
    return prev;
}

GPUCC_API(struct GPUCC_RESULT)
gpuccStartup
(
    uint32_t gpucc_usage_mode
)
{
    GPUCC_PROCESS_CONTEXT_WIN32 *pctx =(GPUCC_PROCESS_CONTEXT_WIN32*) gpuccGetProcessContext();
    GPUCC_RESULT               result = gpuccMakeResult(GPUCC_RESULT_CODE_SUCCESS);

    if (gpucc_usage_mode != GPUCC_USAGE_MODE_OFFLINE && 
        gpucc_usage_mode != GPUCC_USAGE_MODE_RUNTIME) {
        return gpuccMakeResult(GPUCC_RESULT_CODE_INVALID_USAGE_MODE);
    }
    if (pctx->StartupFlag != FALSE) {
        return gpuccMakeResult(GPUCC_RESULT_CODE_ALREADY_INITIALIZED);
    }

    /* TODO: Populate dispatch tables for any available compilers. */
    pctx->StartupFlag = TRUE;
    return result;
}

GPUCC_API(void)
gpuccShutdown
(
    void
)
{
    GPUCC_PROCESS_CONTEXT_WIN32 *pctx =(GPUCC_PROCESS_CONTEXT_WIN32*) gpuccGetProcessContext();

    /* TODO: Invalidate the dispatch tables for any available compilers. */
    pctx->StartupFlag = FALSE;
}

