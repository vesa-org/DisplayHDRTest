//********************************************************* 
// 
// Copyright (c) Microsoft. All rights reserved. 
// This code is licensed under the MIT License (MIT). 
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY 
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR 
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT. 
// 
//*********************************************************

#include "pch.h"
#include "Game.h"

using namespace DirectX;

namespace
{
    std::unique_ptr<Game> g_game;
};

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void ToggleFullscreen(HWND hWnd);

bool CheckTimeBombExpired();
SYSTEMTIME GetExpiryTime();

LONG g_wndStyle = WS_OVERLAPPEDWINDOW;
RECT g_wndRect = {};
bool g_fullscreen = false;
const PWSTR g_appTitle = L"VESA DisplayHDR Compliance Tests";

// Indicates to hybrid graphics systems to prefer the discrete part by default
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

// Entry point
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    if (!XMVerifyCPUSupport())
        return 1;

    HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
    if (FAILED(hr))
        return 1;

    g_game = std::make_unique<Game>(g_appTitle);

    // Register class and create window
    {
        // Register class
        WNDCLASSEX wcex;
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInstance;
        wcex.hIcon = LoadIcon(hInstance, L"IDI_ICON");
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = L"DisplayHDRComplianceTestsWindowClass";
        wcex.hIconSm = LoadIcon(wcex.hInstance, L"IDI_ICON");
        if (!RegisterClassEx(&wcex))
            return 1;

        // Create window
        int w, h;
        g_game->GetDefaultSize(w, h);

        RECT rc;
        rc.top = 0;
        rc.left = 0;
        rc.right = static_cast<LONG>(w); 
        rc.bottom = static_cast<LONG>(h);

        AdjustWindowRect(&rc, g_wndStyle, FALSE);

        HWND hwnd = CreateWindowEx(
            0,
            L"DisplayHDRComplianceTestsWindowClass",
            g_appTitle,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            rc.right - rc.left,
            rc.bottom - rc.top,
            nullptr,
            nullptr,
            hInstance,
            nullptr);

        if (!hwnd)
            return 1;

        // Check timebomb and quit out if needed.

        std::wstringstream msg;

        msg << L"This prerelease version is only valid until: ";

        WCHAR dateStr[255];
        int ignored = GetDateFormatEx(
            LOCALE_NAME_USER_DEFAULT,
            DATE_SHORTDATE,
            &GetExpiryTime(),
            nullptr, // Format
            dateStr,
            ARRAYSIZE(dateStr),
            nullptr); // Reserved
        msg << dateStr;

#if 0    // disable time bomb
        if (CheckTimeBombExpired())
        {
            MessageBoxEx(hwnd, msg.str().c_str(), L"Windows HDR display color performance", MB_OK, 0);
            return 0;
        }
#endif

        ShowWindow(hwnd, nCmdShow);

        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(g_game.get()) );

        GetClientRect(hwnd, &rc);

        g_game->Initialize(hwnd, rc.right - rc.left, rc.bottom - rc.top);

        // TODO: When debugging it can be useful to comment this out and start in windowed mode.
        // After the window is created, set to fullscreen windowed. Don't do this at window
        // creation time so we have the default window state and RECT to restore to.
#ifndef _DEBUG
        ToggleFullscreen(hwnd);
#endif

        // We never want to see the cursor in this app.
        CURSORINFO ci = {};
        ci.cbSize = sizeof(CURSORINFO);
        BOOL status = GetCursorInfo(&ci);
        int cursorCount = 0;

        do
        {
            cursorCount = ShowCursor(FALSE); // This decrements cursorCount by 1.
        } while (cursorCount >= 0); // -1 means invisible cursor.
    }


    // Main message loop
    MSG msg = { 0 };
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            g_game->Tick();
        }
    }

    g_game.reset();

    CoUninitialize();

    return (int) msg.wParam;
}

