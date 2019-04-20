#include <Windows.h>

BOOL WINAPI
DllMain
(
    HINSTANCE instance, 
    DWORD       reason, 
    LPVOID    reserved
)
{
    UNREFERENCED_PARAMETER(instance);
    UNREFERENCED_PARAMETER(reason);
    UNREFERENCED_PARAMETER(reserved);
    return TRUE;
}

