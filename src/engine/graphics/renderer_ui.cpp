#include "renderer.hpp"

void Renderer::createUiPipelineState() {
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

        // TO BE DETERMINED
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
    m_pMapPipelineStateObj->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "Constants")->Set(m_pFrameConstants);

    /* Create a shader resource binding (SRB) through which we can alter the mutable value of shader variables */
  //  m_pMapPipelineStateObj->CreateShaderResourceBinding(&m_pMapShaderResourceBinding, true);
}

void Renderer::renderUi() {

}
