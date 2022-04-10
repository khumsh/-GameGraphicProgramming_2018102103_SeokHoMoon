#pragma once

#include "Cube/BaseCube.h"

class OriginCube : public BaseCube
{
public:
    OriginCube() = default;
    ~OriginCube() = default;

    virtual void Update(_In_ FLOAT deltaTime) override;
};