#include <windows.h>

// Define the dimensions of the board
const int BOARD_WIDTH = 800;
const int BOARD_HEIGHT = 800;
const int SQUARE_SIZE = BOARD_WIDTH / 8;

// Define the colors of the squares
const COLORREF WHITE_SQUARE_COLOR = RGB(255, 206, 158);
const COLORREF BLACK_SQUARE_COLOR = RGB(209, 139, 71);

// Define the file name of the chess piece icon
LPCWSTR PAWN_ICON_FILENAME = L"B_King.png";

// Window procedure for handling messages
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // Handle the message for painting the window
    if (message == WM_PAINT)
    {
        // Get the device context for the window
        HDC hdc = GetDC(hWnd);

        // Create a brush for the white squares
        HBRUSH hWhiteBrush = CreateSolidBrush(WHITE_SQUARE_COLOR);

        // Create a brush for the black squares
        HBRUSH hBlackBrush = CreateSolidBrush(BLACK_SQUARE_COLOR);

        // Loop through each square on the board
        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                // Calculate the position and color of the current square
                int x = i * SQUARE_SIZE;
                int y = j * SQUARE_SIZE;
                COLORREF color = ((i + j) % 2 == 0) ? WHITE_SQUARE_COLOR : BLACK_SQUARE_COLOR;

                // Create a brush for the current square
                HBRUSH hBrush = CreateSolidBrush(color);

                // Select the brush into the device context
                HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);

                // Draw the square
                Rectangle(hdc, x, y, x + SQUARE_SIZE, y + SQUARE_SIZE);

                // Restore the old brush
                SelectObject(hdc, hOldBrush);

                // Delete the current brush
                DeleteObject(hBrush);

                if (j == 0 && i == 0)
                {
                    // Load the pawn icon from the file
                    HICON hIcon = (HICON)LoadImage(NULL, PAWN_ICON_FILENAME, IMAGE_ICON,
                        SQUARE_SIZE, SQUARE_SIZE, LR_LOADFROMFILE);

                    // Draw the pawn icon onto the board
                    DrawIcon(hdc, x, y, hIcon);

                    // Destroy the pawn icon
                    DestroyIcon(hIcon);
                }
            }
        }

        // Delete the white and black brushes
        DeleteObject(hWhiteBrush);
        DeleteObject(hBlackBrush);

        // Release the device context
        ReleaseDC(hWnd, hdc);
    }

    // Handle other messages
    switch (message)
    {
    case WM_DESTROY:
        // Quit the application when the window is closed
        PostQuitMessage(0);
        break;
    default:
        // Let Windows handle other messages
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

// Entry point for the application
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Register the window class
    WNDCLASS wc = { };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"ChessBoardWindowClass";
    RegisterClass(&wc);

    // Create the window
    HWND hWnd = CreateWindow(wc.lpszClassName, L"Chess Board", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, BOARD_WIDTH, BOARD_HEIGHT,
        NULL, NULL, hInstance, NULL);

    // Show the window
    ShowWindow(hWnd, nCmdShow);

    // Run the message loop
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
