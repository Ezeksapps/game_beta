#include "renderer.hpp"

#include <chrono>
#include <cstdint>

/* --- INIT --- */


Renderer::Renderer(const uint32_t& windowWidth, const uint32_t& windowHeight) {

    m_windowWidth = windowWidth;
    m_windowHeight = windowHeight;
    m_pCamera = std::make_unique<Camera>();

    /* Perspective projection matrix */
    float aspectRatio = static_cast<float>(m_windowWidth) / static_cast<float>(m_windowHeight);
    float fov = 75.0f * (std::numbers::pi_v<float> / 180); /* TODO: make adjustable? */
    m_projMatrix = perspective(fov, aspectRatio, /* Z-near */ 0.1f, /* Z-far */ 100.0f);

    /* Camera view matrix */
    m_pCamera->setTargetPos(vec3(0.0f, 0.0f, 0.0f));
    m_pCamera->setOffset(vec3(0.0f, -3.0f, 4.0f));
    /* Only applies effect when in update UBO?
     If camera is made public member, applying rotation incrementally by running rotate() on every keypress of 'ENTER'
     will properly do rotation, but any rotation applied in constructor is cleared and never seen
     target pos and offset are preserved, as is proj matrix
     */

     m_viewMatrix = m_pCamera->getViewMatrix();

    m_lastFrameTime = m_clock.now(); // init last frame time
    m_timeAcc = 0;                   // init accumulator
}

Renderer::~Renderer() {
    if (m_pImmediateContext) m_pImmediateContext->Flush();

    m_pSwapChain        = nullptr;
    m_pImmediateContext = nullptr;
    m_pDevice           = nullptr;
}

bool Renderer::initRenderer(const Diligent::NativeWindow& window, const Diligent::RENDER_DEVICE_TYPE& deviceType) {

    m_window = window;

    /* Create instances, devices and swap chains as needed */

    Diligent::SwapChainDesc swapChainDesc;
    swapChainDesc.Width = m_windowWidth;
    swapChainDesc.Height = m_windowHeight;

    if (deviceType == Diligent::RENDER_DEVICE_TYPE_VULKAN) {

        // Load .so or .dll & get factory
        Diligent::IEngineFactoryVk* pFactoryVk = Diligent::LoadAndGetEngineFactoryVk();
        m_pEngineFactory = pFactoryVk;

        Diligent::EngineVkCreateInfo engineCreateInfo;
        /* Request geometry shaders, engine will fail to initialise and program will exit if not found */
        engineCreateInfo.Features.GeometryShaders = Diligent::DEVICE_FEATURE_STATE_ENABLED;
        pFactoryVk->CreateDeviceAndContextsVk(engineCreateInfo, &m_pDevice, &m_pImmediateContext);
        pFactoryVk->CreateSwapChainVk(m_pDevice, m_pImmediateContext, swapChainDesc, m_window, &m_pSwapChain);
    }
    else if (deviceType == Diligent::RENDER_DEVICE_TYPE_GL) {

        // Load .so or .dll & get factory
        Diligent::IEngineFactoryOpenGL* pFactoryOpenGL = Diligent::LoadAndGetEngineFactoryOpenGL();
        m_pEngineFactory = pFactoryOpenGL;

        Diligent::EngineGLCreateInfo engineCreateInfo;
        /* Request geometry shaders, engine will fail to initialise and program will exit if not found */
        engineCreateInfo.Features.GeometryShaders = Diligent::DEVICE_FEATURE_STATE_ENABLED;
        engineCreateInfo.Window = m_window;
        pFactoryOpenGL->CreateDeviceAndSwapChainGL(engineCreateInfo, &m_pDevice, &m_pImmediateContext, swapChainDesc, &m_pSwapChain);
    }
    else throw std::runtime_error("Failed to initialise renderer -- Device type must be set to either Vulkan or OpenGL");

    if (m_pDevice == nullptr || m_pImmediateContext == nullptr || m_pSwapChain == nullptr) return false;

    /* init other required systems */

    createRenderPass();

    createSharedUniformBuffer();
    createPerSpriteUniformBuffer();

    createMapPipelineState();
    createSpritePipelineState();

    //createIndexBuffer();
    createInstanceBuffer();

    createSpriteTextureArray();

    return true;
}

