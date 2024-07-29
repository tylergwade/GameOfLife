#include <Windows.h>
#include <windowsx.h>

#include <d2d1.h>
#include <dwrite.h>

#include <wrl.h>
#include "GameOfLife.h"
using Microsoft::WRL::ComPtr;

#include <math.h>

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
    , cellSize(0.f)
    , isPlaying(false)
{
    // Set the static instance of this class
    GameOfLifeInstance = this;
    memset(Cells, 0, sizeof(Cells));
    memset(TempCells, 0, sizeof(TempCells));
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

        if (renderTarget)
        {
            renderTarget->Resize(D2D1::SizeU(clientWidth, clientHeight));
            DrawWindow();
        }

        return 0;
    }

    case WM_LBUTTONDOWN:
    {
        const int cursorX = GET_X_LPARAM(lParam);
        const int cursorY = GET_Y_LPARAM(lParam);

        const int col = (int)floorf(cursorX / cellSize);
        const int row = (int)floorf(cursorY / cellSize);

        Cells[row][col] = !Cells[row][col];

        return 0;
    }

    case WM_KEYDOWN:
    {
        if (wParam == VK_SPACE)
        {
            StepSimulation();
        }

        if (wParam == 'G')
        {
            isPlaying = !isPlaying;

            if (isPlaying)
            {
                SetTimer(hWnd, 123, 100, NULL);
            }
            else
            {
                KillTimer(hWnd, 123);
            }
        }

        return 0;
    }

    case WM_TIMER:
    {
        if (wParam == 123)
        {
            StepSimulation();
        }

        return 0;
    }

    default:
        return DefWindowProc(hWnd, Msg, wParam, lParam);
    }
}

void GameOfLife::StepSimulation()
{
    memcpy(TempCells, Cells, sizeof(Cells));

    for (int row = 0; row < GridSize; row++)
    {
        for (int col = 0; col < GridSize; col++)
        {
            // whether or the cell is currently alive.
            bool isAlive = TempCells[row][col];

            int aliveNeighbors = CountAliveNeighbors(row, col);

            // whether or not the cell should stave alive
            bool stayAlive;
            
            // Rules from https://en.wikipedia.org/wiki/Conway's_Game_of_Life#Rules

            // 1. Any live cell with fewer than two live neighbours dies, as if by underpopulation.
            if (isAlive && aliveNeighbors < 2)
            {
                stayAlive = false;
            }
            // 2. Any live cell with two or three live neighbours lives on to the next generation.
            else if (isAlive && (aliveNeighbors == 2 || aliveNeighbors == 3))
            {
                stayAlive = true;
            }
            // 3. Any live cell with more than three live neighbours dies, as if by overpopulation.
            else if (isAlive && (aliveNeighbors > 3))
            {
                stayAlive = false;
            }
            // 4. Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
            else if (!isAlive && aliveNeighbors == 3)
            {
                stayAlive = true;
            }
            else
            {
                stayAlive = false;
            }

            Cells[row][col] = stayAlive;
        }
    }
}

int GameOfLife::CountAliveNeighbors(int trow, int tcol) const
{
    int startRow = trow - 1;
    int startCol = tcol - 1;

    int lastRow = trow + 1;
    int lastCol = tcol + 1;

    if (startRow < 0) startRow = 0;
    if (startCol < 0) startCol = 0;

    if (lastRow >= GridSize) lastRow = GridSize - 1;
    if (lastCol >= GridSize) lastCol = GridSize - 1;

    int alive = 0;

    for (int row = startRow; row <= lastRow; row++)
    {
        for (int col = startCol; col <= lastCol; col++)
        {
            // Skip the center cell because that just the cell we're testing for
            if (trow == row && tcol == col)
                continue;

            if (TempCells[row][col])
                alive++;
        }
    }

    return alive;
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

    clientWidth = 800;
    clientHeight = 800;

    RECT rect = { 0, 0, clientWidth, clientHeight };
    if (!AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE))
    {
        ShowErrorMsg(TEXT("Failed to adjust window rect"));
        return false;
    }

    const int windowWidth = rect.right - rect.left;
    const int windowHeight = rect.bottom - rect.top;

    const int windowX = screenWidth / 2 - windowWidth / 2;
    const int windowY = screenHeight / 2 - windowHeight / 2;

    hWnd = CreateWindowEx(0, wc.lpszClassName, TEXT("GameOfLife"), WS_POPUPWINDOW | WS_CAPTION | WS_MINIMIZEBOX, windowX, windowY, windowWidth, windowHeight, NULL, NULL, hInstance, NULL);

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

    cellSize = (float)clientWidth / GridSize;

    // Setup an initial cell state
    Cells[12][12] = true;
    Cells[12][13] = true;
    Cells[12][14] = true;
    Cells[11][14] = true;
    Cells[10][13] = true;

    //for (size_t i = 0; i < GridSize; i++)
    //{
    //    Cells[i][i] = true;
    //}

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

void GameOfLife::DrawWindow() const
{
    renderTarget->BeginDraw();
    renderTarget->Clear(D2D1::ColorF(0.1f, 0.1f, 0.1f));
    renderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);

    brush->SetColor(D2D1::ColorF(1.0f, 1.0f, 1.0f));

    // Draw the actual cells
    for (size_t y = 0; y < GridSize; y++)
    {
        for (size_t x = 0; x < GridSize; x++)
        {
            if (Cells[y][x])
            {
                const float posX = floorf(x * cellSize);
                const float posY = floorf(y * cellSize);
                renderTarget->FillRectangle(D2D1::RectF(posX, posY, posX + cellSize, posY + cellSize), brush.Get());
            }
        }
    }

    // Draw a grid of lines
    brush->SetColor(D2D1::ColorF(0.25f, 0.25f, 0.25f));

    for (size_t row = 0; row < GridSize; row++)
    {
        const float posY = floorf(row * cellSize) + 0.5f;
        renderTarget->DrawLine(D2D1::Point2F(0.f, posY), D2D1::Point2F(clientWidth, posY), brush.Get(), 1.0f);
    }

    for (size_t col = 0; col < GridSize; col++)
    {
        const float posX = floorf(col * cellSize) + 0.5f;
        renderTarget->DrawLine(D2D1::Point2F(posX, 0.f), D2D1::Point2F(posX, clientHeight), brush.Get(), 1.0f);
    }

    renderTarget->EndDraw();
}
