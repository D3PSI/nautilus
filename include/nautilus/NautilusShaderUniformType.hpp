#ifndef NAUTILUS_SHADER_UNIFORM_TYPE_HPP
#define NAUTILUS_SHADER_UNIFORM_TYPE_HPP

namespace nautilus {

    enum NautilusShaderUniformType {
        NAUTILUS_SHADER_UNIFORM_TYPE_FLOAT,
        NAUTILUS_SHADER_UNIFORM_TYPE_DOUBLE,
        NAUTILUS_SHADER_UNIFORM_TYPE_INT,
        NAUTILUS_SHADER_UNIFORM_TYPE_VEC_2,
        NAUTILUS_SHADER_UNIFORM_TYPE_VEC_3,
        NAUTILUS_SHADER_UNIFORM_TYPE_VEC_4,
        NAUTILUS_SHADER_UNIFORM_TYPE_MAT_2,
        NAUTILUS_SHADER_UNIFORM_TYPE_MAT_3,
        NAUTILUS_SHADER_UNIFORM_TYPE_MAT_4
    };

}

#endif      // NAUTILUS_SHADER_UNIFORM_TYPE_HPP