// CHECK: Maybe rename index to event?
void Renderer::loadSpriteToTextureArray(Diligent::RefCntAutoPtr<Diligent::ITexture>& texArray, const std::shared_ptr<Sprite>& sprite, const int& index) {

    Diligent::RefCntAutoPtr<Diligent::ITextureLoader> textureLoader;
    Diligent::TextureLoadInfo loadInfo;
    loadInfo.IsSRGB = true;
    Diligent::CreateTextureLoaderFromFile(sprite->filepath.c_str(), Diligent::IMAGE_FILE_FORMAT_PNG, loadInfo, &textureLoader);

    sprite->framesPerRow = textureLoader->GetTextureDesc().GetWidth() / sprite->frameWidth;
    sprite->framesPerCol = textureLoader->GetTextureDesc().GetHeight() / sprite->frameHeight;

    /* Get full texture data */
    Diligent::TextureSubResData subResData = textureLoader->GetSubresourceData(0, 0);
    const unsigned char* textureBuffer = static_cast<const unsigned char*>(subResData.pData);
    int rowStride = subResData.Stride;  // Bytes per row in the source data

    // copy each frame into its own slice of the texture array
    for (int row = 0; row < sprite->framesPerCol; ++row) {
        for (int col = 0; col < sprite->framesPerRow; ++col) {

            int sliceIndex = (index * m_maxSpriteDimensions) + (row * sprite->framesPerRow) + col; // current slice index

            /* Create an empty buffer for this frame's image data */
            uint32_t frameDataSize = sprite->frameWidth * sprite->frameHeight * 4;  // RGBA = 4 bytes
            std::vector<unsigned char> frameBuffer(frameDataSize);

            /* Copy current frame to temp frame buffer */
            for (int y = 0; y < sprite->frameHeight; ++y) {
                int srcOffset = ((row * sprite->frameHeight + y) * rowStride) + (col * sprite->frameWidth * 4);
                int dstOffset = y * sprite->frameWidth * 4;
                memcpy(frameBuffer.data() + dstOffset, textureBuffer + srcOffset, sprite->frameWidth * 4);
            }

            /* update slice in main texture array */
            Diligent::Box updateBox;
            updateBox.MinX = 0;
            updateBox.MinY = 0;
            updateBox.MinZ = 0;
            updateBox.MaxX = sprite->frameWidth;
            updateBox.MaxY = sprite->frameHeight;
            updateBox.MaxZ = 1;

            Diligent::TextureSubResData frameSubRes;
            frameSubRes.pData = frameBuffer.data(); // set the update box data to the frame
            frameSubRes.Stride = sprite->frameWidth * 4;
            frameSubRes.DepthStride = 0;

            m_pImmediateContext->UpdateTexture(
                texArray,
                0,
                sliceIndex,
                updateBox,
                frameSubRes,
                Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE,
                Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
            );
        }
    }
}


Diligent::RefCntAutoPtr<Diligent::ITexture> Renderer::createEntitySpriteCache(const int& entityIndex) {
    Diligent::RefCntAutoPtr<Diligent::ITexture> texArray;

    Diligent::TextureDesc textureArrayDesc;
    textureArrayDesc.ArraySize = m_maxSpriteDimensions * ANIM_EVENT_COUNT;
    // 2D array
    textureArrayDesc.Type = Diligent::RESOURCE_DIM_TEX_2D_ARRAY;
    /* All sprite dimensions are 192 x 192 */
    textureArrayDesc.Width  = m_maxSpriteFrameWidth;
    textureArrayDesc.Height = m_maxSpriteFrameHeight;
    /* NOTE: Mip levels refer to number of smaller-sized versions of the texture to create (used for efficiency when rendering faraway objs)
     * this is irrelevant here, only take 1, that is, the original image */
    textureArrayDesc.MipLevels = 1;
    textureArrayDesc.Format    = Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB; // TODO: Set to swap chain's format instead
    textureArrayDesc.Usage     = Diligent::USAGE_DEFAULT;
    textureArrayDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;

    m_pDevice->CreateTexture(textureArrayDesc, nullptr, &texArray);

    // no SRV or SRB since this array is not for rendering

    const std::shared_ptr<Entity>& entity = m_pScene->m_pEntities[entityIndex]; // maybe just pass this directly from setScene?
    const std::unordered_map<AnimEvent, std::shared_ptr<Sprite>>& spriteMap = entity->getSpriteMap();

    /* Loop through entity sprite map and load all Sprite objects into tex array */
    for (const auto& entry : spriteMap) {
        loadSpriteToTextureArray(texArray /* Tex array */, entry.second /* Sprite */, entry.first  /* load to index */);
    }

    return texArray;
}

