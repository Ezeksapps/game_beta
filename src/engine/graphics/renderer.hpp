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

    void setScene(const std::string& sceneDir);
    std::vector<std::shared_ptr<Entity>>& getEntities();


    uint32_t m_windowWidth;
    uint32_t m_windowHeight;

    /* Camera instance */
    std::unique_ptr<Camera>                           m_pCamera;

private:

    struct FrameConstants {
        mat4 projMatrix;
        mat4 viewMatrix;
    };

    struct InstanceData {
        mat4 modelMatrix;
        float texArrayIndex;
        float maxU;
        float maxV;
    };

    struct EntityTransl {
       int  index;                   // index in Scene of entity associated with this translation
       vec3 translVec;               // translation matrix for every step
       float animFrames;             // duration in frames of this translation
       float animFramesAcc;          // number of frames this transformation has been running for
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

    std::unordered_map<Diligent::ITextureView*, Diligent::RefCntAutoPtr<Diligent::IFramebuffer>> m_frameBufferMap;

    /* Map file data */
    std::unique_ptr<Diligent::GLTF::Model>            m_pGlbModel;


    /* ---- Shared UBO, holding matrices for current frame ---- */
    Diligent::RefCntAutoPtr<Diligent::IBuffer>        m_pFrameConstants;
    /* ---- Per-sprite UBO, holding model matrix and texture array index for individual sprites ---- */
    Diligent::RefCntAutoPtr<Diligent::IBuffer>        m_pSpriteConstants;

    /* ---- Pipeline state objects (PSOs) ---- */
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pMapPipelineStateObj;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pSpritePipelineStateObj;

    /* ---- Sprite pipeline buffers & textures ---- */
    //Diligent::RefCntAutoPtr<Diligent::IBuffer>       m_pSpriteIndexBuffer;
    //Diligent::RefCntAutoPtr<Diligent::IBuffer>        m_pSpriteInstanceBuffer;

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

    // model matrix and array index of sprite sheet frame for each Entity
    std::vector<InstanceData> m_instanceData;

    /* Number of sprites game is currently rendering */
    int m_numSprites = 0;
    /* Max number of sprites (aka. Entities that can use this renderer at one time) */
    static const int m_maxInstances = 32;
    /* the maximum dimensions of a sprite sheet */
    static constexpr int m_maxSpriteDimensions = 8 * 10;

    /* The maximum dimensions a single sprite sheet frame can have, used when initialising texture arrays */
    static const int m_maxSpriteFrameWidth = 192;
    static const int m_maxSpriteFrameHeight = 192;

    /* Frame rate */
    static constexpr float m_fps = 60.0f;
    /* Duration of each frame with a constant FPS value, in microseconds */
    static constexpr float m_timePerFrame = 1000000.0f / m_fps;

    /* Equal to the time taken between the previous and current invokation of renderFrame() */
    float m_deltaTime;
    /* Accumulator for time taken, used to make sure frames are drawn at a constant steady rate, with an FPS equal to m_fps */
    float m_timeAcc;

    /* NOTE: num sprites setter must not increment sprites past max instances */

    /* Contains entity translations that are currently being run */
    std::vector<EntityTransl> m_entityTransls;

    /* Stores all pre-loaded sprite sheets for the current scene's entities */
    std::unordered_map<std::string, Diligent::RefCntAutoPtr<Diligent::ITexture>> m_entitySpriteCache;

    void createSharedUniformBuffer();

    void createPerSpriteUniformBuffer(); // per-sprite UBO

    void createMapPipelineState();
    void createSpritePipelineState();

    void createVertexBuffer();
    void createIndexBuffer();
    void createInstanceBuffer();
    void populateInstanceBuffer();

    void createRenderPass();

    Diligent::RefCntAutoPtr<Diligent::IFramebuffer> createFrameBuffer();
    Diligent::IFramebuffer* getCurrentFrameBuffer();

    void loadGLB(const std::string& filename);

    void renderMap();
    void renderSprites();

    void updateUniformBuffer();

    // create a texture for a specified Sprite object and add it to the texture array
    // NOTE: Space in that array is limited to m_maxInstances, this function should be used
    // to occupy the top-most slot with a spritesheet belonging to an Entity that is not yet being rendered
    // If you need to switch the spritesheet used for an entity already using the renderer, use swapSprite()
    // Returns the index of this Sprite in the texture array
    int registerSprite(const std::shared_ptr<Sprite>& sprite, const std::string& cacheKey, const AnimEvent& event);

    // If an Entity already using the renderer for its sprite needs to switch to another sprite sheet,
    // use this function to swap the Sprite object stored at the index of the old Sprite to the new one
    void swapSprite(const int& oldSpriteIndex, const std::shared_ptr<Sprite>& newSprite, const std::string& cacheKey, const AnimEvent& event);

    // loads a Sprite into a given texture array at the specified index
    void loadSpriteToTextureArray(Diligent::RefCntAutoPtr<Diligent::ITexture>& texArray, const std::shared_ptr<Sprite>& sprite, const int& index);
    // gets sprite sheet texture data from an entity texture array in the cache and copies it to the main texture array
    void loadSprite(const std::shared_ptr<Sprite>& sprite, const std::string& cacheKey, const AnimEvent& event);

    // creates a texture array to function as a sprite sheet cache for one entity and returns it
    Diligent::RefCntAutoPtr<Diligent::ITexture> createEntitySpriteCache(const int& entityIndex);
    void createSpriteTextureArray();

    void update();
};
