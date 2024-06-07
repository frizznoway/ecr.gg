#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <commctrl.h>

HWND g_hWndTab;
HWND g_hWndConsole;
HWND g_hWndInjectButton;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD GetProcessID(const wchar_t* processName);
bool InjectMessageBox(DWORD processID); 
void AppendTextToConsole(const char* text);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    HWND hWnd = CreateWindowEx(0, L"STATIC", L"Injector", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 300, nullptr, nullptr, hInstance, nullptr);
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_TAB_CLASSES;
    InitCommonControlsEx(&icex);

    g_hWndTab = CreateWindow(WC_TABCONTROL, L"", WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
        0, 0, 400, 250, hWnd, nullptr, hInstance, nullptr);
    TCITEM tie;
    tie.mask = TCIF_TEXT;
    tie.pszText = const_cast<LPWSTR>(L"Console");
    TabCtrl_InsertItem(g_hWndTab, 0, &tie);
    g_hWndConsole = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL |
        ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, 5, 25, 380, 190, g_hWndTab, nullptr, hInstance, nullptr);
    g_hWndInjectButton = CreateWindow(L"BUTTON", L"Inject", WS_CHILD | WS_VISIBLE,
        150, 230, 100, 25, hWnd, nullptr, hInstance, nullptr);

    SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProc));
    ShowWindow(hWnd, nCmdShow);
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return static_cast<int>(msg.wParam);
}
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_COMMAND:
        if (reinterpret_cast<HWND>(lParam) == g_hWndInjectButton && HIWORD(wParam) == BN_CLICKED) {
            const wchar_t* processName = L"RobloxPlayerBeta.exe";
            DWORD processID = GetProcessID(processName);
            if (processID != 0) {
                if (InjectMessageBox(processID)) {
                    Sleep(3000);
                    AppendTextToConsole("MessageBox injected successfully.\r\n");
                }
                else {
                    AppendTextToConsole("MessageBox injection failed.\r\n");
                    Sleep(5000);
                }
            }
            else {
                AppendTextToConsole("Failed to find process. Did you open Roblox?\r\n");
            }
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}
void AppendTextToConsole(const char* text) {
    int len = GetWindowTextLengthA(g_hWndConsole);
    SendMessage(g_hWndConsole, EM_SETSEL, len, len);
    SendMessageA(g_hWndConsole, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(text));
}
bool InjectMessageBox(DWORD processID) {
    const char* message = "Hello from injected code!";
    const char* title = "Injection Success";
    std::cout << "Attempting to open target process..." << std::endl;
    Sleep(3000);
    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    if (!processHandle) {
        std::cerr << "Failed to open target process." << std::endl;
        Sleep(3000);
        return false;
    }
    Sleep(3000);
    std::cout << "Target process opened successfully." << std::endl;
    Sleep(3000);
    HMODULE user32 = LoadLibrary(L"user32.dll");
    FARPROC pMessageBoxA = GetProcAddress(user32, "MessageBoxA");
    if (!pMessageBoxA) {
        std::cerr << "Failed to get the address of MessageBoxA." << std::endl;
        CloseHandle(processHandle);
        return false;
    }
    std::cout << "Address of MessageBoxA: " << pMessageBoxA << std::endl;
    Sleep(3000);
    LPVOID remoteMessage = VirtualAllocEx(processHandle, NULL, strlen(message) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    LPVOID remoteTitle = VirtualAllocEx(processHandle, NULL, strlen(title) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    LPVOID remoteCode = VirtualAllocEx(processHandle, NULL, 1024, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!remoteMessage || !remoteTitle || !remoteCode) {
        std::cerr << "Failed to allocate memory in the target process." << std::endl;
        CloseHandle(processHandle);
        return false;
    }
    std::cout << "Memory allocated in the target process." << std::endl;
    Sleep(3000);
    if (!WriteProcessMemory(processHandle, remoteMessage, message, strlen(message) + 1, NULL)) {
        std::cerr << "Failed to write message to the target process memory." << std::endl;
        return false;
    }
    if (!WriteProcessMemory(processHandle, remoteTitle, title, strlen(title) + 1, NULL)) {
        std::cerr << "Failed to write title to the target process memory." << std::endl;
        return false;
    }
    std::cout << "Written! You can now execute." << std::endl;


    unsigned char code[] = {
        0x68, 0, 0, 0, 0,            
        0x68, 0, 0, 0, 0,            
        0x6A, 0x00,                 
        0x6A, 0x00,                  
        0xB8, 0, 0, 0, 0,            
        0xFF, 0xD0,                 
        0x31, 0xC0,                 
        0xC3                         
    };
        *(DWORD*)&code[1] = (DWORD)remoteTitle;
        *(DWORD*)&code[6] = (DWORD)remoteMessage;
        *(DWORD*)&code[13] = (DWORD)pMessageBoxA;
        if (!WriteProcessMemory(processHandle, remoteCode, code, sizeof(code), NULL)) {
            std::cerr << "Failed to write code to the target process memory." << std::endl;
            return false;
        }
        std::cout << "Code written to target process memory." << std::endl;
        Sleep(2000);
        HANDLE remoteThread = CreateRemoteThread(processHandle, NULL, 0, (LPTHREAD_START_ROUTINE)remoteCode, NULL, 0, NULL);
        if (!remoteThread) {
            std::cerr << "Failed to create remote thread." << std::endl;
            VirtualFreeEx(processHandle, remoteMessage, 0, MEM_RELEASE);
            VirtualFreeEx(processHandle, remoteTitle, 0, MEM_RELEASE);
            VirtualFreeEx(processHandle, remoteCode, 0, MEM_RELEASE);
            CloseHandle(processHandle);
            return false;
        }
    std::cout << "Remote thread created successfully." << std::endl;
    WaitForSingleObject(remoteThread, INFINITE);
    std::cout << "Remote thread finished execution." << std::endl;
    VirtualFreeEx(processHandle, remoteMessage, 0, MEM_RELEASE);
    VirtualFreeEx(processHandle, remoteTitle, 0, MEM_RELEASE);
    VirtualFreeEx(processHandle, remoteCode, 0, MEM_RELEASE);
    CloseHandle(remoteThread);
    CloseHandle(processHandle);
    Sleep(2000);
    std::cout << "Complete and cleaned up!" << std::endl;
    Sleep(3000);
    return true;
}
DWORD GetProcessID(const wchar_t* processName) {
    DWORD processID = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 processEntry;
        processEntry.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(snapshot, &processEntry)) {
            do {
                if (!_wcsicmp(processEntry.szExeFile, processName)) {
                    processID = processEntry.th32ProcessID;
                    break;
                }
            } while (Process32Next(snapshot, &processEntry));
        }
        CloseHandle(snapshot);
    }
    return processID;
}
int main() {
    const wchar_t* processName = L"RobloxPlayerBeta.exe";
    std::cout << "Searching for process: " << processName << std::endl;
    DWORD processID = GetProcessID(processName);
    if (processID == 0) {
        std::cerr << "Failed to find process: " << processName << std::endl;
        return 1;
    }
    std::cout << "Process found with ID: " << processID << std::endl;
    if (InjectMessageBox(processID)) {
        std::cout << "ECR - Best Scripting Utility" << std::endl;
        std::cout << "Get now at ecr.gg" << std::endl;
        Sleep(2000);
    }
    else {
        std::cerr << "MessageBox injection failed." << std::endl;
    }
    Sleep(5000);
    return 0;
}
