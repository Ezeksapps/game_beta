#include "renderer.hpp"
#include "DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h"
#include <cstdint>

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

// TODO: Use CopyTexture() instead
void Renderer::loadSprite(const std::shared_ptr<Sprite>& sprite, const std::string& cacheKey, const AnimEvent& event) {
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

int Renderer::registerSprite(const std::shared_ptr<Sprite>& sprite, const std::string& cacheKey, const AnimEvent& event) {

    if (m_numSprites + 1 > m_maxInstances) return -1;

    sprite->index = m_numSprites; // first time sprite is being used, so assign the index

    // TODO: Get cache key and event from entity
    loadSprite(sprite, cacheKey, event);

    m_instanceData.push_back(InstanceData()); // allocate a new empty slot in instance data vector

    ++m_numSprites;

    return m_numSprites - 1; // << UNUSED RETURN, TODO: REMOVE
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

    // NO VERTEX BUFFER

    /* Index buffer may be re-enabled in future, left commented out for now */

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