// Windows procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    static bool s_in_sizemove = false;
    static bool s_in_suspend = false;
    static bool s_minimized = false;

    auto game = reinterpret_cast<Game*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (message)
    {
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;

    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
        {
            if (!s_minimized)
            {
                s_minimized = true;
                if (!s_in_suspend && game)
                    game->OnSuspending();
                s_in_suspend = true;
            }
        }
        else if (s_minimized)
        {
            s_minimized = false;
            if (s_in_suspend && game)
                game->OnResuming();
            s_in_suspend = false;
        }
        else if (!s_in_sizemove && game)
        {
            game->OnWindowSizeChanged(LOWORD(lParam), HIWORD(lParam));
        }
        break;

    case WM_ENTERSIZEMOVE:
        s_in_sizemove = true;
        break;

    case WM_EXITSIZEMOVE:
        s_in_sizemove = false;
        if (game)
        {
            RECT rc;
            GetClientRect(hWnd, &rc);

            game->OnWindowSizeChanged(rc.right - rc.left, rc.bottom - rc.top);
        }
        break;

    case WM_GETMINMAXINFO:
        {
            auto info = reinterpret_cast<MINMAXINFO*>(lParam);
            info->ptMinTrackSize.x = 320;
            info->ptMinTrackSize.y = 200;
        }
        break;

    case WM_ACTIVATEAPP:
        if (game)
        {
            if (wParam)
            {
                game->OnActivated();
            }
            else
            {
                game->OnDeactivated();
            }
        }
        break;

    case WM_POWERBROADCAST:
        switch (wParam)
        {
        case PBT_APMQUERYSUSPEND:
            if (!s_in_suspend && game)
                game->OnSuspending();
            s_in_suspend = true;
            return TRUE;

        case PBT_APMRESUMESUSPEND:
            if (!s_minimized)
            {
                if (s_in_suspend && game)
                    game->OnResuming();
                s_in_suspend = false;
            }
            return TRUE;
        }
        break;

    case WM_DESTROY:
		game->SetMetadataNeutral();		// DWM should do this automatically on full-screen exit
        PostQuitMessage(0);
        break;

    case WM_DISPLAYCHANGE:
        // Unsure what other explicit checks should go here, I'm just using it to
        // update DXGI_OUTPUT_DESC1 state.
        game->OnDisplayChange();
        break;

    case WM_KEYUP:
        switch (wParam)
        {
        case VK_SPACE:
            /*bool ignored*/ game->ToggleInfoTextVisible();
            break;

        case VK_HOME:
            game->StartTestPattern();
            break;

        case VK_RIGHT:
        case VK_NEXT:
            game->ChangeTestPattern(true);
            break;

        case VK_LEFT:
        case VK_PRIOR:
            game->ChangeTestPattern(false);
            break;

        case VK_DOWN:
            game->ChangeSubtest(true);
            break;

        case VK_UP:
            game->ChangeSubtest(false);
            break;

        case VK_ESCAPE:         // Youtube compat
            if (g_fullscreen)
            {
                ToggleFullscreen(hWnd);
            }
            break;
        case 0x43:                                                        // 'c'
            game->SetTestPattern(Game::TestPattern::Cooldown);
            break;
        case 0x31:                                                        // '1'
            game->SetTestPattern(Game::TestPattern::TenPercentPeak);
            break;
        case 0x32:                                                        // '2'
            game->SetTestPattern(Game::TestPattern::FlashTest);
            break;
        case 0x33:                                                        // '3'
            game->SetTestPattern(Game::TestPattern::LongDurationWhite);
            break;
        case 0x34:                                                        // '4'
            game->SetTestPattern(Game::TestPattern::DualCornerBox);
            break;
		case 0x35:                                                        // '5'
            game->SetTestPattern(Game::TestPattern::StaticContrastRatio);
            break;
        case 0x36:                                                        // '6'
            game->SetTestPattern(Game::TestPattern::ColorPatches10);
            break;
        case 0x37:                                                        // '7'
            game->SetTestPattern(Game::TestPattern::BitDepthPrecision);
            break;
        case 0x38:                                                        // '8'
            game->SetTestPattern(Game::TestPattern::RiseFallTime);
            break;
		case 0x39:                                                        // '9'
			game->SetTestPattern(Game::TestPattern::ProfileCurve);
			break;
		case 0x30:                                                        // '0'
//			game->SetTestPattern(Game::TestPattern::ProfileCurve);
			break;

        case 0x41: // 'a'
            game->ChangeGradientColor(-0.05f, -0.05f, -0.05f);
            break;

        case 0x53: // 's'
            game->ChangeGradientColor(0.05f, 0.05f, 0.05f);
            break;

        //case 0x38: // '8'
        //    game->ChangeBackBufferFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
        //    break;

        //case 0x30: // '0'
        //    game->ChangeBackBufferFormat(DXGI_FORMAT_R10G10B10A2_UNORM);
        //    break;

        default:
            break;
        }
        break;

    case WM_SYSKEYDOWN:
        // ALT-ENTER
        if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
        {
            ToggleFullscreen(hWnd);
        }
        break;

    case WM_MENUCHAR:
        // A menu is active and the user presses a key that does not correspond
        // to any mnemonic or accelerator key. Ignore so we don't produce an error beep.
        return MAKELRESULT(0, MNC_CLOSE);
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void ToggleFullscreen(HWND hWnd)
{
    // Toggle between windowed and borderless window (fullscreen)
    if (g_fullscreen)
    {
        // TODO: Guard against starting in fullscreen, as we don't have default RECT and style?

        // Restore the window's attributes and size.
        SetWindowLong(hWnd, GWL_STYLE, g_wndStyle);

        SetWindowPos(
            hWnd,
            HWND_NOTOPMOST,
            g_wndRect.left,
            g_wndRect.top,
            g_wndRect.right - g_wndRect.left,
            g_wndRect.bottom - g_wndRect.top,
            SWP_FRAMECHANGED | SWP_NOACTIVATE);

        ShowWindow(hWnd, SW_NORMAL);
    }
    else
    {
        // Save the old window rect so we can restore it when exiting fullscreen mode.
        GetWindowRect(hWnd, &g_wndRect);

		// Make the window borderless so that the client area can fill the screen.
		SetWindowLong(hWnd, GWL_STYLE, g_wndStyle & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME));

#if 0
		// Get the settings of the primary display. We want the app to go into
		// fullscreen mode on the display that supports Independent Flip.
		DEVMODE devMode = {};
		devMode.dmSize = sizeof(DEVMODE);
		EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode);

		SetWindowPos(
			hWnd,
			HWND_TOPMOST,
			devMode.dmPosition.x,
			devMode.dmPosition.y,
			devMode.dmPosition.x + devMode.dmPelsWidth,
			devMode.dmPosition.y + devMode.dmPelsHeight,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);
#else
		WINDOWPLACEMENT g_wpPrev = { sizeof(g_wpPrev) };
		MONITORINFO mi = { sizeof(mi) };
		if (GetWindowPlacement(hWnd, &g_wpPrev) &&
			GetMonitorInfo(MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY), &mi))
		{
			SetWindowPos(hWnd, HWND_TOP,
				mi.rcMonitor.left, mi.rcMonitor.top,
				mi.rcMonitor.right - mi.rcMonitor.left,
				mi.rcMonitor.bottom - mi.rcMonitor.top,
				SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
#endif
		ShowWindow(hWnd, SW_MAXIMIZE);
    }

    g_fullscreen = !g_fullscreen;
}

