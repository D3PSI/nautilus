#ifndef NAUTILUS_SHELL_CONTEXT_HPP
#define NAUTILUS_SHELL_CONTEXT_HPP

namespace nautilus {

    enum NautilusShellContext {
        NAUTILUS_SHELL_CONTEXT_FULLSCREEN,
        NAUTILUS_SHELL_CONTEXT_BORDERLESS,
        NAUTILUS_SHELL_CONTEXT_WINDOWED,
        NAUTILUS_SHELL_CONTEXT_DEFAULT = NAUTILUS_SHELL_CONTEXT_WINDOWED
    };

}

#endif      // NAUTILUS_SHELL_CONTEXT_HPP