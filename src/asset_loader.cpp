#include "asset_loader.hpp"
#include "game.hpp"

void loadTexturedAsset(const std::string& filename, Diligent::RefCntAutoPtr<Diligent::IRenderDevice>& renderDevice,
                       Diligent::RefCntAutoPtr<Diligent::IDeviceContext>& immediateContext, Diligent::RefCntAutoPtr<Diligent::IBuffer>& vertexBuf,
                       Diligent::RefCntAutoPtr<Diligent::IBuffer>& indexBuf) {

    Diligent::GLTF::ModelCreateInfo modelCreateInfo;
    modelCreateInfo.FileName = filename.c_str();

    // to later transfer ownership to renderer
    std::unique_ptr<Diligent::GLTF::Model> model = std::make_unique<Diligent::GLTF::Model>(renderDevice, immediateContext, modelCreateInfo);

    // Both POSITION and TEXCOORD_0 have been loaded to vertex buffer at index 0, use that
    vertexBuf = model->GetVertexBuffer(0, renderDevice, immediateContext);

    indexBuf = model->GetIndexBuffer();

    /* Get textures */

    for (int i = 0; i < model->GetTextureCount(); ++i) {
        Diligent::ITexture* texture = model->GetTexture(i, renderDevice, immediateContext);
        g_pRenderer->pushSceneTexture(Diligent::RefCntAutoPtr<Diligent::ITexture>{texture});
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
                          Diligent::RefCntAutoPtr<Diligent::IDeviceContext>& immediateContext, Diligent::RefCntAutoPtr<Diligent::IBuffer>& vertexBuf,
                          Diligent::RefCntAutoPtr<Diligent::IBuffer>& indexBuf, const std::unordered_map<std::string, std::string>& textureMapping) {

    Diligent::GLTF::ModelCreateInfo modelCreateInfo;
    modelCreateInfo.FileName = filename.c_str();

    // to later transfer ownership to renderer
    std::unique_ptr<Diligent::GLTF::Model> model = std::make_unique<Diligent::GLTF::Model>(renderDevice, immediateContext, modelCreateInfo);

    // Both POSITION and TEXCOORD_0 have been loaded to vertex buffer at index 0, use that
    vertexBuf = model->GetVertexBuffer(0, renderDevice, immediateContext);

    indexBuf = model->GetIndexBuffer();

    /* Get textures */

    /* Need to:
       not create duplicate textures for the same image
       use the node as the parent for this data
     */

    std::unordered_map<std::string_view, uint32_t> idMapping;

    if (model->GetTextureCount() > 0) throw std::runtime_error("Attempted to load a textured glTF as a textureless glTF");
    for (const auto& [key, filepath] : textureMapping) {
        uint32_t texID = g_pRenderer->emplaceSceneTexture(filepath);
        idMapping.emplace(key, texID);
    }

    // NODE -> MESH -> PRIMITIVE (MULTIPLE)
    // TODO: rename MeshData to NodeData

    for (const Diligent::GLTF::Node& node : model->Nodes) {
        std::cout << node.Name << "\t";
        std::cout << "Node contains mesh with name: " << node.pMesh->Name << std::endl;

        /*
         s td*::cout << "Displaying mesh with AABB defined as:\n"
         << "min coord: [" << mesh.BB.Min.x << ", " << mesh.BB.Min.y << ", " << mesh.BB.Min.z << "]\n"
         << "max coord: [" << mesh.BB.Max.x << ", " << mesh.BB.Max.y << ", " << mesh.BB.Max.z << "]\n";
         */

        std::vector<PrimitiveData> primitives;

        for (const Diligent::GLTF::Primitive& primitive : node.pMesh->Primitives) {
            primitives.push_back(PrimitiveData {
                .texID      = idMapping[node.Name],
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
