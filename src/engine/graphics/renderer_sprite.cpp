#include "renderer.hpp"
#include "DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h"
#include <cstdint>

/* Contains renderer functions relevant to the sprites' pipeline only */

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
    blendDesc.SrcBlendAlpha = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
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

    Diligent::LayoutElement layoutElems[] = {

        /* LayoutElement(<inputIndex>, <bufferSlot>, <numComponents>, <valueType>, <isNormalised>, <relativeOffset>, <stride>, <frequency>);
         */

        {0, 0, 4, Diligent::VT_FLOAT32, false,  0, sizeof(InstanceData), Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE},
        {1, 0, 4, Diligent::VT_FLOAT32, false, 16, sizeof(InstanceData), Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE},
        {2, 0, 4, Diligent::VT_FLOAT32, false, 32, sizeof(InstanceData), Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE},
        {3, 0, 4, Diligent::VT_FLOAT32, false, 48, sizeof(InstanceData), Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE},
        {4, 0, 1, Diligent::VT_FLOAT32, false, 64, sizeof(InstanceData), Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE},
        {5, 0, 1, Diligent::VT_FLOAT32, false, 68, sizeof(InstanceData), Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE},
        {6, 0, 1, Diligent::VT_FLOAT32, false, 72, sizeof(InstanceData), Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE},
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

/* Instance buffer for billboards */
void Renderer::createInstanceBuffer() {
    Diligent::BufferDesc instanceBufferDesc;
    instanceBufferDesc.Name = "instance data buffer";
    /* Default usage, as buffer is only updated when grid size changes */
    instanceBufferDesc.Usage     = Diligent::USAGE_DEFAULT;
    instanceBufferDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
    instanceBufferDesc.Size      = sizeof(InstanceData) * m_maxInstances;
    m_pDevice->CreateBuffer(instanceBufferDesc, nullptr, &m_pSpriteInstanceBuffer);
    /* NOTE: instance buffer holds no data until sprites are added, no need to populate upon creation */
}

void Renderer::populateInstanceBuffer() {

    m_instanceData.clear();
    //m_instanceData.reserve(m_pScene->m_pEntities.size());

    // update frame timings for all entities, then repopulate instance buffer w/ any new changes to frame
    int i = 0;
    for (const std::shared_ptr<Entity>& entity : m_pScene->m_pEntities) {

        entity->update(1.0f);
        const std::shared_ptr<Sprite> activeSprite = entity->getActiveSprite();

        int texArrayIndex = (activeSprite->index * m_maxSpriteDimensions) + ((uint8_t)entity->m_direction * activeSprite->framesPerRow) + activeSprite->frame;
        mat4 transform = translate(mat4(1.0f), entity->m_pos);


        // cast to float is necessary here, otherwise maxU and maxV = 0 from precision loss
        float maxU = activeSprite->frameWidth / static_cast<float>(m_maxSpriteFrameWidth);
        float maxV = activeSprite->frameHeight / static_cast<float>(m_maxSpriteFrameHeight);

        m_instanceData.push_back(InstanceData(transform, texArrayIndex, maxU, maxV));
        ++i;
    }

    uint32_t dataSize = static_cast<uint32_t>(sizeof(InstanceData) * m_instanceData.size());
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
    textureArrayDesc.Width  = m_maxSpriteFrameWidth;
    textureArrayDesc.Height = m_maxSpriteFrameHeight;
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

void Renderer::retrieveSprite(const std::shared_ptr<Sprite>& sprite, const std::string& cacheKey, const AnimEvent& event) {
    // fetch ref to needed cache
    const Diligent::RefCntAutoPtr<Diligent::ITexture>& texArray = m_entitySpriteCache[cacheKey];

    for (int row = 0; row < sprite->framesPerCol; ++row) {
        for (int col = 0; col < sprite->framesPerRow; ++col) {

            int cacheSliceIndex = (event * m_maxSpriteDimensions) + (row * sprite->framesPerRow) + col; // current slice index in cache tex array
            int sliceIndex = (sprite->index * m_maxSpriteDimensions) + (row * sprite->framesPerRow) + col; // current slice index in renderer tex array

            Diligent::Box srcBox;
            srcBox.MinX = 0;
            srcBox.MinY = 0;
            srcBox.MinZ = 0;
            srcBox.MaxX = 192;
            srcBox.MaxY = 192;
            srcBox.MaxZ = 1;

            Diligent::CopyTextureAttribs texCopyAttribs;
            texCopyAttribs.pSrcTexture = texArray;
            texCopyAttribs.SrcSlice = cacheSliceIndex;
            texCopyAttribs.SrcMipLevel = 0;
            texCopyAttribs.pSrcBox = &srcBox;
            texCopyAttribs.SrcTextureTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

            texCopyAttribs.pDstTexture = m_pSpriteTextureArray;
            texCopyAttribs.DstSlice = sliceIndex;
            texCopyAttribs.DstMipLevel = 0;
            texCopyAttribs.DstX = 0;
            texCopyAttribs.DstY = 0;
            texCopyAttribs.DstZ = 0;
            texCopyAttribs.DstTextureTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

            m_pImmediateContext->CopyTexture(texCopyAttribs);

        }
    }
}

void Renderer::registerSprite(const std::shared_ptr<Sprite>& sprite, const std::string& cacheKey, const AnimEvent& event) {

    if (m_numSprites + 1 > m_maxInstances) return; // reject any attempt to register more sprites than the array is able to handle

    sprite->index = m_numSprites; // first time sprite is being used, so assign the index

    retrieveSprite(sprite, cacheKey, event);

    m_instanceData.push_back(InstanceData()); // allocate a new empty slot in instance data vector

    ++m_numSprites;
}

void Renderer::swapSprite(const int& oldSpriteIndex, const std::shared_ptr<Sprite>& newSprite, const std::string& cacheKey, const AnimEvent& event) {

    // fetch ref to needed cache
    const Diligent::RefCntAutoPtr<Diligent::ITexture>& texArray = m_entitySpriteCache[cacheKey];

    // copy each frame into its own slice of the texture array
    for (int row = 0; row < newSprite->framesPerCol; ++row) {
        for (int col = 0; col < newSprite->framesPerRow; ++col) {

            int cacheSliceIndex = (event * m_maxSpriteDimensions) + (row * newSprite->framesPerRow) + col; // current slice index in cache tex array
            int sliceIndex = (oldSpriteIndex * m_maxSpriteDimensions) + (row * newSprite->framesPerRow) + col; // current slice index in renderer tex array

            Diligent::Box srcBox;
            srcBox.MinX = 0;
            srcBox.MinY = 0;
            srcBox.MinZ = 0;
            srcBox.MaxX = 192;
            srcBox.MaxY = 192;
            srcBox.MaxZ = 1;

            Diligent::CopyTextureAttribs texCopyAttribs;
            texCopyAttribs.pSrcTexture = texArray;
            texCopyAttribs.SrcSlice = cacheSliceIndex;
            texCopyAttribs.SrcMipLevel = 0;
            texCopyAttribs.pSrcBox = &srcBox;
            texCopyAttribs.SrcTextureTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

            texCopyAttribs.pDstTexture = m_pSpriteTextureArray;
            texCopyAttribs.DstSlice = sliceIndex;
            texCopyAttribs.DstMipLevel = 0;
            texCopyAttribs.DstX = 0;
            texCopyAttribs.DstY = 0;
            texCopyAttribs.DstZ = 0;
            texCopyAttribs.DstTextureTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

            m_pImmediateContext->CopyTexture(texCopyAttribs);
        }
    }
}

/* --- DRAW CALLS --- */

void Renderer::renderSprites() {

    uint64_t offsets[] = {0};
    Diligent::IBuffer* pBuffers[] = {m_pSpriteInstanceBuffer};
    m_pImmediateContext->SetVertexBuffers(0, _countof(pBuffers), pBuffers, offsets, Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);

    m_pImmediateContext->SetPipelineState(m_pSpritePipelineStateObj); // set pipeline to use
    m_pImmediateContext->CommitShaderResources(m_pSpriteShaderResourceBinding, Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE);

    Diligent::DrawAttribs drawAttribs;
    drawAttribs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
    drawAttribs.NumInstances = m_numSprites;
    drawAttribs.NumVertices = 1;
    m_pImmediateContext->Draw(drawAttribs);

}
