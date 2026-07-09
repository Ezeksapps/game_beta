#pragma once

#include "../common.hpp"
#include "camera.hpp"
#include "DiligentCore/Graphics/GraphicsEngine/interface/PipelineState.h"

#include <DiligentCore/Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h>
#include <DiligentCore/Graphics/GraphicsEngineOpenGL/interface/EngineFactoryOpenGL.h>

#include <DiligentTools/TextureLoader/interface/TextureLoader.h>
#include <DiligentTools/TextureLoader/interface/TextureUtilities.h>

#include <DiligentCore/Common/interface/RefCntAutoPtr.hpp> /* CHECK: the advantages given over STL smart pointer don't really matter here */
//#include <DiligentCore/Common/interface/BasicMath.hpp>

#include <DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/RenderPass.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/Framebuffer.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/SwapChain.h>

#include <DiligentCore/Graphics/GraphicsTools/interface/MapHelper.hpp>

#include <DiligentTools/AssetLoader/interface/GLTFLoader.hpp>

#include <DiligentTools/RenderStateNotation/interface/RenderStateNotationLoader.h>

// TODO: Camera math defines its own vec3 type, change this to match diligent's (or change the name)


using namespace glm;
/* ------------------------------ */


struct Sprite {
    vec3 pos; // Position
    int index; // specifically index in tex array
};

class Renderer {

public:

    Renderer();
    ~Renderer();

    bool initRenderer(const Diligent::NativeWindow& window, const Diligent::RENDER_DEVICE_TYPE& deviceType);
    void renderFrame();
    void update();

    void loadGLB(const std::string& filename);
    Sprite loadSprite(const std::string& filename);

    uint32_t m_windowWidth;
    uint32_t m_windowHeight;

private:

    // TODO: UI will need separate PSO

    Diligent::RefCntAutoPtr<Diligent::IEngineFactory> m_pEngineFactory;
    Diligent::RefCntAutoPtr<Diligent::IRenderDevice>  m_pDevice;
    Diligent::RefCntAutoPtr<Diligent::IDeviceContext> m_pImmediateContext;
    Diligent::RefCntAutoPtr<Diligent::ISwapChain>     m_pSwapChain;
    Diligent::NativeWindow                            m_window;

    Diligent::RefCntAutoPtr<Diligent::IRenderPass>    m_pRenderPass;
    Diligent::RefCntAutoPtr<Diligent::IFramebuffer>   m_pFrameBuffer;

    std::unique_ptr<Diligent::GLTF::Model>            m_pGlbModel;

    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_pDebugLineBuffer;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pDebugLinePSO;

    /* ---- Shared UBO, holding matrices for current frame ---- */
    Diligent::RefCntAutoPtr<Diligent::IBuffer>        m_pFrameConstants;

    /* ---- Pipeline state objects (PSOs) ---- */
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pMapPipelineStateObj;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pSpritePipelineStateObj;

    /* ---- Sprite pipeline buffers & textures ---- */
    Diligent::RefCntAutoPtr<Diligent::IBuffer>        m_pSpriteVertexBuffer;
    Diligent::RefCntAutoPtr<Diligent::IBuffer>        m_pSpriteIndexBuffer;
    Diligent::RefCntAutoPtr<Diligent::IBuffer>        m_pSpriteInstanceBuffer;

    Diligent::RefCntAutoPtr<Diligent::ITexture>       m_pSpriteTextureArray;
    Diligent::RefCntAutoPtr<Diligent::ITextureView>   m_pSpriteShaderResourceView;

    /* ---- Map pipeline buffers & textures ---- */
    std::vector<Diligent::IBuffer*>                   m_pMapVertexBuffers;
    std::vector<Diligent::ITexture*>                  m_pMapTextures;
    Diligent::IBuffer*                                m_pMapIndexBuffer;

    Diligent::RefCntAutoPtr<Diligent::ITextureView>   m_pMapShaderResourceView;

    /* Pipeline SRBs */
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding>  m_pMapShaderResourceBinding;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding>  m_pSpriteShaderResourceBinding;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding>  m_pDebugShaderResourceBinding;

    std::unique_ptr<Camera>  m_pCamera;

    /* Matrices */
    mat4 m_projMatrix; // The world space's projection matrix
    mat4 m_viewMatrix; // The camera's matrix

    /* REFERENCE: Vulkan system had two uniform buffers. One for frame constants and one for per-instance transforms
     * Per-instance transforms are handled here by the instancing system, where they are now a mat4 input for the sprites' VS
     * The per-frame UBO exists as m_pFrameConstants, which has proj and view (camera) matrices. The matrices are mapped on render
     */

    std::vector<mat4> m_instanceData; // transform matrices for every billboard instance

    /* Number of sprites game is currently rendering */
    int m_numSprites = 0;
    /* Max number of sprites (instances of billboards) */
    const int m_maxInstances = 32;

    /* NOTE: num sprites setter must not increment sprites past max instances */

    void createSharedUniformBuffer();

    void createMapPipelineState();
    void createSpritePipelineState();

    void createVertexBuffer();
    void createIndexBuffer();
    void createInstanceBuffer();
    void populateInstanceBuffer();

    void createRenderPass();
    void createFrameBuffer();

    void renderMap();
    void renderSprites();
    void renderDebugAxes();

    void updateUniformBuffer();

    void loadTexture(const std::string& filename);

    void createSpriteTextureArray();
};
