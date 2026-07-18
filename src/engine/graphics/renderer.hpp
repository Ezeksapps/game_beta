#pragma once

#include "../common.hpp"
#include "../entity/sprite.hpp"
#include "../scene/scene.hpp"
#include "camera.hpp"

#include <chrono> // used for frame rate stabilisation & sprite animation timings

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

class Renderer {

public:

    Renderer(const uint32_t& windowWidth, const uint32_t& windowHeight);
    ~Renderer();

    bool initRenderer(const Diligent::NativeWindow& window, const Diligent::RENDER_DEVICE_TYPE& deviceType);
    void renderFrame();
    void update();

    void setScene(const std::string& sceneDir);

    void loadGLB(const std::string& filename);

    // create a texture for a specified Sprite object and add it to the texture array
    // NOTE: Space in that array is limited to m_maxInstances, this function should be used
    // to occupy the top-most slot with a spritesheet belonging to an Entity that is not yet being rendered
    // If you need to switch the spritesheet used for an entity already using the renderer, use swapSprite()
    // Returns the index of this Sprite in the texture array
    int registerSprite(const std::shared_ptr<Sprite>& sprite);

    // If an Entity already using the renderer for its sprite needs to switch to another sprite sheet,
    // use this function to swap the Sprite object stored at the index of the old Sprite to the new one
    void swapSprite(const int& oldSpriteIndex, const std::shared_ptr<Sprite>& newSprite);

    Sprite loadSprite(const std::string& filename); // DEPRECATED

    void playSpriteAnim(const int& spriteIn);

    uint32_t m_windowWidth;
    uint32_t m_windowHeight;

private:

    struct FrameConstants {
        mat4 projMatrix;
        mat4 viewMatrix;
    };

    /* Renderer clock */
    std::chrono::steady_clock m_clock;
    /* Time point where the last frame was drawn */
    std::chrono::time_point<std::chrono::steady_clock> m_lastFrameTime;
    /* Rendered scene */
    std::unique_ptr<Scene> m_pScene;

    /* ---- Core components ---- */
    Diligent::RefCntAutoPtr<Diligent::IEngineFactory> m_pEngineFactory;
    Diligent::RefCntAutoPtr<Diligent::IRenderDevice>  m_pDevice;
    Diligent::RefCntAutoPtr<Diligent::IDeviceContext> m_pImmediateContext;
    Diligent::RefCntAutoPtr<Diligent::ISwapChain>     m_pSwapChain;
    Diligent::NativeWindow                            m_window;

    Diligent::RefCntAutoPtr<Diligent::IRenderPass>    m_pRenderPass;
    Diligent::RefCntAutoPtr<Diligent::IFramebuffer>   m_pFrameBuffer;

    /* Map file data */
    std::unique_ptr<Diligent::GLTF::Model>            m_pGlbModel;
    /* Camera instance */
    std::unique_ptr<Camera>                           m_pCamera;

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
    /* Max number of sprites (aka. Entities that can use this renderer at one time) */
    static const int m_maxInstances = 32;
    /* the maximum dimensions of a sprite sheet */
    static constexpr int m_maxSpriteDimensions = 10 * 10;

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

    void registerTexture(const std::string& filepath);
    void swapTexture(const int& oldTextureIndex, const std::string& newTextureFilepath);

    void createSpriteTextureArray();
};
