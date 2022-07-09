#pragma once
#include <windows.h>
#include <Psapi.h>

inline MODULEINFO getModuleInfo(const LPCTSTR lpModuleName)
{
    static MODULEINFO  miInfos{ };
    auto* const hmModule = GetModuleHandle(lpModuleName);

    if (hmModule)
    {
        GetModuleInformation(GetCurrentProcess(), hmModule, &miInfos, sizeof(MODULEINFO));
    }

    return miInfos;
}

inline bool CompareByteArray(BYTE* data, BYTE* sig, SIZE_T size)
{
    for (SIZE_T i = 0; i < size; i++) {
        if (data[i] != sig[i]) {
            if (sig[i] == 0x00) continue;
            return false;
        }
    }
    return true;
}

inline BYTE* FindSignature(BYTE* start, BYTE* end, BYTE* sig, SIZE_T size)
{
    for (BYTE* it = start; it < end - size; it++) {
        if (CompareByteArray(it, sig, size)) {
            return it;
        };
    }
    return 0;
}

inline void* FindPointer(BYTE* sig, SIZE_T size, int addition = 0)
{
    auto base = static_cast<BYTE*>(getModuleInfo(NULL).lpBaseOfDll);
    auto address = FindSignature(base, base + getModuleInfo(NULL).SizeOfImage - 1, sig, size);
    if (!address) return nullptr;
    auto k = 0;
    for (; sig[k]; k++);
    auto offset = *reinterpret_cast<UINT32*>(address + k);
    return address + k + 4 + offset + addition;
}

template <typename T>
inline bool FindWorld(T& storeVar)
{
    static BYTE sig[] = { 0x48, 0x8B, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x88, 0x00, 0x00, 0x00, 0x00, 0x48, 0x85, 0xC9, 0x74, 0x06, 0x48, 0x8B, 0x49, 0x70 };
    storeVar = reinterpret_cast<T>(FindPointer(sig, sizeof(sig)));
    return storeVar;
}

