#include "Game/Game.h"

namespace library
{
    /*--------------------------------------------------------------------
      Global Variables
    --------------------------------------------------------------------*/
    /*--------------------------------------------------------------------
      TODO: Initialize global variables (remove the comment)
    --------------------------------------------------------------------*/
    const wchar_t CLASS_NAME[] = L"Sample Window Class Test";
    HINSTANCE               g_hInst = nullptr;
    HWND                    g_hWnd = nullptr;


    /*--------------------------------------------------------------------
      Forward declarations
    --------------------------------------------------------------------*/

    /*F+F+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      Function: WindowProc
      Summary:  Defines the behavior of the window—its appearance, how
                it interacts with the user, and so forth
      Args:     HWND hWnd
                  Handle to the window
                UINT uMsg
                  Message code
                WPARAM wParam
                  Additional data that pertains to the message
                LPARAM lParam
                  Additional data that pertains to the message
      Returns:  LRESULT
                  Integer value that your program returns to Windows
    -----------------------------------------------------------------F-F*/
    LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
    {
        PAINTSTRUCT ps;
        HDC hdc;

        switch (uMsg)
        {
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }

        return 0;
    }

    /*--------------------------------------------------------------------
      TODO: Function definitions (remove this comment)
    --------------------------------------------------------------------*/

    HRESULT InitWindow(_In_ HINSTANCE hInstance, _In_ INT nCmdShow)
    {
        // Register the window class.
        WNDCLASS wc = { };

        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = CLASS_NAME;  

        RegisterClass(&wc);

        // Create Window
        g_hWnd = CreateWindowEx(
            0,                                      // Optional window styles.
            CLASS_NAME,                             // Window class
            L"Game Graphic Programming LAB01",      // Window text
            WS_OVERLAPPEDWINDOW,                    // Window style

            // Size and position
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

            NULL,       // Parent window    
            NULL,       // Menu
            hInstance,  // Instance handle
            NULL        // Additional application data
        );

        if (g_hWnd == NULL)
        {
            return 0;
        }

        // Show Window
        ShowWindow(g_hWnd, nCmdShow);
    }

    HRESULT InitDevice();

    void CleanupDevice();

    void Render();
}