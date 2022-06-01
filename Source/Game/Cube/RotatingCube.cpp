#include "Cube/RotatingCube.h"

RotatingCube::RotatingCube(const XMFLOAT4& outputColor)
    : BaseCube(outputColor)
{
}

void RotatingCube::Update(_In_ FLOAT deltaTime)
{
    // Rotate cube around the origin
    static FLOAT t = 0.0f;
    t += deltaTime;

    XMMATRIX mSpin = XMMatrixRotationZ(-t);
    XMMATRIX mOrbit = XMMatrixRotationY(-t * 2.0f);
    XMMATRIX mTranslate = XMMatrixTranslation(0.0f, 0.0f, -5.0f);
    XMMATRIX mScale = XMMatrixScaling(0.3f, 0.3f, 0.3f);

    m_world = mScale * mSpin * mTranslate * mOrbit;
}

/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   RotatingPointLight::Update
  Summary:  Update every frame
  Args:     FLOAT deltaTime
  Modifies: [m_position, m_eye, m_eye, m_at,
            m_view].
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
/*--------------------------------------------------------------------
  TODO: RotatingPointLight::Update definition (remove the comment)
--------------------------------------------------------------------*/