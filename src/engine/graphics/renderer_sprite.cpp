#include "renderer.hpp"

/* Contains renderer functions relevant to the sprites' pipeline only */


/* Billboards will all be identically-sized quads (made of two triangle primitives),
 * they therefore always have the same vertices and indices, which are defined here
 */

// NOTE: billboards are actually generated in GS, these are equivalently-sized constants for buffer init
const std::vector<uint16_t> billboardIndices = {
    0, 1, 2, 3, 0
};

const std::vector<vec3> billboardVertices = {
    {-0.5f, -0.5f, 0.0f},
    { 0.5f, -0.5f, 0.0f},
    { 0.5f,  0.5f, 0.0f},
    {-0.5f,  0.5f, 0.0f}
};


/* --- PIPELINE --- */

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
         * LayoutElement(<inputIndex>, <bufferSlot>, <numComponents>, <valueType>, <isNormalised>, <relativeOffset>, <stride>, <frequency>);
         */

        /* --- Per-vertex Data (from vertex buffer) --- */

        // vertex position
        Diligent::LayoutElement{0, 0, 3, Diligent::VT_FLOAT32, false},
        // NOTE: UVs not needed as they are always the same and set by the geometry shader

        /* --- Per-instance Data (from instance buffer) --- */

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
        // texture array index of current frame
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

/* --- BUFFERS --- */

/* Vertex buffer for billboards */
void Renderer::createVertexBuffer() {
    Diligent::BufferDesc vertexBufferDesc;
    vertexBufferDesc.Name      = "billboard vertex buffer";
    vertexBufferDesc.Usage     = Diligent::USAGE_IMMUTABLE;
    vertexBufferDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
    vertexBufferDesc.Size      = billboardVertices.size() * sizeof(vec3);
    vertexBufferDesc.Size = sizeof(billboardVertices);

    Diligent::BufferData vertexBufferData;
    vertexBufferData.pData    = billboardVertices.data();
    vertexBufferData.DataSize = billboardVertices.size() * sizeof(vec3);
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


/* --- TEXTURES --- */

/* creates texture array for sprites */
void Renderer::createSpriteTextureArray() {
    Diligent::TextureDesc textureArrayDesc;
    textureArrayDesc.ArraySize = m_maxInstances * m_maxSpriteDimensions;
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

void Renderer::registerTexture(const std::string& filepath) {

    Diligent::RefCntAutoPtr<Diligent::ITextureLoader> textureLoader;
    Diligent::TextureLoadInfo loadInfo;
    loadInfo.IsSRGB = true;
    Diligent::CreateTextureLoaderFromFile(filepath.c_str(), Diligent::IMAGE_FILE_FORMAT_PNG, loadInfo, &textureLoader);


    /* Get pixel subres data */
    Diligent::TextureSubResData subResData = textureLoader->GetSubresourceData(0, 0);
    /* Box representing slice */
    Diligent::Box updateBox;
    updateBox.MinX = 0;
    updateBox.MinY = 0;
    updateBox.MinZ = 0;
    updateBox.MaxX = 192;
    updateBox.MaxY = 192;
    updateBox.MaxZ = m_maxSpriteDimensions;  /* Z-axis represents slices */

    // NOTE: All Sprites, regardless of how many slices they actually require will be treated as having a number of
    // frames equal to the constexpr m_maxSpriteDimensions. This allows all spritesheets to occupy equal amounts of memory,
    // meaning differently-sized spritesheets will not interfere with each other

    /* Update slice w/ new texture */
    m_pImmediateContext->UpdateTexture(
        m_pSpriteTextureArray,
        0,
        m_numSprites * m_maxSpriteDimensions,
        updateBox,
        subResData,
        Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE,
        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
    );
}

void Renderer::swapTexture(const int& oldTextureIndex, const std::string& newTextureFilepath) {
    Diligent::RefCntAutoPtr<Diligent::ITextureLoader> textureLoader;
    Diligent::TextureLoadInfo loadInfo;
    loadInfo.IsSRGB = true;
    Diligent::CreateTextureLoaderFromFile(newTextureFilepath.c_str(), Diligent::IMAGE_FILE_FORMAT_PNG, loadInfo, &textureLoader);

    /* Get pixel subres data */
    Diligent::TextureSubResData subResData = textureLoader->GetSubresourceData(0, 0);

    /* Box representing slice */
    Diligent::Box updateBox;
    updateBox.MinX = 0;
    updateBox.MinY = 0;
    updateBox.MinZ = 0;
    updateBox.MaxX = 192;
    updateBox.MaxY = 192;
    updateBox.MaxZ = m_maxSpriteDimensions;  /* Z-axis represents slices */

    // NOTE: All Sprites, regardless of how many slices they actually require will be treated as having a number of
    // frames equal to the constexpr m_maxSpriteDimensions. This allows all spritesheets to occupy equal amounts of memory,
    // meaning differently-sized spritesheets will not interfere with each other

    /* Update slice w/ new texture */
    m_pImmediateContext->UpdateTexture(
        m_pSpriteTextureArray,
        0,
        oldTextureIndex,
        updateBox,
        subResData,
        Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE,
        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
    );
}

/* --- LOADERS --- */

int Renderer::registerSprite(const std::shared_ptr<Sprite>& sprite) {
    if (m_numSprites + 1 > m_maxInstances) return -1;

    sprite->index = m_numSprites; // first time sprite is being used, so assign the index

    registerTexture(sprite->filepath);

    mat4 transform = translate(mat4(1.0f), vec3(sprite->pos.x, sprite->pos.y, sprite->pos.z));
    m_instanceData.push_back(transform);

    ++m_numSprites;

    return m_numSprites - 1;
}

void Renderer::swapSprite(const int& oldSpriteIndex, const std::shared_ptr<Sprite>& newSprite) {
    // NOTE: sprite->index refers to the Entity 'number' that Sprite belongs to. The start index in the tex array
    // the Sprite's texture exists in can be found with index * m_maxSpriteDimensions
    swapTexture(oldSpriteIndex * m_maxSpriteDimensions, newSprite->filepath);

    mat4 transform = translate(mat4(1.0f), vec3(newSprite->pos.x, newSprite->pos.y, newSprite->pos.z));
    m_instanceData.push_back(transform);

    populateInstanceBuffer();
}

/* --- DRAW CALLS --- */

void Renderer::renderSprites() {

    /* This can no longer just render the texture. The texture is now comprised of an entire, multi-frame spritesheet
     * This should render all entities, but the texture should be the specific slice that is needed for the animation */

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
