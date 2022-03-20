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

    D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
    D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;

    Microsoft::WRL::ComPtr<ID3D11Device> g_pD3dDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> g_pImmediateContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain> g_pSwapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> g_pRenderTargetView;

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
      Function definitions
    --------------------------------------------------------------------*/

    HRESULT InitWindow(_In_ HINSTANCE hInstance, _In_ INT nCmdShow)
    {
        // Register the window class.
        WNDCLASS wc = { };

        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = CLASS_NAME;

        if (!RegisterClass(&wc))
            return E_FAIL;


        // Create Window
        g_hInst = hInstance;
        RECT rc = { 0, 0, 800, 600 };
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
        g_hWnd = CreateWindow(
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

        if (!g_hWnd)
            return E_FAIL;

        // Show Window
        ShowWindow(g_hWnd, nCmdShow);

        return S_OK;
    }

    HRESULT InitDevice()
    {
        // Creating Direct3D device and context -------------------
        HRESULT hr = S_OK;

        RECT rc;
        GetClientRect(g_hWnd, &rc);
        UINT width = rc.right - rc.left;
        UINT height = rc.bottom - rc.top;

        D3D_FEATURE_LEVEL levels[] = {
            D3D_FEATURE_LEVEL_9_1,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_11_1
        };

        // This flag adds support for surfaces with a color-channel ordering different
        // from the API default. It is required for compatibility with Direct2D.
        UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

        #if defined(DEBUG) || defined(_DEBUG)
        deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
        #endif

        // Create the Direct3D 11 API device object and a corresponding context.
        D3D_FEATURE_LEVEL m_featureLevel;

        hr = D3D11CreateDevice(
            nullptr,                    // Specify nullptr to use the default adapter.
            D3D_DRIVER_TYPE_HARDWARE,   // Create a device using the hardware graphics driver.
            0,                          // Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
            deviceFlags,                // Set debug and Direct2D compatibility flags.
            levels,                     // List of feature levels this app can support.
            ARRAYSIZE(levels),          // Size of the list above.
            D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Windows Store apps.
            &g_pD3dDevice,                    // Returns the Direct3D device created.
            &m_featureLevel,            // Returns feature level of device created.
            &g_pImmediateContext                    // Returns the device immediate context.
        );

        if (FAILED(hr))
        {
            return hr;
        }

        // Store pointers to the Direct3D 11.1 API device and immediate context.
        Microsoft::WRL::ComPtr<ID3D11Device> m_pd3dDevice;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pd3dDeviceContext;
        hr = g_pD3dDevice.As(&m_pd3dDevice);

        if (SUCCEEDED(hr))
        {
            (void)g_pImmediateContext.As(&m_pd3dDeviceContext);
        }

        // -----------------------------------------------------
        // DXGI Factory ----------------------------------------

        Microsoft::WRL::ComPtr<IDXGIFactory> pIDXGIFactory;
        {
            Microsoft::WRL::ComPtr<IDXGIDevice> pDXGIDevice;
            hr = g_pD3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDXGIDevice);
            if (SUCCEEDED(hr))
            {
                Microsoft::WRL::ComPtr<IDXGIAdapter> pDXGIAdapter;
                hr = pDXGIDevice->GetAdapter(&pDXGIAdapter);
                if (SUCCEEDED(hr))
                {
                    hr = pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&pIDXGIFactory);

                }
            }
        }

        if (FAILED(hr))
        {
            return hr;
        }
            
        // -----------------------------------------------------
        // Create Swap Chain------------------------------------

        DXGI_SWAP_CHAIN_DESC desc;
        ZeroMemory(&desc, sizeof(DXGI_SWAP_CHAIN_DESC));
        desc.Windowed = TRUE; // Sets the initial state of full-screen mode.
        desc.BufferCount = 2;
        desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.SampleDesc.Count = 1;      //multisampling setting
        desc.SampleDesc.Quality = 0;    //vendor-specific flag
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        desc.OutputWindow = g_hWnd;


        hr = pIDXGIFactory->CreateSwapChain(
            m_pd3dDevice.Get(),
            &desc,
            g_pSwapChain.GetAddressOf());
        
        if (FAILED(hr))
        {
            return hr;
        }

        // -----------------------------------------------------
        // Creating a Render Target ----------------------------
        Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pBackBuffer;
        hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&m_pBackBuffer);

        if (FAILED(hr))
        {
            return hr;
        }

        hr = m_pd3dDevice->CreateRenderTargetView(
            m_pBackBuffer.Get(),
            nullptr,
            g_pRenderTargetView.GetAddressOf()
        );


        if (FAILED(hr))
        {
            return hr;
        }

        g_pImmediateContext->OMSetRenderTargets(1, g_pRenderTargetView.GetAddressOf(), nullptr);

        // Setup the viewport
        D3D11_VIEWPORT vp;
        vp.Width = (FLOAT)width;
        vp.Height = (FLOAT)height;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        g_pImmediateContext->RSSetViewports(1, &vp);

        // -----------------------------------------------------

        return S_OK;

    }

    void CleanupDevice()
    {
        if (g_pImmediateContext) g_pImmediateContext->ClearState();

        if (g_pRenderTargetView) g_pRenderTargetView.Reset();
        if (g_pSwapChain) g_pSwapChain.Reset();
        if (g_pImmediateContext) g_pImmediateContext.Reset();
        if (g_pD3dDevice) g_pD3dDevice.Reset();
    }

    void Render()
    {
        // Just clear the backbuffer
        g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView.Get(), Colors::MidnightBlue);
        g_pSwapChain->Present(0, 0);
    }
}