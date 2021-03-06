#ifndef NAUTILUS_SHADER_HPP
#define NAUTILUS_SHADER_HPP

#include "NautilusAPI.hpp"
#include "NautilusStatus.hpp"
#include "NautilusShaderComponent.hpp"
#include "NautilusShaderComponentUniform.hpp"
#include "NautilusShaderComponentUniformType.hpp"
#include "NautilusShaderComponentInOut.hpp"

#include <glad/glad.h>

#include <string>
#include <vector>

namespace nautilus {

    class NautilusShader {
    public:

        /**
         * Default constructor
         */ 
        NautilusShader(void);

        /**
         * Compiles shader from a source file
         * @param _path The path to the source file
         * @return Returns a pointer to this instance
         */ 
        NautilusShader* fromFile(const std::string& _path);

        /**
         * Adds a uniform variable to the shader source
         * @param _comp The shader component to attach to the source
         * @return Returns a NautilusStatus status code
         */ 
        NautilusStatus attach(NautilusShaderComponent* _comp);

        /**
         * Compiles a shader
         * @param _api The nautilus api to construct shader source for
         * @return Returns a NautilusStatus status code
         */ 
        NautilusStatus compile(NautilusAPI _api);

        /**
         * Returns the OpenGL handle for the shader
         * @return Returns a handle to an OpenGL shader object
         */ 
        uint32_t handle(void);

        /**
         * Deletes a shader after compilation and linkage
         * @param _api The API with which the shader has been compiled
         * @return Returns a NautilusStatus status code
         */ 
        NautilusStatus del(NautilusAPI _api);

        /**
         * Default destructor
         */ 
        ~NautilusShader(void);

    private:

    protected:

        std::string                     m_shadersrc;
        uint32_t                        m_handle            = 0;
        uint32_t                        m_glShaderType;
        std::string                     m_path              = "";

        /**
         * Compiles a shader for OpenGL
         * @return Returns a NautilusStatus status code
         */ 
        NautilusStatus compileOpenGL(void);

        /**
         * Compiles a shader for Vulkan
         * @return Returns a NautilusStatus status code
         */ 
        NautilusStatus compileVulkan(void);

    };

}

#endif      // NAUTILUS_SHADER_HPP