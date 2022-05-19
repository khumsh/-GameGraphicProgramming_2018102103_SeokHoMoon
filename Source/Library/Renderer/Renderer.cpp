#include "Renderer/Renderer.h"

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Renderer
      Summary:  Constructor
      Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
                 m_immediateContext, m_immediateContext1, m_swapChain,
                 m_swapChain1, m_renderTargetView, m_depthStencil,
                 m_depthStencilView, m_cbChangeOnResize, m_camera,
                 m_projection, m_renderables, m_vertexShaders,
                 m_pixelShaders].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    
    Renderer::Renderer()
        : m_driverType(D3D_DRIVER_TYPE_NULL),
        m_featureLevel(D3D_FEATURE_LEVEL_11_0),
        m_d3dDevice(nullptr),
        m_d3dDevice1(nullptr),
        m_immediateContext(nullptr),
        m_immediateContext1(nullptr),
        m_swapChain(nullptr),
        m_swapChain1(nullptr),
        m_renderTargetView(nullptr),
        m_depthStencil(nullptr),
        m_depthStencilView(nullptr),
        m_cbChangeOnResize(nullptr),
        m_cbLights(nullptr),
        m_camera(XMVECTOR()),
        m_projection(XMMatrixIdentity()),
        m_renderables(std::unordered_map<std::wstring, std::shared_ptr<Renderable>>()),
        m_aPointLights{ std::shared_ptr<PointLight>() },
        m_vertexShaders(std::unordered_map<std::wstring, std::shared_ptr<VertexShader>>()),
        m_pixelShaders(std::unordered_map<std::wstring, std::shared_ptr<PixelShader>>()),
        m_pszMainSceneName()
    {
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Initialize
      Summary:  Creates Direct3D device and swap chain
      Args:     HWND hWnd
                  Handle to the window
      Modifies: [m_d3dDevice, m_featureLevel, m_immediateContext,
                 m_d3dDevice1, m_immediateContext1, m_swapChain1,
                 m_swapChain, m_renderTargetView, m_cbChangeOnResize,
                 m_projection, m_camera, m_vertexShaders,
                 m_pixelShaders, m_renderables].
      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    
    HRESULT Renderer::Initialize(_In_ HWND hWnd)
    {
        HRESULT hr = S_OK;

        RECT rc;
        GetClientRect(hWnd, &rc);
        UINT width = rc.right - static_cast<UINT>(rc.left);
        UINT height = rc.bottom - static_cast<UINT>(rc.top);

        UINT createDeviceFlags = 0;

        D3D_DRIVER_TYPE driverTypes[] =
        {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_REFERENCE,
        };
        UINT numDriverTypes = ARRAYSIZE(driverTypes);

        D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };
        UINT numFeatureLevels = ARRAYSIZE(featureLevels);

        for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
        {
            m_driverType = driverTypes[driverTypeIndex];
            hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

            if (hr == E_INVALIDARG)
            {
                // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                    D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());
            }

            if (SUCCEEDED(hr))
                break;
        }
        if (FAILED(hr))
            return hr;

        // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
        ComPtr<IDXGIFactory1>           dxgiFactory(nullptr);
        {
            ComPtr<IDXGIDevice>           dxgiDevice(nullptr);
            hr = m_d3dDevice.As(&dxgiDevice);
            if (SUCCEEDED(hr))
            {
                ComPtr<IDXGIAdapter>           adapter(nullptr);

                hr = dxgiDevice->GetAdapter(adapter.GetAddressOf());
                if (SUCCEEDED(hr))
                {
                    hr = adapter->GetParent(__uuidof(IDXGIFactory1), (&dxgiFactory));
                }
            }
        }
        if (FAILED(hr))
            return hr;

        // Create swap chain
        ComPtr<IDXGIFactory2>           dxgiFactory2(nullptr);
        hr = dxgiFactory.As(&dxgiFactory2);
        if (dxgiFactory2)
        {
            // DirectX 11.1 or later
            hr = m_d3dDevice.As(&m_d3dDevice1);
            if (SUCCEEDED(hr))
            {
                hr = m_immediateContext.As(&m_immediateContext1);
            }

            DXGI_SAMPLE_DESC sampleDesc = {
                .Count = 1,
                .Quality = 0
            };

            DXGI_SWAP_CHAIN_DESC1 sd =
            {
                .Width = width,
                .Height = height,
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                .SampleDesc = sampleDesc,
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1
            };



            hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &sd, nullptr, nullptr, m_swapChain1.GetAddressOf());
            if (SUCCEEDED(hr))
            {
                hr = m_swapChain1.As(&m_swapChain);
            }
        }
        else
        {
            DXGI_RATIONAL refreshRate = {
                .Numerator = 60,
                .Denominator = 1
            };

            DXGI_MODE_DESC bufferDesc = {
                .Width = width,
                .Height = height,
                .RefreshRate = refreshRate,
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM
            };

            DXGI_SAMPLE_DESC sampleDesc = {
                .Count = 1,
                .Quality = 0
            };

            // DirectX 11.0 systems
            DXGI_SWAP_CHAIN_DESC sd =
            {
                .BufferDesc = bufferDesc,
                .SampleDesc = sampleDesc,
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1,
                .OutputWindow = hWnd,
                .Windowed = TRUE
            };

            hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf());
        }

        // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
        dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);


        if (FAILED(hr))
            return hr;

        // Create a render target view
        ComPtr<ID3D11Texture2D>           pBackBuffer(nullptr);

        hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (&pBackBuffer));
        if (FAILED(hr))
            return hr;

        hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
        if (FAILED(hr))
            return hr;

        

        // Setup the viewport
        D3D11_VIEWPORT vp =
        {
            .TopLeftX = 0,
            .TopLeftY = 0,
            .Width = (FLOAT)width,
            .Height = (FLOAT)height,
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f
        };

        m_immediateContext->RSSetViewports(1, &vp);




        //Create depth stencil texture and the depth stencil view
        //Initialize view matrixand the projection matrix
        //Initialize the shaders, then the renderables
        //    Shaders and renderables are stored in Hash maps
        //    Strings are used as the key, and the shader / renderable objects are the value
        //    When iterating, use iterators


        // Create depth stencil texture
        DXGI_SAMPLE_DESC sampleDesc =
        {
            // The default sampler mode, with no anti-aliasing
            .Count = 1,
            .Quality = 0
        };

        D3D11_TEXTURE2D_DESC descDepth =
        {
            .Width = width,
            .Height = height,
            .MipLevels = 1,
            .ArraySize = 1,
            .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
            .SampleDesc = sampleDesc,
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_DEPTH_STENCIL,
            .CPUAccessFlags = 0,
            .MiscFlags = 0
        };


        hr = m_d3dDevice->CreateTexture2D(&descDepth, NULL, m_depthStencil.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // Create the depth stencil view
        D3D11_TEX2D_DSV texture2D =
        {
            .MipSlice = 0
        };

        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV =
        {
            .Format = descDepth.Format,
            .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
            .Texture2D = texture2D
        };

        hr = m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &descDSV, m_depthStencilView.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

        // projection
        float fovAngleY = XM_PIDIV2;
        float aspectRatio = width / height;
        float nearZ = 0.01f;
        float farZ = 100.0f;

        m_projection = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, nearZ, farZ);

        // Initializing Objects
        hr = m_camera.Initialize(m_d3dDevice.Get());
        if (FAILED(hr))
        {
            return hr;
        }
            

        std::unordered_map<std::wstring, std::shared_ptr<VertexShader>>::iterator it_vertexShaders;

        for (it_vertexShaders = m_vertexShaders.begin(); it_vertexShaders != m_vertexShaders.end(); it_vertexShaders++)
        {
            hr = it_vertexShaders->second->Initialize(m_d3dDevice.Get());
            if (FAILED(hr))
            {
                return hr;
            }
        }


        std::unordered_map<std::wstring, std::shared_ptr<PixelShader>>::iterator it_pixelShaders;

        for (it_pixelShaders = m_pixelShaders.begin(); it_pixelShaders != m_pixelShaders.end(); it_pixelShaders++)
        {
            hr = it_pixelShaders->second->Initialize(m_d3dDevice.Get());
            if (FAILED(hr))
            {
                return hr;
            }
        }

        std::unordered_map<std::wstring, std::shared_ptr<Renderable>>::iterator it_renderables;

        for (it_renderables = m_renderables.begin(); it_renderables != m_renderables.end(); it_renderables++)
        {
            hr = it_renderables->second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
            if (FAILED(hr))
            {
                return hr;
            }
        }

        std::unordered_map<std::wstring, std::shared_ptr<Scene>>::iterator it_scenes;

        for (it_scenes = m_scenes.begin(); it_scenes != m_scenes.end(); it_scenes++)
        {
            hr = it_scenes->second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
            if (FAILED(hr))
            {
                return hr;
            }
        }

        // create constant buffer
        D3D11_BUFFER_DESC constantBd = {
            .ByteWidth = sizeof(CBChangeOnResize),
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
            .CPUAccessFlags = 0,
            .MiscFlags = 0,
            .StructureByteStride = 0
        };

        hr = m_d3dDevice->CreateBuffer(&constantBd, nullptr, m_cbChangeOnResize.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // CBChangeOnResize constant buffer must be created, and set when initializing the renderer
        CBChangeOnResize cb_projection = {
                .Projection = XMMatrixTranspose(m_projection)
        };

        m_immediateContext->UpdateSubresource(m_cbChangeOnResize.Get(), 0, nullptr, &cb_projection, 0, 0);

        D3D11_BUFFER_DESC cbLights = {
            .ByteWidth = sizeof(CBLights),
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
            .CPUAccessFlags = 0,
            .MiscFlags = 0,
            .StructureByteStride = 0
        };

        hr = m_d3dDevice->CreateBuffer(&cbLights, nullptr, m_cbLights.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddModel
      Summary:  Add a model object
      Args:     PCWSTR pszModelName
                  Key of the model object
                const std::shared_ptr<Model>& pModel
                  Shared pointer to the model object
      Modifies: [m_models].
      Returns:  HRESULT
                  Status code.
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::AddModel definition (remove the comment)
    --------------------------------------------------------------------*/


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddRenderable
      Summary:  Add a renderable object and initialize the object
      Args:     PCWSTR pszRenderableName
                  Key of the renderable object
                const std::shared_ptr<Renderable>& renderable
                  Unique pointer to the renderable object
      Modifies: [m_renderables].
      Returns:  HRESULT
                  Status code.
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    
    HRESULT Renderer::AddRenderable(_In_ PCWSTR pszRenderableName, _In_ const std::shared_ptr<Renderable>& renderable)
    {
        if (m_renderables.contains(pszRenderableName))
        {
            return E_FAIL;
        }
        else
        {
            m_renderables.insert(std::make_pair(pszRenderableName, renderable));

            return S_OK;
        }
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
     Method:   Renderer::AddPointLight
     Summary:  Add a point light
     Args:     size_t index
                 Index of the point light
               const std::shared_ptr<PointLight>& pointLight
                 Shared pointer to the point light object
     Modifies: [m_aPointLights].
     Returns:  HRESULT
                 Status code.
   M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    HRESULT Renderer::AddPointLight(_In_ size_t index, _In_ const std::shared_ptr<PointLight>& pPointLight)
    {
       /* When the index exceeds the size of possible lights, it returns E_FAIL
        else, add the light to designated index*/
        if (index >= NUM_LIGHTS)
        {
            return E_FAIL;
        }
        m_aPointLights[index] = pPointLight;
        
        return S_OK;
        
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddVertexShader
      Summary:  Add the vertex shader into the renderer
      Args:     PCWSTR pszVertexShaderName
                  Key of the vertex shader
                const std::shared_ptr<VertexShader>&
                  Vertex shader to add
      Modifies: [m_vertexShaders].
      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    
    HRESULT Renderer::AddVertexShader(_In_ PCWSTR pszVertexShaderName, _In_ const std::shared_ptr<VertexShader>& vertexShader)
    {
        if (m_vertexShaders.contains(pszVertexShaderName))
        {
            return E_FAIL;
        }
        else
        {
            m_vertexShaders.insert(std::make_pair(pszVertexShaderName, vertexShader));

            return S_OK;
        }
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddPixelShader
      Summary:  Add the pixel shader into the renderer
      Args:     PCWSTR pszPixelShaderName
                  Key of the pixel shader
                const std::shared_ptr<PixelShader>&
                  Pixel shader to add
      Modifies: [m_pixelShaders].
      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    
    HRESULT Renderer::AddPixelShader(_In_ PCWSTR pszPixelShaderName, _In_ const std::shared_ptr<PixelShader>& pixelShader)
    {
        if (m_pixelShaders.contains(pszPixelShaderName))
        {
            return E_FAIL;
        }
        else
        {
            m_pixelShaders.insert(std::make_pair(pszPixelShaderName, pixelShader));

            return S_OK;
        }
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddScene
      Summary:  Add a scene
      Args:     PCWSTR pszSceneName
                  Key of a scene
                const std::filesystem::path& sceneFilePath
                  File path to initialize a scene
      Modifies: [m_scenes].
      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    HRESULT Renderer::AddScene(_In_ PCWSTR pszSceneName, const std::filesystem::path& sceneFilePath)
    {
        if (m_scenes.count(pszSceneName) > 0)
            return E_FAIL;

        m_scenes.insert(std::make_pair(pszSceneName, std::make_shared<Scene>(sceneFilePath)));

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetMainScene
      Summary:  Set the main scene
      Args:     PCWSTR pszSceneName
                  Name of the scene to set as the main scene
      Modifies: [m_pszMainSceneName].
      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    HRESULT Renderer::SetMainScene(_In_ PCWSTR pszSceneName)
    {
        if (m_pszMainSceneName == pszSceneName)
            return E_FAIL;

        m_pszMainSceneName = pszSceneName;

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::HandleInput
      Summary:  Add the pixel shader into the renderer and initialize it
      Args:     const DirectionsInput& directions
                  Data structure containing keyboard input data
                const MouseRelativeMovement& mouseRelativeMovement
                  Data structure containing mouse relative input data
      Modifies: [m_camera].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    
    void Renderer::HandleInput(_In_ const DirectionsInput& directions, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime)
    {
        m_camera.HandleInput(
            directions,
            mouseRelativeMovement,
            deltaTime
        );
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Update
      Summary:  Update the renderables each frame
      Args:     FLOAT deltaTime
                  Time difference of a frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    
    void Renderer::Update(_In_ FLOAT deltaTime)
    {
        std::unordered_map<std::wstring, std::shared_ptr<Renderable>>::iterator it_renderables;

        for (it_renderables = m_renderables.begin(); it_renderables != m_renderables.end(); it_renderables++)
        {
            it_renderables->second->Update(deltaTime);
        }

        m_camera.Update(deltaTime);

        for (int i = 0; i < NUM_LIGHTS; ++i)
        {
            m_aPointLights[i]->Update(deltaTime);
        }
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Render
      Summary:  Render the frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    
    void Renderer::Render()
    {
        // Just clear the backbuffer
        m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::MidnightBlue);

        // Clear depth stencil view
        // Clear the depth buffer
        m_immediateContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

        // Create camera constant buffer and update
        CBChangeOnCameraMovement cb_view = {
               .View = XMMatrixTranspose(m_camera.GetView()),
        };

        XMStoreFloat4(&cb_view.CameraPosition, m_camera.GetEye());
        
        m_immediateContext->UpdateSubresource(m_camera.GetConstantBuffer().Get(), 0, nullptr, &cb_view, 0, 0);

        CBLights cbLight = {
              .LightPositions = {},
              .LightColors = {}
        };

        for (int i = 0; i < NUM_LIGHTS; i++)
        {
            cbLight.LightPositions[i] = m_aPointLights[i]->GetPosition();
            cbLight.LightColors[i] = m_aPointLights[i]->GetColor();
        }

        m_immediateContext->UpdateSubresource(m_cbLights.Get(), 0, nullptr, &cbLight, 0, 0);

        std::unordered_map<std::wstring, std::shared_ptr<Renderable>>::iterator it_renderables;

        // for each renderables
        for (it_renderables = m_renderables.begin(); it_renderables != m_renderables.end(); it_renderables++)
        {
            // Set the vertex buffer, index buffer, and the input layout

            // Set vertex buffer
            UINT uStride = sizeof(SimpleVertex);
            UINT uOffset = 0;

            m_immediateContext->IASetVertexBuffers(
                0u,             // the first input slot for binding
                1u,             // the number of buffers in the array
                it_renderables->second->GetVertexBuffer().GetAddressOf(), // the array of vertex buffers
                &uStride,       // array of stride values, one for each buffer
                &uOffset        // array of offset values, one for each buffer
            );

            // Set index buffer
            m_immediateContext->IASetIndexBuffer(
                it_renderables->second->GetIndexBuffer().Get(),
                DXGI_FORMAT_R16_UINT,
                0
            );

            // Set input layout
            m_immediateContext->IASetInputLayout(it_renderables->second->GetVertexLayout().Get());



            // Update constant buffer
            //   You must transpose the matrices when passing them to GPU!!
            //   XMMATRIX is a row - major matrix, however HLSL expects column - major matrix

            // Create renderable constant buffer and update
            CBChangesEveryFrame cb_world =
            {
                .World = XMMatrixTranspose(it_renderables->second->GetWorldMatrix()),
                .OutputColor = it_renderables->second->GetOutputColor()
            };

            m_immediateContext->UpdateSubresource(it_renderables->second->GetConstantBuffer().Get(), 0, nullptr, &cb_world, 0, 0);


            // Set shadersand constant buffers, shader resources, and samplers

            // Set vertex shader
            m_immediateContext->VSSetShader(
                it_renderables->second->GetVertexShader().Get(),
                nullptr,
                0
            );

            // VS set
            m_immediateContext->VSSetConstantBuffers(
                0, 
                1,
                m_camera.GetConstantBuffer().GetAddressOf()
            );
            m_immediateContext->VSSetConstantBuffers(
                1, 
                1,
                m_cbChangeOnResize.GetAddressOf()
            );
            m_immediateContext->VSSetConstantBuffers(
                2, 
                1, 
                it_renderables->second->GetConstantBuffer().GetAddressOf()
            );

            // Set pixel shader
            m_immediateContext->PSSetShader(
                it_renderables->second->GetPixelShader().Get(),
                nullptr,
                0u
            );

            // PS set

            m_immediateContext->PSSetConstantBuffers(
                0,
                1,
                m_camera.GetConstantBuffer().GetAddressOf()
            );

            m_immediateContext->PSSetConstantBuffers(
                2,
                1,
                it_renderables->second->GetConstantBuffer().GetAddressOf()
            );

            m_immediateContext->PSSetConstantBuffers(
                3,
                1,
                m_cbLights.GetAddressOf()
            );
            

            if (it_renderables->second->HasTexture())
            {
                for (UINT i = 0u; i < it_renderables->second->GetNumMeshes(); ++i)
                {
                    const UINT materialIndex = it_renderables->second->GetMesh(i).uMaterialIndex;
                    if (it_renderables->second->GetMaterial(materialIndex).pDiffuse)
                    {
                        // Set texture resource view of the renderable into the pixel shader
                        m_immediateContext->PSSetShaderResources(0u, 1u, it_renderables->second->GetMaterial(materialIndex).pDiffuse->GetTextureResourceView().GetAddressOf());

                        // Set sampler state of the renderable into the pixel shader
                        m_immediateContext->PSSetSamplers(0u, 1u, it_renderables->second->GetMaterial(materialIndex).pDiffuse->GetSamplerState().GetAddressOf());
                    }

                    // Render the triangles
                    m_immediateContext->DrawIndexed(it_renderables->second->GetMesh(i).uNumIndices,
                        it_renderables->second->GetMesh(i).uBaseIndex,
                        it_renderables->second->GetMesh(i).uBaseVertex);
                }
            }
            else
            {
                // draw
                m_immediateContext->DrawIndexed(it_renderables->second->GetNumIndices(), 0, 0);
            }
            
            

            

        }

        // for voxels
        std::vector<std::shared_ptr<Voxel>>::iterator it_voxels;

        for (it_voxels = m_scenes.find(m_pszMainSceneName)->second->GetVoxels().begin(); it_voxels != m_scenes.find(m_pszMainSceneName)->second->GetVoxels().end(); it_voxels++)
        {
            // Set the vertex buffer, index buffer, and the input layout

            // Set vertex buffer
            UINT uStride = sizeof(SimpleVertex);
            UINT uOffset = 0;

            m_immediateContext->IASetVertexBuffers(
                0u,             // the first input slot for binding
                1u,             // the number of buffers in the array
                it_voxels->get()->GetVertexBuffer().GetAddressOf(), // the array of vertex buffers
                &uStride,       // array of stride values, one for each buffer
                &uOffset        // array of offset values, one for each buffer
            );

            uStride = sizeof(InstanceData);
            m_immediateContext->IASetVertexBuffers(
                1u, 
                1u, 
                it_voxels->get()->GetInstanceBuffer().GetAddressOf(),
                &uStride,
                &uOffset
            );

            // Set index buffer
            m_immediateContext->IASetIndexBuffer(
                it_voxels->get()->GetIndexBuffer().Get(),
                DXGI_FORMAT_R16_UINT,
                0
            );

            // Set input layout
            m_immediateContext->IASetInputLayout(it_voxels->get()->GetVertexLayout().Get());



            // Update constant buffer
            //   You must transpose the matrices when passing them to GPU!!
            //   XMMATRIX is a row - major matrix, however HLSL expects column - major matrix

            // Create renderable constant buffer and update
            CBChangesEveryFrame cb_world =
            {
                .World = XMMatrixTranspose(it_voxels->get()->GetWorldMatrix()),
                .OutputColor = it_voxels->get()->GetOutputColor()
            };

            m_immediateContext->UpdateSubresource(it_voxels->get()->GetConstantBuffer().Get(), 0, nullptr, &cb_world, 0, 0);


            // Set shadersand constant buffers, shader resources, and samplers

            // Set vertex shader
            m_immediateContext->VSSetShader(
                it_voxels->get()->GetVertexShader().Get(),
                nullptr,
                0u
            );

            // VS set
            m_immediateContext->VSSetConstantBuffers(
                0,
                1,
                m_camera.GetConstantBuffer().GetAddressOf()
            );
            m_immediateContext->VSSetConstantBuffers(
                1,
                1,
                m_cbChangeOnResize.GetAddressOf()
            );
            m_immediateContext->VSSetConstantBuffers(
                2,
                1,
                it_voxels->get()->GetConstantBuffer().GetAddressOf()
            );

            // Set pixel shader
            m_immediateContext->PSSetShader(
                it_voxels->get()->GetPixelShader().Get(),
                nullptr,
                0u
            );

            // PS set

            m_immediateContext->PSSetConstantBuffers(
                0,
                1,
                m_camera.GetConstantBuffer().GetAddressOf()
            );

            m_immediateContext->PSSetConstantBuffers(
                2,
                1,
                it_voxels->get()->GetConstantBuffer().GetAddressOf()
            );

            m_immediateContext->PSSetConstantBuffers(
                3,
                1,
                m_cbLights.GetAddressOf()
            );


            if (it_voxels->get()->HasTexture())
            {
                for (UINT i = 0u; i < it_voxels->get()->GetNumMeshes(); ++i)
                {
                    const UINT materialIndex = it_voxels->get()->GetMesh(i).uMaterialIndex;
                    if (it_voxels->get()->GetMaterial(materialIndex).pDiffuse)
                    {
                        // Set texture resource view of the renderable into the pixel shader
                        m_immediateContext->PSSetShaderResources(0u, 1u, it_voxels->get()->GetMaterial(materialIndex).pDiffuse->GetTextureResourceView().GetAddressOf());

                        // Set sampler state of the renderable into the pixel shader
                        m_immediateContext->PSSetSamplers(0u, 1u, it_voxels->get()->GetMaterial(materialIndex).pDiffuse->GetSamplerState().GetAddressOf());
                    }

                    // Render the triangles
                    m_immediateContext->DrawIndexed(it_voxels->get()->GetMesh(i).uNumIndices,
                        it_voxels->get()->GetMesh(i).uBaseIndex,
                        it_voxels->get()->GetMesh(i).uBaseVertex);
                }
            }
            else
            {
                // draw
                m_immediateContext->DrawIndexedInstanced(it_voxels->get()->GetNumIndices(), it_voxels->get()->GetNumInstances(), 0, 0, 0);
            }





        }


        m_swapChain->Present(0, 0);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetVertexShaderOfModel
      Summary:  Sets the pixel shader for a model
      Args:     PCWSTR pszModelName
                  Key of the model
                PCWSTR pszVertexShaderName
                  Key of the vertex shader
      Modifies: [m_renderables].
      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::SetVertexShaderOfModel definition (remove the comment)
    --------------------------------------------------------------------*/

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetPixelShaderOfModel
      Summary:  Sets the pixel shader for a model
      Args:     PCWSTR pszModelName
                  Key of the model
                PCWSTR pszPixelShaderName
                  Key of the pixel shader
      Modifies: [m_renderables].
      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::SetPixelShaderOfModel definition (remove the comment)
    --------------------------------------------------------------------*/


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetVertexShaderOfRenderable
      Summary:  Sets the vertex shader for a renderable
      Args:     PCWSTR pszRenderableName
                  Key of the renderable
                PCWSTR pszVertexShaderName
                  Key of the vertex shader
      Modifies: [m_renderables].
      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    
    HRESULT Renderer::SetVertexShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszVertexShaderName)
    {
        if (!m_renderables.contains(pszRenderableName))
        {
            return E_FAIL;
        }
        else
        {
            if (m_vertexShaders.contains(pszVertexShaderName))
            {
                m_renderables.find(pszRenderableName)->second->SetVertexShader(m_vertexShaders.find(pszVertexShaderName)->second);
                return S_OK;
            }

            return E_FAIL;

        }
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetPixelShaderOfRenderable
      Summary:  Sets the pixel shader for a renderable
      Args:     PCWSTR pszRenderableName
                  Key of the renderable
                PCWSTR pszPixelShaderName
                  Key of the pixel shader
      Modifies: [m_renderables].
      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    
    HRESULT Renderer::SetPixelShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszPixelShaderName)
    {
        if (!m_renderables.contains(pszRenderableName))
        {
            return E_FAIL;
        }
        else
        {
            if (m_vertexShaders.contains(pszPixelShaderName))
            {
                m_renderables.find(pszRenderableName)->second->SetPixelShader(m_pixelShaders.find(pszPixelShaderName)->second);
                return S_OK;
            }

            return E_FAIL;

        }
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetVertexShaderOfScene
      Summary:  Sets the vertex shader for the voxels in a scene
      Args:     PCWSTR pszSceneName
                  Key of the scene
                PCWSTR pszVertexShaderName
                  Key of the vertex shader
      Modifies: [m_scenes].
      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    HRESULT Renderer::SetVertexShaderOfScene(_In_ PCWSTR pszSceneName, _In_ PCWSTR pszVertexShaderName)
    {
        if (!m_scenes.contains(pszSceneName))
        {
            return E_FAIL;
        }
        else
        {
            if (m_vertexShaders.contains(pszVertexShaderName))
            {
                for (UINT i = 0u; i < m_scenes.find(pszSceneName)->second->GetVoxels().size(); ++i)
                {
                    m_scenes.find(pszSceneName)->second->GetVoxels()[i]->SetVertexShader(m_vertexShaders[pszVertexShaderName]);
                }
                return S_OK;
            }

            return E_FAIL;

        }

    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetPixelShaderOfScene
      Summary:  Sets the pixel shader for the voxels in a scene
      Args:     PCWSTR pszRenderableName
                  Key of the renderable
                PCWSTR pszPixelShaderName
                  Key of the pixel shader
      Modifies: [m_renderables].
      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    HRESULT Renderer::SetPixelShaderOfScene(_In_ PCWSTR pszSceneName, _In_ PCWSTR pszPixelShaderName)
    {
        if (!m_scenes.contains(pszSceneName))
        {
            return E_FAIL;
        }
        else
        {
            if (m_pixelShaders.contains(pszPixelShaderName))
            {
                for (UINT i = 0u; i < m_scenes.find(pszSceneName)->second->GetVoxels().size(); ++i)
                {
                    m_scenes.find(pszSceneName)->second->GetVoxels()[i]->SetPixelShader(m_pixelShaders[pszPixelShaderName]);
                }
                return S_OK;
            }

            return E_FAIL;

        }
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::GetDriverType
      Summary:  Returns the Direct3D driver type
      Returns:  D3D_DRIVER_TYPE
                  The Direct3D driver type used
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    
    D3D_DRIVER_TYPE Renderer::GetDriverType() const
    {
        return m_driverType;
    }

}