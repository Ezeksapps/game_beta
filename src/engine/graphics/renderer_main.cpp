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

    createMapPipelineState();
    createSpritePipelineState();

    createIndexBuffer();
    createInstanceBuffer();

    /* States of buffers must be updated prior to starting render pass, since no state transitions can occur when one is active */
    Diligent::StateTransitionDesc barriers[] = {
        {m_pSpriteIndexBuffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_INDEX_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
    };

    m_pImmediateContext->TransitionResourceStates(_countof(barriers), barriers);

    createSpriteTextureArray();
    createFrameBuffer();

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

/* --- RENDER PASS AND FRAMEBUFFER --- */

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

void Renderer::createFrameBuffer() {
    Diligent::ITextureView* pRenderTaregtView = m_pSwapChain->GetCurrentBackBufferRTV();
    Diligent::ITextureView* pDepthStencilView = m_pSwapChain->GetDepthBufferDSV();

    Diligent::ITextureView* attachments[] = {pRenderTaregtView, pDepthStencilView};

    Diligent::FramebufferDesc frameBufferDesc;
    frameBufferDesc.Name = "Game frame buffer";
    frameBufferDesc.pRenderPass = m_pRenderPass;
    frameBufferDesc.ppAttachments = attachments;
    frameBufferDesc.AttachmentCount = 2;
    frameBufferDesc.Width = m_windowWidth;
    frameBufferDesc.Height = m_windowHeight;

    m_pDevice->CreateFramebuffer(frameBufferDesc, &m_pFrameBuffer);
}


/* --- DRAW CALLS --- */

void Renderer::renderFrame() {

    /* Render frame must run in two steps:
     * 1. Render the buffers and textures associated with the main 3D map
     * 2. Render the buffers and textures associated with the 2D sprites
     *
     * These will both be done in one subpass
    */

    /* Both render pass attachments need clear values */
    Diligent::OptimizedClearValue clearValues[2];
    clearValues[0].SetColor(m_pSwapChain->GetDesc().ColorBufferFormat, 0.0f, 0.0f, 0.0f, 1.0f);  // Colour attachment
    clearValues[1].SetDepthStencil(Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB, 1.0f, 0);              // Depth attachment

    m_pImmediateContext->BeginRenderPass({
        m_pRenderPass,
        m_pFrameBuffer,
        _countof(clearValues),
        clearValues,
        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
    });

    {
        updateUniformBuffer();

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

    m_deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(m_clock.now() - m_lastFrameTime).count();
    m_lastFrameTime = m_clock.now(); // update last frame time for next frame

    // update frame timings for all entities, then repopulate instance buffer w/ any new changes to frame
    int i = 0;
    for (const std::shared_ptr<Entity>& entity : m_pScene->m_pEntities) {
        entity->update(m_deltaTime);
        const std::shared_ptr<Sprite> activeSprite = entity->getActiveSprite();
        int texArrayIndex = (activeSprite->index * m_maxSpriteDimensions) + ((uint8_t)entity->m_direction * activeSprite->frame);
        mat4 transform = translate(mat4(1.0f), entity->m_pos);
        m_instanceData[i] = InstanceData(transform, texArrayIndex);
        ++i;
    }

    populateInstanceBuffer();

    renderMap();
    renderSprites();

    m_pImmediateContext->EndRenderPass();

    m_pImmediateContext->Flush();
    m_pSwapChain->Present(1 /* VSync on */); /* NOTE: Must signal present, or unpresented resources pile up in dynamic heap and crash program */

}

void Renderer::updateUniformBuffer() {
    m_viewMatrix = m_pCamera->getViewMatrix();
}


