#include "Shader/SkyMapVertexShader.h"

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   SkyMapVertexShader::SkyMapVertexShader

      Summary:  Constructor

      Args:     PCWSTR pszFileName
                  Name of the file that contains the shader code
                PCSTR pszEntryPoint
                  Name of the shader entry point functino where shader
                  execution begins
                PCSTR pszShaderModel
                  Specifies the shader target or set of shader features
                  to compile against
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    SkyMapVertexShader::SkyMapVertexShader(_In_ PCWSTR pszFileName, _In_ PCSTR pszEntryPoint, _In_ PCSTR pszShaderModel)
        :VertexShader(pszFileName, pszEntryPoint, pszShaderModel)
    {

    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   SkyMapVertexShader::Initialize

      Summary:  Initializes the vertex shader and the input layout

      Args:     ID3D11Device* pDevice
                  The Direct3D device to create the vertex shader

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    HRESULT SkyMapVertexShader::Initialize(_In_ ID3D11Device* pDevice)
    {
        HRESULT hr = S_OK;

        ComPtr<ID3DBlob> VSBlob;
        hr = compile(VSBlob.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        hr = pDevice->CreateVertexShader(VSBlob->GetBufferPointer(),
            VSBlob->GetBufferSize(),
            nullptr,
            m_vertexShader.GetAddressOf()
        );
        if (FAILED(hr))
        {
            return hr;
        }

        // Define and Create Input Layout
        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            {
                "POSITION",
                0,
                DXGI_FORMAT_R32G32B32_FLOAT,
                0,
                0,
                D3D11_INPUT_PER_VERTEX_DATA,
                0
            }
        };
        const UINT numElements = ARRAYSIZE(layout);

        hr = pDevice->CreateInputLayout(layout,
            numElements,
            VSBlob->GetBufferPointer(),
            VSBlob->GetBufferSize(),
            &m_vertexLayout
        );
        if (FAILED(hr))
        {
            return hr;
        }

        return hr;
    }
}