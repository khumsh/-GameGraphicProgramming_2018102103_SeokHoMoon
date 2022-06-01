#include "Renderer/Renderer.h"

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Renderer
      Summary:  Constructor
      Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
                  m_immediateContext, m_immediateContext1, m_swapChain,
                  m_swapChain1, m_renderTargetView, m_depthStencil,
                  m_depthStencilView, m_cbChangeOnResize, m_cbShadowMatrix,
                  m_pszMainSceneName, m_camera, m_projection, m_scenes
                  m_invalidTexture, m_shadowMapTexture, m_shadowVertexShader,
                  m_shadowPixelShader].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::Renderer definition (remove the comment)
    --------------------------------------------------------------------*/
    Renderer::Renderer()
        : m_driverType(D3D_DRIVER_TYPE_NULL)
        , m_featureLevel(D3D_FEATURE_LEVEL_11_0)
        , m_d3dDevice()
        , m_d3dDevice1()
        , m_immediateContext()
        , m_immediateContext1()
        , m_swapChain()
        , m_swapChain1()
        , m_renderTargetView()
        , m_depthStencil()
        , m_depthStencilView()
        , m_cbChangeOnResize()
        , m_pszMainSceneName(nullptr)
        , m_padding{ '\0' }
        , m_camera(XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f))
        , m_projection()
        , m_scenes()
        , m_invalidTexture(std::make_shared<Texture>(L"Content/Common/InvalidTexture.png"))
    {
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
     Method:   Renderer::Initialize
     Summary:  Creates Direct3D device and swap chain
     Args:     HWND hWnd
                 Handle to the window
     Modifies: [m_d3dDevice, m_featureLevel, m_immediateContext,
                 m_d3dDevice1, m_immediateContext1, m_swapChain1,
                 m_swapChain, m_renderTargetView, m_vertexShader,
                 m_vertexLayout, m_pixelShader, m_vertexBuffer
                 m_cbShadowMatrix].
     Returns:  HRESULT
                 Status code
   M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
   /*--------------------------------------------------------------------
     TODO: Renderer::Initialize definition (remove the comment)
   --------------------------------------------------------------------*/
    HRESULT Renderer::Initialize(_In_ HWND hWnd)
    {
        HRESULT hr = S_OK;

        RECT rc;
        GetClientRect(hWnd, &rc);
        UINT uWidth = static_cast<UINT>(rc.right - rc.left);
        UINT uHeight = static_cast<UINT>(rc.bottom - rc.top);

        UINT uCreateDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(DEBUG) || defined(_DEBUG)
        uCreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

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
            hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, featureLevels, numFeatureLevels,
                D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

            if (hr == E_INVALIDARG)
            {
                // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                    D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());
            }

            if (SUCCEEDED(hr))
            {
                break;
            }
        }
        if (FAILED(hr))
        {
            return hr;
        }

        // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
        ComPtr<IDXGIFactory1> dxgiFactory;
        {
            ComPtr<IDXGIDevice> dxgiDevice;
            hr = m_d3dDevice.As(&dxgiDevice);
            if (SUCCEEDED(hr))
            {
                ComPtr<IDXGIAdapter> adapter;
                hr = dxgiDevice->GetAdapter(&adapter);
                if (SUCCEEDED(hr))
                {
                    hr = adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
                }
            }
        }
        if (FAILED(hr))
        {
            return hr;
        }

        // Create swap chain
        ComPtr<IDXGIFactory2> dxgiFactory2;
        hr = dxgiFactory.As(&dxgiFactory2);
        if (SUCCEEDED(hr))
        {
            // DirectX 11.1 or later
            hr = m_d3dDevice.As(&m_d3dDevice1);
            if (SUCCEEDED(hr))
            {
                m_immediateContext.As(&m_immediateContext1);
            }

            DXGI_SWAP_CHAIN_DESC1 sd =
            {
                .Width = uWidth,
                .Height = uHeight,
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                .SampleDesc = {.Count = 1u, .Quality = 0u },
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1u
            };

            hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &sd, nullptr, nullptr, m_swapChain1.GetAddressOf());
            if (SUCCEEDED(hr))
            {
                hr = m_swapChain1.As(&m_swapChain);
            }
        }
        else
        {
            // DirectX 11.0 systems
            DXGI_SWAP_CHAIN_DESC sd =
            {
                .BufferDesc = {.Width = uWidth, .Height = uHeight, .RefreshRate = {.Numerator = 60, .Denominator = 1 }, .Format = DXGI_FORMAT_R8G8B8A8_UNORM },
                .SampleDesc = {.Count = 1, .Quality = 0 },
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1u,
                .OutputWindow = hWnd,
                .Windowed = TRUE
            };

            hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf());
        }

        // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
        dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

        if (FAILED(hr))
        {
            return hr;
        }

        // Create a render target view
        ComPtr<ID3D11Texture2D> pBackBuffer;
        hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        if (FAILED(hr))
        {
            return hr;
        }

        hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // Create depth stencil texture
        D3D11_TEXTURE2D_DESC descDepth =
        {
            .Width = uWidth,
            .Height = uHeight,
            .MipLevels = 1u,
            .ArraySize = 1u,
            .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
            .SampleDesc = {.Count = 1u, .Quality = 0u },
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_DEPTH_STENCIL,
            .CPUAccessFlags = 0u,
            .MiscFlags = 0u
        };
        hr = m_d3dDevice->CreateTexture2D(&descDepth, nullptr, m_depthStencil.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // Create the depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV =
        {
            .Format = descDepth.Format,
            .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
            .Texture2D = {.MipSlice = 0 }
        };
        hr = m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &descDSV, m_depthStencilView.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

        // Setup the viewport
        D3D11_VIEWPORT vp =
        {
            .TopLeftX = 0.0f,
            .TopLeftY = 0.0f,
            .Width = static_cast<FLOAT>(uWidth),
            .Height = static_cast<FLOAT>(uHeight),
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f,
        };
        m_immediateContext->RSSetViewports(1, &vp);

        // Set primitive topology
        m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Create the constant buffers
        D3D11_BUFFER_DESC bd =
        {
            .ByteWidth = sizeof(CBChangeOnResize),
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
            .CPUAccessFlags = 0
        };
        hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbChangeOnResize.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // Initialize the projection matrix
        m_projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, static_cast<FLOAT>(uWidth) / static_cast<FLOAT>(uHeight), 0.01f, 1000.0f);

        CBChangeOnResize cbChangesOnResize =
        {
            .Projection = XMMatrixTranspose(m_projection)
        };
        m_immediateContext->UpdateSubresource(m_cbChangeOnResize.Get(), 0, nullptr, &cbChangesOnResize, 0, 0);

        bd.ByteWidth = sizeof(CBLights);
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = 0u;

        hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbLights.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        m_camera.Initialize(m_d3dDevice.Get());

        if (!m_scenes.contains(m_pszMainSceneName))
        {
            return E_FAIL;
        }

        hr = m_scenes[m_pszMainSceneName]->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
        if (FAILED(hr))
        {
            return hr;
        }

        hr = m_invalidTexture->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
        if (FAILED(hr))
        {
            return hr;
        }

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddScene
      Summary:  Add scene to renderer
      Args:     PCWSTR pszSceneName
                  The name of the scene
                const std::shared_ptr<Scene>&
                  The shared pointer to Scene
      Modifies: [m_scenes].
      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddScene(_In_ PCWSTR pszSceneName, _In_ const std::shared_ptr<Scene>& scene)
    {
        if (m_scenes.contains(pszSceneName))
        {
            return E_FAIL;
        }

        m_scenes[pszSceneName] = scene;

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::GetSceneOrNull
      Summary:  Return scene with the given name or null
      Args:     PCWSTR pszSceneName
                  The name of the scene
      Returns:  std::shared_ptr<Scene>
                  The shared pointer to Scene
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    std::shared_ptr<Scene> Renderer::GetSceneOrNull(_In_ PCWSTR pszSceneName)
    {
        if (m_scenes.contains(pszSceneName))
        {
            return m_scenes[pszSceneName];
        }

        return nullptr;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetMainScene
      Summary:  Set the main scene
      Args:     PCWSTR pszSceneName
                  The name of the scene
      Modifies: [m_pszMainSceneName].
      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    HRESULT Renderer::SetMainScene(_In_ PCWSTR pszSceneName)
    {
        if (!m_scenes.contains(pszSceneName))
        {
            return E_FAIL;
        }

        m_pszMainSceneName = pszSceneName;

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::HandleInput
      Summary:  Handle user mouse input
      Args:     DirectionsInput& directions
                MouseRelativeMovement& mouseRelativeMovement
                FLOAT deltaTime
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    
    void Renderer::HandleInput(_In_ const DirectionsInput& directions, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime)
    {
        m_camera.HandleInput(directions, mouseRelativeMovement, deltaTime);
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Update
      Summary:  Update the renderables each frame
      Args:     FLOAT deltaTime
                  Time difference of a frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::Update(_In_ FLOAT deltaTime)
    {
        m_scenes[m_pszMainSceneName]->Update(deltaTime);

        m_camera.Update(deltaTime);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
     Method:   Renderer::Render
     Summary:  Render the frame
   M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
   /*--------------------------------------------------------------------
     TODO: Renderer::Render definition (remove the comment)
   --------------------------------------------------------------------*/

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

        for (UINT i = 0; i < NUM_LIGHTS; i++)
        {
            cbLight.LightPositions[i] = m_scenes[m_pszMainSceneName]->GetPointLight(i)->GetPosition();
            cbLight.LightColors[i] = m_scenes[m_pszMainSceneName]->GetPointLight(i)->GetColor();
        }

        m_immediateContext->UpdateSubresource(m_cbLights.Get(), 0, nullptr, &cbLight, 0, 0);

        std::unordered_map<std::wstring, std::shared_ptr<Renderable>>::iterator it_renderables;

        // for each renderables
        for (it_renderables = m_scenes[m_pszMainSceneName]->GetRenderables().begin(); it_renderables != m_scenes[m_pszMainSceneName]->GetRenderables().end(); it_renderables++)
        {
            
            // Set the vertex buffer, index buffer, and the input layout

            // Set vertex buffer
            UINT uStrides[2] = { sizeof(SimpleVertex), sizeof(NormalData) };
            UINT uOffsets[2] = { 0, 0 };

            ComPtr<ID3D11Buffer> vertexNormalBuffers[2] = { it_renderables->second->GetVertexBuffer(), it_renderables->second->GetNormalBuffer() };

            m_immediateContext->IASetVertexBuffers(
                0u,             // the first input slot for binding
                2u,             // the number of buffers in the array
                vertexNormalBuffers->GetAddressOf(), // the array of vertex buffers
                uStrides,       // array of stride values, one for each buffer
                uOffsets        // array of offset values, one for each buffer
            );

            // Set normal buffer
            //uStride = sizeof(NormalData);
            //m_immediateContext->IASetVertexBuffers(1u, 1u, it_renderables->second->GetNormalBuffer().GetAddressOf(), &uStride, &uOffset);

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
                .OutputColor = it_renderables->second->GetOutputColor(),
                .HasNormalMap = it_renderables->second->HasNormalMap()
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
                    if (it_renderables->second->GetMaterial(materialIndex)->pDiffuse)
                    {
                        // Set texture resource view of the renderable into the pixel shader
                        m_immediateContext->PSSetShaderResources(0u, 1u, it_renderables->second->GetMaterial(materialIndex)->pDiffuse->GetTextureResourceView().GetAddressOf());

                        // Set sampler state of the renderable into the pixel shader
                        m_immediateContext->PSSetSamplers(0u, 1u, it_renderables->second->GetMaterial(materialIndex)->pDiffuse->GetSamplerState().GetAddressOf());
                    }

                    if (it_renderables->second->GetMaterial(materialIndex)->pNormal)
                    {
                        // Set texture resource view of the renderable into the pixel shader
                        m_immediateContext->PSSetShaderResources(1u, 1u, it_renderables->second->GetMaterial(materialIndex)->pNormal->GetTextureResourceView().GetAddressOf());

                        // Set sampler state of the renderable into the pixel shader
                        m_immediateContext->PSSetSamplers(1u, 1u, it_renderables->second->GetMaterial(materialIndex)->pNormal->GetSamplerState().GetAddressOf());
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

        


        std::vector<std::shared_ptr<Voxel>>::iterator voxels;
        for (voxels = m_scenes[m_pszMainSceneName]->GetVoxels().begin(); voxels != m_scenes[m_pszMainSceneName]->GetVoxels().end(); voxels++)
        {
            UINT strides[3] = { sizeof(SimpleVertex), sizeof(NormalData), sizeof(InstanceData) };
            UINT offsets[3] = { 0, 0, 0 };

            ComPtr<ID3D11Buffer> vertexInstanceBuffers[3] = { voxels->get()->GetVertexBuffer(), voxels->get()->GetNormalBuffer(), voxels->get()->GetInstanceBuffer() };

            m_immediateContext->IASetVertexBuffers(
                0,
                3,
                vertexInstanceBuffers->GetAddressOf(),
                strides, 
                offsets);

            m_immediateContext->IASetIndexBuffer(
                voxels->get()->GetIndexBuffer().Get(),
                DXGI_FORMAT_R16_UINT,
                0
            );
            m_immediateContext->IASetInputLayout(
                voxels->get()->GetVertexLayout().Get()
            );

            CBChangesEveryFrame cb = {
                .World = XMMatrixTranspose(voxels->get()->GetWorldMatrix()),
                .OutputColor = voxels->get()->GetOutputColor(),
                .HasNormalMap = voxels->get()->HasNormalMap()
            };

            m_immediateContext->UpdateSubresource(
                voxels->get()->GetConstantBuffer().Get(),
                0,
                nullptr,
                &cb,
                0,
                0
            );
            m_immediateContext->VSSetShader(
                voxels->get()->GetVertexShader().Get(),
                nullptr,
                0
            );


            m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(2, 1, voxels->get()->GetConstantBuffer().GetAddressOf());

            m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(2, 1, voxels->get()->GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

            m_immediateContext->PSSetShader(voxels->get()->GetPixelShader().Get(), nullptr, 0);


            if (voxels->get()->HasTexture())
            {
                ComPtr<ID3D11ShaderResourceView> shaderResources[2] = { voxels->get()->GetMaterial(0)->pDiffuse->GetTextureResourceView(),
                                                voxels->get()->GetMaterial(0)->pNormal->GetTextureResourceView() };
                ComPtr<ID3D11SamplerState> samplerStates[2] = { voxels->get()->GetMaterial(0)->pDiffuse->GetSamplerState(),
                                                voxels->get()->GetMaterial(0)->pNormal->GetSamplerState() };
                m_immediateContext->PSSetShaderResources(0, 2, shaderResources->GetAddressOf());
                m_immediateContext->PSSetSamplers(0, 2, samplerStates->GetAddressOf());
                m_immediateContext->DrawIndexedInstanced(voxels->get()->GetNumIndices(), voxels->get()->GetNumInstances(), 0, 0, 0);

            }
            else
            {
                m_immediateContext->DrawIndexedInstanced(voxels->get()->GetNumIndices(), voxels->get()->GetNumInstances(), 0, 0, 0);
            }

        }
        




        std::unordered_map<std::wstring, std::shared_ptr<Model>>::iterator it_models;

        // for models
        for (it_models = m_scenes[m_pszMainSceneName]->GetModels().begin(); it_models != m_scenes[m_pszMainSceneName]->GetModels().end(); it_models++)
        {
            // Set the vertex buffer, index buffer, and the input layout

            // Set vertex buffer
            UINT aStrides[2] = {
                static_cast<UINT>(sizeof(SimpleVertex)),
                static_cast<UINT>(sizeof(AnimationData)),
            };
            UINT aOffsets[2] = { 0u, 0u };

            ComPtr<ID3D11Buffer> aBuffers[2]
            {
                it_models->second->GetVertexBuffer().Get(),
                it_models->second->GetAnimationBuffer().Get(),
            };

            m_immediateContext->IASetVertexBuffers(
                0u,             // the first input slot for binding
                2u,             // the number of buffers in the array
                aBuffers->GetAddressOf(), // the array of vertex buffers
                aStrides,       // array of stride values, one for each buffer
                aOffsets        // array of offset values, one for each buffer
            );

            // Set index buffer
            m_immediateContext->IASetIndexBuffer(
                it_models->second->GetIndexBuffer().Get(),
                DXGI_FORMAT_R16_UINT,
                0
            );

            // Set input layout
            m_immediateContext->IASetInputLayout(it_models->second->GetVertexLayout().Get());



            // Update constant buffer
            //   You must transpose the matrices when passing them to GPU!!
            //   XMMATRIX is a row - major matrix, however HLSL expects column - major matrix

            // Create renderable constant buffer and update
            CBChangesEveryFrame cb_world =
            {
                .World = XMMatrixTranspose(it_models->second->GetWorldMatrix()),
                .OutputColor = it_models->second->GetOutputColor()
            };

            m_immediateContext->UpdateSubresource(it_models->second->GetConstantBuffer().Get(), 0, nullptr, &cb_world, 0, 0);

            CBSkinning cb_skinning =
            {
                .BoneTransforms = {}
            };

            for (UINT i = 0u; i < it_models->second->GetBoneTransforms().size(); i++)
            {
                cb_skinning.BoneTransforms[i] = XMMatrixTranspose(it_models->second->GetBoneTransforms()[i]);
            }

            m_immediateContext->UpdateSubresource(it_models->second->GetSkinningConstantBuffer().Get(), 0, nullptr, &cb_skinning, 0, 0);

            // Set shadersand constant buffers, shader resources, and samplers

            // Set vertex shader
            m_immediateContext->VSSetShader(
                it_models->second->GetVertexShader().Get(),
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
                it_models->second->GetConstantBuffer().GetAddressOf()
            );
            m_immediateContext->VSSetConstantBuffers(
                4,
                1,
                it_models->second->GetSkinningConstantBuffer().GetAddressOf()
            );

            // Set pixel shader
            m_immediateContext->PSSetShader(
                it_models->second->GetPixelShader().Get(),
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
                it_models->second->GetConstantBuffer().GetAddressOf()
            );

            m_immediateContext->PSSetConstantBuffers(
                3,
                1,
                m_cbLights.GetAddressOf()
            );


            if (it_models->second->HasTexture())
            {
                for (UINT i = 0u; i < it_models->second->GetNumMeshes(); ++i)
                {
                    const UINT materialIndex = it_models->second->GetMesh(i).uMaterialIndex;

                    // Set texture resource view of the renderable into the pixel shader
                    m_immediateContext->PSSetShaderResources(0u, 1u, it_models->second->GetMaterial(materialIndex)->pDiffuse->GetTextureResourceView().GetAddressOf());

                    // Set sampler state of the renderable into the pixel shader
                    m_immediateContext->PSSetSamplers(0u, 1u, it_models->second->GetMaterial(materialIndex)->pDiffuse->GetSamplerState().GetAddressOf());
                    

                    // Render the triangles
                    m_immediateContext->DrawIndexed(it_models->second->GetMesh(i).uNumIndices,
                        it_models->second->GetMesh(i).uBaseIndex,
                        it_models->second->GetMesh(i).uBaseVertex);
                }
            }
            else
            {
                // draw
                m_immediateContext->DrawIndexed(it_models->second->GetNumIndices(), 0, 0);
            }





        }

        


        m_swapChain->Present(0, 0);
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

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetShadowMapShaders
      Summary:  Set shaders for the shadow mapping
      Args:     std::shared_ptr<ShadowVertexShader>
                  vertex shader
                std::shared_ptr<PixelShader>
                  pixel shader
      Modifies: [m_shadowVertexShader, m_shadowPixelShader].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::SetShadowMapShaders(_In_ std::shared_ptr<ShadowVertexShader> vertexShader, _In_ std::shared_ptr<PixelShader> pixelShader)
    {
        m_shadowVertexShader = move(vertexShader);
        m_shadowPixelShader = move(pixelShader);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::RenderSceneToTexture
      Summary:  Render scene to the texture
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::RenderSceneToTexture definition (remove the comment)
    --------------------------------------------------------------------*/
}