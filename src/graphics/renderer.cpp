#include "renderer.hpp"

#include <cstdint>

// TODO:

// Once renderer complete, associate sprites with an 'Entity'
// set one entity as the player, others as NPCs (Likely classes extending Entity)
// Collision logic for 3D space (stairs will always be at a fixed angle, so any angle > stair angle = impassable)
// X & Y coords should work like a grid system (ortho proj?)
// Along w/ GLB, map also defined by a JSON giving NPCs in map and exits/links to other areas within map
// Create dialogue system and UI Renderer (defined by XML reader, then render in here, maybe in separate subpass?)
// Create other systems, assign to game events

// CURRENTLY COMPLETE:
// Renderer init
// GLB loader
// Sprite loader and renderer
// Graphics pipelines, render pass and frame buffer
// Sprite billboards' vertex, index and instance buffers
// window creation and input handling (needs to be associated to a callback)
// shaders for both pipelines
// Camera system
// sprite billboards' positioning relative to camera

/* CURRENT STATUS:
 * Compiles successfully, renderer and pipelines initialise with no problems.
 * Map pipeline renders with no issues. Camera system is properly set up and can rotate with yaw or pitch with no issues
 * Sprite pipeline renders sprite with proper UVs and properly matches the camera's rotation to appear flat. Transparency needs handling though
 */

struct FrameConstants {
    mat4 projMatrix;
    mat4 viewMatrix;
};

struct Vertex {
    vec3 pos;
    vec2 uv;
};

/* Billboards will all be identically-sized quads (made of two triangle primitives),
 * they therefore always have the same vertices and indices, which are defined here
*/

// NOTE: billboards are actually generated in GS, these are equivalently-sized constants for buffer init
const std::vector<uint16_t> billboardIndices = {
    0, 1, 2, 3, 0
};

