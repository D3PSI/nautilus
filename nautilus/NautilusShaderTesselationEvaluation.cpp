#ifndef NAUTILUS_SHADER_TESSELATION_EVALUATION_CPP
#define NAUTILUS_SHADER_TESSELATION_EVALUATION_CPP

#include "NautilusShaderTesselationEvaluation.hpp"

NautilusShaderTesselationEvaluation::NautilusShaderTesselationEvaluation() {
    m_glShaderType = GL_TESS_EVALUATION_SHADER;
}

NautilusShaderTesselationEvaluation::NautilusShaderTesselationEvaluation(const std::string& _path) {
    m_glShaderType = GL_TESS_EVALUATION_SHADER;
}

NautilusShaderTesselationEvaluation::~NautilusShaderTesselationEvaluation() {
}

#endif      // NAUTILUS_SHADER_TESSELATION_EVALUATION_CPP