#ifndef NAUTILUS_PIPELINE_CPP
#define NAUTILUS_PIPELINE_CPP

#include "NautilusPipeline.hpp"

NautilusPipeline::NautilusPipeline() {
}

NautilusPipeline::NautilusPipeline(const std::string& _identifier) : m_identifier(_identifier) {
}

NautilusPipeline::~NautilusPipeline() {
}

nautilus::NautilusStatus NautilusPipeline::compute(NautilusShaderCompute& _shader) { 
    m_shaders[nautilus::NAUTILUS_SHADER_STAGE_COMPUTE] = &_shader; 
    return nautilus::NAUTILUS_STATUS_OK;
}

nautilus::NautilusStatus NautilusPipeline::fragment(NautilusShaderFragment& _shader) {
    m_shaders[nautilus::NAUTILUS_SHADER_STAGE_FRAGMENT] = &_shader; 
    return nautilus::NAUTILUS_STATUS_OK;
}

nautilus::NautilusStatus NautilusPipeline::geometry(NautilusShaderGeometry& _shader) {
    m_shaders[nautilus::NAUTILUS_SHADER_STAGE_GEOMETRY] = &_shader; 
    return nautilus::NAUTILUS_STATUS_OK;
}

nautilus::NautilusStatus NautilusPipeline::tesselation(NautilusShaderTesselation& _shader) {
    m_shaders[nautilus::NAUTILUS_SHADER_STAGE_TESSELATION_CONTROL] = _shader.control(); 
    m_shaders[nautilus::NAUTILUS_SHADER_STAGE_TESSELATION_EVALUATION] = _shader.evaluation(); 
    return nautilus::NAUTILUS_STATUS_OK;
}

nautilus::NautilusStatus NautilusPipeline::tesselationControl(NautilusShaderTesselationControl& _shader) {
    m_shaders[nautilus::NAUTILUS_SHADER_STAGE_TESSELATION_CONTROL] = &_shader; 
    return nautilus::NAUTILUS_STATUS_OK;
}

nautilus::NautilusStatus NautilusPipeline::tesselationEvaluation(NautilusShaderTesselationEvaluation& _shader) {
    m_shaders[nautilus::NAUTILUS_SHADER_STAGE_TESSELATION_EVALUATION] = &_shader; 
    return nautilus::NAUTILUS_STATUS_OK;
}

nautilus::NautilusStatus NautilusPipeline::vertex(NautilusShaderVertex& _shader) {
    m_shaders[nautilus::NAUTILUS_SHADER_STAGE_VERTEX] = &_shader; 
    return nautilus::NAUTILUS_STATUS_OK;
}

nautilus::NautilusStatus NautilusPipeline::attach(nautilus::NautilusAPI _api) {
    compile(_api);
    return nautilus::NAUTILUS_STATUS_OK;
}

nautilus::NautilusStatus NautilusPipeline::compile(nautilus::NautilusAPI _api) {
    switch(_api) {
    case nautilus::NAUTILUS_API_OPENGL:
        compileOpenGL();
        break;
    case nautilus::NAUTILUS_API_VULKAN:
        compileVulkan();
        break;
    default:
        nautilus::logger::log("Unsupported API", nautilus::NAUTILUS_STATUS_FATAL);
        break;
    }
    return nautilus::NAUTILUS_STATUS_OK;
}

const std::string& NautilusPipeline::identifier() {
    return m_identifier;
}

nautilus::NautilusStatus NautilusPipeline::activate(nautilus::NautilusAPI _api) {
    switch(_api) {
    case nautilus::NAUTILUS_API_OPENGL:
        glUseProgram(m_program);
        break;
    case nautilus::NAUTILUS_API_VULKAN:
        break;
    default:
        nautilus::logger::log("Unsupported API", nautilus::NAUTILUS_STATUS_FATAL);
        break;
    }
    return nautilus::NAUTILUS_STATUS_OK;
}

nautilus::NautilusStatus NautilusPipeline::compileOpenGL() {
    for(auto entry : m_shaders) 
        entry.second->compile(nautilus::NAUTILUS_API_OPENGL);
    m_program = glCreateProgram();
    for(auto entry : m_shaders)
        glAttachShader(m_program, entry.second->handle());
    glLinkProgram(m_program);
    int32_t success;
    char infoLog[512];
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(m_program, 512, nullptr, infoLog);
        nautilus::logger::log("Failed to link shader: " + std::string(infoLog), nautilus::NAUTILUS_STATUS_FATAL);
    }
    for(auto entry : m_shaders)
        entry.second->del(nautilus::NAUTILUS_API_OPENGL);
    return nautilus::NAUTILUS_STATUS_OK;
}

nautilus::NautilusStatus NautilusPipeline::compileVulkan() {
    return nautilus::NAUTILUS_STATUS_OK;
}

#endif      // NAUTILUS_PIPELINE_CPP