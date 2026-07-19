#include "renderer.hpp"

/* Contains renderer functions relevant to the map's pipeline only */

/* --- PIPELINE --- */

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

/* --- LOADER FUNCS --- */

void Renderer::loadGLB(const std::string& filename) {
    Diligent::GLTF::ModelCreateInfo modelCreateInfo;
    modelCreateInfo.FileName = filename.c_str();

    m_pGlbModel = std::make_unique<Diligent::GLTF::Model>(m_pDevice, m_pImmediateContext, modelCreateInfo);

    // Both POSITION and TEXCOORD_0 have been loaded to vertex buffer at index 0, use that
    m_pMapVertexBuffer = m_pGlbModel->GetVertexBuffer(0, m_pDevice, m_pImmediateContext);

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

/* --- DRAW CALLS --- */

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