void Renderer::setScene(const std::string& sceneDir) {
    m_pScene = std::make_unique<Scene>(sceneDir);

    for (const std::shared_ptr<Entity>& entity : m_pScene->m_pEntities /* TODO: CHANGE TO GETTER */) {

        // only add a new texture array to the cache if one doesn't already exist for the required sprite sheets
        // NOTE: use try_emplace() since checking the index requires operator[], which will always create that index if it doesnt exist
        m_entitySpriteCache.try_emplace(entity->m_animJsonFilepath, createEntitySpriteCache(entity->m_index));

        /* Register initial sprite (whatever default AnimEvent the entity is performing based on scene JSON) */
        if (entity->getActiveSprite()) registerSprite(entity->getActiveSprite(), entity->m_animJsonFilepath, entity->m_event);

        /* Callback, runs whenever active sprite is changed */
        entity->setSpriteChangeCallback([this, entity](const int& oldSpriteIndex, std::shared_ptr<Sprite> newSprite) {
            /* Swap sprite for new sprite */
            this->swapSprite(oldSpriteIndex, newSprite, entity->m_animJsonFilepath, entity->m_event);
        });

        /* Callback, runs whenever an entity needs to be moved on-screen */
        entity->setMovementCallback([this](const int& index, vec3 translVec, const float& animFrames) {
            m_entityTransls.push_back(EntityTransl{
                .index = index,
                .translVec = translVec,
                .animFrames = animFrames
            });
        });
    }

    loadGLB(m_pScene->m_glbFilepath);
}

std::vector<std::shared_ptr<Entity>>& Renderer::getEntities() {
    // If no scene exists, return empty vector or throw
    if (!m_pScene) {
        static std::vector<std::shared_ptr<Entity>> empty;
        return empty;
    }
    return m_pScene->m_pEntities;
}

void Renderer::createSharedUniformBuffer() {

    /* Dynamic UBO for transform matrix */
    Diligent::BufferDesc uniformBufferDesc;
    uniformBufferDesc.Name           = "frame constants desc";
    uniformBufferDesc.Size           = sizeof(FrameConstants);
    uniformBufferDesc.Usage          = Diligent::USAGE_DYNAMIC;
    uniformBufferDesc.BindFlags      = Diligent::BIND_UNIFORM_BUFFER;
    uniformBufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
    m_pDevice->CreateBuffer(uniformBufferDesc, nullptr, &m_pFrameConstants);
}

/* --- RENDER PASS AND FRAMEBUFFERS --- */

void Renderer::createRenderPass() {
    /* Need two attachments, the framebuffer image to render to and a depth attachment to handle 3D depth testing */
    Diligent::RenderPassAttachmentDesc renderPassAttachments[2];
    renderPassAttachments[0].Format = m_pSwapChain->GetDesc().ColorBufferFormat;
    renderPassAttachments[0].LoadOp = Diligent::ATTACHMENT_LOAD_OP_CLEAR;      // clear attachment before rendering (using clear colour)
    renderPassAttachments[0].StoreOp = Diligent::ATTACHMENT_STORE_OP_STORE;    // store rendered contents after rendering
    renderPassAttachments[0].InitialState = Diligent::RESOURCE_STATE_RENDER_TARGET;
    renderPassAttachments[0].FinalState = Diligent::RESOURCE_STATE_PRESENT;

    renderPassAttachments[1].Format = m_pSwapChain->GetDesc().DepthBufferFormat;
    renderPassAttachments[1].InitialState = Diligent::RESOURCE_STATE_DEPTH_WRITE;
    renderPassAttachments[1].FinalState = Diligent::RESOURCE_STATE_DEPTH_READ;
    renderPassAttachments[1].LoadOp = Diligent::ATTACHMENT_LOAD_OP_CLEAR;
    renderPassAttachments[1].StoreOp = Diligent::ATTACHMENT_STORE_OP_DISCARD;

    Diligent::SubpassDesc supbassDesc{};

    /* suppass will use the attachment as its render target, define ref to attachment */
    Diligent::AttachmentReference renderTargetAttachmentRef(
        {0, Diligent::RESOURCE_STATE_RENDER_TARGET}
    );

    Diligent::AttachmentReference depthStencilAttachmentRef(
        {1, Diligent::RESOURCE_STATE_DEPTH_WRITE}
    );

    supbassDesc.RenderTargetAttachmentCount = 1;
    supbassDesc.pRenderTargetAttachments = &renderTargetAttachmentRef;
    supbassDesc.pDepthStencilAttachment = &depthStencilAttachmentRef;

    Diligent::RenderPassDesc renderPassDesc;
    renderPassDesc.Name = "Game render pass desc";
    renderPassDesc.pAttachments = renderPassAttachments;
    renderPassDesc.AttachmentCount = _countof(renderPassAttachments);
    renderPassDesc.SubpassCount = 1;
    renderPassDesc.pSubpasses = &supbassDesc;

    m_pDevice->CreateRenderPass(renderPassDesc, &m_pRenderPass);
}

