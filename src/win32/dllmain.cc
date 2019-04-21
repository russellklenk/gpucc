/**
 * @summary dllmain.cc: Implement the entry point for the DLL and perform 
 * per-process and per-thread setup and teardown. What can be done from DllMain
 * is very limited - most of the actual work is deferred to gpuccStartup and 
 * gpuccShutdown, which must be called by the application outside of DllMain.
 */
#include <assert.h>
#include <stdarg.h>
#include <wchar.h>
#include "gpucc.h"
#include "gpucc_internal.h"

static GPUCC_PROCESS_CONTEXT_WIN32  g_ProcessContextData = {
    TLS_OUT_OF_INDEXES, /* TlsSlot_ThreadContext */
    FALSE             , /* InitializationFlag */
    FALSE               /* StartupFlag */
};

static GPUCC_PROCESS_CONTEXT_WIN32 *g_ProcessContext     = &g_ProcessContextData;

/* @summary Helper function to emit printf-style output to OutputDebugString.
 * The output can be viewed in the VS debugger output window or using DbgView.
 * GpuCC pipes debug output from startup and loading functions to this endpoint.
 * @param format A nul-terminated wide character string following printf formatting conventions.
 * @param ... Substitution arguments for the format string.
 */
static void
DebugPrintfW
(
    LPCWSTR format, 
    ...
)
{
    WCHAR buffer[2048];
    va_list       args;
    size_t      mchars = sizeof(buffer) / sizeof(WCHAR);
    int         nchars;

    va_start(args, format);
    nchars = vswprintf_s(buffer, format, args);
    va_end(args);
    if (nchars < 0) {
        assert(0 && "Error formatting debug output");
        OutputDebugStringW(L"GpuCC: Error formatting debug output.\n");
        return;
    }
    UNREFERENCED_PARAMETER(mchars);
    assert(nchars  <  (int)mchars && "Increase DebugPrintfW buffer size");
    OutputDebugStringW(buffer);
}

/* @summary Perform initialization required when a process loads the DLL into its address space.
 * This function is called from DllMain, so the scope of what it can do is very limited. See MSDN:
 * https://docs.microsoft.com/en-us/windows/desktop/Dlls/dynamic-link-library-best-practices
 * @return TRUE if initialization completed successfully, or FALSE if an error occurred and the DLL cannot load into the process.
 */
static BOOL
DllMain_InitializeProcessContext
(
    void
)
{
    GPUCC_PROCESS_CONTEXT_WIN32 *pctx = g_ProcessContext;
    DWORD                   tctx_slot = TLS_OUT_OF_INDEXES;

    assert(pctx->InitializationFlag == FALSE);
    if ((tctx_slot = TlsAlloc()) == TLS_OUT_OF_INDEXES) {
        OutputDebugStringW(L"GpuCC: Failed to allocate TLS slots. Loading cannot proceed.\n");
        return FALSE;
    }
    pctx->TlsSlot_ThreadContext = tctx_slot;
    pctx->InitializationFlag    = TRUE;
    pctx->StartupFlag           = FALSE;
    return TRUE;
}

/* @summary Perform cleanup required when a process unloads the DLL from its address space.
 * This function is called from DllMain, so the scope of what it can do is very limited. See MSDN:
 * https://docs.microsoft.com/en-us/windows/desktop/Dlls/dynamic-link-library-best-practices
 * @return This function always returns TRUE.
 */
static BOOL
DllMain_ReleaseProcessContext
(
    void
)
{
    GPUCC_PROCESS_CONTEXT_WIN32 *pctx = g_ProcessContext;
    DWORD                   tctx_slot = pctx->TlsSlot_ThreadContext;

    assert(pctx->InitializationFlag  != FALSE);
    if (tctx_slot != TLS_OUT_OF_INDEXES) {
        pctx->TlsSlot_ThreadContext   = TLS_OUT_OF_INDEXES;
        TlsFree(tctx_slot);
    }
    pctx->InitializationFlag = FALSE;
    pctx->StartupFlag        = FALSE;
    return TRUE;
}

/* @summary Create the per-thread context object when a thread attaches to the DLL.
 * This function is called from DllMain, so the scope of what it can do is very limited. See MSDN:
 * https://docs.microsoft.com/en-us/windows/desktop/Dlls/dynamic-link-library-best-practices
 * @return TRUE if initialization completed successfully, or FALSE if an error occurred and the DLL cannot load into the process.
 */
