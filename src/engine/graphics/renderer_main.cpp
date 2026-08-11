#include "renderer.hpp"

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

    m_lastFrameTime = m_clock.now(); // init last frame time
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

    createSpriteTextureArray();

    return true;
}

void Renderer::setScene(const std::string& sceneDir) {
    m_pScene = std::make_unique<Scene>(sceneDir);

    for (const std::shared_ptr<Entity>& entity : m_pScene->m_pEntities /* TODO: CHANGE TO GETTER */) {

        /* Register initial sprite (whatever default AnimEvent the entity is performing based on scene JSON) */
        if (entity->getActiveSprite()) {
            registerSprite(entity->getActiveSprite());
        }

        /* Callback, runs whenever active sprite is changed */
        entity->setSpriteChangeCallback([this](std::shared_ptr<Entity> entity) {
            /* Swap sprite for new sprite */
            const std::shared_ptr<Sprite>& newSprite = entity->getActiveSprite();
            this->swapSprite(newSprite->index, newSprite);
        });

        // CHECK: Might work better in another location
        /* Callback, runs whenever an entity needs to be moved on-screen */
        entity->setMovementCallback([this](std::weak_ptr<Entity> entity, vec3 translVec, const float& animFrames) {
            m_entityTransls.push_back(EntityTransl {
                .entity = entity,
                .translVec = translVec,
                .animFrames = animFrames
            });
        });
    }

    loadGLB(m_pScene->m_glbFilepath);
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

void Renderer::update() {

    if (!m_entityTransls.empty()) {
        for (int i = 0; i < m_entityTransls.size();) {
            EntityTransl& transl = m_entityTransls[i];
            ++transl.animFramesAcc;

            // Apply translation to entity's position
            if (std::shared_ptr<Entity> entity = transl.entity.lock()) entity->m_pos += transl.translVec;

            // Check if translation is complete
            if (transl.animFramesAcc >= transl.animFrames) {
                /* Remove complete translation, done by swapping this and the last EntityTransl so that this now complete translation can be popped */
                if (i < m_entityTransls.size() - 1) std::swap(m_entityTransls[i], m_entityTransls.back());
                m_entityTransls.pop_back();
            } else ++i;
        }
    }

    // issue, will fail if no pre-exisiting instance data exists, as indices not allocated until pushed to vector

    m_instanceData.clear();
    //m_instanceData.reserve(m_pScene->m_pEntities.size());

    // update frame timings for all entities, then repopulate instance buffer w/ any new changes to frame
    int i = 0;
    for (const std::shared_ptr<Entity>& entity : m_pScene->m_pEntities) {
        entity->update(1.0f);
        const std::shared_ptr<Sprite> activeSprite = entity->getActiveSprite();
        int texArrayIndex = (activeSprite->index * m_maxSpriteDimensions) + ((uint8_t)entity->m_direction * activeSprite->framesPerRow) + activeSprite->frame;
        mat4 transform = translate(mat4(1.0f), entity->m_pos);
        m_instanceData.push_back(InstanceData(transform, texArrayIndex));
        ++i;
    }
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


    m_deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(m_clock.now() - m_lastFrameTime).count();
    m_timeAcc += m_deltaTime;
    m_lastFrameTime = m_clock.now(); // update last frame time for next frame

    /* Whilst presentation must occur every time the GPU is free to draw, the rate at which the game is actually updated needs to be constant and controlled
     * (Since GPU performance will always be fluctuating and if updates to movement/animation/position etc. data were tied to the GPU, the game speed would also
     * fluctuate unpredictably) This is set to 60fps. The timeAcc accumulates dt until it grows greater than the pre-calculated (constexpr) timePerFrame,
     * which then runs the game update after its proper constant interval.
     * Because of this, the frame rate technically still fluctuates, but the frames that matter are always at the constant rate of 60fps
     */

    while (m_timeAcc >= m_timePerFrame) {
        update();  // update entities and other relevant variable data
        m_timeAcc -= m_timePerFrame;
    }

    /* Both render pass attachments need clear values */
    Diligent::OptimizedClearValue clearValues[2];
    clearValues[0].SetColor(m_pSwapChain->GetDesc().ColorBufferFormat, 0.0f, 0.0f, 0.0f, 1.0f);  // Colour attachment
    clearValues[1].SetDepthStencil(Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB, 1.0f, 0);              // Depth attachment


    m_pImmediateContext->BeginRenderPass({
        m_pRenderPass,
        getCurrentFrameBuffer(),
        _countof(clearValues),
        clearValues,
        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
    });

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


