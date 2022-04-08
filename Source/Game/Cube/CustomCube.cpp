#include "Cube/CustomCube.h"

void CustomCube::Update(_In_ FLOAT deltaTime)
{
    m_world = XMMatrixRotationY(deltaTime);
}