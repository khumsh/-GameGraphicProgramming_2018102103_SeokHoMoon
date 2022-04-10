#include "Cube/OriginCube.h"

void OriginCube::Update(_In_ FLOAT deltaTime)
{
    m_world = XMMatrixRotationY(deltaTime);
}