const std::vector<Vertex> billboardVertices = {
    {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}},
    {{ 0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}},
    {{ 0.5f,  0.5f, 0.0f}, {1.0f, 1.0f}},
    {{-0.5f,  0.5f, 0.0f}, {0.0f, 1.0f}}
};

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

    createVertexBuffer();
    createIndexBuffer();
    createInstanceBuffer();

    /* States of buffers must be updated prior to starting render pass, since no state transitions can occur when one is active */
    Diligent::StateTransitionDesc barriers[] = {
        {m_pSpriteVertexBuffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_VERTEX_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
        {m_pSpriteIndexBuffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_INDEX_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
    };

    m_pImmediateContext->TransitionResourceStates(_countof(barriers), barriers);

    createSpriteTextureArray();
    createFrameBuffer();

    return true;
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


/* ---- PIPELINES ---- */

/* Map PSO creation, takes vertex and index buffers from GLB loader */
void Renderer::createMapPipelineState() {
    Diligent::GraphicsPipelineStateCreateInfo PipelineStateObjCreateInfo;

    PipelineStateObjCreateInfo.PSODesc.Name = "Map Pipeline State Object";
    PipelineStateObjCreateInfo.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;

    /* Defines what kind of primitives will be rendered by this pipeline state */
    PipelineStateObjCreateInfo.GraphicsPipeline.PrimitiveTopology                       = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    /* Face culling mode */
    PipelineStateObjCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode                 = Diligent::CULL_MODE_NONE;
    /* Enable depth testing */
    PipelineStateObjCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable            = true;
    /* Render pass used by this pipeline */
    PipelineStateObjCreateInfo.GraphicsPipeline.pRenderPass                             = m_pRenderPass;
    /* initial supbpass to start render pass from */
    PipelineStateObjCreateInfo.GraphicsPipeline.SubpassIndex                            = 0;


    Diligent::ShaderCreateInfo shaderCreateInfo;

    shaderCreateInfo.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_GLSL;

    Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory> pShaderSourceFactory;
    m_pEngineFactory->CreateDefaultShaderSourceStreamFactory("assets/shaders", &pShaderSourceFactory);
    shaderCreateInfo.pShaderSourceStreamFactory = pShaderSourceFactory;

    /* NOTE: GLB loader will already generate vertex and index buffers, manual creation is not necessary & therefore omitted */

    /* Vertex shader */
    Diligent::RefCntAutoPtr<Diligent::IShader> pMapVertexShader;
    {
        shaderCreateInfo.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
        shaderCreateInfo.EntryPoint      = "main";
        shaderCreateInfo.Desc.Name       = "vertex shader desc";
        shaderCreateInfo.FilePath        = "map_vertex.glsl";
        m_pDevice->CreateShader(shaderCreateInfo, &pMapVertexShader);
    }

    /* NOTE: Diligent Engine internally refers to fragment shaders as 'pixel shaders' */
    Diligent::RefCntAutoPtr<Diligent::IShader> pMapFragmentShader;
    {
        shaderCreateInfo.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
        shaderCreateInfo.EntryPoint      = "main";
        shaderCreateInfo.Desc.Name       = "fragment shader desc";
        shaderCreateInfo.FilePath        = "map_fragment.glsl";
        m_pDevice->CreateShader(shaderCreateInfo, &pMapFragmentShader);
    }

    /* Layout of shader input (that being what is passed to the first shader in the pipeline, the vertex shader) */
    Diligent::LayoutElement layoutElems[] = {

        /* LayoutElement(<inputIndex>, <bufferSlot>, <numComponents>, <valueType>, <isNormalised>, <relativeOffset>, <stride>, <frequency>);
         */

        // vertex position
        Diligent::LayoutElement{0, 0, 3, Diligent::VT_FLOAT32, false, 0, 32, Diligent::INPUT_ELEMENT_FREQUENCY_PER_VERTEX},
        // TEXCOORD_0: offset 24, stride 32
        Diligent::LayoutElement{2, 0, 2, Diligent::VT_FLOAT32, false, 24, 32, Diligent::INPUT_ELEMENT_FREQUENCY_PER_VERTEX}
    };

    /* Create pipeline state */

    /* Set shaders */
    PipelineStateObjCreateInfo.pVS                                         = pMapVertexShader;
    PipelineStateObjCreateInfo.pPS                                         = pMapFragmentShader;
    /* Layout of input elements to shader pipeline */
    PipelineStateObjCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = layoutElems;
    PipelineStateObjCreateInfo.GraphicsPipeline.InputLayout.NumElements    = _countof(layoutElems);
    /* Referring to variables in the GLSL shader code */
    PipelineStateObjCreateInfo.PSODesc.ResourceLayout.DefaultVariableType  = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;


    /* NOTE: mutable shader vars should be used, since they change on a per-instance basis
     * Set shader variables that can be set in this code
     */
    Diligent::ShaderResourceVariableDesc shaderVars[] = {
        {Diligent::SHADER_TYPE_PIXEL, "g_texture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}
    };
    PipelineStateObjCreateInfo.PSODesc.ResourceLayout.Variables    = shaderVars;
    PipelineStateObjCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(shaderVars);

    /* Texture sampler definition for g_texture var */

    Diligent::SamplerDesc samplerLinearClampDesc {
        Diligent::FILTER_TYPE_LINEAR, Diligent::FILTER_TYPE_LINEAR, Diligent::FILTER_TYPE_LINEAR,
        Diligent::TEXTURE_ADDRESS_CLAMP, Diligent::TEXTURE_ADDRESS_CLAMP, Diligent::TEXTURE_ADDRESS_CLAMP
    };

    Diligent::ImmutableSamplerDesc immutableSamplers[] = {
        {Diligent::SHADER_TYPE_PIXEL, "g_texture", samplerLinearClampDesc}
    };

    PipelineStateObjCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers    = immutableSamplers;
    PipelineStateObjCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(immutableSamplers);

    /* Create pipeline with obj create info */
    m_pDevice->CreateGraphicsPipelineState(PipelineStateObjCreateInfo, &m_pMapPipelineStateObj);

    /* Set Constants variable (holds matrices for current frame) for all shaders that use it */
    m_pMapPipelineStateObj->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "Constants")->Set(m_pFrameConstants);

    /* Create a shader resource binding (SRB) through which we can alter the mutable value of shader variables */
    m_pMapPipelineStateObj->CreateShaderResourceBinding(&m_pMapShaderResourceBinding, true);
}

/* Sprite PSO creation, needs manual vertex and index buffer creation and includes per-instance data such as texture array index and transforms */
void Renderer::createSpritePipelineState() {
    Diligent::GraphicsPipelineStateCreateInfo PipelineStateObjCreateInfo;

    PipelineStateObjCreateInfo.PSODesc.Name = "Sprite Pipeline State Object";
    PipelineStateObjCreateInfo.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;

    /* Defines what kind of primitives will be rendered by this pipeline state */
    PipelineStateObjCreateInfo.GraphicsPipeline.PrimitiveTopology                       = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    /* Face culling mode */
    PipelineStateObjCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode                 = Diligent::CULL_MODE_BACK;
    /* Enable depth testing */
    PipelineStateObjCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable            = true;
    /* Render pass used by this pipeline */
    PipelineStateObjCreateInfo.GraphicsPipeline.pRenderPass                             = m_pRenderPass;
    /* initial supbpass to start render pass from */
    PipelineStateObjCreateInfo.GraphicsPipeline.SubpassIndex                            = 0;

    /* NOTE: Weirdly, Blend is applied to a 'render target', even if you're using a render pass system? */
    // TODO: Transparency might need sorting (grid-like system may mean this isn't necessary though)
    Diligent::RenderTargetBlendDesc blendDesc;
    blendDesc.BlendEnable = true;
    blendDesc.SrcBlend = Diligent::BLEND_FACTOR_SRC_ALPHA;
    blendDesc.DestBlend = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
    blendDesc.BlendOp = Diligent::BLEND_OPERATION_ADD;
    blendDesc.SrcBlendAlpha = Diligent::BLEND_FACTOR_ONE;
    blendDesc.DestBlendAlpha = Diligent::BLEND_FACTOR_ZERO;
    blendDesc.BlendOpAlpha = Diligent::BLEND_OPERATION_ADD;
    blendDesc.RenderTargetWriteMask = Diligent::COLOR_MASK_ALL;

    PipelineStateObjCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[0] = blendDesc;

    Diligent::ShaderCreateInfo shaderCreateInfo;

    shaderCreateInfo.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_GLSL;

    Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory> pShaderSourceFactory;
    m_pEngineFactory->CreateDefaultShaderSourceStreamFactory("assets/shaders", &pShaderSourceFactory);
    shaderCreateInfo.pShaderSourceStreamFactory = pShaderSourceFactory;

    /* Vertex shader */
    Diligent::RefCntAutoPtr<Diligent::IShader> pSpriteVertexShader;
    {
        shaderCreateInfo.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
        shaderCreateInfo.EntryPoint      = "main";
        shaderCreateInfo.Desc.Name       = "vertex shader desc";
        shaderCreateInfo.FilePath        = "sprite_vertex.glsl";
        m_pDevice->CreateShader(shaderCreateInfo, &pSpriteVertexShader);
    }

    /* Geometry shader determines dimensions & matrix of billboards */
    Diligent::RefCntAutoPtr<Diligent::IShader> pSpriteGeometryShader;
    {
        shaderCreateInfo.Desc.ShaderType = Diligent::SHADER_TYPE_GEOMETRY;
        shaderCreateInfo.EntryPoint      = "main";
        shaderCreateInfo.Desc.Name       = "geometry shader desc";
        shaderCreateInfo.FilePath        = "sprite_geometry.glsl";
        m_pDevice->CreateShader(shaderCreateInfo, &pSpriteGeometryShader);
    }

    /* NOTE: Diligent Engine internally refers to fragment shaders as 'pixel shaders' */
    Diligent::RefCntAutoPtr<Diligent::IShader> pSpriteFragmentShader;
    {
        shaderCreateInfo.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
        shaderCreateInfo.EntryPoint      = "main";
        shaderCreateInfo.Desc.Name       = "fragment shader desc";
        shaderCreateInfo.FilePath        = "sprite_fragment.glsl";
        m_pDevice->CreateShader(shaderCreateInfo, &pSpriteFragmentShader);
    }

    /* Layout of shader input (that being what is passed to the first shader in the pipeline, the vertex shader) */
    Diligent::LayoutElement layoutElems[] = {

        /* LayoutElement(<inputIndex>, <bufferSlot>, <numComponents>, <valueType>, <isNormalised>, <frequency>);
           LayoutElement(<inputIndex>, <bufferSlot>, <numComponents>, <valueType>, <isNormalised>, <relativeOffset>, <stride>, <frequency>);
        */

        /* --- Per-vertex Data --- */

        // vertex position
        Diligent::LayoutElement{0, 0, 3, Diligent::VT_FLOAT32, false},
        // texture UVs
        Diligent::LayoutElement{1, 0, 2, Diligent::VT_FLOAT32, false},

        /* --- Per-instance Data --- */

        /* NOTE: This differs from the tutorial's setup in that the program only uses GLSL, so these attribs will represent
         * the columns of the matrix, not the rows */

        // rotation matrix, col 1
        Diligent::LayoutElement{2, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE},
        // rotation matrix, col 2
        Diligent::LayoutElement{3, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE},
        // rotation matrix, col 3
        Diligent::LayoutElement{4, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE},
        // rotation matrix, col 4
        Diligent::LayoutElement{5, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE},
        // texture array index
        Diligent::LayoutElement{6, 1, 1, Diligent::VT_FLOAT32, false, Diligent::LAYOUT_ELEMENT_AUTO_OFFSET,
                                Diligent::LAYOUT_ELEMENT_AUTO_STRIDE, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE}
    };

    /* Create pipeline state */

    /* Set shaders */
    PipelineStateObjCreateInfo.pVS                                         = pSpriteVertexShader;
    PipelineStateObjCreateInfo.pGS                                         = pSpriteGeometryShader;
    PipelineStateObjCreateInfo.pPS                                         = pSpriteFragmentShader;
    /* Layout of input elements to shader pipeline */
    PipelineStateObjCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = layoutElems;
    PipelineStateObjCreateInfo.GraphicsPipeline.InputLayout.NumElements    = _countof(layoutElems);
    /* Referring to variables in the GLSL shader code */
    PipelineStateObjCreateInfo.PSODesc.ResourceLayout.DefaultVariableType  = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;


    /* NOTE: mutable shader vars should be used, since they change on a per-instance basis
     * Set shader variables that can be set in this code
     */
    Diligent::ShaderResourceVariableDesc shaderVars[] = {
        {Diligent::SHADER_TYPE_PIXEL, "g_texture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}
    };
    PipelineStateObjCreateInfo.PSODesc.ResourceLayout.Variables    = shaderVars;
    PipelineStateObjCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(shaderVars);

    /* Texture sampler definition for g_texture var */

    Diligent::SamplerDesc samplerLinearClampDesc {
        Diligent::FILTER_TYPE_LINEAR, Diligent::FILTER_TYPE_LINEAR, Diligent::FILTER_TYPE_LINEAR,
        Diligent::TEXTURE_ADDRESS_CLAMP, Diligent::TEXTURE_ADDRESS_CLAMP, Diligent::TEXTURE_ADDRESS_CLAMP
    };

    Diligent::ImmutableSamplerDesc immutableSamplers[] = {
        {Diligent::SHADER_TYPE_PIXEL, "g_texture", samplerLinearClampDesc}
    };

    PipelineStateObjCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers    = immutableSamplers;
    PipelineStateObjCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(immutableSamplers);

    /* Create pipeline with obj create info */
    m_pDevice->CreateGraphicsPipelineState(PipelineStateObjCreateInfo, &m_pSpritePipelineStateObj);

    /* Set Constants variable (holds matrices for current frame) for all shaders that use it */
    m_pSpritePipelineStateObj->GetStaticVariableByName(Diligent::SHADER_TYPE_GEOMETRY, "Constants")->Set(m_pFrameConstants);

    /* Create a shader resource binding (SRB) through which we can alter the mutable value of shader variables */
    m_pSpritePipelineStateObj->CreateShaderResourceBinding(&m_pSpriteShaderResourceBinding, true);
}

/* creates texture array for sprites */
void Renderer::createSpriteTextureArray() {
    Diligent::TextureDesc textureArrayDesc;
    // AKA max sprites
    textureArrayDesc.ArraySize = m_maxInstances;
    // 2D array
    textureArrayDesc.Type = Diligent::RESOURCE_DIM_TEX_2D_ARRAY;
    /* All sprite dimensions are 192 x 192 */
    textureArrayDesc.Width  = 192;
    textureArrayDesc.Height = 192;
    /* NOTE: Mip levels refer to number of smaller-sized versions of the texture to create (used for efficiency when rendering faraway objs)
     * this is irrelevant here, only take 1, that is, the original image */
    textureArrayDesc.MipLevels = 1;
    textureArrayDesc.Format    = Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB; // TODO: Set to swap chain's format instead
    textureArrayDesc.Usage     = Diligent::USAGE_DEFAULT;
    textureArrayDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;

    m_pDevice->CreateTexture(textureArrayDesc, nullptr /* No initial data */, &m_pSpriteTextureArray);

    m_pSpriteShaderResourceView = m_pSpriteTextureArray->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);

    m_pSpriteShaderResourceBinding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_texture")->Set(m_pSpriteShaderResourceView);
}

/* Vertex buffer for billboards */
void Renderer::createVertexBuffer() {
    Diligent::BufferDesc vertexBufferDesc;
    vertexBufferDesc.Name      = "billboard vertex buffer";
    vertexBufferDesc.Usage     = Diligent::USAGE_IMMUTABLE;
    vertexBufferDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
    vertexBufferDesc.Size      = billboardVertices.size() * sizeof(Vertex);
    vertexBufferDesc.Size = sizeof(billboardVertices);

    Diligent::BufferData vertexBufferData;
    vertexBufferData.pData    = billboardVertices.data();
    vertexBufferData.DataSize = billboardVertices.size() * sizeof(Vertex);
    m_pDevice->CreateBuffer(vertexBufferDesc, &vertexBufferData, &m_pSpriteVertexBuffer);
}

/* Index buffer for billboards */
void Renderer::createIndexBuffer() {

    Diligent::BufferDesc indexBufferDesc;
    indexBufferDesc.Name      = "billboard index buffer";
    indexBufferDesc.Usage     = Diligent::USAGE_IMMUTABLE;
    indexBufferDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
    indexBufferDesc.Size      = billboardIndices.size() * sizeof(uint16_t);

    Diligent::BufferData indexBufferData;
    indexBufferData.pData    = billboardIndices.data();
    indexBufferData.DataSize = billboardIndices.size() * sizeof(uint16_t);
    m_pDevice->CreateBuffer(indexBufferDesc, &indexBufferData, &m_pSpriteIndexBuffer);
}

/* Instance buffer for billboards */
void Renderer::createInstanceBuffer() {
    Diligent::BufferDesc instanceBufferDesc;
    instanceBufferDesc.Name = "instance data buffer";
    /* Default usage, as buffer is only updated when grid size changes */
    instanceBufferDesc.Usage     = Diligent::USAGE_DEFAULT;
    instanceBufferDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
    instanceBufferDesc.Size      = sizeof(mat4) * m_maxInstances;
    m_pDevice->CreateBuffer(instanceBufferDesc, nullptr, &m_pSpriteInstanceBuffer);
    /* NOTE: instance buffer holds no data until sprites are added, no need to populate upon creation */
}

void Renderer::populateInstanceBuffer() {

    uint32_t dataSize = static_cast<uint32_t>(sizeof(mat4) * m_instanceData.size());
    /* NOTE: IBuffer::UpdateData() is no longer a function, use IDeviceContext::UpdateBuffer() now (see patch notes for v2.4) */
    m_pImmediateContext->UpdateBuffer(m_pSpriteInstanceBuffer, 0, dataSize, m_instanceData.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    /* Updating the buffer causes a resource state change, update back to a vertex buffer prior to render pass */
    Diligent::StateTransitionDesc barrier{
         m_pSpriteInstanceBuffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_VERTEX_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE
    };
    m_pImmediateContext->TransitionResourceStates(1, &barrier);
}


void Renderer::loadTexture(const std::string& filename) {

    Diligent::RefCntAutoPtr<Diligent::ITextureLoader> textureLoader;
    Diligent::TextureLoadInfo loadInfo;
    loadInfo.IsSRGB = true;
    Diligent::CreateTextureLoaderFromFile(filename.c_str(), Diligent::IMAGE_FILE_FORMAT_PNG, loadInfo, &textureLoader);

    /* Get pixel subres data */
    Diligent::TextureSubResData subResData = textureLoader->GetSubresourceData(0, 0);

    /* Box representing slice */
    Diligent::Box updateBox;
    updateBox.MinX = 0;
    updateBox.MinY = 0;
    updateBox.MinZ = 0;
    updateBox.MaxX = 192;
    updateBox.MaxY = 192;
    updateBox.MaxZ = 1;  /* Z-axis represents slices */

    /* Update slice w/ new texture */
    m_pImmediateContext->UpdateTexture(
        m_pSpriteTextureArray,
        0,
        m_numSprites,
        updateBox,
        subResData,
        Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE,
        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
    );
}

Sprite Renderer::loadSprite(const std::string& filename) {
    if (m_numSprites + 1 > m_maxInstances) return {};
    loadTexture(filename);

    Sprite sprite;
    sprite.index = m_numSprites;
    sprite.pos = vec3(0.0f, 0.0f, 0.1f);
    // for PMD overworld walk anim
    sprite.pages = 56;
    sprite.pagesPerAnim = 7;

    mat4 transform = translate(mat4(1.0f), vec3(sprite.pos.x, sprite.pos.y, sprite.pos.z));
    m_instanceData.push_back(transform);

    ++m_numSprites;

    populateInstanceBuffer();

    return sprite;
}

void Renderer::loadGLB(const std::string& filename) {
    Diligent::GLTF::ModelCreateInfo modelCreateInfo;
    modelCreateInfo.FileName = filename.c_str();

    m_pGlbModel = std::make_unique<Diligent::GLTF::Model>(m_pDevice, m_pImmediateContext, modelCreateInfo);

    // Both POSITION and TEXCOORD_0 have been loaded to vertex buffer at index 0, use that
    // TODO: Vertex buffer now no longer needs vector (unless animation node support is added later)
    m_pMapVertexBuffer = m_pGlbModel->GetVertexBuffer(0, m_pDevice, m_pImmediateContext);

    /* Get buffers */

    m_pMapIndexBuffer = m_pGlbModel->GetIndexBuffer();

    /* Get textures */

    for (int i = 0; i < m_pGlbModel->GetTextureCount(); ++i) {
        m_pMapTextures.push_back(m_pGlbModel->GetTexture(i, m_pDevice, m_pImmediateContext));
    };

     /* TODO: Make some mechanism where program gracefully exits on encountering a textureless GLB, rather than crashing from an unset g_texture global in shaders */
    if (!m_pMapTextures.empty()) {
        // TODO: Must bind all textures!
       // Diligent::ITextureView* texViews;
       // for (int i = 0; i < m_pMapTextures.size(); ++i) {
       //     Diligent::ITextureView* pTexView = m_pMapTextures[0]->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
       // }
        Diligent::ITextureView* pTexView = m_pMapTextures[0]->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
        m_pMapShaderResourceBinding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_texture")->Set(pTexView);
    }

    // vertex buffers, index buffer and textures now loaded, proceed to render GLB scene
}

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

void Renderer::renderMap() {
    m_pImmediateContext->SetVertexBuffers(0, 1, &m_pMapVertexBuffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
    m_pImmediateContext->SetIndexBuffer(m_pMapIndexBuffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE);
    m_pImmediateContext->SetPipelineState(m_pMapPipelineStateObj); // set pipeline to use
    m_pImmediateContext->CommitShaderResources(m_pMapShaderResourceBinding, Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE);

    /* Draw each primitive individually, or indices and vertices will overlap and draw over each other */
    for (const Diligent::GLTF::Mesh& mesh : m_pGlbModel->Meshes) {
        for (const Diligent::GLTF::Primitive& primitive : mesh.Primitives) {

            Diligent::DrawIndexedAttribs drawAttribs;
            drawAttribs.NumIndices = primitive.IndexCount;
            drawAttribs.FirstIndexLocation = primitive.FirstIndex;
            drawAttribs.BaseVertex = 0;
            drawAttribs.IndexType = Diligent::VT_UINT32;
            drawAttribs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
            m_pImmediateContext->DrawIndexed(drawAttribs);
        }
    }
}

void Renderer::renderSprites() {

    // vertex buffer has proper size, print vals

    uint64_t offsets[] = {0, 0};
    Diligent::IBuffer* pBuffers[] = {m_pSpriteVertexBuffer, m_pSpriteInstanceBuffer};
    m_pImmediateContext->SetVertexBuffers(0, _countof(pBuffers), pBuffers, offsets, Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
    m_pImmediateContext->SetIndexBuffer(m_pSpriteIndexBuffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE);
    m_pImmediateContext->SetPipelineState(m_pSpritePipelineStateObj); // set pipeline to use
    m_pImmediateContext->CommitShaderResources(m_pSpriteShaderResourceBinding, Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE);

    Diligent::DrawIndexedAttribs drawAttribs;
    drawAttribs.IndexType = Diligent::VT_UINT16; /* sprite indices are 16-bit uint */
    drawAttribs.NumIndices = m_pSpriteIndexBuffer->GetDesc().Size / sizeof(uint16_t);
    drawAttribs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
    drawAttribs.NumInstances = m_numSprites;
    m_pImmediateContext->DrawIndexed(drawAttribs);

}


void Renderer::updateUniformBuffer() {

    m_viewMatrix = m_pCamera->getViewMatrix();

}

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

    renderMap();
    renderSprites();

    m_pImmediateContext->EndRenderPass();

    m_pImmediateContext->Flush();
    m_pSwapChain->Present(1 /* VSync on */); /* NOTE: Must signal present, or unpresented resources pile up in dynamic heap and crash program */

}

void Renderer::update() {

}


