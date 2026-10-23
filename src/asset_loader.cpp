#include "asset_loader.hpp"
#include "game.hpp"

void loadTexturedAsset(const std::string& filename, Diligent::RefCntAutoPtr<Diligent::IRenderDevice>& renderDevice,
                       Diligent::RefCntAutoPtr<Diligent::IDeviceContext>& immediateContext, void* vertexBuf, void* indexBuf) {

    Diligent::GLTF::ModelCreateInfo modelCreateInfo;
    modelCreateInfo.FileName = filename.c_str();

    // to later transfer ownership to renderer
    std::unique_ptr<Diligent::GLTF::Model> model = std::make_unique<Diligent::GLTF::Model>(renderDevice, immediateContext, modelCreateInfo);

    // Both POSITION and TEXCOORD_0 have been loaded to vertex buffer at index 0, use that
    vertexBuf = model->GetVertexBuffer(0, renderDevice, immediateContext);

    indexBuf = model->GetIndexBuffer();

    /* Get textures */

    for (int i = 0; i < model->GetTextureCount(); ++i) {
        g_pRenderer->pushSceneTexture(model->GetTexture(i, renderDevice, immediateContext));
    }

    // now must cache required data from these iterators
    for (const Diligent::GLTF::Mesh& mesh : model->Meshes) {

        std::vector<PrimitiveData> primitives;

        for (const Diligent::GLTF::Primitive& primitive : mesh.Primitives) {
            primitives.push_back(PrimitiveData {
                .texID      = (uint32_t)model->Materials[primitive.MaterialId].GetTextureId(Diligent::GLTF::DefaultBaseColorTextureAttribId),
                .indexCount = primitive.IndexCount,
                .firstIndex = primitive.FirstIndex
            });
        }

        g_pRenderer->pushSceneMesh(MeshData {
            .primitives  = primitives,
            // .boundingBox = mesh.BB // will be used later for collision detection
        });
    }

    // vertex buffers, index buffer and textures now loaded, proceed to render scene glTF
}

void loadTexturelessAsset(const std::string& filename, Diligent::RefCntAutoPtr<Diligent::IRenderDevice>& renderDevice,
                          Diligent::RefCntAutoPtr<Diligent::IDeviceContext>& immediateContext, void* vertexBuf, void* indexBuf,
                          const std::unordered_map<std::string, std::string>& textureMapping) {

    Diligent::GLTF::ModelCreateInfo modelCreateInfo;
    modelCreateInfo.FileName = filename.c_str();

    // to later transfer ownership to renderer
    std::unique_ptr<Diligent::GLTF::Model> model = std::make_unique<Diligent::GLTF::Model>(renderDevice, immediateContext, modelCreateInfo);

    // Both POSITION and TEXCOORD_0 have been loaded to vertex buffer at index 0, use that
    vertexBuf = model->GetVertexBuffer(0, renderDevice, immediateContext);

    indexBuf = model->GetIndexBuffer();

    /* Get textures */

    std::unordered_map<std::string_view, uint32_t> idMapping;

    if (model->GetTextureCount() > 0) throw std::runtime_error("Attempted to load a textured glTF as a textureless glTF");
    for (const auto& [key, filepath] : textureMapping) {
        uint32_t texID = g_pRenderer->emplaceSceneTexture(filepath);
        idMapping.emplace(key, texID);
    }

    // now must cache required data from these iterators
    for (const Diligent::GLTF::Mesh& mesh : model->Meshes) {

        std::vector<PrimitiveData> primitives;

        for (const Diligent::GLTF::Primitive& primitive : mesh.Primitives) {
            primitives.push_back(PrimitiveData {
                .texID      = idMapping[mesh.Name],
                .indexCount = primitive.IndexCount,
                .firstIndex = primitive.FirstIndex
            });
        }

        g_pRenderer->pushSceneMesh(MeshData {
            .primitives  = primitives,
            // .boundingBox = mesh.BB // will be used later for collision detection
        });
    }
    // vertex buffers, index buffer and textures now loaded, proceed to render scene glTF
}
