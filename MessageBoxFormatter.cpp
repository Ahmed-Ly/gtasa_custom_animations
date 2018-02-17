#include <Windows.h>
#include <stdio.h>

VOID SHOW_DWORD_HEX_MsgBox(DWORD output) // show handle ID of proxy_sa.exe
{
    char szTest[100];

    // swprintf_s is the same as sprintf_s for wide characters
    sprintf_s(szTest, "%#.8X", output);

    //OutputDebugStringW(szTest);
    MessageBoxA(0, szTest, "Hi", MB_ICONINFORMATION);
}

VOID SHOW_OUTPUT_BOXW(LPCWSTR output) // show handle ID of proxy_sa.exe
{
    WCHAR szTest[100]; // WCHAR is the same as wchar_t

                       // swprintf_s is the same as sprintf_s for wide characters
    swprintf_s(szTest, 100, output);

    MessageBoxW(0, szTest, L"Hi", MB_ICONINFORMATION);
}



VOID SHOW_OUTPUT_BOX(LPCSTR output) // show handle ID of proxy_sa.exe
{
    CHAR szTest[100]; // WCHAR is the same as wchar_t

                      // swprintf_s is the same as sprintf_s for wide characters
    sprintf_s(szTest, 100, output);

    MessageBoxA(0, szTest, "Hi", MB_ICONINFORMATION);
}

