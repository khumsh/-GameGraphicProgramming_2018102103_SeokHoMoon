#include "Cube/OriginCube.h"

/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   OriginCube::Cube
  Summary:  Constructor
  Args:     const std::filesystem::path& textureFilePath
              Path to the texture to use
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
OriginCube::OriginCube(const std::filesystem::path& textureFilePath)
    :BaseCube(textureFilePath)
{
}

/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   OriginCube::Update
  Summary:  Updates the cube every frame
  Args:     FLOAT deltaTime
              Elapsed time
  Modifies: [m_world].
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
void OriginCube::Update(_In_ FLOAT deltaTime)
{
    static FLOAT s_totalTime = 0.0f;
    s_totalTime += deltaTime;

    m_world = XMMatrixRotationY(s_totalTime) * XMMatrixTranslation(3.0f, XMScalarSin(s_totalTime), 0.0f);
}