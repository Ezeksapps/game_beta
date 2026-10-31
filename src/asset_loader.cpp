#include "asset_loader.hpp"
#include "game.hpp"

std::unique_ptr<Diligent::GLTF::Model> g_model;

void loadTexturedAsset(const std::string& filename, Diligent::RefCntAutoPtr<Diligent::IRenderDevice>& renderDevice,
                       Diligent::RefCntAutoPtr<Diligent::IDeviceContext>& immediateContext, Diligent::RefCntAutoPtr<Diligent::IBuffer>& vertexBuf,
                       Diligent::RefCntAutoPtr<Diligent::IBuffer>& indexBuf) {

    Diligent::GLTF::ModelCreateInfo g_modelCreateInfo;
    g_modelCreateInfo.FileName = filename.c_str();


    g_model = std::make_unique<Diligent::GLTF::Model>(renderDevice, immediateContext, g_modelCreateInfo);

    // Both POSITION and TEXCOORD_0 have been loaded to vertex buffer at index 0, use that
    vertexBuf = g_model->GetVertexBuffer(0, renderDevice, immediateContext);

    indexBuf = g_model->GetIndexBuffer();

    /* Get textures */

    for (int i = 0; i < g_model->GetTextureCount(); ++i) {
        Diligent::ITexture* texture = g_model->GetTexture(i, renderDevice, immediateContext);
        g_pRenderer->pushSceneTexture(Diligent::RefCntAutoPtr<Diligent::ITexture>{texture});
    }

    // now must cache required data from these iterators
    for (const Diligent::GLTF::Mesh& mesh : g_model->Meshes) {

        std::vector<PrimitiveData> primitives;

        for (const Diligent::GLTF::Primitive& primitive : mesh.Primitives) {
            primitives.push_back(PrimitiveData {
                .texID      = (uint32_t)g_model->Materials[primitive.MaterialId].GetTextureId(Diligent::GLTF::DefaultBaseColorTextureAttribId),
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
                          Diligent::RefCntAutoPtr<Diligent::IDeviceContext>& immediateContext, Diligent::RefCntAutoPtr<Diligent::IBuffer>& vertexBuf,
                          Diligent::RefCntAutoPtr<Diligent::IBuffer>& indexBuf, const std::unordered_map<std::string, std::string>& textureMapping) {

    Diligent::GLTF::ModelCreateInfo g_modelCreateInfo;
    g_modelCreateInfo.FileName = filename.c_str();

    g_model = std::make_unique<Diligent::GLTF::Model>(renderDevice, immediateContext, g_modelCreateInfo);

    // Both POSITION and TEXCOORD_0 have been loaded to vertex buffer at index 0, use that
    vertexBuf = g_model->GetVertexBuffer(0, renderDevice, immediateContext);

    indexBuf = g_model->GetIndexBuffer();

    /* Get textures */

    std::unordered_map<std::string_view, uint32_t> idMapping;

    if (g_model->GetTextureCount() > 0) throw std::runtime_error("Attempted to load a textured glTF as a textureless glTF");
    for (const auto& [key, filepath] : textureMapping) {
        uint32_t texID = g_pRenderer->emplaceSceneTexture(filepath); // TODO: figure out how to make this not duplicate textures
        idMapping.emplace(key, texID);
    }

    // NODE -> MESH -> PRIMITIVE (MULTIPLE)
    // TODO: rename MeshData to NodeData

    for (const Diligent::GLTF::Node& node : g_model->Nodes) {

        std::vector<PrimitiveData> primitives;

        for (const Diligent::GLTF::Primitive& primitive : node.pMesh->Primitives) {

            primitives.push_back(PrimitiveData {
                .texID      = idMapping.at(node.Name),
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

vec3 getNodePosition(const std::string& nodeName) {
    auto it = std::find_if(g_model->Nodes.begin(), g_model->Nodes.end(),
                           [nodeName](const Diligent::GLTF::Node& node) { return node.Name == nodeName; });

    if (it == g_model->Nodes.end()) throw std::runtime_error("Node " + nodeName + " not found");

    vec3 min = vec3(it->pMesh->BB.Min.x, it->pMesh->BB.Min.y, it->pMesh->BB.Min.z);
    vec3 max = vec3(it->pMesh->BB.Max.x, it->pMesh->BB.Max.y, it->pMesh->BB.Max.z);
    return (min + max) / 2.0f;
}