// TODO: Check compatibility with OpenGL
Diligent::RefCntAutoPtr<Diligent::IFramebuffer> Renderer::createFrameBuffer() {
    Diligent::ITextureView* pRenderTargetView = m_pSwapChain->GetCurrentBackBufferRTV();
    Diligent::ITextureView* pDepthStencilView = m_pSwapChain->GetDepthBufferDSV();

    Diligent::ITextureView* attachments[] = {pRenderTargetView, pDepthStencilView};

    Diligent::RefCntAutoPtr<Diligent::IFramebuffer> pFrameBuffer;

    Diligent::FramebufferDesc frameBufferDesc;
    frameBufferDesc.Name = "Game frame buffer";
    frameBufferDesc.pRenderPass = m_pRenderPass;
    frameBufferDesc.ppAttachments = attachments;
    frameBufferDesc.AttachmentCount = 2;
    frameBufferDesc.Width = m_windowWidth;
    frameBufferDesc.Height = m_windowHeight;

    m_pDevice->CreateFramebuffer(frameBufferDesc, &pFrameBuffer);

    return pFrameBuffer;
}

Diligent::IFramebuffer* Renderer::getCurrentFrameBuffer() {
    Diligent::ITextureView* pCurrentBackBufferRTV = //m_pDevice->GetDeviceInfo().IsGLDevice() ? nullptr : // check reason for ternary, does OpenGL need separate handling?
    m_pSwapChain->GetCurrentBackBufferRTV();

    auto fb_it = m_frameBufferMap.find(pCurrentBackBufferRTV);
    if (fb_it != m_frameBufferMap.end())
    {
        return fb_it->second;
    }
    else
    {
        auto it = m_frameBufferMap.emplace(pCurrentBackBufferRTV, createFrameBuffer());
        VERIFY_EXPR(it.second);
        return it.first->second;
    }
}

/* --- GAME DATA UPDATE FUNC --- */

void Renderer::update() { // TODO: Make more efficient

    if (!m_entityTransls.empty()) {
        for (int i = 0; i < m_entityTransls.size();) {
            EntityTransl& transl = m_entityTransls[i];
            ++transl.animFramesAcc;

            // Apply translation to entity's position
            m_pScene->m_pEntities[transl.index]->m_pos            += transl.translVec;
          //  m_pScene->m_pEntities[transl.index]->m_pos += lerp(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), 0.5f);
            vec3 pos =  m_pScene->m_pEntities[transl.index]->m_pos;

            // Check if translation is complete
            if (transl.animFramesAcc >= transl.animFrames) {
                m_pScene->m_pEntities[transl.index]->doAnimEvent(ANIM_EVENT_IDLE); // reset to idle animation
                /* Remove complete translation, done by swapping this and the last EntityTransl so that this now complete translation can be popped */
                if (i < m_entityTransls.size() - 1) std::swap(m_entityTransls[i], m_entityTransls.back());
                m_entityTransls.pop_back();
            } else ++i;
        }
    }

    populateInstanceBuffer();
}

/* --- DRAW CALLS --- */


