#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>

#include "common.hpp"

#include <DiligentTools/AssetLoader/interface/GLTFLoader.hpp>

struct PrimitiveData {
    uint32_t texID;
    uint32_t indexCount;
    uint32_t firstIndex;
};

struct MeshData {
    std::vector<PrimitiveData> primitives;
    // Diligent::BoundBox         boundingBox; // Will be important for entity-scene collisions, but not useful for now
};

void loadTexturedAsset(const std::string& filename, Diligent::RefCntAutoPtr<Diligent::IRenderDevice>& renderDevice,
                       Diligent::RefCntAutoPtr<Diligent::IDeviceContext>& immediateContext, void* vertexBuf, void* indexBuf);

void loadTexturelessAsset(const std::string& filename, Diligent::RefCntAutoPtr<Diligent::IRenderDevice>& renderDevice,
                          Diligent::RefCntAutoPtr<Diligent::IDeviceContext>& immediateContext, void* vertexBuf, void* indexBuf,
                          const std::unordered_map<std::string, std::string>& textureMapping);
