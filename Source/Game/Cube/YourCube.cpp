#include "Cube/YourCube.h"

void YourCube::Update(_In_ FLOAT deltaTime)
{
    XMMATRIX mSpin = XMMatrixRotationZ(-1.0f * deltaTime);
    XMMATRIX mOrbit = XMMatrixRotationY(-2.0f * deltaTime);
    XMMATRIX mTranslate = XMMatrixTranslation(-4.0f, 0.0f, 0.0f);
    XMMATRIX mScale = XMMatrixScaling(0.3f, 0.3f, 0.3f);

    m_world = mScale * mSpin * mTranslate * mOrbit;
}