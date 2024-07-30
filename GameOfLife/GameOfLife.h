#pragma once

#include <Windows.h>

#include <d2d1.h>
#include <dwrite.h>

#include <wrl.h>

struct GameOfLife
{
	static constexpr size_t gridSize = 25;

	// Game Of Life is composed of a grid of cells
	// All the data for the cells is stored in this 2D array.
	// Each cell can be either ALIVE or DEAD.
	bool Cells[gridSize][gridSize];

	// Temporary cells to help aid in computing which cells live and die.
	bool TempCells[gridSize][gridSize];

	// The size of a cell (width and height)
	float cellSize;

	bool isPlaying = false;

	float translationX = 0.f;
	float translationY = 0.f;

	float startTranslationX = 0.f;
	float startTranslationY = 0.f;

	float cursorStartX = 0.f;
	float cursorStartY = 0.f;

	float centerOffsetX = 0.f;
	float centerOffsetY = 0.f;

	bool isPanning = false;

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
	void DrawWindow() const;

	// Message handling
	static LRESULT StaticWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	LRESULT WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

	void StepSimulation();

	// Gets the number of alive neighbors a temp cell has.
	int CountAliveNeighbors(int row, int col) const;
};
