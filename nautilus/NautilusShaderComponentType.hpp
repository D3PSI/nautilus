#ifndef NAUTILUS_SHADER_COMPONENT_TYPE_HPP
#define NAUTILUS_SHADER_COMPONENT_TYPE_HPP

namespace nautilus {

    enum NautilusShaderComponentType {
        NAUTILUS_SHADER_COMPONENT_TYPE_UNIFORM,
        NAUTILUS_SHADER_COMPONENT_TYPE_INPUT_BUFFER,
        NAUTILUS_SHADER_COMPONENT_TYPE_IN_OUT,
        NAUTILUS_SHADER_COMPONENT_TYPE_FUNCTION,
        NAUTILUS_SHADER_COMPONENT_TYPE_MAIN_FUNCTION,
        NAUTILUS_SHADER_COMPONENT_TYPE_UNSPECIFIED
    };

}

#endif      // NAUTILUS_SHADER_COMPONENT_TYPE_HPP