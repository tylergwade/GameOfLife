#include "GameOfLife.h"

#include <Windows.h>
#include <memory>

int WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nShowCmd)
{
    std::unique_ptr<GameOfLife> gol = std::make_unique<GameOfLife>();

    if (!gol->Initialize(hInstance))
        return -1;

    gol->Run();

    return 0;
}