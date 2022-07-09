#include <windows.h>
#include <TlHelp32.h>
#include <filesystem>
#include <iostream>
#include <AccCtrl.h>
#include <AclAPI.h>
#include <sddl.h>
#include <atlbase.h>
#include <appmodel.h>

namespace fs = std::filesystem;

DWORD GetProcessId(const char* name) {
    DWORD procID = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 procEntry;
        procEntry.dwSize = sizeof(procEntry);

        if (Process32First(hSnap, &procEntry)) {
            do {
                if (!_stricmp(procEntry.szExeFile, name)) {
                    procID = procEntry.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnap, &procEntry));
        }
    }
    CloseHandle(hSnap);
    return procID;
}

bool GetFileExtFromDir(const fs::path& dir, const char* ext, fs::path& file) {
    for (const auto& entry : fs::directory_iterator(dir)) {
        if (!entry.is_regular_file())
            continue;
        const fs::path curFile = entry.path();
        if (curFile.extension() == ext) {
            file = curFile;
            return true;
        }
    }
    return false;
}

void DoSteamInjection(std::string dllOutPath) {
    DWORD pid = 0;
    LPVOID RemoteString, LoadLibAddy;

    while (!pid) {
        pid = GetProcessId("Phasmophobia.exe");
        Sleep(30);
    }

    if (!pid) {
        std::cout << "Couldnt find PID" << std::endl;
        std::cin.get();
    }

    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);

    LoadLibAddy = (LPVOID)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");

    RemoteString = (LPVOID)VirtualAllocEx(hProc, NULL, strlen(dllOutPath.c_str()), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    WriteProcessMemory(hProc, (LPVOID)RemoteString, dllOutPath.c_str(), strlen(dllOutPath.c_str()), NULL);

    void* remoteThread = CreateRemoteThread(hProc, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibAddy, (LPVOID)RemoteString, NULL, NULL);

    if (remoteThread) {
        WaitForSingleObject(remoteThread, INFINITE);
        CloseHandle(remoteThread);
    }

    if (hProc)
        CloseHandle(hProc);
}

std::wstring GetRunningDirectory()
{
    wchar_t RunPath[MAX_PATH];
    GetModuleFileNameW(GetModuleHandleW(nullptr), RunPath, MAX_PATH);
    PathRemoveFileSpecW(RunPath);
    return std::wstring(RunPath);
}

void SetAccessControl(const std::wstring& ExecutableName, const wchar_t* AccessString)
{
    PSECURITY_DESCRIPTOR SecurityDescriptor = nullptr;
    EXPLICIT_ACCESSW ExplicitAccess = { 0 };
    PACL AccessControlCurrent = nullptr;
    PACL AccessControlNew = nullptr;

    SECURITY_INFORMATION SecurityInfo = DACL_SECURITY_INFORMATION;
    PSID SecurityIdentifier = nullptr;

    if (GetNamedSecurityInfoW(ExecutableName.c_str(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, nullptr, nullptr, &AccessControlCurrent, nullptr, &SecurityDescriptor) == ERROR_SUCCESS)
    {
        ConvertStringSidToSidW(AccessString, &SecurityIdentifier);
        if (SecurityIdentifier != nullptr)
        {
            ExplicitAccess.grfAccessPermissions = GENERIC_READ | GENERIC_EXECUTE;
            ExplicitAccess.grfAccessMode = SET_ACCESS;
            ExplicitAccess.grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
            ExplicitAccess.Trustee.TrusteeForm = TRUSTEE_IS_SID;
            ExplicitAccess.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
            ExplicitAccess.Trustee.ptstrName = reinterpret_cast<wchar_t*>(SecurityIdentifier);

            if (SetEntriesInAclW(1, &ExplicitAccess, AccessControlCurrent, &AccessControlNew) == ERROR_SUCCESS)
            {
                SetNamedSecurityInfoW(const_cast<wchar_t*>(ExecutableName.c_str()), SE_FILE_OBJECT, SecurityInfo, nullptr, nullptr, AccessControlNew, nullptr);
            }
        }
    }
    if (SecurityDescriptor)
    {
        LocalFree(reinterpret_cast<HLOCAL>(SecurityDescriptor));
    }
    if (AccessControlNew)
    {
        LocalFree(reinterpret_cast<HLOCAL>(AccessControlNew));
    }
}

bool InjectRemote(uint32_t ProcessID, const std::wstring& DLLPath) {
    const std::size_t DLLPathSize = ((DLLPath.size() + 1) * sizeof(wchar_t));
    std::uint32_t Result;

    if (!ProcessID) {
        std::cout << "Invalid Proc id" << std::endl;
        return false;
    }

    if (GetFileAttributesW(DLLPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
        std::cout << "DLL FILE does not exist" << std::endl;
        return false;
    }

    SetAccessControl(DLLPath, L"S-1-15-2-1");

    void* ProcLoadLibrary = reinterpret_cast<void*>(GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW"));

    if (!ProcLoadLibrary) {
        std::cout << "Couldnt get Load Library proc" << std::endl;
        return false;
    }

    void* Process = OpenProcess(PROCESS_ALL_ACCESS, false, ProcessID);
    if (Process == nullptr) {
        std::cout << "Cant open process" << std::endl;
        return false;
    }

    void* VirtualAlloc = reinterpret_cast<void*>(VirtualAllocEx(Process, nullptr, DLLPathSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));

    if (VirtualAlloc == nullptr) {
        std::cout << "Unable to remotely allocate memory" << std::endl;
        CloseHandle(Process);
        return false;
    }

    SIZE_T BytesWritten = 0;
    Result = WriteProcessMemory(Process, VirtualAlloc, DLLPath.data(), DLLPathSize, &BytesWritten);

    if (Result == 0) {
        std::cout << "Cant write process memory" << std::endl;
        CloseHandle(Process);
        return false;
    }

    if (BytesWritten != DLLPathSize) {
        std::cout << "Failed to write remote DLL path name" << std::endl;
        CloseHandle(Process);
        return false;
    }

    void* RemoteThread = CreateRemoteThread(Process, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(ProcLoadLibrary), VirtualAlloc, 0, nullptr);


    if (RemoteThread) {
        WaitForSingleObject(RemoteThread, INFINITE);
        CloseHandle(RemoteThread);
    }
    else {
        std::cout << "Unable to create remote thread" << std::endl;
        return false;
    }
    VirtualFreeEx(Process, VirtualAlloc, 0, MEM_RELEASE);
    CloseHandle(Process);
    return true;
}

int wmain(int argc, wchar_t* argv[]) {
    const fs::path dir = fs::path(argv[0]).remove_filename();
    fs::path dll;

    if (!GetFileExtFromDir(dir, ".dll", dll)) {
        std::cout << "Couldnt find DLl" << std::endl;
        std::cin.get();
    }

    std::cout << dll.string() << std::endl;

    const std::string dllPath = dll.string();
    std::string dllOutPath = "";

    const auto dllPathChar = dllPath.c_str();

    for (auto& ch : dllPath) {
        if (ch == '\\') {
            dllOutPath += '\\';
        }
        dllOutPath += ch;
    }

    DoSteamInjection(dllOutPath);
    
    return 0;
}