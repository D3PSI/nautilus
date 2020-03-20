#ifndef DEV_SHELL_HPP
#define DEV_SHELL_HPP

#include <nautilus/Nautilus.hpp>

class DevShell 
    : public NautilusOpenGLShell {
    using NautilusOpenGLShell::NautilusOpenGLShell;
public:

    /**
     * Gets executed when the shell is attached to the core
     * @return Returns a NautilusStatus status code
     */
    void onAttach(void); 

    /**
     * Gets executed at the specified frequency to compute rendering operations
     * @return Returns a NautilusStatus status code
     */
    void onRender(void);

};

#endif      // DEV_SHELL_HPP