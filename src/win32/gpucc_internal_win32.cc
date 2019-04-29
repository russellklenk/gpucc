/**
 * gpucc_internal_win32.cc: Implement the platform-specific portions of the 
 * internal library interface from gpucc_internal.h and gpucc_internal_win32.h.
 */
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <wchar.h>
#include "gpucc.h"
#include "gpucc_internal.h"

GPUCC_API(void)
gpuccDebugPrintf
(
    WCHAR const *format, 
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

GPUCC_API(char*)
gpuccPutStringUtf8
(
    uint8_t   *&dst, 
    char const *src
)
{
    char *p  =(char*) dst;
    if (src != nullptr) {
        size_t nb = strlen(src);
        memcpy(dst, src, nb); 
        dst += nb; /* string data */
    }
   *dst  = 0;  /* put nul */
    dst += 1;  /* nul */
    return p;
}

GPUCC_API(WCHAR*)
gpuccPutStringUtf16
(
    uint8_t    *&dst, 
    WCHAR const *src
)
{
    WCHAR *p =(WCHAR*) dst;
    if (src != nullptr) {
        size_t nb = wcslen(src) * sizeof(WCHAR);
        memcpy(dst, src, nb);
        dst += nb; /* string data */
    }
   *dst++ = 0; /* nul byte 0 */
   *dst++ = 0; /* nul byte 1 */
    return p;
}

GPUCC_API(int32_t)
gpuccExtractDirect3DShaderModel
(
    char const *target, 
    char    o_stage[3], 
    int    *o_sm_major, 
    int    *o_sm_minor
)
{
    unsigned int level_major = 0;
    unsigned int level_minor = 0;
    char                 st0 = 0;
    char                 st1 = 0;
    int              nbmatch = 0;

    if ((nbmatch = sscanf_s(target, "%c%c_%u_%u", &st0, 1, &st1, 1, &level_major, &level_minor)) == 4) {
        o_stage[0] = st0;
        o_stage[1] = st1;
        o_stage[2] = 0;
       *o_sm_major =(int) level_major;
       *o_sm_minor =(int) level_minor;
        return  0;
    } else { /* The target profile string doesn't match the expected format. */
        o_stage[0] = 0;
        o_stage[1] = 0;
        o_stage[2] = 0;
       *o_sm_major = 0;
       *o_sm_minor = 0;
        return -1;
    }
}

GPUCC_API(struct GPUCC_RESULT)
gpuccMakeResult
(
    int32_t library_result
)
{
    return GPUCC_RESULT { library_result, ERROR_SUCCESS };
}

GPUCC_API(struct GPUCC_RESULT)
gpuccMakeResult_errno
(
    int32_t library_result
)
{
    return GPUCC_RESULT { library_result, (int32_t) errno };
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
gpuccSetLastResult
(
    struct GPUCC_RESULT result
)
{
    GPUCC_THREAD_CONTEXT_WIN32 *tctx = gpuccGetThreadContext_();
    GPUCC_RESULT                prev = tctx->LastResult;
    tctx->LastResult = result;
    return prev;
}

GPUCC_API(struct GPUCC_RESULT)
gpuccSetProgramEntryPoint
(
    struct GPUCC_PROGRAM_BYTECODE *bytecode, 
    char const                 *entry_point, 
    char const                 *source_path
)
{
    GPUCC_PROGRAM_BYTECODE_BASE *bytecode_ =(GPUCC_PROGRAM_BYTECODE_BASE*) bytecode;
    GPUCC_RESULT                    result = gpuccMakeResult(GPUCC_RESULT_CODE_SUCCESS);
    uint8_t                        *buffer = nullptr;
    uint8_t                           *ptr = nullptr;
    size_t                          nbneed = 0;
    size_t                          nbpath = 0;
    size_t                         nbentry = 0;

    /* Determine the amount of memory required to store copies of the strings. */
    nbentry = (entry_point != nullptr) ? (strlen(entry_point) + 1) : 1;
    nbpath  = (source_path != nullptr) ? (strlen(source_path) + 1) : 1;
    nbneed  = nbentry + nbpath;

    /* Allocate memory for all string data in one buffer. */
    if ((buffer = (uint8_t*) malloc(nbneed)) == nullptr) {
        result  = gpuccMakeResult_errno(GPUCC_RESULT_CODE_OUT_OF_HOST_MEMORY);
        gpuccDebugPrintf(L"GpuCC: Failed to allocate %Iu bytes for storing program entry point.\n", nbneed);
        gpuccSetLastResult(result);
        return result;
    } ptr = buffer;

    /* Write the string data to the buffer. */
    bytecode_->EntryPoint = gpuccPutStringUtf8(ptr, entry_point);
    bytecode_->SourcePath = gpuccPutStringUtf8(ptr, source_path);
    return result;
}

GPUCC_API(int32_t)
gpuccBytecodeContainerIsEmpty
(
    struct GPUCC_PROGRAM_BYTECODE *bytecode
)
{
    if (bytecode != nullptr) {
        GPUCC_RESULT r = gpuccQueryBytecodeCompileResult_(bytecode);
        return r.LibraryResult == GPUCC_RESULT_CODE_EMPTY_BYTECODE_CONTAINER;
    } else {
        assert(0 && "Invalid bytecode container");
        return 0;
    }
}

