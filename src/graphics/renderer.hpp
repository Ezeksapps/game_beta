#pragma once

#include "../common.hpp"
#include "camera.hpp"
#include "DiligentCore/Graphics/GraphicsEngine/interface/PipelineState.h"

#include <DiligentCore/Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h>
#include <DiligentCore/Graphics/GraphicsEngineOpenGL/interface/EngineFactoryOpenGL.h>

#include <DiligentTools/TextureLoader/interface/TextureLoader.h>
#include <DiligentTools/TextureLoader/interface/TextureUtilities.h>

#include <DiligentCore/Common/interface/RefCntAutoPtr.hpp> /* CHECK: the advantages given over STL smart pointer don't really matter here */

#include <DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/RenderPass.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/Framebuffer.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/SwapChain.h>

#include <DiligentCore/Graphics/GraphicsTools/interface/MapHelper.hpp>

#include <DiligentTools/AssetLoader/interface/GLTFLoader.hpp>

#include <DiligentTools/RenderStateNotation/interface/RenderStateNotationLoader.h>

using namespace glm;

struct Sprite {
    vec3 pos; // Position (before accounting for world-view-model matrix)
    int index; // start index in tex array
    int pages; // number of pages taken up by this spritesheet
    int pagesPerAnim; // pages taken up by each animation
    // (endPage = index + pages)
};

class Renderer {

public:

    Renderer(const uint32_t& windowWidth, const uint32_t& windowHeight);
    ~Renderer();

    bool initRenderer(const Diligent::NativeWindow& window, const Diligent::RENDER_DEVICE_TYPE& deviceType);
    void renderFrame();
    void update();

    void loadGLB(const std::string& filename);
    Sprite loadSprite(const std::string& filename);

    void playSpriteAnim(const int& spriteIn);

    uint32_t m_windowWidth;
    uint32_t m_windowHeight;

    /* Camera instance */
    std::unique_ptr<Camera>                           m_pCamera;

private:

    // TODO: UI will need separate PSO

    Diligent::RefCntAutoPtr<Diligent::IEngineFactory> m_pEngineFactory;
    Diligent::RefCntAutoPtr<Diligent::IRenderDevice>  m_pDevice;
    Diligent::RefCntAutoPtr<Diligent::IDeviceContext> m_pImmediateContext;
    Diligent::RefCntAutoPtr<Diligent::ISwapChain>     m_pSwapChain;
    Diligent::NativeWindow                            m_window;

    Diligent::RefCntAutoPtr<Diligent::IRenderPass>    m_pRenderPass;
    Diligent::RefCntAutoPtr<Diligent::IFramebuffer>   m_pFrameBuffer;

    /* Map file data */
    std::unique_ptr<Diligent::GLTF::Model>            m_pGlbModel;


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
    Diligent::RefCntAutoPtr<Diligent::IBuffer>        m_pMapVertexBuffer;
    std::vector<Diligent::ITexture*>                  m_pMapTextures;
    Diligent::IBuffer*                                m_pMapIndexBuffer;

    Diligent::RefCntAutoPtr<Diligent::ITextureView>   m_pMapShaderResourceView;

    /* Pipeline SRBs */
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding>  m_pMapShaderResourceBinding;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding>  m_pSpriteShaderResourceBinding;

    /* Matrices */
    mat4 m_projMatrix; // The world space's projection matrix
    mat4 m_viewMatrix; // The camera's matrix

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

    void updateUniformBuffer();

    void loadTexture(const std::string& filename);

    void createSpriteTextureArray();
};