static BOOL
DllMain_CreateThreadContext
(
    void
)
{
    GPUCC_PROCESS_CONTEXT_WIN32 *pctx = g_ProcessContext;
    GPUCC_THREAD_CONTEXT_WIN32  *tctx = nullptr;
    DWORD                   tctx_slot = pctx->TlsSlot_ThreadContext;

    assert(pctx->InitializationFlag != FALSE);
    assert(pctx->TlsSlot_ThreadContext != TLS_OUT_OF_INDEXES);
    if ((tctx = (GPUCC_THREAD_CONTEXT_WIN32*) malloc(sizeof(GPUCC_THREAD_CONTEXT_WIN32))) == nullptr) {
        DebugPrintfW(L"GpuCC: Allocating thread context for TID %u failed with errno = %d.\n", GetCurrentThreadId(), errno);
        goto cleanup_and_fail;
    }
    memset(tctx, 0, sizeof(GPUCC_THREAD_CONTEXT_WIN32));
    if (TlsSetValue(tctx_slot, tctx) == FALSE) {
        DebugPrintfW(L"GpuCC: Setting thread context for TID %u failed with Win32 error %08X.\n", GetCurrentThreadId(), GetLastError());
        goto cleanup_and_fail;
    }
    /* TODO: Additional initialization */
    return TRUE;

cleanup_and_fail:
    free(tctx);
    return FALSE;
}

/* @summary Free resources associated with per-thread context.
 * This function is called from DllMain, so the scope of what it can do is very limited. See MSDN:
 * https://docs.microsoft.com/en-us/windows/desktop/Dlls/dynamic-link-library-best-practices
 * @return This function always returns TRUE.
 */
static BOOL
DllMain_DeleteThreadContext
(
    void
)
{
    GPUCC_PROCESS_CONTEXT_WIN32 *pctx = g_ProcessContext;
    GPUCC_THREAD_CONTEXT_WIN32  *tctx = nullptr;
    DWORD                   tctx_slot = pctx->TlsSlot_ThreadContext;

    assert(pctx->InitializationFlag != FALSE);
    if ((tctx = (GPUCC_THREAD_CONTEXT_WIN32*) TlsGetValue(tctx_slot)) != nullptr) {
        /* TODO: Clean up any resources referenced by the thread context */
        TlsSetValue(tctx_slot, nullptr);
        free(tctx);
    }
    return TRUE;
}

GPUCC_API(struct GPUCC_PROCESS_CONTEXT*)
gpuccGetProcessContext
(
    void
)
{
    return (struct GPUCC_PROCESS_CONTEXT*) g_ProcessContext;
}

GPUCC_API(struct GPUCC_THREAD_CONTEXT *)
gpuccGetThreadContext
(
    void
)
{
    GPUCC_PROCESS_CONTEXT_WIN32 *pctx = g_ProcessContext;
    GPUCC_THREAD_CONTEXT_WIN32  *tctx =(GPUCC_THREAD_CONTEXT_WIN32*) TlsGetValue(pctx->TlsSlot_ThreadContext);
    if (tctx != nullptr) {
        return (struct GPUCC_THREAD_CONTEXT*) tctx;
    } else {
        /* When the DLL is loaded using LoadLibrary, threads that 
         * existed prior to the LoadLibrary call do not DLL_THREAD_ATTACH.
         * This is one of those instances, so initialize a context for them. */
        if (DllMain_CreateThreadContext() != FALSE) {
            return (struct GPUCC_THREAD_CONTEXT*) TlsGetValue(pctx->TlsSlot_ThreadContext);
        } else {
            /* Debug output was generated by DllMain_CreateThreadContext */
            /* TODO: should we crash the process in this case? */
            abort();
        }
    }
}

BOOL WINAPI
DllMain
(
    HINSTANCE instance, 
    DWORD       reason, 
    LPVOID    reserved
)
{
    switch (reason) {
        case DLL_PROCESS_ATTACH: return DllMain_InitializeProcessContext();
        case DLL_PROCESS_DETACH: return DllMain_ReleaseProcessContext();
        case DLL_THREAD_ATTACH : return DllMain_CreateThreadContext();
        case DLL_THREAD_DETACH : return DllMain_DeleteThreadContext();
        default: {
          DebugPrintfW(L"GpuCC: DllMain called with unknown reason %d.\n", reason);
        } return TRUE;
    }
    UNREFERENCED_PARAMETER(instance);
    UNREFERENCED_PARAMETER(reserved);
}

