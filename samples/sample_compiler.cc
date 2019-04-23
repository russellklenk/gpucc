/**
 * @summary sample_compiler.cc: Shows how to load GpuCC into the process at 
 * runtime and use it for offline builds.
 */

// In this file only, define GPUCC_LOADER_IMPLEMENTATION and GPUCC_LOCAL_RUNTIME_IMPLEMENTATION.
// This synthesizes the GpuCC runtime loader and function definitions into this translation unit.
// There is no need to link with anything.
// You can safely call any public API function. If it is not present, or GpuCC could not be loaded, 
// it will fall back to a stub function.
#include <stdio.h>

#define   GPUCC_LOADER_IMPLEMENTATION
#define   GPUCC_LOCAL_RUNTIME_IMPLEMENTATION
#include "gpucc.h"

int main
(
    int    argc, 
    char **argv
)
{
    GPUCC_RESULT r;
    int32_t mj, mi, pa;

    // Before calling any gpucc functions, initialize the runtime.
    if (gpuccSuccess((r = gpuccLocalRuntimeStartup(GPUCC_USAGE_MODE_OFFLINE)))) {
        gpuccVersion(&mj, &mi, &pa);
        printf("Hello from gpucc %d.%d.%d!\n", mj, mi, pa);
        GPUCC_PROGRAM_COMPILER_INIT config;
        char const                *symbols[3] = {"Symbol1", "Symbol2", "Symbol3"};
        char const                 *values[3] = {"A", "B", "C"};
        config.DefineSymbols = symbols;
        config.DefineValues  = values;
        config.DefineCount   = 3;
        config.BytecodeType  = GPUCC_BYTECODE_TYPE_DXBC;
        config.TargetRuntime = GPUCC_TARGET_RUNTIME_DIRECT3D12;
        config.TargetProfile = "cs_5_0";
        struct GPUCC_PROGRAM_COMPILER *c = gpuccCreateCompiler(&config);
        GPUCC_COMPILER_TYPE ct = (GPUCC_COMPILER_TYPE) gpuccQueryCompilerType(c);
        GPUCC_BYTECODE_TYPE bt = (GPUCC_BYTECODE_TYPE) gpuccQueryBytecodeType(c);
        GPUCC_LOADER_UNUSED(ct);
        GPUCC_LOADER_UNUSED(bt);
        gpuccDeleteCompiler(c);
    }
    (void) argc;
    (void) argv;
    gpuccLocalRuntimeShutdown();
    return 0;
}