void Renderer::renderFrame() {

    /* Render frame must run in two steps:
     * 1. Render the buffers and textures associated with the main 3D map
     * 2. Render the buffers and textures associated with the 2D sprites
     *
     * These will both be done in one subpass
     */

    // TODO: Add the third step (The UI's pipeline, which renders buffers and textures associated with the UI and draws over the other two pipelines)

    /*
    ISSUE:
    ---------
    The word 'frame' gets used to refer to multiple things. There are the frames when renderFrame() is called,
    the frames which have an actual effect (those that call update() to update game entities and position data) which occur at a rate of 60fps,
    & the frames of animation in a sprite sheet, which are also referred to as slices in the context of the texture array by the Diligent tutorials

    Variable names and function names in the code use frame to refer to all three. TODO: Figure out a less confusing naming scheme? (Maybe all sprite animation frames are just slices)
    ---------
    */

    m_deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(m_clock.now() - m_lastFrameTime).count();
    m_timeAcc += m_deltaTime;
    m_lastFrameTime = m_clock.now(); // update last frame time for next frame

    /* Whilst presentation must occur every time the GPU is free to draw, the rate at which the game is actually updated needs to be constant and controlled
     * (Since GPU performance will always be fluctuating and if updates to movement/animation/position etc. data were tied to the GPU, the game speed would also
     * fluctuate unpredictably) This is set to 60fps. The timeAcc accumulates dt until it grows greater than the pre-calculated (constexpr) timePerFrame,
     * which then runs the game update after its proper constant interval.
     * Because of this, the frame rate technically still fluctuates, but the frames that matter are always at the constant rate of 60fps
     */


    while (m_timeAcc >= m_timePerFrame) { // now infinite loop for some reason?
        update();  // update entities and other relevant variable data
        m_timeAcc -= m_timePerFrame;
    }

    /* Both render pass attachments need clear values */
    Diligent::OptimizedClearValue clearValues[2];
    clearValues[0].SetColor(m_pSwapChain->GetDesc().ColorBufferFormat, 0.0f, 0.0f, 0.0f, 1.0f);  // Colour attachment
    clearValues[1].SetDepthStencil(Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB, 1.0f, 0);              // Depth attachment

    populateInstanceBuffer();
    {
        updateUniformBuffer(); // for now just updates camera, since proj matrix is constant unless FOV is changed

        FrameConstants constants;
        constants.projMatrix = m_projMatrix;
        constants.viewMatrix = m_viewMatrix;

        Diligent::MapHelper<FrameConstants> uniformConstants(
            m_pImmediateContext,
            m_pFrameConstants,
            Diligent::MAP_WRITE,
            Diligent::MAP_FLAG_DISCARD
        );
        *uniformConstants = constants;
    }


    // BEGIN NEEDS WORK


    std::vector<InstanceData> spriteData;
    spriteData.reserve(m_instanceData.size());

    for (const auto& data : m_instanceData) {
        InstanceData psd;
        psd.modelMatrix = data.modelMatrix;
        psd.texArrayIndex = static_cast<float>(data.texArrayIndex);
        spriteData.push_back(psd);
    }

    if (!m_instanceData.empty()) {
        uint32_t dataSize = static_cast<uint32_t>(sizeof(InstanceData) * m_instanceData.size());

        Diligent::MapHelper<InstanceData> mappedData(
            m_pImmediateContext,
            m_pSpriteConstants,
            Diligent::MAP_WRITE,
            Diligent::MAP_FLAG_DISCARD
        );

        if (mappedData) {
            // Copy only the active instances to the beginning of the buffer
            memcpy(mappedData, m_instanceData.data(), dataSize);
        }
    }

    m_pImmediateContext->BeginRenderPass({
        m_pRenderPass,
        getCurrentFrameBuffer(),
        _countof(clearValues),
        clearValues,
        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
    });

    // --- END NEEDS WORK


    renderMap();
    renderSprites();

    m_pImmediateContext->EndRenderPass();

    m_pImmediateContext->Flush();
    m_pSwapChain->Present(1 /* VSync on */); /* NOTE: Must signal present, or unpresented resources pile up in dynamic heap and crash program */

}

void Renderer::updateUniformBuffer() {
    m_viewMatrix = m_pCamera->getViewMatrix();
}


