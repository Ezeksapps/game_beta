#include "renderer.hpp"
#include "../ui/ui.h"

Diligent::Viewport g_viewport;

struct nk_rect {float x,y,w,h;};

void Renderer::createUiPipelineState() {

    //nk_diligent_context* nk_dlg_ctx = new nk_diligent_context;

   //nk_init_default(&nk_dlg_ctx->ctx, 0);
   // nk_buffer_init_default(&nk_dlg_ctx->cmds);

    Diligent::GraphicsPipelineStateCreateInfo PipelineStateObjCreateInfo;

    PipelineStateObjCreateInfo.PSODesc.Name = "UI Pipeline State Object";
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

    /* Vertex shader */
    Diligent::RefCntAutoPtr<Diligent::IShader> pMapVertexShader;
    {
        shaderCreateInfo.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
        shaderCreateInfo.EntryPoint      = "main";
        shaderCreateInfo.Desc.Name       = "vertex shader desc";
        shaderCreateInfo.FilePath        = "ui_vertex.glsl";
        m_pDevice->CreateShader(shaderCreateInfo, &pMapVertexShader);
    }

    /* NOTE: Diligent Engine internally refers to fragment shaders as 'pixel shaders' */
    Diligent::RefCntAutoPtr<Diligent::IShader> pMapFragmentShader;
    {
        shaderCreateInfo.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
        shaderCreateInfo.EntryPoint      = "main";
        shaderCreateInfo.Desc.Name       = "fragment shader desc";
        shaderCreateInfo.FilePath        = "ui_fragment.glsl";
        m_pDevice->CreateShader(shaderCreateInfo, &pMapFragmentShader);
    }

    /* Layout of shader input (that being what is passed to the first shader in the pipeline, the vertex shader) */
    Diligent::LayoutElement layoutElems[] = {

        /* LayoutElement(<inputIndex>, <bufferSlot>, <numComponents>, <valueType>, <isNormalised>, <relativeOffset>, <stride>, <frequency>);
         */

        {0, 0, 2, Diligent::VT_FLOAT32},     // pos
        {1, 0, 2, Diligent::VT_FLOAT32},     // uv
        {2, 0, 4, Diligent::VT_UINT8, true}  // col
    };

    /* Create pipeline state */

    /* Set shaders */
    PipelineStateObjCreateInfo.pVS                                         = pMapVertexShader;
    PipelineStateObjCreateInfo.pPS                                         = pMapFragmentShader;
    /* Layout of input elements to shader pipeline */
    PipelineStateObjCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = layoutElems;
    //PipelineStateObjCreateInfo.GraphicsPipeline.InputLayout.NumElements    = _countof(layoutElems);
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
    m_pDevice->CreateGraphicsPipelineState(PipelineStateObjCreateInfo, &m_pUiPipelineStateObj);

    /* Set Constants variable (holds matrices for current frame) for all shaders that use it */
    m_pUiPipelineStateObj->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "Constants")->Set(m_pFrameConstants);

    /* Create a shader resource binding (SRB) through which we can alter the mutable value of shader variables */
    m_pUiPipelineStateObj->CreateShaderResourceBinding(&m_pUiShaderResourceBinding, true);

    {
        Diligent::BufferDesc vertexBufferDesc;
        vertexBufferDesc.Name           = "Nuklear vertex buffer";
        vertexBufferDesc.BindFlags      = Diligent::BIND_VERTEX_BUFFER;
        vertexBufferDesc.Size           = 512 * 1024;
        vertexBufferDesc.Usage          = Diligent::USAGE_DYNAMIC;
        vertexBufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
        m_pDevice->CreateBuffer(vertexBufferDesc, nullptr, &m_pUiVertexBuffer);
    }

    {
        Diligent::BufferDesc indexBufferDesc;
        indexBufferDesc.Name           = "Nuklear index buffer";
        indexBufferDesc.BindFlags      = Diligent::BIND_INDEX_BUFFER;
        indexBufferDesc.Size           = 128 * 1024;
        indexBufferDesc.Usage          = Diligent::USAGE_DYNAMIC;
        indexBufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
        m_pDevice->CreateBuffer(indexBufferDesc, nullptr, &m_pUiIndexBuffer);
    }

    g_viewport.TopLeftX = 0.0f;
    g_viewport.TopLeftY = 0.0f;
    g_viewport.Width    = static_cast<float>(m_windowWidth);
    g_viewport.Height   = static_cast<float>(m_windowHeight);
    g_viewport.MinDepth = 0.0f;
    g_viewport.MaxDepth = 1.0f;

}

void Renderer::renderUi() {
    const float blendFactors[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    Diligent::IBuffer*    pBuffers[]           = {m_pUiVertexBuffer};
    m_pImmediateContext->SetVertexBuffers(0, 1, pBuffers, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
    m_pImmediateContext->SetIndexBuffer(m_pUiIndexBuffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    m_pImmediateContext->SetPipelineState(m_pUiPipelineStateObj); // set pipeline to use
    m_pImmediateContext->CommitShaderResources(m_pUiShaderResourceBinding, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    m_pImmediateContext->SetBlendFactors(blendFactors);

    Diligent::DrawIndexedAttribs attribs;
    attribs.Flags     = Diligent::DRAW_FLAG_VERIFY_STATES;
    attribs.IndexType = Diligent::VT_UINT16;

    m_pImmediateContext->SetViewports(1, &g_viewport, static_cast<uint32_t>(g_viewport.Width), static_cast<uint32_t>(g_viewport.Height));


    // Convert from command queue into draw list and draw to screen
    // Load draw vertices & elements directly into vertex + element buffer
    const struct nk_draw_command* cmd = nullptr;

    uint32_t offset = 0;
    {
        Diligent::MapHelper<UiVertex> vertices(m_pImmediateContext, m_pUiVertexBuffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
        Diligent::MapHelper<uint16_t> indices(m_pImmediateContext, m_pUiIndexBuffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);

        convertVertices(vertices, indices);
    }

    execDrawCmds([this, attribs](struct nk_rect clipRect, void* texPtr, unsigned int elemCount) { // resolve issue: Lambda not ideal here
        Diligent::ITextureView* textureView = reinterpret_cast<Diligent::ITextureView*>(texPtr);


        Diligent::Rect scissor;
        scissor.left   = std::max(static_cast<int32_t>(clipRect.x), 0);
        scissor.right  = std::max(static_cast<int32_t>(clipRect.x + clipRect.w), scissor.left);
        scissor.top    = std::max(static_cast<int32_t>(clipRect.y), 0);
        scissor.bottom = std::max(static_cast<int32_t>(clipRect.y + clipRect.h), scissor.top);

        attribs.NumIndices         = static_cast<uint32_t>(elemCount);
        attribs.FirstIndexLocation = offset;
        m_pImmediateContext->SetScissorRects(1, &scissor, static_cast<uint32_t>(g_viewport.Width), static_cast<uint32_t>(g_viewport.Height));
        m_pImmediateContext->DrawIndexed(attribs);
        offset += cmd->elem_count;
    });
}