// Exit helper
void ExitGame()
{
    PostQuitMessage(0);
}

SYSTEMTIME GetExpiryTime()
{
    // Set this Time bomb to approximately one month after compilation.
    SYSTEMTIME expiryTime = {};
    expiryTime.wYear = 2018;
    expiryTime.wMonth = 12;
    expiryTime.wDay = 31;

    return expiryTime;
}

// Returns true if we have exceeded the date/time allowed for the app.
bool CheckTimeBombExpired()
{
    SYSTEMTIME expiryTime = GetExpiryTime();

    // To compare times we must convert to FILETIME and then ULARGE_INTEGER
    FILETIME expiryFileTime = {};
    BOOL ignored = SystemTimeToFileTime(&expiryTime, &expiryFileTime);

    ULARGE_INTEGER expiryULarge = {};
    expiryULarge.HighPart = expiryFileTime.dwHighDateTime;
    expiryULarge.LowPart = expiryFileTime.dwLowDateTime;

    SYSTEMTIME localTime = {};
    GetLocalTime(&localTime);

    FILETIME localFileTime = {};
    ignored = SystemTimeToFileTime(&localTime, &localFileTime);

    ULARGE_INTEGER localULarge = {};
    localULarge.HighPart = localFileTime.dwHighDateTime;
    localULarge.LowPart = localFileTime.dwLowDateTime;

    if (localULarge.QuadPart >= expiryULarge.QuadPart)
    {
        return true;
    }
    else
    {
        return false;
    }
}
#if 0
#define MAX_VERTS 144
#define MAX_PRIMS 255

float3 myPositions[MAX_VERTS];            // :SV_POSITION;  VS_OUTPUT(POS);

typedef struct vertAttr {
    float2 baseTextureCoords;
    float2 lightMapCoords;
};
[[vertexAttributes]]
vertAttr myVertAttributes[MAX_VERTS];    // VS_OUTPUT(VERT);

typedef struct primAttr {
    uint TriangleID;
    half3 faceNormal;
};
[[primitiveAttributes]]
primAttr myPrimAttributes[MAX_PRIMS];    // VS_OUTPUT(PRIM);

typedef struct meshAttr {
    uint patchID;
    half3 meshColor;
};
[[meshletAttributes]]
meshAttr myMeshAttributes;                // VS_OUTPUT(MESH);

// compute shader
[255 1 1]
main()
{
    // load positions, transform, skin, etc.
    ProcessPositions( myPositions );
    groupMemoryBarrierWithGroupSync();

    // cull/compact primitives and vertices
    ProcessPrimitives( myPositions, myPrimAttributes );
    groupMemoryBarrierWithGroupSync();

    // load vertex attributes for surviving vertices
    ProcessAttributes( myVertAttributes );
}
#endif