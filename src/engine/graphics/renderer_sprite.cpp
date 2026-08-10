#include "renderer.hpp"

/* Contains renderer functions relevant to the sprites' pipeline only */

// index buffer removed, unused.
// TODO: input layout not working properly, all calculations setting instance data seem fine

/* Indices are always the same for a quad, so this is fine as a constant */
const std::vector<uint16_t> billboardIndices = {
    0, 1, 2, 3, 0
};

/* --- PIPELINE --- */

/* Sprite PSO creation, needs manual vertex and index buffer creation and includes per-instance data such as texture array index and transforms */
void Renderer::createSpritePipelineState() {
    Diligent::GraphicsPipelineStateCreateInfo PipelineStateObjCreateInfo;

    PipelineStateObjCreateInfo.PSODesc.Name = "Sprite Pipeline State Object";
    PipelineStateObjCreateInfo.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;

    /* Defines what kind of primitives will be rendered by this pipeline state */
    PipelineStateObjCreateInfo.GraphicsPipeline.PrimitiveTopology                       = Diligent::PRIMITIVE_TOPOLOGY_POINT_LIST;
    /* Face culling mode */
    PipelineStateObjCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode                 = Diligent::CULL_MODE_NONE;
    /* Enable depth testing */
    PipelineStateObjCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable            = true;
    /* Render pass used by this pipeline */
    PipelineStateObjCreateInfo.GraphicsPipeline.pRenderPass                             = m_pRenderPass;
    /* initial supbpass to start render pass from */
    PipelineStateObjCreateInfo.GraphicsPipeline.SubpassIndex                            = 0;

    /* NOTE: Weirdly, Blend is applied to a 'render target', even if you're using a render pass system? */
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

    // --- NO SHADER INPUT LAYOUT, AS PIPELINE TAKES NO INPUT FOR VERTEX SHADER --- //

    /* Create pipeline state */

    /* Set shaders */
    PipelineStateObjCreateInfo.pVS                                         = pSpriteVertexShader; // may be redundant?
    PipelineStateObjCreateInfo.pGS                                         = pSpriteGeometryShader;
    PipelineStateObjCreateInfo.pPS                                         = pSpriteFragmentShader;

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
    /* Set SpriteConstants for all shaders */
    m_pSpritePipelineStateObj->GetStaticVariableByName(Diligent::SHADER_TYPE_GEOMETRY, "SpriteConstants")->Set(m_pSpriteConstants);

    /* Create a shader resource binding (SRB) through which we can alter the mutable value of shader variables */
    m_pSpritePipelineStateObj->CreateShaderResourceBinding(&m_pSpriteShaderResourceBinding, true);
}

/* --- BUFFERS --- */

/* Index buffer for billboards */
/*void Renderer::createIndexBuffer() {

    Diligent::BufferDesc indexBufferDesc;
    indexBufferDesc.Name      = "billboard index buffer";
    indexBufferDesc.Usage     = Diligent::USAGE_IMMUTABLE;
    indexBufferDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
    indexBufferDesc.Size      = billboardIndices.size() * sizeof(uint16_t);

    Diligent::BufferData indexBufferData;
    indexBufferData.pData    = billboardIndices.data();
    indexBufferData.DataSize = billboardIndices.size() * sizeof(uint16_t);
    m_pDevice->CreateBuffer(indexBufferDesc, &indexBufferData, &m_pSpriteIndexBuffer);
}*/

