#pragma once

#include <Windows.h>

#include <d2d1.h>
#include <dwrite.h>

#include <wrl.h>

struct GameOfLife
{
	// Window
	HWND hWnd;
	UINT clientWidth;
	UINT clientHeight;

	// Direct2D resources
	Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> renderTarget;
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush;

	// Default constructor
	GameOfLife();

	// Initializes Game Of Life
	// Returns true on success and false on failure.
	bool Initialize(HINSTANCE hInstance);

	// Runs the main loop
	void Run();

	// Draws a frame to the window
	void DrawWindow();

	// Message handling
	static LRESULT StaticWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	LRESULT WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);


};
