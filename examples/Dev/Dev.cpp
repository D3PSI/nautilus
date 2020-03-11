#include <Nautilus.hpp>

#include "DevShell.hpp"

#include <iostream>

namespace dev {

    NautilusCore*   core;
    NautilusShell*  shell;

    /**
     * Initializes everything
     * @return Returns a NautilusStatus status code
     */
    NautilusStatus init(void) {
        core = new NautilusCore();
        core->start();
        shell = new DevShell();
        shell->setShellContext(NAUTILUS_SHELL_CONTEXT_WINDOWED);
        shell->setShellTitle("Dev Example");
        shell->setShellExtent(1280, 720);
        shell->setShellIcon("res/images/icons/nautilus.png");
        core->attachShell(shell);
        return NAUTILUS_STATUS_OK;
    }

    /**
     * Cleans allocated resources
     * @return Returns a NautilusStatus status code
     */ 
    NautilusStatus clean(void) {
        delete dev::shell;
        delete dev::core;
        return NAUTILUS_STATUS_OK;
    }

}

/**
 * Main entry point for the application
 */
int main() {
    dev::init();
    return NAUTILUS_STATUS_OK;
}