void Renderer::createPerSpriteUniformBuffer() {
    Diligent::BufferDesc uniformBufferDesc;
    uniformBufferDesc.Name           = "sprite constants desc";
    uniformBufferDesc.Size           = sizeof(InstanceData);
    uniformBufferDesc.Usage          = Diligent::USAGE_DYNAMIC;
    uniformBufferDesc.BindFlags      = Diligent::BIND_UNIFORM_BUFFER;
    uniformBufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
    m_pDevice->CreateBuffer(uniformBufferDesc, nullptr, &m_pSpriteConstants);
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

/* --- LOADERS --- */

int Renderer::registerSprite(const std::shared_ptr<Sprite>& sprite) {
    if (m_numSprites + 1 > m_maxInstances) return -1;

    sprite->index = m_numSprites; // first time sprite is being used, so assign the index

    Diligent::RefCntAutoPtr<Diligent::ITextureLoader> textureLoader;
    Diligent::TextureLoadInfo loadInfo;
    loadInfo.IsSRGB = true;
    Diligent::CreateTextureLoaderFromFile(sprite->filepath.c_str(), Diligent::IMAGE_FILE_FORMAT_PNG, loadInfo, &textureLoader);

    // UPDATE: all slices can't be updated in one go by one box, they must be individually updated

    sprite->framesPerRow = textureLoader->GetTextureDesc().GetWidth() / 192;
    int framesPerCol = textureLoader->GetTextureDesc().GetHeight() / 192;

    // 2. Get the full texture data
    Diligent::TextureSubResData subResData = textureLoader->GetSubresourceData(0, 0);
    const unsigned char* fullData = static_cast<const unsigned char*>(subResData.pData);
    int rowPitch = subResData.Stride;  // Bytes per row in the source data

    // copy each frame into its own slice
    for (int row = 0; row < framesPerCol; ++row) {
        for (int col = 0; col < sprite->framesPerRow; ++col) {

            int sliceIndex = (sprite->index * m_maxSpriteDimensions) + (row * sprite->framesPerRow) + col;

            // 4. Create a staging buffer for this cell
            uint32_t cellSize = 192 * 192 * 4;  // RGBA = 4 bytes
            std::vector<unsigned char> cellData(cellSize);

            // 5. Copy the cell from the spritesheet to the staging buffer
            for (int y = 0; y < 192; ++y) {
                int srcOffset = ((row * 192 + y) * rowPitch) + (col * 192 * 4);
                int dstOffset = y * 192 * 4;
                memcpy(cellData.data() + dstOffset, fullData + srcOffset, 192 * 4);
            }

            // 6. Upload the cell to the texture array
            Diligent::BufferDesc stagingDesc;
            stagingDesc.Name = "staging buffer";
            stagingDesc.Usage = Diligent::USAGE_STAGING;
            stagingDesc.BindFlags = Diligent::BIND_NONE;
            stagingDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
            stagingDesc.Size = cellSize;

            Diligent::BufferData stagingData;
            stagingData.pData = cellData.data();
            stagingData.DataSize = cellSize;

            Diligent::RefCntAutoPtr<Diligent::IBuffer> pStagingBuffer;
            m_pDevice->CreateBuffer(stagingDesc, &stagingData, &pStagingBuffer);

            // 7. Copy from staging buffer to texture array
            Diligent::Box updateBox;
            updateBox.MinX = 0;
            updateBox.MinY = 0;
            updateBox.MinZ = 0;
            updateBox.MaxX = 192;
            updateBox.MaxY = 192;
            updateBox.MaxZ = 1;

            Diligent::TextureSubResData cellSubRes;
            cellSubRes.pData = cellData.data();
            cellSubRes.Stride = 192 * 4;
            cellSubRes.DepthStride = 0;

            m_pImmediateContext->UpdateTexture(
                m_pSpriteTextureArray,
                0,
                sliceIndex,
                updateBox,
                cellSubRes,
                Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE,
                Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
            );
        }
    }

    m_instanceData.push_back(InstanceData()); // allocate a new empty slot in instance data vector

    ++m_numSprites;

    return m_numSprites - 1; // unused return?
}

void Renderer::swapSprite(const int& oldSpriteIndex, const std::shared_ptr<Sprite>& newSprite) {
    // NOTE: sprite->index refers to the Entity 'number' that Sprite belongs to. The start index in the tex array
    // the Sprite's texture exists in can be found with index * m_maxSpriteDimensions
    Diligent::RefCntAutoPtr<Diligent::ITextureLoader> textureLoader;
    Diligent::TextureLoadInfo loadInfo;
    loadInfo.IsSRGB = true;
    Diligent::CreateTextureLoaderFromFile(newSprite->filepath.c_str(), Diligent::IMAGE_FILE_FORMAT_PNG, loadInfo, &textureLoader);

    // UPDATE: all slices can't be updated in one go by one box, they must be individually updated

    newSprite->framesPerRow = textureLoader->GetTextureDesc().GetWidth() / 192;
    int framesPerCol = textureLoader->GetTextureDesc().GetHeight() / 192;

    /* Get pixel subres data */
    Diligent::TextureSubResData subResData = textureLoader->GetSubresourceData(0, 0);

    // NOTE: All Sprites, regardless of how many slices they actually require will be treated as having a number of
    // frames equal to the constexpr m_maxSpriteDimensions. This allows all spritesheets to occupy equal amounts of memory,
    // meaning differently-sized spritesheets will not interfere with each other

    for (int row = 0; row < framesPerCol; ++row) {
        for (int col = 0; col < newSprite->framesPerRow; ++col) {

            int sliceIndex = oldSpriteIndex * m_maxSpriteDimensions + (row * newSprite->framesPerRow) + col;

            Diligent::Box updateBox;
            updateBox.MinX = col * 192;
            updateBox.MinY = row * 192;
            updateBox.MinZ = 0;
            updateBox.MaxX = (col + 1) * 192;
            updateBox.MaxY = (row + 1) * 192;
            updateBox.MaxZ = 1;

            /* Update slice w/ new texture */
            m_pImmediateContext->UpdateTexture(
                m_pSpriteTextureArray,
                0,
                sliceIndex,
                updateBox,
                subResData,
                Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE,
                Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
            );
        }
    }
}

/* --- DRAW CALLS --- */

void Renderer::renderSprites() {

    // NO VERTEX BUFFER

    // m_pImmediateContext->SetIndexBuffer(m_pSpriteIndexBuffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE);
    m_pImmediateContext->SetPipelineState(m_pSpritePipelineStateObj); // set pipeline to use
    m_pImmediateContext->CommitShaderResources(m_pSpriteShaderResourceBinding, Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE);

    Diligent::DrawAttribs drawAttribs;
    //drawAttribs.IndexType = Diligent::VT_UINT16; /* sprite indices are 16-bit uint */
    //drawAttribs.NumIndices = m_pSpriteIndexBuffer->GetDesc().Size / sizeof(uint16_t);
    drawAttribs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
    drawAttribs.NumInstances = m_numSprites;
    drawAttribs.NumVertices = 1;
    m_pImmediateContext->Draw(drawAttribs);

}
