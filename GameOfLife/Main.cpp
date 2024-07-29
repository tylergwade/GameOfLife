#include "GameOfLife.h"

#include <Windows.h>

int WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nShowCmd)
{
    GameOfLife gol;

    if (!gol.Initialize(hInstance))
        return -1;

    gol.Run();

    return 0;
}