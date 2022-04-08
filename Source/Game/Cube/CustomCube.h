#pragma once

#include "Cube/BaseCube.h"

class CustomCube : public BaseCube
{
public:
    CustomCube() = default;
    ~CustomCube() = default;

    virtual void Update(_In_ FLOAT deltaTime) override;
};