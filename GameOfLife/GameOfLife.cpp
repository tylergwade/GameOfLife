#include <Windows.h>

#include <d2d1.h>
#include <dwrite.h>

#include <wrl.h>
#include "GameOfLife.h"
using Microsoft::WRL::ComPtr;

static GameOfLife* GameOfLifeInstance;

// Pumps any pending messages
// Returns false if the app should exit, otherwise true.
static bool PumpMessages()
{
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
            return false;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return true;
}

// Shows an error message to the user
static void ShowErrorMsg(const TCHAR* msg)
{
    MessageBox(NULL, msg, TEXT("GameOfLife Error"), MB_OK | MB_ICONERROR);
}

GameOfLife::GameOfLife()
    : hWnd(NULL)
    , clientWidth(0)
    , clientHeight(0)
{
    // Set the static instance of this class
    GameOfLifeInstance = this;
}

LRESULT GameOfLife::StaticWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return GameOfLifeInstance->WndProc(hWnd, Msg, wParam, lParam);
}

LRESULT GameOfLife::WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch (Msg)
    {
    case WM_DESTROY:
    {
        // Quit the application when the window is destroyed.
        PostQuitMessage(0);
        return 0;
    }

    case WM_SIZE:
    {
        clientWidth = LOWORD(lParam);
        clientHeight = HIWORD(lParam);

        renderTarget->Resize(D2D1::SizeU(clientWidth, clientHeight));
        DrawWindow();

        return 0;
    }

    default:
        return DefWindowProc(hWnd, Msg, wParam, lParam);
    }
}

bool GameOfLife::Initialize(HINSTANCE hInstance)
{
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpszClassName = TEXT("GameOfLifeClass");
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hInstance = hInstance;
    wc.lpfnWndProc = &GameOfLife::StaticWndProc;

    if (RegisterClassEx(&wc) == INVALID_ATOM)
    {
        ShowErrorMsg(TEXT("Failed to register class"));
        return -1;
    }

    // Calculate the position and size of the
    // window so that is appears centered on the screen.
    const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    const int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    const int windowWidth = 1200;
    const int windowHeight = 800;

    const int windowX = screenWidth / 2 - windowWidth / 2;
    const int windowY = screenHeight / 2 - windowHeight / 2;

    hWnd = CreateWindowEx(0, wc.lpszClassName, TEXT("GameOfLife"), WS_OVERLAPPEDWINDOW, windowX, windowY, windowWidth, windowHeight, NULL, NULL, hInstance, NULL);

    if (hWnd == NULL)
    {
        ShowErrorMsg(TEXT("Failed to create window"));
        return false;
    }

    ComPtr<ID2D1Factory> factory;
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, IID_PPV_ARGS(factory.GetAddressOf()));

    if (FAILED(hr))
    {
        ShowErrorMsg(TEXT("Failed to create D2D1 factory"));
        return false;
    }

    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    clientWidth = clientRect.right - clientRect.left;
    clientHeight = clientRect.bottom - clientRect.top;

    hr = factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hWnd, D2D1::SizeU(clientWidth, clientHeight)), renderTarget.GetAddressOf());

    if (FAILED(hr))
    {
        ShowErrorMsg(TEXT("Failed to create D2D1 hwnd render target"));
        return false;
    }

    hr = renderTarget->CreateSolidColorBrush(D2D1_COLOR_F{}, brush.GetAddressOf());

    if (FAILED(hr))
    {
        ShowErrorMsg(TEXT("Failed to create D2D1 solid color brush"));
        return false;
    }

    return true;
}

void GameOfLife::Run()
{
    ShowWindow(hWnd, SW_SHOW);

    while (PumpMessages())
    {
        DrawWindow();
    }
}

void GameOfLife::DrawWindow()
{
    renderTarget->BeginDraw();
    renderTarget->Clear();

    brush->SetColor(D2D1::ColorF(1.0f, 1.0f, 1.0f));
    renderTarget->FillRectangle(D2D1::RectF(100.f, 100.f, 400.f, 400.f), brush.Get());

    renderTarget->EndDraw();
}
