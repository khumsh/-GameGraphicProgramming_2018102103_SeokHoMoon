#include "Cube/CustomCube.h"

void CustomCube::Update(_In_ FLOAT deltaTime)
{
    XMMATRIX mSpin = XMMatrixRotationY(-1.0f * deltaTime);
    XMMATRIX mOrbit = XMMatrixRotationX(2.0f * deltaTime);
    XMMATRIX mTranslate = XMMatrixTranslation(0.0f, 4.0f, 0.0f);
    XMMATRIX mScale = XMMatrixScaling(0.3f, 0.3f, 0.3f);

    m_world = mScale * mSpin * mTranslate * mOrbit;
}