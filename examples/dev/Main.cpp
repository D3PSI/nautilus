#include <nautilus/Nautilus.hpp>

#include "DevShell.hpp"

#include <iostream>

namespace dev {

    NautilusShell*  shell;

    /**
     * Initializes everything
     * @return Returns a nautilus::NautilusStatus status code
     */
    nautilus::NautilusStatus run(void) {
        shell = new DevShell();
        shell->setShellContext(nautilus::NAUTILUS_SHELL_CONTEXT_WINDOWED);
        shell->setShellTitle("Dev Example 1");
        shell->setShellExtent(1280, 720);
        shell->setShellIcon("res/images/icons/nautilus.png");
        shell->setShellRefreshRate(30);
        NautilusCore::setEnableVulkanValidationLayers();
        NautilusCore::attachShell(shell);
        return nautilus::NAUTILUS_STATUS_OK;
    }

    /**
     * Cleans allocated resources
     * @return Returns a nautilus::NautilusStatus status code
     */ 
    nautilus::NautilusStatus clean(void) {
        NautilusCore::terminate();
        delete dev::shell;
        return nautilus::NAUTILUS_STATUS_OK;
    }

}

/**
 * Main entry point for the application
 */
int main() {
    dev::run();
    dev::clean();
    return nautilus::NAUTILUS_STATUS_OK;
}