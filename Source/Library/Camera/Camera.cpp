#include "Camera/Camera.h"

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Camera::Camera
      Summary:  Constructor
      Modifies: [m_yaw, m_pitch, m_moveLeftRight, m_moveBackForward,
                 m_moveUpDown, m_travelSpeed, m_rotationSpeed,
                 m_padding, m_cameraForward, m_cameraRight, m_cameraUp,
                 m_eye, m_at, m_up, m_rotation, m_view].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    Camera::Camera(_In_ const XMVECTOR& position)
        : m_yaw(), m_pitch(), m_moveLeftRight(), m_moveBackForward(),
        m_moveUpDown(), m_travelSpeed(), m_rotationSpeed(),
        m_padding(), m_cameraForward(XMVECTOR()), m_cameraRight(XMVECTOR()),
        m_cameraUp(XMVECTOR()), m_eye(position), m_at(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)), m_up(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)),
        m_rotation(XMMATRIX()), m_view(XMMATRIX()), m_cbChangeOnCameraMovement(nullptr)
    {
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Camera::GetEye
      Summary:  Returns the eye vector
      Returns:  const XMVECTOR&
                  The eye vector
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    const XMVECTOR& Camera::GetEye() const
    {
        return m_eye;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Camera::GetAt
      Summary:  Returns the at vector
      Returns:  const XMVECTOR&
                  The at vector
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    
    const XMVECTOR& Camera::GetAt() const
    {
        return m_at;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Camera::GetUp
      Summary:  Returns the up vector
      Returns:  const XMVECTOR&
                  The up vector
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    
    const XMVECTOR& Camera::GetUp() const
    {
        return m_up;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Camera::GetView
      Summary:  Returns the view matrix
      Returns:  const XMMATRIX&
                  The view matrix
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    
    const XMMATRIX& Camera::GetView() const
    {
        return m_view;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Camera::GetConstantBuffer
      Summary:  Returns the constant buffer
      Returns:  ComPtr<ID3D11Buffer>&
                  The constant buffer
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    ComPtr<ID3D11Buffer>& Camera::GetConstantBuffer()
    {
        return m_cbChangeOnCameraMovement;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Camera::HandleInput
      Summary:  Sets the camera state according to the given input
      Args:     const DirectionsInput& directions
                  Keyboard directional input
                const MouseRelativeMovement& mouseRelativeMovement
                  Mouse relative movement input
                FLOAT deltaTime
                  Time difference of a frame
      Modifies: [m_yaw, m_pitch, m_moveLeftRight, m_moveBackForward,
                 m_moveUpDown].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    
    void Camera::HandleInput(_In_ const DirectionsInput& directions, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime)
    {
        m_travelSpeed = 180 * deltaTime;
        m_rotationSpeed = 10 * deltaTime;

        if (directions.bLeft)
        {
            m_moveLeftRight = -1 * m_travelSpeed;

        }
        if (directions.bRight)
        {
            m_moveLeftRight = m_travelSpeed;
        }
        if (directions.bFront)
        {
            m_moveBackForward = m_travelSpeed;
        }
        if (directions.bBack)
        {
            m_moveBackForward = -1 * m_travelSpeed;
        }
        if (directions.bUp)
        {
            m_moveUpDown = m_travelSpeed;
        }
        if (directions.bDown)
        {
            m_moveUpDown = -1 * m_travelSpeed;
        }

        m_pitch = m_pitch + mouseRelativeMovement.Y * m_rotationSpeed;

        m_yaw = m_yaw + mouseRelativeMovement.X * m_rotationSpeed;

        // -XM_PIDIV2 < m_pitch < XM_PIDIV2
        m_pitch = std::clamp(m_pitch, -XM_PIDIV2 + 0.01f, XM_PIDIV2 - 0.01f);

        Update(deltaTime);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Camera::Initialize
      Summary:  Initialize the view matrix constant buffers
      Args:     ID3D11Device* pDevice
                  Pointer to a Direct3D 11 device
      Modifies: [m_cbChangeOnCameraMovement].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    HRESULT Camera::Initialize(_In_ ID3D11Device* device)
    {
        //create constant buffer
        D3D11_BUFFER_DESC constantBd = {
            .ByteWidth = sizeof(CBChangeOnCameraMovement),
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
            .CPUAccessFlags = 0,
            .MiscFlags = 0,
            .StructureByteStride = 0
        };
        HRESULT hr = device->CreateBuffer(&constantBd, nullptr, m_cbChangeOnCameraMovement.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Camera::Update
      Summary:  Updates the camera based on its state
      Args:     FLOAT deltaTime
                  Time difference of a frame
      Modifies: [m_rotation, m_at, m_cameraRight, m_cameraUp,
                 m_cameraForward, m_eye, m_moveLeftRight,
                 m_moveBackForward, m_moveUpDown, m_up, m_view].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    
    void Camera::Update(_In_ FLOAT deltaTime)
    {
        // rotation matrix
        m_rotation = XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, 0);
        m_at = XMVector3TransformCoord(DEFAULT_FORWARD, m_rotation);
        m_at = XMVector3Normalize(m_at);

        // new right / up / forward vector
        XMMATRIX RotateYTempMatrix;
        RotateYTempMatrix = XMMatrixRotationY(m_yaw);

        m_cameraRight = XMVector3TransformCoord(DEFAULT_RIGHT, RotateYTempMatrix);
        m_cameraUp = XMVector3TransformCoord(DEFAULT_UP, RotateYTempMatrix);
        m_cameraForward = XMVector3TransformCoord(DEFAULT_FORWARD, RotateYTempMatrix);

        // new eye, at, up
        m_eye += m_moveLeftRight * m_cameraRight * m_travelSpeed;
        m_eye += m_moveBackForward * m_cameraForward * m_travelSpeed;
        m_eye += m_moveUpDown * m_cameraUp * m_travelSpeed;

        //reset movement
        m_moveLeftRight = 0.0f;
        m_moveBackForward = 0.0f;
        m_moveUpDown = 0.0f;

        m_at = m_eye + m_at;

        // determine the view matrix
        m_view = XMMatrixLookAtLH(m_eye, m_at, m_up);
    }
}